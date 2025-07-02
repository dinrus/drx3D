// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZATION_HELPER_H__
#define __SERIALIZATION_HELPER_H__

//-----------------------------------------------------------------------------
class CSerializationHelper
{
public:
	//-----------------------------------------------------------------------------
	CSerializationHelper(i32 size)
		: m_pos(0)
		, m_size(size)
		, m_bBufferOverflow(false)
		, m_bWriting(true)
	{
		m_pBuffer = new char[size];
	}

	CSerializationHelper(tuk buffer, i32 size)
		: m_pos(0)
		, m_size(size)
		, m_bBufferOverflow(false)
		, m_bWriting(false)
	{
		m_pBuffer = buffer;
	}

	//-----------------------------------------------------------------------------
	~CSerializationHelper()
	{
		if (m_bWriting)
		{
			delete[] m_pBuffer;
		}
	}

	//-----------------------------------------------------------------------------
	tuk GetBuffer()         { return m_pBuffer; }
	i32   GetUsedSize() const { return m_pos; }
	bool  Overflow() const    { return m_bBufferOverflow; }
	bool  IsWriting() const   { return m_bWriting; }
	void  Clear()             { m_pos = 0; m_bBufferOverflow = false; }

	void  ExpandBuffer(i32 spaceRequired)
	{
		while (spaceRequired > m_size)
		{
			m_size *= 2;
		}
		tuk pNewBuffer = new char[m_size];
		memcpy(pNewBuffer, m_pBuffer, m_pos);
		SAFE_DELETE_ARRAY(m_pBuffer);
		m_pBuffer = pNewBuffer;
	}

	//-----------------------------------------------------------------------------
	template<class T>
	void Write(const T& constData)
	{
		DRX_ASSERT(m_bWriting);

		T data = constData;

		if (eBigEndian)
		{
			SwapEndian(data, eBigEndian);   //swap to Big Endian
		}

		if (m_pos + (i32)sizeof(T) > m_size)
		{
			ExpandBuffer(m_pos + (i32)sizeof(T));
		}

		memcpy(m_pBuffer + m_pos, &data, sizeof(T));
		m_pos += sizeof(T);
	}

	//-----------------------------------------------------------------------------
	void WriteString(tukk pString)
	{
		DRX_ASSERT(m_bWriting);

		i32 length = strlen(pString);
		// Write the length of the string followed by the string itself
		Write(length);
		if (m_pos + length + 1 > m_size)
		{
			ExpandBuffer(m_pos + length + 1);
		}
		memcpy(m_pBuffer + m_pos, pString, length);
		m_pBuffer[m_pos + length] = '\0';
		m_pos += length + 1;
	}

	//-----------------------------------------------------------------------------
	void WriteBuffer(tukk pData, i32 length)
	{
		if (m_pos + length > m_size)
		{
			ExpandBuffer(m_pos + length);
		}
		memcpy(m_pBuffer + m_pos, pData, length);
		m_pos += length;
	}

	//-----------------------------------------------------------------------------
	template<class T>
	void Read(T& data)
	{
		DRX_ASSERT(!m_bWriting);

		if (m_pos + (i32)sizeof(T) <= m_size)
		{
			memcpy(&data, m_pBuffer + m_pos, sizeof(T));
			m_pos += sizeof(T);
			if (eBigEndian)
			{
				SwapEndian(data, eBigEndian);   //swap to Big Endian
			}
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
	}

	//-----------------------------------------------------------------------------
	tukk ReadString()
	{
		DRX_ASSERT(!m_bWriting);

		tukk pResult = NULL;
		i32 length = 0;
		// Read the length of the string followed by the string itself
		Read(length);
		if (m_pos + length <= m_size)
		{
			pResult = m_pBuffer + m_pos;
			m_pos += length + 1;
		}
		else
		{
			m_bBufferOverflow = true;
			DRX_ASSERT_MESSAGE(false, "Buffer size is not large enough");
		}
		return pResult;
	}

private:
	//-----------------------------------------------------------------------------
	tuk m_pBuffer;
	i32   m_pos;
	i32   m_size;
	bool  m_bBufferOverflow;
	bool  m_bWriting;
};

#endif  // __SERIALIZATION_HELPER_H__
