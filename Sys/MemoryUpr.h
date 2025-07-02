// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MemoryUpr.h
//  Version:     v1.00
//  Created:     27/7/2007 by Timur.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MemoryUpr_h__
#define __MemoryUpr_h__
#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Memory/DrxMemoryUpr.h>

#if !defined(_RELEASE) && !defined(MEMMAN_STATIC)
	#define DRXMM_SUPPORT_DEADLIST
#endif

//////////////////////////////////////////////////////////////////////////
// Class that implements IMemoryUpr interface.
//////////////////////////////////////////////////////////////////////////
#ifndef MEMMAN_STATIC
class CDrxMemoryUpr : public IMemoryUpr
{
public:
	static i32 s_sys_MemoryDeadListSize;

public:
	// Singleton
	static CDrxMemoryUpr* GetInstance();
	static void               RegisterCVars();

	//////////////////////////////////////////////////////////////////////////
	virtual bool                     GetProcessMemInfo(SProcessMemInfo& minfo);
	virtual void                     FakeAllocation(long size);

	virtual HeapHandle               TraceDefineHeap(tukk heapName, size_t size, ukk pBase);
	virtual void                     TraceHeapAlloc(HeapHandle heap, uk mem, size_t size, size_t blockSize, tukk sUsage, tukk sNameHint = 0);
	virtual void                     TraceHeapFree(HeapHandle heap, uk mem, size_t blockSize);
	virtual void                     TraceHeapSetColor(u32 color);
	virtual u32                   TraceHeapGetColor();
	virtual void                     TraceHeapSetLabel(tukk sLabel);

	virtual struct IMemReplay*       GetIMemReplay();
	virtual ICustomMemoryHeap* const CreateCustomMemoryHeapInstance(IMemoryUpr::EAllocPolicy const eAllocPolicy);
	virtual IGeneralMemoryHeap*      CreateGeneralExpandingMemoryHeap(size_t upperLimit, size_t reserveSize, tukk sUsage);
	virtual IGeneralMemoryHeap*      CreateGeneralMemoryHeap(uk base, size_t sz, tukk sUsage);

	virtual IMemoryAddressRange*     ReserveAddressRange(size_t capacity, tukk sName);
	virtual IPageMappingHeap*        CreatePageMappingHeap(size_t addressSpace, tukk sName);

	virtual IDefragAllocator*        CreateDefragAllocator();

	virtual uk                    AllocPages(size_t size);
	virtual void                     FreePages(uk p, size_t size);
};
#else
typedef IMemoryUpr CDrxMemoryUpr;
#endif

#endif // __MemoryUpr_h__
