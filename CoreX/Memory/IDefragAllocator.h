// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef IDEFRAGALLOCATOR_H
#define IDEFRAGALLOCATOR_H

struct IDefragAllocatorStats
{
	size_t nCapacity;
	size_t nInUseSize;
	u32 nInUseBlocks;
	u32 nFreeBlocks;
	u32 nPinnedBlocks;
	u32 nMovingBlocks;
	u32 nLargestFreeBlockSize;
	u32 nSmallestFreeBlockSize;
	u32 nMeanFreeBlockSize;
	u32 nCancelledMoveCount;
};

struct IDefragAllocatorCopyNotification
{
	IDefragAllocatorCopyNotification()
		: bDstIsValid(false)
		, bSrcIsUnneeded(false)
		, bCancel(false)
	{
	}

	bool bDstIsValid;
	bool bSrcIsUnneeded;

	//! Flag to indicate that the copy can't be initiated after all -
	// currently only cancelling before a relocate
	//! is begun is supported, and the destination region must be stable.
	bool bCancel;
};

class IDefragAllocatorPolicy
{
public:
	enum : u32
	{
		InvalidUserMoveId = 0xffffffff
	};

public:
	// <interfuscator:shuffle>
	virtual u32 BeginCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pNotification) = 0;
	virtual void   Relocate(u32 userMoveId, uk pContext, UINT_PTR newOffset, UINT_PTR oldOffset, UINT_PTR size) = 0;
	virtual void   CancelCopy(u32 userMoveId, uk pContext, bool bSync) = 0;

	//! Perform the copy and relocate immediately - will only be called when UnAppendSegment is.
	virtual void SyncCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size) = 0;
	
	virtual u32 Hash(UINT_PTR offset, UINT_PTR size) { return 0; }
	// </interfuscator:shuffle>

protected:
	virtual ~IDefragAllocatorPolicy() {}
};

class IDefragAllocator
{
public:
	typedef u32 Hdl;
	enum
	{
		InvalidHdl = 0,
	};

	struct AllocatePinnedResult
	{
		Hdl      hdl;
		UINT_PTR offs;
		UINT_PTR usableSize;
	};

	enum EBlockSearchKind
	{
		eBSK_BestFit,
		eBSK_FirstFit
	};

	struct Policy
	{
		Policy()
			: pDefragPolicy(NULL)
			, maxAllocs(0)
			, maxSegments(1)
			, blockSearchKind(eBSK_BestFit)
		{
		}

		IDefragAllocatorPolicy* pDefragPolicy;
		size_t                  maxAllocs;
		size_t                  maxSegments;
		EBlockSearchKind        blockSearchKind;
	};

public:
	// <interfuscator:shuffle>
	virtual void                  Release(bool bDiscard = false) = 0;

	virtual void                  Init(UINT_PTR capacity, UINT_PTR alignment, const Policy& policy = Policy()) = 0;

	virtual bool                  AppendSegment(UINT_PTR capacity) = 0;
	virtual void                  UnAppendSegment() = 0;

	virtual Hdl                   Allocate(size_t sz, tukk source, uk pContext = NULL) = 0;
	virtual Hdl                   AllocateAligned(size_t sz, size_t alignment, tukk source, uk pContext = NULL) = 0;
	virtual AllocatePinnedResult  AllocatePinned(size_t sz, tukk source, uk pContext = NULL) = 0;
	virtual AllocatePinnedResult  AllocateAlignedPinned(size_t sz, size_t alignment, tukk source, uk pContext = NULL) = 0;
	virtual bool                  Free(Hdl hdl) = 0;

	virtual void                  ChangeContext(Hdl hdl, uk pNewContext) = 0;
	virtual uk                 GetContext(Hdl hdl) = 0;

	virtual size_t                GetAllocated() const = 0;
	virtual IDefragAllocatorStats GetStats() = 0;

	virtual size_t                DefragmentTick(size_t maxMoves, size_t maxAmount, bool bForce = false) = 0;

	virtual UINT_PTR              UsableSize(Hdl hdl) = 0;

	//! Pin the chunk until the next defrag tick, when it will be automatically unpinned.
	virtual UINT_PTR WeakPin(Hdl hdl) = 0;

	//! Pin the chunk until Unpin is called.
	virtual UINT_PTR    Pin(Hdl hdl) = 0;

	virtual void        Unpin(Hdl hdl) = 0;

	virtual tukk GetSourceOf(Hdl hdl) = 0;
	// </interfuscator:shuffle>

#ifndef _RELEASE
	virtual void DumpState(tukk filename) = 0;
	virtual void RestoreState(tukk filename) = 0;
#endif

protected:
	virtual ~IDefragAllocator() {}
};

#endif
