// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <xg.h>
#include <numeric>

#if BUFFER_ENABLE_DIRECT_ACCESS
extern const GUID BufferPointerGuid;
#endif

class SGPUMemHdl
{
public:
	SGPUMemHdl()
		: m_handleAndFlags(0)
	{
	}

	explicit SGPUMemHdl(IDefragAllocator::Hdl hdl)
		: m_handleAndFlags((UINT_PTR)hdl << FlagsShift)
	{
	}

	explicit SGPUMemHdl(uk pFixed)
		: m_handleAndFlags((UINT_PTR)pFixed | IsFixedFlag)
	{
	}

	ILINE bool IsValid() const
	{
		return m_handleAndFlags != 0;
	}

	ILINE i32 IsFixed() const
	{
		return m_handleAndFlags & IsFixedFlag;
	}

	uk GetFixedAddress() const
	{
		assert(IsFixed());
		return (uk )(m_handleAndFlags & ~FlagsMask);
	}

	IDefragAllocator::Hdl GetHandle() const
	{
		assert(!IsFixed());
		return (IDefragAllocator::Hdl)(m_handleAndFlags >> FlagsShift);
	}

	friend bool operator == (const SGPUMemHdl& a, const SGPUMemHdl& b)
	{
		return a.m_handleAndFlags == b.m_handleAndFlags;
	}

	friend bool operator < (const SGPUMemHdl& a, const SGPUMemHdl& b)
	{
		return a.m_handleAndFlags < b.m_handleAndFlags;
	}

private:
	enum
	{
		IsFixedFlag = 0x1,
		FlagsMask = 0x1,
		FlagsShift = 1,
	};

private:
	UINT_PTR m_handleAndFlags;
};

class CDurangoGPUMemoryUpr : public IDefragAllocatorPolicy
{
public:
	struct AllocateResult
	{
		AllocateResult()
			: baseAddress()
		{
		}

		SGPUMemHdl hdl;
		uk baseAddress;
	};

	enum
	{
		AllocAlign = 32 * 1024,
	};

public:
	CDurangoGPUMemoryUpr();
	~CDurangoGPUMemoryUpr();

	IDefragAllocatorStats GetStats();

	bool Init(size_t size, size_t bankSize, size_t reserveSize, u32 xgMemType, bool allowAdditionalBanks);
	void DeInit();

	size_t GetPoolSize() const;
	size_t GetPoolAllocated() const;

	void RT_Tick();

	AllocateResult AllocatePinned(size_t amount, size_t align);
	void Free(SGPUMemHdl hdl, UINT64 fence = ~0ull);
	void FreeUnused(SGPUMemHdl hdl);

	void BindContext(SGPUMemHdl hdl, CDeviceTexture* pDevTex);
	uk WeakPin(SGPUMemHdl hdl);
	uk Pin(SGPUMemHdl hdl);
	void Unpin(SGPUMemHdl hdl);

	void GpuPin(SGPUMemHdl hdl);
	void GpuUnpin(SGPUMemHdl hdl, ID3DXboxPerformanceContext* pCtx);
	void GpuUnpin(SGPUMemHdl hdl, ID3D11DmaEngineContextX* pCtx);

public: // IDefragAllocatorPolicy Members
	virtual u32 BeginCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pCompleteNotification);
	virtual void Relocate(u32 userMoveId, uk pContext, UINT_PTR newOffset, UINT_PTR oldOffset, UINT_PTR size);
	virtual void CancelCopy(u32 userMoveId, uk pContext, bool bSync);
	virtual void SyncCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size);

	virtual u32 Hash(UINT_PTR offset, UINT_PTR size);

private:
	enum
	{
		BankAlign = 64 * 1024,
		MaxCopies = 64,

		// Minimum alignment that can be requested of D3DAllocateGraphicsMemory
		MinD3DAlignment = 4096,
	};

	struct PendingFree
	{
		UINT64     fence;
		SGPUMemHdl hdl;

		friend bool operator < (const PendingFree& a, const PendingFree& b)
		{
			return a.hdl < b.hdl;
		}
	};

	struct Bank
	{
		Bank()
			: pBase(NULL)
			, pBuffer(NULL)
		{
		}

		uk  pBase;
		ID3D11Buffer* pBuffer;
	};

	struct CopyDesc
	{
		UINT_PTR src;
		UINT_PTR dst;
		UINT_PTR size;
		u32 idx;
		uint64* fence;
		bool copied;

		CopyDesc()
			: src()
			, dst()
			, size()
			, idx(~0u)
			, fence()
			, copied()
		{}
	};

	struct InflightCopy
	{
		InflightCopy()
		{
			memset(this, 0, sizeof(*this));
		}

		InflightCopy(UINT_PTR srcOffset, UINT_PTR dstOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pNotification)
			: srcOffset(srcOffset)
			, dstOffset(dstOffset)
			, size(size)
			, copying(false)
			, cooling(false)
			, cancelled(false)
			, copyFence(0)
			, inUseFence(0)
			, pNotification(pNotification)
		{
		}

		UINT_PTR srcOffset;
		UINT_PTR dstOffset;
		UINT_PTR size;
		bool copying;
		bool cooling;
		bool cancelled;
		uint64 copyFence;
		uint64 inUseFence;
		IDefragAllocatorCopyNotification* pNotification;
	};

	typedef std::vector<PendingFree> PendingFreeVec;
	typedef std::vector<CopyDesc, stl::STLGlobalAllocator<CopyDesc> > QueuedCopyVec;

private:
	CDurangoGPUMemoryUpr(const CDurangoGPUMemoryUpr&);
	CDurangoGPUMemoryUpr& operator=(const CDurangoGPUMemoryUpr&);

private:
	void TickFrees_Locked();
	void TickUnpins_Locked();
	void RemovePendingUnpin_Locked(IDefragAllocator::Hdl hdl);
	void PushPendingUnpin(SGPUMemHdl hdl, UINT64 fence);

	void CollectGarbage(size_t maxMoves, size_t maxAmount);
	void CompleteMoves();
	void PerformCopies();
	void QueueCopy(const CopyDesc& copy);	
	void ScheduleCopies(CopyDesc* descriptions, size_t ncopies);
	void Relocate_Int(CDeviceTexture* pDevTex, tuk pOldTexBase, tuk pTexBase, UINT_PTR size);
	tuk GetPhysicalAddress(UINT_PTR offs);

private:
	DrxCriticalSectionNonRecursive m_lock;

	IDefragAllocator* m_pAllocator;
	uk m_pCPUAddr;
	UINT_PTR m_bankShift;
	UINT m_memType;
	bool m_allowAdditionalBanks;

	PendingFreeVec m_pendingFrees;
	std::vector<Bank> m_banks;

	PendingFreeVec m_pendingUnpins;

	uint64 m_tickFence;
	QueuedCopyVec m_queuedCopies;
	InflightCopy m_copies[MaxCopies];
	u32 m_nLastCopyIdx;
};

interface ID3D11DmaEngineContextX;

class CDurangoGPURingMemAllocator
{
public:
	typedef u16 TAllocateContext;

public:
	CDurangoGPURingMemAllocator();

	bool  Init(ID3D11DmaEngineContextX* pContext, u32 size);

	uk BeginAllocate(u32 size, u32 align, TAllocateContext& contextOut);
	void  EndAllocate(TAllocateContext context, uint64 freeFence);

private:
	typedef u16 TBlockId;

	struct Block
	{
		uint64   fence;
		u32   offset;
		TBlockId next;
	};

	enum
	{
		InvalidBlockId = 0xffff,
		MaxBlocks = 32768,
		BaseAlignment = 64 * 1024,
	};

private:
	TBlockId AllocateBlockId()
	{
		if (m_freeBlocks.empty())
		{
			return InvalidBlockId;
		}
		TBlockId ret = m_freeBlocks.back();
		m_freeBlocks.pop_back();
		return ret;
	}

	void ReleaseBlockId(TBlockId id)
	{
		m_freeBlocks.push_back(id);
	}

private:
	ID3D11DmaEngineContextX* m_pContext;
	tuk                    m_pCPUAddr;
	u32                   m_capacity;

	// First block in a series of allocates
	TBlockId              m_allocateHead;
	size_t                m_allocateDepth;

	std::vector<Block>    m_blocks;
	std::vector<TBlockId> m_freeBlocks;
	TBlockId              m_freeHead;
	TBlockId              m_startSentinalId;
	TBlockId              m_endSentinalId;
};

union SMinimisedBufferDesc
{
	uint64 key;

	struct
	{
		u32 size;
	};

	friend bool operator<(SMinimisedBufferDesc a, SMinimisedBufferDesc b)
	{
		return a.key < b.key;
	}
};

union SMinimisedTexture2DDesc
{
	uint64 key;

	struct
	{
		u16 width;
		u16 height;

		u16 mips : 4;
		u16 arraySize : 12;
		u8  format;
		u8  isCube : 1;
		u8  tileMode : 7;
	};

	friend bool operator<(SMinimisedTexture2DDesc a, SMinimisedTexture2DDesc b)
	{
		return a.key < b.key;
	}
};

struct SDeviceBufferDesc
{
	SMinimisedBufferDesc    desc;
	XG_RESOURCE_LAYOUT      layout;
};

struct SDeviceTextureDesc
{
	XG_RESOURCE_LAYOUT layout;
	XG_TILE_MODE xgTileMode;
	D3D11_TEXTURE2D_DESC d3dDesc;
};
