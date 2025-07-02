// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MEMENTOMEMORYMANAGER_H__
#define __MEMENTOMEMORYMANAGER_H__

#pragma once

#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
#include <drx3D/Network/Config.h>

#ifdef USE_GLOBAL_BUCKET_ALLOCATOR
	#define MMM_USE_BUCKET_ALLOCATOR            1
	#define MMM_BUCKET_ALLOCATOR_SIZE           (4 * 1024 * 1024)
	#define LOG_BUCKET_ALLOCATOR_HIGH_WATERMARK 0
#else
	#define MMM_USE_BUCKET_ALLOCATOR            0
#endif

#if MMM_USE_BUCKET_ALLOCATOR
	#include <drx3D/CoreX/Memory/BucketAllocator.h>
#endif

#define MMM_GENERAL_HEAP_SIZE           (1024 * 1024)

struct IMementoManagedThing
{
	virtual ~IMementoManagedThing(){}
	virtual void Release() = 0;
};

#define MMM_MUTEX_ENABLE (0)

// handle based memory manager that can repack data to save fragmentation
// TODO: re-introduce repacking for handles (but not pointers)
class CMementoMemoryUpr : public CMultiThreadRefCount
{
	friend class CMementoStreamAllocator;

public:
	// hack for arithmetic alphabet stuff
	u32                arith_zeroSizeHdl;
	IMementoManagedThing* pThings[64];

	// who's using pThings:
	//   0 - arith row sym cache
	//   1 - arith row low cache

	CMementoMemoryUpr(const string& name);
	~CMementoMemoryUpr();

	typedef u32 Hdl;
	static const Hdl InvalidHdl = ~Hdl(0);

	uk        AllocPtr(size_t sz, uk callerOverride = 0);
	void         FreePtr(uk p, size_t sz);
	Hdl          AllocHdl(size_t sz, uk callerOverride = 0);
	Hdl          CloneHdl(Hdl hdl);
	void         ResizeHdl(Hdl hdl, size_t sz);
	void         FreeHdl(Hdl hdl);
	void         AddHdlToSizer(Hdl hdl, IDrxSizer* pSizer);
	ILINE uk  PinHdl(Hdl hdl) const     { return CMementoMemoryUprAllocator::GetAllocator()->PinHdl(hdl); }
	ILINE size_t GetHdlSize(Hdl hdl) const { return CMementoMemoryUprAllocator::GetAllocator()->GetHdlSize(hdl); }

	void         GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false);

	static void  DebugDraw();
	static void  Tick();

private:
	class CMementoMemoryUprAllocator
	{
		struct SPoolStats
		{
			SPoolStats() : allocated(0), used(0) {}
			size_t allocated;
			size_t used;

			float  GetWastePercent() const
			{
				return allocated ? 100.0f * (1.0f - float(used) / float(allocated)) : 0.0f;
			}
		};

#if MMM_USE_BUCKET_ALLOCATOR
		typedef BucketAllocator<BucketAllocatorDetail::DefaultTraits<MMM_BUCKET_ALLOCATOR_SIZE, BucketAllocatorDetail::SyncPolicyUnlocked, false>> MMMBuckets;

		static MMMBuckets m_bucketAllocator;
		size_t            m_bucketTotalRequested;
		size_t            m_bucketTotalAllocated;

		struct SHandleData
		{
			uk  p;
			size_t size;
			size_t capacity;
		};

#else
		static const size_t ALIGNMENT = 8; // alignment for mementos

		// pool sizes are 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
		static i32k FIRST_POOL = 3; // == 8bytes
		static i32k LAST_POOL = 12; // == 4096bytes
		static i32k NPOOLS = LAST_POOL - FIRST_POOL + 1;

		struct SFreeListHeader
		{
			SFreeListHeader* pNext;
		};

		SFreeListHeader m_freeList[NPOOLS];
		i32             m_numAllocated[NPOOLS];
		i32             m_numFree[NPOOLS];
		stl::PoolAllocator<4096, stl::PoolAllocatorSynchronizationSinglethreaded, ALIGNMENT> m_pool;

		struct SHandleData
		{
			uk  p;
			size_t size;
			size_t capacity;
		};
#endif

#if !defined(PURE_CLIENT)
		IGeneralMemoryHeap* m_pGeneralHeap;
		size_t              m_generalHeapTotalRequested;
		size_t              m_generalHeapTotalAllocated;
#endif

	public:
		CMementoMemoryUprAllocator();
		~CMementoMemoryUprAllocator();

		static CMementoMemoryUprAllocator* GetAllocator() { return m_allocator; }
		static void                            AddCMementoMemoryUpr();
		static void                            RemoveCMementoMemoryUpr();

		void                                   Tick();
		Hdl                                    AllocHdl(size_t sz);
		void                                   FreeHdl(Hdl hdl);
		uk                                  AllocPtr(size_t sz);
		void                                   FreePtr(uk p, size_t sz);
		void                                   ResizeHdl(Hdl hdl, size_t sz);
		void                                   InitHandleData(SHandleData& hd, size_t sz);
		void                                   DebugDraw(i32 x, i32& y, size_t& totalAllocated);
		ILINE uk                            PinHdl(Hdl hdl) const
		{
			hdl = UnprotectHdl(hdl);
			return (hdl != InvalidHdl) ? m_handles[hdl].p : NULL;
		}
		ILINE size_t GetHdlSize(Hdl hdl) const
		{
			hdl = UnprotectHdl(hdl);
			return (hdl != InvalidHdl) ? m_handles[hdl].size : 0;
		}

#if LOG_BUCKET_ALLOCATOR_HIGH_WATERMARK
		size_t m_bucketHighWaterMark;
		size_t m_generalHeapHighWaterMark;
#endif // LOG_BUCKET_ALLOCATOR_HIGH_WATERMARK

	private:
		ILINE Hdl ProtectHdl(Hdl x) const
		{
#if !DRX_PLATFORM_APPLE && !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_ORBIS && DRXNETWORK_RELEASEBUILD
			if (x != InvalidHdl)
			{
				return (x << 1) ^ ((u32)UINT_PTR(this) + 1);     // ensures 0xFFFFFFFF cannot be a valid result (this will always be at least 4 byte aligned)
			}
#endif
			return x;
		}

		ILINE Hdl UnprotectHdl(Hdl x) const
		{
#if !DRX_PLATFORM_APPLE && !(DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT) && !DRX_PLATFORM_ORBIS && DRXNETWORK_RELEASEBUILD
			if (x != InvalidHdl)
			{
				return (x ^ ((u32)UINT_PTR(this) + 1)) >> 1;
			}
#endif
			return x;
		}

		std::vector<SHandleData>               m_handles;
		std::vector<u32>                    m_freeHandles;

		static CMementoMemoryUprAllocator* m_allocator;
		static i32                             m_numCMementoMemoryUprs;
#if MMM_MUTEX_ENABLE
		static DrxLockT<DRXLOCK_RECURSIVE>     m_mutex;
#endif
	};

	size_t m_totalAllocations;
	string m_name;

#if MMM_CHECK_LEAKS
	std::map<uk , uk>  m_ptrToAlloc;
	std::map<u32, uk> m_hdlToAlloc;
	std::map<uk , u32> m_allocAmt;
#endif

#if ENABLE_NETWORK_MEM_INFO
	typedef std::list<CMementoMemoryUpr*> TUprs;
	static TUprs* m_pUprs;
#endif
};

typedef CMementoMemoryUpr::Hdl TMemHdl;
const TMemHdl TMemInvalidHdl = CMementoMemoryUpr::InvalidHdl;

typedef _smart_ptr<CMementoMemoryUpr> CMementoMemoryUprPtr;

class CMementoStreamAllocator : public IStreamAllocator
{
public:
	CMementoStreamAllocator(const CMementoMemoryUprPtr& mmm);

	uk   Alloc(size_t sz, uk callerOverride);
	uk   Realloc(uk old, size_t sz);
	void    Free(uk old); // WARNING: no-op (calling code uses GetHdl() to grab this...)

	TMemHdl GetHdl() const { return m_hdl; }

private:
	TMemHdl                  m_hdl;
	uk                    m_pPin;
	CMementoMemoryUprPtr m_mmm;
};

#endif
