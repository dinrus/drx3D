// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GALLOCATOR_DRXMEM_H_
#define _GALLOCATOR_DRXMEM_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

	#pragma warning(push)
	#pragma warning(disable : 6326)// Potential comparison of a constant with another constant
	#pragma warning(disable : 6011)// Dereferencing NULL pointer
	#include <GSysAlloc.h>
	#pragma warning(pop)

class GFxMemoryArenaWrapper : public GSysAllocPaged
{
public:
	// GSysAllocPaged interface
	virtual void  GetInfo(Info* i) const;
	virtual uk Alloc(UPInt size, UPInt align);
	virtual bool  Free(uk ptr, UPInt size, UPInt align);

public:
	GFxMemoryArenaWrapper();
	~GFxMemoryArenaWrapper();

public:
	bool AnyActive() const { return m_arenasActive != 0; }

	i32  Create(u32 arenaID, bool resetCache);
	void Destroy(u32 arenaID);

public:
	void SetAlloc(GSysAllocPaged* pAlloc);

public:
	static void InitCVars();

private:
	static i32 ms_sys_flash_use_arenas;

private:
	enum { MaxNumArenas = 32 };

private:
	DrxCriticalSection m_lock;
	GSysAllocPaged*    m_pAlloc;
	u32       m_arenasActive;
	u32       m_arenasResetCache;
	i32                m_arenasRefCnt[MaxNumArenas];
};

struct DrxGFxMemInterface
{
public:
	struct Stats
	{
		LONG AllocCount;
		LONG FreeCount;
		LONG Allocated;
		LONG AllocatedInHeap;
	};

public:
	virtual Stats                  GetStats() const = 0;
	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	virtual GSysAllocBase*         GetSysAllocImpl() const = 0;
	virtual GFxMemoryArenaWrapper& GetMemoryArenas() = 0;
	virtual float                  GetFlashHeapFragmentation() const = 0;

	virtual ~DrxGFxMemInterface() {}
};

class GSysAllocDrxMem : public GSysAllocPaged, public DrxGFxMemInterface
{
public:
	// GSysAllocPaged interface
	virtual void  GetInfo(Info* i) const;
	virtual uk Alloc(UPInt size, UPInt align);
	virtual bool  Free(uk ptr, UPInt size, UPInt align);

public:
	// DrxGFxMemInterface interface
	virtual Stats                  GetStats() const;
	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual GSysAllocBase*         GetSysAllocImpl() const;
	virtual GFxMemoryArenaWrapper& GetMemoryArenas();
	virtual float                  GetFlashHeapFragmentation() const;

public:
	explicit GSysAllocDrxMem(size_t addressSpaceSize);
	virtual ~GSysAllocDrxMem();

private:
	enum
	{
		MinGranularity              = 64 * 1024,
		FlashHeapAllocSizeThreshold = 1 * 1024 * 1024
	};

private:
	IPageMappingHeap*     m_pHeap;
	const size_t          m_addressSpaceSize;
	Stats                 m_stats;
	GFxMemoryArenaWrapper m_arenas;
};

class GSysAllocStaticDrxMem : public DrxGFxMemInterface
{
public:
	// DrxGFxMemInterface interface
	virtual Stats                  GetStats() const;
	virtual void                   GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual GSysAllocBase*         GetSysAllocImpl() const;
	virtual GFxMemoryArenaWrapper& GetMemoryArenas();
	virtual float                  GetFlashHeapFragmentation() const;

public:
	GSysAllocStaticDrxMem(size_t poolSize);
	virtual ~GSysAllocStaticDrxMem();

	bool IsValid() const { return m_pStaticAlloc && m_pMem; }

private:
	GSysAllocStatic*      m_pStaticAlloc;
	uk                 m_pMem;
	size_t                m_size;
	GFxMemoryArenaWrapper m_arenas;
};

#endif // #ifdef INCLUDE_SCALEFORM_SDK

#endif // #ifndef _GALLOCATOR_DRXMEM_H_
