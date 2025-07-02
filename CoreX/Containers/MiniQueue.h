// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  a small memory overhead, fixed size, efficient, iterable
               queue class (used for CContextView::SObjectClone)
   -------------------------------------------------------------------------
   История:
   - 02/09/2004   12:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __MINIQUEUE_H__
#define __MINIQUEUE_H__

#pragma once

//! This class implements a very small queue of plain-old-data.
template<typename T, u8 N>
struct MiniQueue
{
public:
	MiniQueue() : m_nValues(0), m_nOffset(0) {}

	MiniQueue(const MiniQueue& mq)
	{
		m_nValues = mq.m_nValues;
		m_nOffset = 0;
		for (i32 i = 0; i < m_nValues; i++)
		{
			m_vValues[i] = mq[i];
		}
	}

	bool Empty() const
	{
		return m_nValues == 0;
	}
	bool Full() const
	{
		return m_nValues == N;
	}
	u8 Size() const
	{
		return m_nValues;
	}
	u8 Capacity() const
	{
		return N;
	}
	ILINE void Push(T nValue)
	{
		assert(!Full());
		m_vValues[(m_nOffset + m_nValues) % N] = nValue;
		m_nValues++;
	}
	//! Push, but if the queue is full, then pop first.
	void CyclePush(T nValue)
	{
		if (Full())
			Pop();
		Push(nValue);
	}
	void PushFront(T nValue)
	{
		assert(!Full());
		m_nOffset = (m_nOffset + (N - 1)) % N;
		m_vValues[m_nOffset] = nValue;
		m_nValues++;
	}
	T Front() const
	{
		assert(!Empty());
		return m_vValues[m_nOffset];
	}
	T& Front()
	{
		assert(!Empty());
		return m_vValues[m_nOffset];
	}
	T& operator[](i32 i)
	{
		return m_vValues[(m_nOffset + i) % N];
	}
	T operator[](i32 i) const
	{
		return m_vValues[(m_nOffset + i) % N];
	}
	T Back() const
	{
		assert(!Empty());
		return m_vValues[(m_nOffset + m_nValues - 1) % N];
	}
	T& Back()
	{
		assert(!Empty());
		return m_vValues[(m_nOffset + m_nValues - 1) % N];
	}
	void Pop()
	{
		assert(!Empty());
		m_nOffset = (m_nOffset + 1) % N;
		m_nValues--;
	}
	void PopBack()
	{
		assert(!Empty());
		m_nValues--;
	}
	void Clear()
	{
		m_nOffset = m_nValues = 0;
	}

	struct SIterator
	{
	public:
		SIterator() {}
		SIterator(T* pValues, u8 nOffset) : m_pValues(pValues), m_nOffset(nOffset) {}
		T& operator*()
		{
			return m_pValues[m_nOffset % N];
		}
		T* operator->()
		{
			return &m_pValues[m_nOffset % N];
		}
		SIterator& operator++()
		{
			m_nOffset++;
			return *this;
		}
		SIterator operator++(i32)
		{
			SIterator itor = *this;
			++m_nOffset;
			return itor;
		}
		SIterator& operator--()
		{
			m_nOffset--;
			return *this;
		}
		SIterator& operator+=(u8 delta)
		{
			m_nOffset += delta;
			return *this;
		}
		SIterator& operator-=(u8 delta)
		{
			m_nOffset -= delta;
			return *this;
		}

		friend bool operator!=(const SIterator& a, const SIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			return a.m_nOffset != b.m_nOffset;
		}
		friend bool operator==(const SIterator& a, const SIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			return a.m_nOffset == b.m_nOffset;
		}

		friend i32 operator-(const SIterator& a, const SIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			i32 diff = i32(a.m_nOffset) - i32(b.m_nOffset);
			return diff;
		}

		u8 Offset() { return m_nOffset; }

	private:
		T*    m_pValues;
		u8 m_nOffset;
	};

	SIterator Begin()  { return SIterator(m_vValues, m_nOffset); }
	SIterator End()    { return SIterator(m_vValues, m_nOffset + m_nValues); }

	SIterator RBegin() { return SIterator(m_vValues, m_nOffset + m_nValues - 1); }
	SIterator REnd()   { return SIterator(m_vValues, m_nOffset - 1); }

	struct SConstIterator
	{
	public:
		SConstIterator() {}
		SConstIterator(const T* pValues, u8 nOffset) : m_pValues(pValues), m_nOffset(nOffset) {}
		const T& operator*()
		{
			return m_pValues[m_nOffset % N];
		}
		const T* operator->()
		{
			return &m_pValues[m_nOffset % N];
		}
		SConstIterator& operator++()
		{
			m_nOffset++;
			return *this;
		}
		SConstIterator& operator--()
		{
			m_nOffset--;
			return *this;
		}
		SConstIterator& operator+=(u8 delta)
		{
			m_nOffset += delta;
			return *this;
		}
		SConstIterator& operator-=(u8 delta)
		{
			m_nOffset -= delta;
			return *this;
		}
		friend bool operator!=(const SConstIterator& a, const SConstIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			return a.m_nOffset != b.m_nOffset;
		}
		friend bool operator==(const SConstIterator& a, const SConstIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			return a.m_nOffset == b.m_nOffset;
		}

		friend i32 operator-(const SConstIterator& a, const SConstIterator& b)
		{
			assert(a.m_pValues == b.m_pValues);
			i32 diff = i32(a.m_nOffset) - i32(b.m_nOffset);
			return diff;
		}

		u8 Offset() { return m_nOffset; }

	private:
		const T* m_pValues;
		u8    m_nOffset;
	};

	SConstIterator Begin() const  { return SConstIterator(m_vValues, m_nOffset); }
	SConstIterator End() const    { return SConstIterator(m_vValues, m_nOffset + m_nValues); }

	SConstIterator RBegin() const { return SConstIterator(m_vValues, m_nOffset + m_nValues - 1); }
	SConstIterator REnd() const   { return SConstIterator(m_vValues, m_nOffset - 1); }

	void           Erase(SIterator where)
	{
		assert(where.Offset() >= m_nOffset);
		assert(where.Offset() < m_nOffset + m_nValues);
		for (size_t offset = where.Offset(); offset < (size_t)(m_nOffset + m_nValues - 1); offset++)
		{
			m_vValues[offset % N] = m_vValues[(offset + 1) % N];
		}
		m_nValues--;
	}

	void Erase(SIterator first, SIterator last)
	{
		i32 num = last - first;
		if (num == 0)
			return;
		assert(num > 0);
		assert(num <= Size());
		assert(first.Offset() >= m_nOffset);
		assert(first.Offset() < m_nOffset + m_nValues);
		assert(last.Offset() >= m_nOffset);
		assert(last.Offset() <= m_nOffset + m_nValues);
		for (i32 offset = (i32)first.Offset(); offset < m_nOffset + m_nValues - 1; offset++)
		{
			m_vValues[offset % N] = m_vValues[(offset + num) % N];
		}
		m_nValues -= num;
	}

private:
	u8 m_nValues;
	u8 m_nOffset;
	T     m_vValues[N];
};

#endif
