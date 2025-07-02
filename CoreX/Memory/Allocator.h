// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Created by: Scott Peter
//---------------------------------------------------------------------------

#ifndef _ALLOCATOR_H__
#define _ALLOCATOR_H__

#include "DrxMemoryAllocator.h"

//! Дефолтная реализация разместителя.
struct StdAllocator
{
	// Функции класса alloc/free/size. Версии алиновки использовать только при необходимости.
	template<class T>
	static uk Allocate(T*& p)
	{
		return p = NeedAlign<T>() ?
		           (T*)DrxModuleMemalign(sizeof(T), alignof(T)) :
		           (T*)DrxModuleMalloc(sizeof(T));
	}

	template<class T>
	static void Deallocate(T* p)
	{
		if (NeedAlign<T>())
			DrxModuleMemalignFree(p);
		else
			DrxModuleFree(p);
	}

	template<class T>
	static size_t GetMemSize(const T* p)
	{
		return NeedAlign<T>() ?
		       sizeof(T) + alignof(T) :
		       sizeof(T);
	}

	template<typename T>
	void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
protected:

	template<class T>
	static bool NeedAlign()
	{ PREFAST_SUPPRESS_WARNING(6326); return alignof(T) > _ALIGNMENT; }
};

//! Удаляет шаблонную функцию для любого разместителя.
template<class TAlloc, class T>
void Delete(TAlloc& alloc, T* ptr)
{
	if (ptr)
	{
		ptr->~T();
		alloc.Deallocate(ptr);
	}
}

#endif // _ALLOCATOR_H__
