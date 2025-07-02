// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/Memory/DrxMemoryUpr.h>
#include <drx3D/Sys/MemoryUpr.h>
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sys/MemReplay.h>
#include <drx3D/Sys/CustomMemoryHeap.h>
#include <drx3D/Sys/GeneralMemoryHeap.h>
#include <drx3D/Sys/PageMappingHeap.h>
#include <drx3D/Sys/DefragAllocator.h>

#if DRX_PLATFORM_WINDOWS
	#include <Psapi.h>
#endif

#if DRX_PLATFORM_APPLE
	#include <mach/mach.h> // task_info
#endif

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <sys/types.h> // required by mman.h
	#include <sys/mman.h>  //mmap - virtual memory manager
#endif
extern LONG g_TotalAllocatedMemory;

#ifdef MEMMAN_STATIC
CDrxMemoryUpr g_memoryUpr;
#endif

//////////////////////////////////////////////////////////////////////////
CDrxMemoryUpr* CDrxMemoryUpr::GetInstance()
{
#ifdef MEMMAN_STATIC
	return &g_memoryUpr;
#else
	static CDrxMemoryUpr memman;
	return &memman;
#endif
}

#ifndef MEMMAN_STATIC
i32 CDrxMemoryUpr::s_sys_MemoryDeadListSize;

void CDrxMemoryUpr::RegisterCVars()
{
	REGISTER_CVAR2("sys_MemoryDeadListSize", &s_sys_MemoryDeadListSize, 0, VF_REQUIRE_APP_RESTART, "Keep upto size bytes in a \"deadlist\" of allocations to assist in capturing tramples");
}
#endif

//////////////////////////////////////////////////////////////////////////
bool CDrxMemoryUpr::GetProcessMemInfo(SProcessMemInfo& minfo)
{
	ZeroStruct(minfo);
#if DRX_PLATFORM_WINDOWS

	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	GlobalMemoryStatusEx(&mem);

	minfo.TotalPhysicalMemory = mem.ullTotalPhys;
	minfo.FreePhysicalMemory = mem.ullAvailPhys;

	//////////////////////////////////////////////////////////////////////////
	typedef BOOL (WINAPI * GetProcessMemoryInfoProc)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD);

	PROCESS_MEMORY_COUNTERS pc;
	ZeroStruct(pc);
	pc.cb = sizeof(pc);
	static HMODULE hPSAPI = LoadLibraryA("psapi.dll");
	if (hPSAPI)
	{
		static GetProcessMemoryInfoProc pGetProcessMemoryInfo = (GetProcessMemoryInfoProc)GetProcAddress(hPSAPI, "GetProcessMemoryInfo");
		if (pGetProcessMemoryInfo)
		{
			if (pGetProcessMemoryInfo(GetCurrentProcess(), &pc, sizeof(pc)))
			{
				minfo.PageFaultCount = pc.PageFaultCount;
				minfo.PeakWorkingSetSize = pc.PeakWorkingSetSize;
				minfo.WorkingSetSize = pc.WorkingSetSize;
				minfo.QuotaPeakPagedPoolUsage = pc.QuotaPeakPagedPoolUsage;
				minfo.QuotaPagedPoolUsage = pc.QuotaPagedPoolUsage;
				minfo.QuotaPeakNonPagedPoolUsage = pc.QuotaPeakNonPagedPoolUsage;
				minfo.QuotaNonPagedPoolUsage = pc.QuotaNonPagedPoolUsage;
				minfo.PagefileUsage = pc.PagefileUsage;
				minfo.PeakPagefileUsage = pc.PeakPagefileUsage;

				return true;
			}
		}
	}
	return false;

#elif DRX_PLATFORM_ORBIS

	size_t mainMemory, videoMemory;
	VirtualAllocator::QueryMemory(mainMemory, videoMemory); // GlobalMemoryStatus would be more accurate but also slower
	minfo.TotalPhysicalMemory = sceKernelGetDirectMemorySize();
	minfo.PeakPagefileUsage = minfo.PagefileUsage = mainMemory + videoMemory;
	minfo.FreePhysicalMemory = minfo.TotalPhysicalMemory - (mainMemory + videoMemory);
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID

	MEMORYSTATUS MemoryStatus;
	GlobalMemoryStatus(&MemoryStatus);
	minfo.PagefileUsage = minfo.PeakPagefileUsage = MemoryStatus.dwTotalPhys - MemoryStatus.dwAvailPhys;

	minfo.FreePhysicalMemory = MemoryStatus.dwAvailPhys;
	minfo.TotalPhysicalMemory = MemoryStatus.dwTotalPhys;

	#if DRX_PLATFORM_ANDROID
	// On Android, mallinfo() is an EXTREMELY time consuming operation. Nearly 80% CPU time will be spent
	// on this operation once -memreplay is given. Since WorkingSetSize is only used for statistics and
	// debugging purpose, it's simply ignored.
	minfo.WorkingSetSize = 0;
	#else
	struct mallinfo meminfo = mallinfo();
	minfo.WorkingSetSize = meminfo.usmblks + meminfo.uordblks;
	#endif

#elif DRX_PLATFORM_APPLE

	MEMORYSTATUS MemoryStatus;
	GlobalMemoryStatus(&MemoryStatus);
	minfo.PagefileUsage = minfo.PeakPagefileUsage = MemoryStatus.dwTotalPhys - MemoryStatus.dwAvailPhys;

	minfo.FreePhysicalMemory = MemoryStatus.dwAvailPhys;
	minfo.TotalPhysicalMemory = MemoryStatus.dwTotalPhys;

	// Retrieve WorkingSetSize from task_info
	task_basic_info kTaskInfo;
	mach_msg_type_number_t uInfoCount(sizeof(kTaskInfo) / sizeof(natural_t));
	if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&kTaskInfo, &uInfoCount) != 0)
	{
		gEnv->pLog->LogError("task_info failed\n");
		return false;
	}
	minfo.WorkingSetSize = kTaskInfo.resident_size;
#elif DRX_PLATFORM_DURANGO

	//Memory status
	TITLEMEMORYSTATUS DurangoMemoryStatus;
	DurangoMemoryStatus.dwLength = sizeof(DurangoMemoryStatus);
	if (TitleMemoryStatus(&DurangoMemoryStatus) != 0)
	{
		uint64 titleUsed = (uint64)(DurangoMemoryStatus.ullTitleUsed);
		uint64 legacyUsed = (uint64)(DurangoMemoryStatus.ullLegacyUsed);
		uint64 total = (uint64)(DurangoMemoryStatus.ullTotalMem);

		minfo.PagefileUsage = legacyUsed + titleUsed;

		static uint64 peak = minfo.PagefileUsage;
		if (peak < minfo.PagefileUsage)
			peak = minfo.PagefileUsage;
		minfo.PeakPagefileUsage = peak;

		minfo.TotalPhysicalMemory = total;
		minfo.FreePhysicalMemory = total - (legacyUsed + titleUsed);
	}
#else
	return false;
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CDrxMemoryUpr::FakeAllocation(long size)
{
	DrxInterlockedExchangeAdd((LONG *)&g_TotalAllocatedMemory, (LONG)size);
}

//////////////////////////////////////////////////////////////////////////
CDrxMemoryUpr::HeapHandle CDrxMemoryUpr::TraceDefineHeap(tukk heapName, size_t size, ukk pBase)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CDrxMemoryUpr::TraceHeapAlloc(HeapHandle heap, uk mem, size_t size, size_t blockSize, tukk sUsage, tukk sNameHint)
{

}

//////////////////////////////////////////////////////////////////////////
void CDrxMemoryUpr::TraceHeapFree(HeapHandle heap, uk mem, size_t blockSize)
{

}

//////////////////////////////////////////////////////////////////////////
void CDrxMemoryUpr::TraceHeapSetColor(u32 color)
{

}

//////////////////////////////////////////////////////////////////////////
void CDrxMemoryUpr::TraceHeapSetLabel(tukk sLabel)
{

}

//////////////////////////////////////////////////////////////////////////
u32 CDrxMemoryUpr::TraceHeapGetColor()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
IMemReplay* CDrxMemoryUpr::GetIMemReplay()
{
#if CAPTURE_REPLAY_LOG
	return CMemReplay::GetInstance();
#else
	static IMemReplay m;
	return &m;
#endif
}

//////////////////////////////////////////////////////////////////////////
ICustomMemoryHeap* const CDrxMemoryUpr::CreateCustomMemoryHeapInstance(IMemoryUpr::EAllocPolicy const eAllocPolicy)
{
	return new CCustomMemoryHeap(eAllocPolicy);
}

IGeneralMemoryHeap* CDrxMemoryUpr::CreateGeneralExpandingMemoryHeap(size_t upperLimit, size_t reserveSize, tukk sUsage)
{
	return new CGeneralMemoryHeap(static_cast<UINT_PTR>(0), upperLimit, reserveSize, sUsage);
}

IGeneralMemoryHeap* CDrxMemoryUpr::CreateGeneralMemoryHeap(uk base, size_t sz, tukk sUsage)
{
	return new CGeneralMemoryHeap(base, sz, sUsage);
}

IMemoryAddressRange* CDrxMemoryUpr::ReserveAddressRange(size_t capacity, tukk sName)
{
	return new CMemoryAddressRange(capacity, sName);
}

IPageMappingHeap* CDrxMemoryUpr::CreatePageMappingHeap(size_t addressSpace, tukk sName)
{
	return new CPageMappingHeap(addressSpace, sName);
}

IDefragAllocator* CDrxMemoryUpr::CreateDefragAllocator()
{
	return new CDefragAllocator();
}

uk CDrxMemoryUpr::AllocPages(size_t size)
{
	uk ret = NULL;
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

#if DRX_PLATFORM_ORBIS

	return DrxModuleMemalign(size, PAGE_SIZE);

#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	ret = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
#else

	ret = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

#endif

	if (ret)
	{
		MEMREPLAY_SCOPE_ALLOC(ret, size, 4096);
	}

	return ret;
}

void CDrxMemoryUpr::FreePages(uk p, size_t size)
{
	UINT_PTR id = (UINT_PTR)p;
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

#if DRX_PLATFORM_ORBIS

	DrxModuleMemalignFree(p);

#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#if defined(_DEBUG)
	i32 ret = munmap(p, size);
	DRX_ASSERT_MESSAGE(ret == 0, "munmap returned error.");
	#else
	munmap(p, size);
	#endif
#else

	VirtualFree(p, 0, MEM_RELEASE);

#endif

	MEMREPLAY_SCOPE_FREE(id);
}

//////////////////////////////////////////////////////////////////////////
extern "C"
{
#include <drx3D/CoreX/Memory/DrxMemoryUpr.h>

	DRXMEMORYMANAGER_API void DrxGetIMemoryUprInterface(uk * pIMemoryUpr)
	{
		// Static instance of the memory manager
		*pIMemoryUpr = CDrxMemoryUpr::GetInstance();
	}
};
