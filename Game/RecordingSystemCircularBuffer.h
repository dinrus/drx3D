// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RECORDINGSYSTEMCIRCULARBUFFER_H__
#define __RECORDINGSYSTEMCIRCULARBUFFER_H__

template<i32 N>
class CCircularBuffer
{
public:
	class iterator
	{
	public:
		iterator()
		{
			m_buffer=NULL;
			m_salt=m_current=0;
		}

		iterator(const CCircularBuffer<N> *buffer, i32 salt, size_t current)
		{
			m_buffer=buffer;
			m_salt=salt;
			m_current=current;
		}

		bool operator<=(const iterator &rhs)
		{
			if (m_salt<rhs.m_salt)
			{
				return true;
			}
			else if (m_salt==rhs.m_salt)
			{
				return m_current<=rhs.m_current;
			}
			return false;
		}

		bool HasData()
		{
			CheckValid();
			return m_current!=m_buffer->head;
		}

		uk GetData(size_t datasize)
		{
			CheckValid();
			if (m_current+datasize>=sizeof(m_buffer->buffer))
			{
				m_current=0;
				m_salt++;
			}
			uk ret=(uk )&m_buffer->buffer[m_current];
			m_current+=datasize;
			return ret;
		}

	protected:
		void CheckValid()
		{
			if (m_salt<m_buffer->salt || (m_salt==m_buffer->salt && m_current<m_buffer->tail))
			{
				m_salt=m_buffer->salt;
				m_current=m_buffer->tail;
			}
		}

		const CCircularBuffer<N> *m_buffer;
		size_t m_current;
		i32 m_salt;

		friend class CCircularBuffer<N>;
	};

	CCircularBuffer()
	{
		Clear();
	}

	void Clear()
	{
		salt=head=tail=0;
	}

	bool AddData(ukk data, size_t dataSize)
	{
#ifndef _RELEASE
		if (dataSize>sizeof(buffer))
		{
			DrxFatalError("Trying to push more data (%" PRISIZE_T " bytes) than the total capacity of the circular buffer (%" PRISIZE_T " bytes)\n", dataSize, sizeof(buffer));
		}
#endif
		if (tail>head && head+dataSize>=tail)
			return false;
		if (head+dataSize>sizeof(buffer))
		{
			if (dataSize>=tail)
				return false;
			head=0;
		}
		memcpy(&buffer[head], data, dataSize);
		head+=dataSize;
		return true;
	}

	uk GetData(size_t requestedSize)
	{
		if (tail+requestedSize>sizeof(buffer))
		{
			tail=0;
			salt++;
		}
		uk ret=&buffer[tail];
		tail+=requestedSize;
		return ret;
	}

	void Erase(size_t size)
	{
		tail+=size;
		if (tail>sizeof(buffer))
			tail-=sizeof(buffer);
	}

	bool Empty()
	{
		return head==tail;
	}

	iterator Begin()
	{
		iterator it(this, salt, tail);
		return it;
	}

//protected:
	u8 buffer[N];
	size_t head;
	size_t tail;
	i32		 salt;

	friend class iterator;
};

#endif // __RECORDINGSYSTEMCIRCULARBUFFER_H__