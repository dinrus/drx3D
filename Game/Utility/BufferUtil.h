// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BUFFER_UTIL_H__
#define __BUFFER_UTIL_H__

//-----------------------------------------------------------------------------
class CBufferUtil
{
public:
//-----------------------------------------------------------------------------
	CBufferUtil(i32 size, bool bWriting)
		: m_pos(0)
		, m_size(size)
		, m_bBufferOverflow(false)
		, m_bWriting(bWriting)
	{
		m_pBuffer = new char[size];
	}

//-----------------------------------------------------------------------------
	~CBufferUtil()
	{
		delete [] m_pBuffer;
	}

//-----------------------------------------------------------------------------
	template <class T>
	void Serialise(T &data)
	{
		if (m_bWriting)
		{
			Write(data);
		}
		else
		{
			Read(data);
		}
	}

//-----------------------------------------------------------------------------
	void SerialiseString(tukk* ppString)
	{
		if (m_bWriting)
		{
			WriteString(*ppString);
		}
		else
		{
			ReadString(ppString);
		}
	}

//-----------------------------------------------------------------------------
	tuk GetBuffer() { return m_pBuffer; }
	i32 GetUsedSize() const { return m_pos; }
	bool Overflow() const { return m_bBufferOverflow; }
	bool IsWriting() const { return m_bWriting; }

private:
//-----------------------------------------------------------------------------
	template <class T>
	void Write(T &data)
	{
		DRX_ASSERT(m_bWriting);

		if (eBigEndian)
		{
			SwapEndian(data, eBigEndian);		//swap to Big Endian
		}

		if (m_pos + (i32)sizeof(T) <= m_size)
		{
			memcpy(m_pBuffer + m_pos, &data, sizeof(T));
			m_pos += sizeof(T);
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}

		if (eBigEndian)
		{
			SwapEndian(data, eBigEndian);	//swap back again
		}
	}

//-----------------------------------------------------------------------------
	void WriteString(tukk pString)
	{
		DRX_ASSERT(m_bWriting);

		i32 length = strlen(pString);
		// Write the length of the string followed by the string itself
		Write(length);
		if (m_pos + length + 1 <= m_size)
		{
			memcpy(m_pBuffer + m_pos, pString, length);
			m_pBuffer[m_pos + length] = '\0';
			m_pos += length + 1;
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
	}

//-----------------------------------------------------------------------------
	template <class T>
	void Read(T &data)
	{
		DRX_ASSERT(!m_bWriting);

		if (m_pos + (i32)sizeof(T) <= m_size)
		{
			memcpy(&data, m_pBuffer + m_pos, sizeof(T));
			m_pos += sizeof(T);
			if (eBigEndian)
			{
				SwapEndian(data, eBigEndian);		//swap to Big Endian
			}
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
	}

//-----------------------------------------------------------------------------
	void ReadString(tukk* ppString)
	{
		DRX_ASSERT(!m_bWriting);

		i32 length = 0;
		// Read the length of the string followed by the string itself
		Read(length);
		if (m_pos + length <= m_size)
		{
			*ppString = m_pBuffer + m_pos;
			m_pos += length + 1;
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
	}

//-----------------------------------------------------------------------------
	tuk m_pBuffer;
	i32 m_pos;
	i32 m_size;
	bool m_bBufferOverflow;
	bool m_bWriting;
};

#endif	// __BUFFER_UTIL_H__
