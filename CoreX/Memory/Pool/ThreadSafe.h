// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Created by: Michael Kopietz
// Modified: -
//
//---------------------------------------------------------------------------

#ifndef __CDRXPOOLTHREADSAVE__
#define __CDRXPOOLTHREADSAVE__

namespace NDrxPoolAlloc
{

template<class TAllocator>
class CThreadSafe : public TAllocator
{
	DrxCriticalSection m_Mutex;
public:

	template<class T>
	ILINE T Allocate(size_t Size, size_t Align = 1)
	{
		DrxAutoLock<DrxCriticalSection> lock(m_Mutex);
		return TAllocator::template Allocate<T>(Size, Align);
	}

	template<class T>
	ILINE bool Free(T pData, bool ForceBoundsCheck = false)
	{
		DrxAutoLock<DrxCriticalSection> lock(m_Mutex);
		return TAllocator::Free(pData, ForceBoundsCheck);
	}

	template<class T>
	ILINE bool Resize(T** pData, size_t Size, size_t Alignment)
	{
		DrxAutoLock<DrxCriticalSection> lock(m_Mutex);
		return TAllocator::Resize(pData, Size, Alignment);
	}

};

}

#endif
