// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ARRAY2D_H_
#define _ARRAY2D_H_

// Dynamic replacement for static 2d array
template<class T> struct Array2d
{
	Array2d()
	{
		m_nSize = 0;
		m_pData = 0;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_pData, m_nSize * m_nSize * sizeof(T));
	}

	i32  GetSize()     { return m_nSize; }
	i32  GetDataSize() { return m_nSize * m_nSize * sizeof(T); }

	T*   GetData()     { return m_pData; }

	T*   GetDataEnd()  { return &m_pData[m_nSize * m_nSize]; }

	void SetData(T* pData, i32 nSize)
	{
		Allocate(nSize);
		memcpy(m_pData, pData, nSize * nSize * sizeof(T));
	}

	void Allocate(i32 nSize)
	{
		if (m_nSize == nSize)
			return;

		delete[] m_pData;

		m_nSize = nSize;
		m_pData = new T[nSize * nSize];
		memset(m_pData, 0, nSize * nSize * sizeof(T));
	}

	~Array2d()
	{
		delete[] m_pData;
	}

	void Reset()
	{
		delete[] m_pData;
		m_pData = 0;
		m_nSize = 0;
	}

	T*  m_pData;
	i32 m_nSize;

	T* operator[](i32k& nPos) const
	{
		assert(nPos >= 0 && nPos < m_nSize);
		return &m_pData[nPos * m_nSize];
	}

	Array2d& operator=(const Array2d& other)
	{
		Allocate(other.m_nSize);
		memcpy(m_pData, other.m_pData, m_nSize * m_nSize * sizeof(T));
		return *this;
	}
};

#endif // _ARRAY2D_H_
