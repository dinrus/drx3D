// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Created by: Michael Kopietz
// Modified: -
//
//---------------------------------------------------------------------------

#ifndef __CDRXPOOLMEMORY__
#define __CDRXPOOLMEMORY__

namespace NDrxPoolAlloc
{

class CMemoryDynamic
{
	size_t m_Size;
	u8* m_pData;

protected:
	ILINE CMemoryDynamic() :
		m_Size(0),
		m_pData(0){}

public:
	ILINE void InitMem(const size_t S, u8* pData)
	{
		m_Size = S;
		m_pData = pData;
		CPA_ASSERT(S);
		CPA_ASSERT(pData);
	}

	ILINE size_t       MemSize() const { return m_Size; }
	ILINE u8*       Data()          { return m_pData; }
	ILINE u8k* Data() const    { return m_pData; }
};

template<size_t TSize>
class CMemoryStatic
{
	u8 m_Data[TSize];

protected:
	ILINE CMemoryStatic()
	{
	}
public:
	ILINE void InitMem(const size_t S, u8* pData)
	{
	}
	ILINE size_t       MemSize() const { return TSize; }
	ILINE u8*       Data()          { return m_Data; }
	ILINE u8k* Data() const    { return m_Data; }
};

}

#endif
