// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxMemoryUpr_impl.h
//  Version:     v1.00
//  Created:     27/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:    Предоставляет реализацию глобально определяемых функций для DrxMemoryUpr.
//               Этот файл включается только в platform_impl.inl,
//               не следует включать его в коде напрямую!
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DrxMemoryUpr_impl_h__
#define __DrxMemoryUpr_impl_h__
#pragma once

#ifdef _LIB
	#include <drx3D/Sys/ISystem.h> // <> required for Interfuscator
#endif

#include <drx3D/CoreX/Platform/DrxLibrary.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#define DLL_ENTRY_DRXMALLOC     "DrxMalloc"
#define DLL_ENTRY_DRXFREE       "DrxFree"
#define DLL_ENTRY_DRXREALLOC    "DrxRealloc"
#define DLL_ENTRY_DRXGETMEMSIZE "DrxGetMemSize"
#define DLL_ENTRY_DRXCRTMALLOC  "DrxSystemCrtMalloc"
#define DLL_ENTRY_DRXCRTFREE    "DrxSystemCrtFree"
#define DLL_ENTRY_DRXCRTSIZE    "DrxSystemCrtSize"
#define DLL_ENTRY_GETMEMMANAGER "DrxGetIMemoryUprInterface"

//////////////////////////////////////////////////////////////////////////
// _PoolHelper definition.
//////////////////////////////////////////////////////////////////////////
struct _DrxMemoryUprPoolHelper
{
	typedef uk (*   FNC_DrxMalloc)(size_t size, size_t& allocated, size_t alignment);
	typedef uk (*   FNC_DrxCrtMalloc)(size_t size);
	typedef uk (*   FNC_DrxRealloc)(uk memblock, size_t size, size_t& allocated, size_t& oldsize, size_t alignment);
	typedef size_t (* FNC_DrxFree)(uk p, size_t alignment);
	typedef size_t (* FNC_DrxCrtFree)(uk p);
	typedef size_t (* FNC_DrxCrtSize)(uk p);
	typedef void (*   FNC_DrxGetIMemoryUprInterface)(uk * p);
	typedef uk (*   FNC_DrxCrtRealloc)(uk p, size_t size);

	static  LONG allocatedMemory;
	static  LONG freedMemory;
	static  LONG requestedMemory;
	static  i32  numAllocations;

	//	typedef size_t (*FNC_DrxFree)(void);
	typedef size_t (* FNC_DrxGetMemSize)(uk p, size_t);
	typedef i32 (*    FNC_DrxStats)(tuk buf);

	static FNC_DrxMalloc                     _DrxMalloc;
	static FNC_DrxRealloc                    _DrxRealloc;
	static FNC_DrxFree                       _DrxFree;
	static FNC_DrxGetMemSize                 _DrxGetMemSize;
	static FNC_DrxCrtMalloc                  _DrxCrtMalloc;
	static FNC_DrxCrtFree                    _DrxCrtFree;
	static FNC_DrxCrtRealloc                 _DrxCrtRealloc;
	static FNC_DrxCrtSize                    _DrxCrtSize;
	static FNC_DrxGetIMemoryUprInterface _DrxGetIMemoryUprInterface;

	static i32                               m_bInitialized;

	// Use template as most modules do not define memory management functions
	// and use the one from DinrusSystem
	// which may be part of the Launcher if not build as _LIB (monolithic).
	template<i32> static void BindMemoryFunctionPointers();

	static void Init();
	static void FakeAllocation(long size)
	{
		if (!m_bInitialized)
			Init();
		DrxInterlockedExchangeAdd(&allocatedMemory, size);
		DrxInterlockedExchangeAdd(&requestedMemory, size);
		GetISystem()->GetIMemoryUpr()->FakeAllocation(size);
	}

	//////////////////////////////////////////////////////////////////////////
	static IMemoryUpr* GetIMemoryUpr()
	{
		if (!m_bInitialized)
			Init();
		uk ptr = 0;
#ifdef _LIB
		DrxGetIMemoryUprInterface((uk *)&ptr);
#else
		if (_DrxGetIMemoryUprInterface)
			_DrxGetIMemoryUprInterface((uk *)&ptr);
#endif
		return (IMemoryUpr*)ptr;
	}

	static void GetMemoryInfo(DrxModuleMemoryInfo* pMmemInfo)
	{
		pMmemInfo->allocated = allocatedMemory;
		pMmemInfo->freed = freedMemory;
		pMmemInfo->requested = requestedMemory;
		pMmemInfo->num_allocations = numAllocations;
#ifdef DRX_STRING
		pMmemInfo->DrxString_allocated = string::_usedMemory(0) + wstring::_usedMemory(0);
#endif
	}

	//! Local version of allocations, does memory counting per module.
	static ILINE uk Malloc(size_t size, size_t alignment)
	{
		if (!m_bInitialized)
			Init();

		size_t allocated;
		uk p = _DrxMalloc(size, allocated, alignment);

		DrxInterlockedExchangeAdd(&allocatedMemory, allocated);
		DrxInterlockedExchangeAdd(&requestedMemory, size);
		DrxInterlockedIncrement(&numAllocations);

		return p;
	}

	//////////////////////////////////////////////////////////////////////////
	static ILINE uk Realloc(uk memblock, size_t size, size_t alignment)
	{
		if (!m_bInitialized)
			Init();

		size_t allocated, oldsize;
		uk p = _DrxRealloc(memblock, size, allocated, oldsize, alignment);

		DrxInterlockedExchangeAdd(&allocatedMemory, allocated);
		DrxInterlockedExchangeAdd(&requestedMemory, size);
		DrxInterlockedIncrement(&numAllocations);

		DrxInterlockedExchangeAdd(&freedMemory, oldsize);

		return p;
	}

	//////////////////////////////////////////////////////////////////////////
	static ILINE size_t Free(uk memblock, size_t alignment)
	{
		size_t freed = 0;
		if (memblock != 0)
		{
			freed = _DrxFree(memblock, alignment);
			DrxInterlockedExchangeAdd(&freedMemory, freed);
		}
		return freed;
	}

	static ILINE size_t MemSize(uk ptr, size_t sz)
	{
		if (!m_bInitialized)
			Init();

		size_t realSize = _DrxGetMemSize(ptr, sz);

		return realSize;
	}
};

template<i32> void _DrxMemoryUprPoolHelper::BindMemoryFunctionPointers()
{
	// On first iteration check ourself
	HMODULE hMod = DrxGetCurrentModule();
	for (i32 i = 0; i < 2; i++)
	{
		if (hMod)
		{
			_DrxMalloc = (FNC_DrxMalloc)DrxGetProcAddress(hMod, DLL_ENTRY_DRXMALLOC);
			_DrxRealloc = (FNC_DrxRealloc)DrxGetProcAddress(hMod, DLL_ENTRY_DRXREALLOC);
			_DrxFree = (FNC_DrxFree)DrxGetProcAddress(hMod, DLL_ENTRY_DRXFREE);
			_DrxGetMemSize = (FNC_DrxGetMemSize)DrxGetProcAddress(hMod, DLL_ENTRY_DRXGETMEMSIZE);
			_DrxCrtMalloc = (FNC_DrxCrtMalloc)DrxGetProcAddress(hMod, DLL_ENTRY_DRXCRTMALLOC);
			_DrxCrtSize = (FNC_DrxCrtSize)DrxGetProcAddress(hMod, DLL_ENTRY_DRXCRTSIZE);
			_DrxCrtFree = (FNC_DrxCrtFree)DrxGetProcAddress(hMod, DLL_ENTRY_DRXCRTFREE);
			_DrxGetIMemoryUprInterface = (FNC_DrxGetIMemoryUprInterface)DrxGetProcAddress(hMod, DLL_ENTRY_GETMEMMANAGER);

			if ((_DrxMalloc && _DrxRealloc && _DrxFree && _DrxGetMemSize && _DrxCrtMalloc && _DrxCrtFree && _DrxCrtSize && _DrxGetIMemoryUprInterface))
				break;
		}

		hMod = DrxLoadLibraryDefName("drx3D_Sys");
	}

	if (!hMod || !_DrxMalloc || !_DrxRealloc || !_DrxFree || !_DrxGetMemSize || !_DrxCrtMalloc || !_DrxCrtFree || !_DrxCrtSize || !_DrxGetIMemoryUprInterface)
	{
		char errMsg[10240];
		if (hMod)
		{
			sprintf(errMsg, "%s", "Управление Памятью: Не удаётся привязать функции управления памятью.");
		}
		else
		{
#ifdef DRX_PLATFORM_LINUX
			sprintf(errMsg, "%s\nДетали ошибки: %s", "Управление Памятью:  Не удаётся привязать функции управления памятью. Нет доступа к " DrxLibraryDefName("drx3D_Sys")" (проверьте рабочую директорию)", dlerror());
#else
			sprintf(errMsg, "%s", "Управление Памятью:  Не удаётся привязать функции управления памятью. Нет доступа к " DrxLibraryDefName("drx3D_Sys")" (проверьте рабочую директорию)");
#endif
		}

		DrxMessageBox(errMsg, "Управление Памятью", eMB_Error);
		__debugbreak();
		abort();
	}
	
}

template<> inline void _DrxMemoryUprPoolHelper::BindMemoryFunctionPointers<eDrxM_System>()
{
	_DrxMalloc = DrxMalloc;
	_DrxRealloc = DrxRealloc;
	_DrxFree = DrxFree;
	_DrxGetMemSize = DrxGetMemSize;
	_DrxCrtMalloc = DrxSystemCrtMalloc;
	_DrxCrtRealloc = DrxSystemCrtRealloc;
	_DrxCrtFree = (FNC_DrxCrtFree)DrxSystemCrtFree;
	_DrxCrtSize = (FNC_DrxCrtSize)DrxSystemCrtSize;
	_DrxGetIMemoryUprInterface = (FNC_DrxGetIMemoryUprInterface)DrxGetIMemoryUprInterface;
}

void _DrxMemoryUprPoolHelper::Init()
{
	if (m_bInitialized)
		return;

	m_bInitialized = 1;
	allocatedMemory = 0;
	freedMemory = 0;
	requestedMemory = 0;
	numAllocations = 0;

#ifndef eDrxModule
	BindMemoryFunctionPointers<eDrxM_EnginePlugin>();
#else
	#if defined(DRX_IS_MONOLITHIC_BUILD)
		BindMemoryFunctionPointers<eDrxM_System>();
	#else
		BindMemoryFunctionPointers<eDrxModule>();
	#endif
#endif
}

//////////////////////////////////////////////////////////////////////////
// Static variables.
//////////////////////////////////////////////////////////////////////////
 LONG _DrxMemoryUprPoolHelper::allocatedMemory;
 LONG _DrxMemoryUprPoolHelper::freedMemory;
 LONG _DrxMemoryUprPoolHelper::requestedMemory;
 i32 _DrxMemoryUprPoolHelper::numAllocations;

_DrxMemoryUprPoolHelper::FNC_DrxMalloc _DrxMemoryUprPoolHelper::_DrxMalloc = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxGetMemSize _DrxMemoryUprPoolHelper::_DrxGetMemSize = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxRealloc _DrxMemoryUprPoolHelper::_DrxRealloc = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxFree _DrxMemoryUprPoolHelper::_DrxFree = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxCrtMalloc _DrxMemoryUprPoolHelper::_DrxCrtMalloc = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxCrtRealloc _DrxMemoryUprPoolHelper::_DrxCrtRealloc = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxCrtFree _DrxMemoryUprPoolHelper::_DrxCrtFree = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxCrtSize _DrxMemoryUprPoolHelper::_DrxCrtSize = NULL;
_DrxMemoryUprPoolHelper::FNC_DrxGetIMemoryUprInterface _DrxMemoryUprPoolHelper::_DrxGetIMemoryUprInterface = NULL;

i32 _DrxMemoryUprPoolHelper::m_bInitialized = 0;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if !defined(NOT_USE_DRX_MEMORY_MANAGER)
//////////////////////////////////////////////////////////////////////////
uk DrxModuleMalloc(size_t size) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = _DrxMemoryUprPoolHelper::Malloc(size, 0);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
};

//////////////////////////////////////////////////////////////////////////
uk DrxModuleRealloc(uk ptr, size_t size)  noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = _DrxMemoryUprPoolHelper::Realloc(ptr, size, 0);
	MEMREPLAY_SCOPE_REALLOC(ptr, ret, size, 0);
	return ret;
};

//////////////////////////////////////////////////////////////////////////
void DrxModuleFree(uk ptr) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	_DrxMemoryUprPoolHelper::Free(ptr, 0);
	MEMREPLAY_SCOPE_FREE(ptr);
};

size_t DrxModuleMemSize(uk ptr, size_t sz) noexcept
{
	return _DrxMemoryUprPoolHelper::MemSize(ptr, sz);
}

//////////////////////////////////////////////////////////////////////////
uk DrxModuleMemalign(size_t size, size_t alignment) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = _DrxMemoryUprPoolHelper::Malloc(size, alignment);
	MEMREPLAY_SCOPE_ALLOC(ret, size, alignment);
	return ret;
};

void DrxModuleMemalignFree(uk ptr) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	_DrxMemoryUprPoolHelper::Free(ptr, 1000); // Free with alignment
	MEMREPLAY_SCOPE_FREE(ptr);
};

uk DrxModuleCalloc(size_t a, size_t b) noexcept
{
	uk p = DrxModuleMalloc(a * b);
	memset(p, 0, a * b);
	return p;
}

//////////////////////////////////////////////////////////////////////////
uk DrxModuleReallocAlign(uk ptr, size_t size, size_t alignment) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = _DrxMemoryUprPoolHelper::Realloc(ptr, size, alignment);
	MEMREPLAY_SCOPE_REALLOC(ptr, ret, size, alignment);
	return ret;
};
#endif //!defined(NOT_USE_DRX_MEMORY_MANAGER)

//////////////////////////////////////////////////////////////////////////
void DrxModuleGetMemoryInfo(DrxModuleMemoryInfo* pMemInfo)
{
	_DrxMemoryUprPoolHelper::GetMemoryInfo(pMemInfo);
};
//////////////////////////////////////////////////////////////////////////
void DrxGetMemoryInfoForModule(DrxModuleMemoryInfo* pInfo)
{
	_DrxMemoryUprPoolHelper::GetMemoryInfo(pInfo);
};

uk DrxCrtMalloc(size_t size)
{
	_DrxMemoryUprPoolHelper::Init();
	return _DrxMemoryUprPoolHelper::_DrxCrtMalloc(size);
}

size_t DrxCrtFree(uk p)
{
	return _DrxMemoryUprPoolHelper::_DrxCrtFree(p);
};

size_t DrxCrtSize(uk p)
{
	return _DrxMemoryUprPoolHelper::_DrxCrtSize(p);
};


// Redefine new & delete for entire module.
#if !defined(NOT_USE_DRX_MEMORY_MANAGER)

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new(std::size_t size)
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	uk ret = DrxModuleMalloc(size);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
}

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new(std::size_t size, const std::nothrow_t& nothrow_value) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	uk ret = DrxModuleMalloc(size);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
}

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new[](std::size_t size)
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
	uk ret = DrxModuleMalloc(size);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
}

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new[](std::size_t size, const std::nothrow_t& nothrow_value) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
	uk ret = DrxModuleMalloc(size);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
}

void __cdecl operator delete (uk ptr) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	DrxModuleFree(ptr);
	MEMREPLAY_SCOPE_FREE(ptr);
}

void __cdecl operator delete (uk ptr, const std::nothrow_t& nothrow_constant) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	DrxModuleFree(ptr);
	MEMREPLAY_SCOPE_FREE(ptr);
}

void __cdecl operator delete[](uk ptr) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	DrxModuleFree(ptr);
	MEMREPLAY_SCOPE_FREE(ptr);
}

void __cdecl operator delete[](uk ptr, const std::nothrow_t& nothrow_constant) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
	DrxModuleFree(ptr);
	MEMREPLAY_SCOPE_FREE(ptr);
}
#endif

//////////////////////////////////////////////////////////////////////////
#ifndef MEMMAN_STATIC
IMemoryUpr* DrxGetIMemoryUpr()
{
	static IMemoryUpr* memMan = 0;
	if (!memMan)
		memMan = _DrxMemoryUprPoolHelper::GetIMemoryUpr();
	return memMan;
}
#endif //!defined(_LIB)

// ~memReplay
#endif // __DrxMemoryUpr_impl_h__
