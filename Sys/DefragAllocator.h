// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DEVBUFFERALLOCATOR_H
#define DEVBUFFERALLOCATOR_H

#include <drx3D/CoreX/Memory/IDefragAllocator.h>

#ifndef _RELEASE
	#define CDBA_DEBUG
#endif

//#define CDBA_MORE_DEBUG

#ifdef CDBA_DEBUG
	#define CDBA_ASSERT(x) do { assert(x); if (!(x)) __debugbreak(); } while (0)
#else
	#define CDBA_ASSERT(x) assert(x)
#endif

union SDefragAllocChunkAttr
{
	enum
	{
#if defined(CDBA_MORE_DEBUG)
		SizeWidth        = 26,
		InvalidatedMask  = 1 << 26,
#else
		SizeWidth        = 27,
#endif
		SizeMask         = (1 << SizeWidth) - 1,
		BusyMask         = 1 << 27,
		MovingMask       = 1 << 28,
		MaxPinCount      = 7,
		PinnedCountShift = 29,
		PinnedIncMask    = 1 << PinnedCountShift,
		PinnedCountMask  = MaxPinCount << PinnedCountShift,
	};

	u32             ui;

	ILINE u32 GetSize() const             { return ui & SizeMask; }
	ILINE void         SetSize(u32 size)  { ui = (ui & ~SizeMask) | size; }
	ILINE void         AddSize(i32 size)           { ui += size; }
	ILINE bool         IsBusy() const              { return (ui & BusyMask) != 0; }
	ILINE void         SetBusy(bool b)             { ui = b ? (ui | BusyMask) : (ui & ~BusyMask); }
	ILINE bool         IsMoving() const            { return (ui & MovingMask) != 0; }
	ILINE void         SetMoving(bool m)           { ui = m ? (ui | MovingMask) : (ui & ~MovingMask); }
	ILINE void         IncPinCount()               { ui += PinnedIncMask; }
	ILINE void         DecPinCount()               { ui -= PinnedIncMask; }
	ILINE bool         IsPinned() const            { return (ui & PinnedCountMask) != 0; }
	ILINE u32 GetPinCount() const         { return (ui & PinnedCountMask) >> PinnedCountShift; }
	ILINE void         SetPinCount(u32 p) { ui = (ui & ~PinnedCountMask) | (p << PinnedCountShift); }
#if defined(CDBA_MORE_DEBUG)
	ILINE void Invalidate()                        { ui |= InvalidatedMask; }
	ILINE void Validate()                          { ui &= ~InvalidatedMask; }
	ILINE bool IsInvalid() const                   { return (ui & InvalidatedMask) != 0; }
#endif
};

struct SDefragAllocChunk
{
	enum
	{
		AlignBitCount = 4,
	};
	typedef IDefragAllocator::Hdl Index;

	Index addrPrevIdx;
	Index addrNextIdx;

	union
	{
		struct
		{
			UINT_PTR ptr : sizeof(UINT_PTR) * 8 - AlignBitCount;
			UINT_PTR logAlign: AlignBitCount;
		};
		UINT_PTR packedPtr;
	};
	SDefragAllocChunkAttr attr;

	union
	{
		uk pContext;
		struct
		{
			Index freePrevIdx;
			Index freeNextIdx;
		};
	};

#ifndef _RELEASE
	tukk source;
#endif

#ifdef CDBA_MORE_DEBUG
	u32 hash;
#endif

	void SwapEndian()
	{
		::SwapEndian(addrPrevIdx, true) ;
		::SwapEndian(addrNextIdx, true) ;
		::SwapEndian(packedPtr, true) ;
		::SwapEndian(attr.ui, true);

		if (attr.IsBusy())
		{
			::SwapEndian(pContext, true);
		}
		else
		{
			::SwapEndian(freePrevIdx, true) ;
			::SwapEndian(freeNextIdx, true);
		}

#ifndef _RELEASE
		::SwapEndian(source, true);
#endif

#ifdef CDBA_MORE_DEBUG
		::SwapEndian(hash, true);
#endif
	}
};

struct SDefragAllocSegment
{
	u32                   address;
	u32                   capacity;
	SDefragAllocChunk::Index headSentinalChunkIdx;

	void                     SwapEndian()
	{
		::SwapEndian(address, true) ;
		::SwapEndian(capacity, true) ;
		::SwapEndian(headSentinalChunkIdx, true);
	}
};

class CDefragAllocator;

class CDefragAllocatorWalker
{
public:
	explicit CDefragAllocatorWalker(CDefragAllocator& alloc);
	~CDefragAllocatorWalker();

	const SDefragAllocChunk* Next();

private:
	CDefragAllocatorWalker(const CDefragAllocatorWalker&);
	CDefragAllocatorWalker& operator=(const CDefragAllocatorWalker&);

private:
	CDefragAllocator*        m_pAlloc;
	SDefragAllocChunk::Index m_nChunkIdx;
};

class CDefragAllocator : public IDefragAllocator
{
	friend class CDefragAllocatorWalker;
	typedef SDefragAllocChunk::Index Index;

public:
	CDefragAllocator();

	void Release(bool bDiscard);

	void Init(UINT_PTR capacity, UINT_PTR minAlignment, const Policy& policy);

#ifndef _RELEASE
	void DumpState(tukk filename);
	void RestoreState(tukk filename);
#endif

	bool                  AppendSegment(UINT_PTR capacity);
	void                  UnAppendSegment();

	Hdl                   Allocate(size_t sz, tukk source, uk pContext = NULL);
	Hdl                   AllocateAligned(size_t sz, size_t alignment, tukk source, uk pContext = NULL);
	AllocatePinnedResult  AllocatePinned(size_t sz, tukk source, uk pContext = NULL);
	AllocatePinnedResult  AllocateAlignedPinned(size_t sz, size_t alignment, tukk source, uk pContext = NULL);
	bool                  Free(Hdl hdl);

	void                  ChangeContext(Hdl hdl, uk pNewContext);
	uk                 GetContext(Hdl hdl);

	size_t                GetAllocated() const { return (size_t)(m_capacity - m_available) << m_logMinAlignment; }
	IDefragAllocatorStats GetStats();

	size_t                DefragmentTick(size_t maxMoves, size_t maxAmount, bool bForce);

	ILINE UINT_PTR        UsableSize(Hdl hdl)
	{
		Index chunkIdx = ChunkIdxFromHdl(hdl);
		CDBA_ASSERT(chunkIdx < m_chunks.size());

		SDefragAllocChunk& chunk = m_chunks[chunkIdx];
		SDefragAllocChunkAttr attr = chunk.attr;

		CDBA_ASSERT(attr.IsBusy());
		return (UINT_PTR)attr.GetSize() << m_logMinAlignment;
	}

	// Pin the chunk until the next defrag tick, when it will be automatically unpinned
	ILINE UINT_PTR WeakPin(Hdl hdl)
	{
		Index chunkIdx = ChunkIdxFromHdl(hdl);
		CDBA_ASSERT(chunkIdx < m_chunks.size());

		SDefragAllocChunk& chunk = m_chunks[chunkIdx];
		SDefragAllocChunkAttr attr = chunk.attr;
		CDBA_ASSERT(attr.IsBusy());

		if (attr.IsMoving())
			CancelMove(chunkIdx, true);

		return chunk.ptr << m_logMinAlignment;
	}

	// Pin the chunk until Unpin is called
	ILINE UINT_PTR Pin(Hdl hdl)
	{
		Index chunkIdx = ChunkIdxFromHdl(hdl);

		SDefragAllocChunk& chunk = m_chunks[chunkIdx];
		SDefragAllocChunkAttr attr;
		SDefragAllocChunkAttr newAttr;

		do
		{
			attr.ui = const_cast< u32&>(chunk.attr.ui);
			newAttr.ui = attr.ui;

			CDBA_ASSERT(attr.GetPinCount() < SDefragAllocChunkAttr::MaxPinCount);
			CDBA_ASSERT(attr.IsBusy());

			newAttr.IncPinCount();
#if defined(CDBA_MORE_DEBUG)
			newAttr.Invalidate();
#endif
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&chunk.attr.ui), newAttr.ui, attr.ui) != attr.ui);

		// Potentially a Relocate could be in progress here. Either the Relocate is mid-way, in which case 'IsMoving()' will
		// still be set, and CancelMove will sync and all is well.

		// If 'IsMoving()' is not set, the Relocate should have just completed, in which case ptr should validly point
		// to the new location.

		if (attr.IsMoving())
			CancelMove(chunkIdx, true);

		return chunk.ptr << m_logMinAlignment;
	}

	ILINE void Unpin(Hdl hdl)
	{
		SDefragAllocChunk& chunk = m_chunks[ChunkIdxFromHdl(hdl)];
		SDefragAllocChunkAttr attr;
		SDefragAllocChunkAttr newAttr;
		do
		{
			attr.ui = const_cast< u32&>(chunk.attr.ui);
			newAttr.ui = attr.ui;

			CDBA_ASSERT(attr.IsPinned());
			CDBA_ASSERT(attr.IsBusy());

			newAttr.DecPinCount();
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&chunk.attr.ui), newAttr.ui, attr.ui) != attr.ui);
	}

	ILINE tukk GetSourceOf(Hdl hdl)
	{
#ifndef _RELEASE
		return m_chunks[ChunkIdxFromHdl(hdl)].source;
#else
		return "";
#endif
	}

private:
	enum
	{
		NumBuckets        = 31, // 2GB
		MaxPendingMoves   = 64,

		AddrStartSentinal = 0,
		AddrEndSentinal   = 1,
	};

	struct SplitResult
	{
		bool  bSuccessful;
		Index nLeftSplitChunkIdx;
		Index nRightSplitChunkIdx;
	};

	struct PendingMove
	{
		PendingMove()
			: srcChunkIdx(InvalidChunkIdx)
			, dstChunkIdx(InvalidChunkIdx)
			, userMoveId(0)
			, relocated(false)
			, cancelled(false)
		{
		}

		Index                            srcChunkIdx;
		Index                            dstChunkIdx;
		u32                           userMoveId;
		IDefragAllocatorCopyNotification notify;
		bool                             relocated;
		bool                             cancelled;

		void                             SwapEndian()
		{
			::SwapEndian(srcChunkIdx, true) ;
			::SwapEndian(dstChunkIdx, true) ;
			::SwapEndian(userMoveId, true);
		}
	};

	struct PendingMoveSrcChunkPredicate
	{
		PendingMoveSrcChunkPredicate(Index ci) : m_ci(ci) {}
		bool operator()(const PendingMove& pm) const { return pm.srcChunkIdx == m_ci; }
		Index m_ci;
	};

	typedef DynArray<PendingMove>            PendingMoveVec;
	typedef std::vector<SDefragAllocSegment> SegmentVec;

	static const Index InvalidChunkIdx = (Index) - 1;

private:
	static ILINE Index ChunkIdxFromHdl(Hdl id)    { return id - 1; }
	static ILINE Hdl   ChunkHdlFromIdx(Index idx) { return idx + 1; }

private:
	~CDefragAllocator();

	Index      AllocateChunk();
	void       ReleaseChunk(Index idx);

	ILINE void MarkAsInUse(SDefragAllocChunk& chunk)
	{
		CDBA_ASSERT(!chunk.attr.IsBusy());

		chunk.attr.SetBusy(true);
		m_available -= chunk.attr.GetSize();
		++m_numAllocs;

		// m_available is unsigned, so check for underflow
		CDBA_ASSERT(m_available <= m_capacity);
	}

	ILINE void MarkAsFree(SDefragAllocChunk& chunk)
	{
		CDBA_ASSERT(chunk.attr.IsBusy());

		chunk.attr.SetPinCount(0);
		chunk.attr.SetMoving(false);
		chunk.attr.SetBusy(0);
		m_available += chunk.attr.GetSize();
		--m_numAllocs;

		// m_available is unsigned, so check for underflow
		CDBA_ASSERT(m_available <= m_capacity);
	}

	void LinkFreeChunk(Index idx);
	void UnlinkFreeChunk(Index idx)
	{
		SDefragAllocChunk& chunk = m_chunks[idx];
		m_chunks[chunk.freePrevIdx].freeNextIdx = chunk.freeNextIdx;
		m_chunks[chunk.freeNextIdx].freePrevIdx = chunk.freePrevIdx;
	}

	void LinkAddrChunk(Index idx, Index afterIdx)
	{
		SDefragAllocChunk& chunk = m_chunks[idx];
		SDefragAllocChunk& afterChunk = m_chunks[afterIdx];

		chunk.addrNextIdx = afterChunk.addrNextIdx;
		chunk.addrPrevIdx = afterIdx;
		m_chunks[chunk.addrNextIdx].addrPrevIdx = idx;
		afterChunk.addrNextIdx = idx;
	}

	void UnlinkAddrChunk(Index id)
	{
		SDefragAllocChunk& chunk = m_chunks[id];

		m_chunks[chunk.addrPrevIdx].addrNextIdx = chunk.addrNextIdx;
		m_chunks[chunk.addrNextIdx].addrPrevIdx = chunk.addrPrevIdx;
	}

	void PrepareMergePopNext(Index* pLists)
	{
		for (i32 bucketIdx = 0; bucketIdx < NumBuckets; ++bucketIdx)
		{
			Index hdrChunkId = m_freeBuckets[bucketIdx];
			Index nextId = m_chunks[hdrChunkId].freeNextIdx;
			if (nextId != hdrChunkId)
			{
				pLists[bucketIdx] = nextId;
			}
			else
			{
				pLists[bucketIdx] = InvalidChunkIdx;
			}
		}
	}

	size_t MergePeekNextChunk(Index* pLists)
	{
		size_t farList = (size_t)-1;
		UINT_PTR farPtr = (UINT_PTR)-1;

		for (size_t listIdx = 0; listIdx < NumBuckets; ++listIdx)
		{
			Index chunkIdx = pLists[listIdx];
			if (chunkIdx != InvalidChunkIdx)
			{
				SDefragAllocChunk& chunk = m_chunks[chunkIdx];
				if (chunk.ptr < farPtr)
				{
					farPtr = chunk.ptr;
					farList = listIdx;
				}
			}
		}

		return farList;
	}

	void MergePopNextChunk(Index* pLists, size_t list)
	{
		using std::swap;

		Index fni = m_chunks[pLists[list]].freeNextIdx;
		pLists[list] = fni;
		if (m_chunks[fni].attr.IsBusy())
		{
			// End of the list
			pLists[list] = InvalidChunkIdx;
		}
	}

	void MergePatchNextRemove(Index* pLists, Index removeIdx)
	{
		for (i32 bucketIdx = 0; bucketIdx < NumBuckets; ++bucketIdx)
		{
			if (pLists[bucketIdx] == removeIdx)
			{
				Index nextIdx = m_chunks[removeIdx].freeNextIdx;
				if (!m_chunks[nextIdx].attr.IsBusy())
					pLists[bucketIdx] = nextIdx;
				else
					pLists[bucketIdx] = InvalidChunkIdx;
			}
		}
	}

	void MergePatchNextInsert(Index* pLists, Index insertIdx)
	{
		SDefragAllocChunk& insertChunk = m_chunks[insertIdx];
		i32 bucket = BucketForSize(insertChunk.attr.GetSize());

		if (pLists[bucket] != InvalidChunkIdx)
		{
			SDefragAllocChunk& listChunk = m_chunks[pLists[bucket]];
			if (listChunk.ptr > insertChunk.ptr)
				pLists[bucket] = insertIdx;
		}
	}

	void PrepareMergePopPrev(Index* pLists)
	{
		for (i32 bucketIdx = 0; bucketIdx < NumBuckets; ++bucketIdx)
		{
			Index hdrChunkId = m_freeBuckets[bucketIdx];
			Index prevIdx = m_chunks[hdrChunkId].freePrevIdx;
			if (prevIdx != hdrChunkId)
			{
				pLists[bucketIdx] = prevIdx;
			}
			else
			{
				pLists[bucketIdx] = InvalidChunkIdx;
			}
		}
	}

	size_t MergePeekPrevChunk(Index* pLists)
	{
		size_t farList = (size_t)-1;
		UINT_PTR farPtr = 0;

		for (size_t listIdx = 0; listIdx < NumBuckets; ++listIdx)
		{
			Index chunkIdx = pLists[listIdx];
			if (chunkIdx != InvalidChunkIdx)
			{
				SDefragAllocChunk& chunk = m_chunks[chunkIdx];
				if (chunk.ptr >= farPtr)
				{
					farPtr = chunk.ptr;
					farList = listIdx;
				}
			}
		}

		return farList;
	}

	void MergePopPrevChunk(Index* pLists, size_t list)
	{
		using std::swap;

		Index fpi = m_chunks[pLists[list]].freePrevIdx;
		pLists[list] = fpi;
		if (m_chunks[fpi].attr.IsBusy())
		{
			// End of the list
			pLists[list] = InvalidChunkIdx;
		}
	}

	void MergePatchPrevInsert(Index* pLists, Index insertIdx)
	{
		SDefragAllocChunk& insertChunk = m_chunks[insertIdx];
		i32 bucket = BucketForSize(insertChunk.attr.GetSize());

		if (pLists[bucket] != InvalidChunkIdx)
		{
			SDefragAllocChunk& listChunk = m_chunks[pLists[bucket]];
			if (listChunk.ptr < insertChunk.ptr)
				pLists[bucket] = insertIdx;
		}
	}

	void MergePatchPrevRemove(Index* pLists, Index removeIdx)
	{
		for (i32 bucketIdx = 0; bucketIdx < NumBuckets; ++bucketIdx)
		{
			if (pLists[bucketIdx] == removeIdx)
			{
				Index nextIdx = m_chunks[removeIdx].freePrevIdx;
				if (!m_chunks[nextIdx].attr.IsBusy())
					pLists[bucketIdx] = nextIdx;
				else
					pLists[bucketIdx] = InvalidChunkIdx;
			}
		}
	}

	void MarkAsMoving(SDefragAllocChunk& src)
	{
		SDefragAllocChunkAttr srcAttr, srcNewAttr;
		do
		{
			srcAttr.ui = const_cast< u32&>(src.attr.ui);
			srcNewAttr.ui = srcAttr.ui;
			srcNewAttr.SetMoving(true);
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&src.attr.ui), srcNewAttr.ui, srcAttr.ui) != srcAttr.ui);
	}

	void MarkAsNotMoving(SDefragAllocChunk& src)
	{
		SDefragAllocChunkAttr srcAttr, srcNewAttr;
		do
		{
			srcAttr.ui = const_cast< u32&>(src.attr.ui);
			srcNewAttr.ui = srcAttr.ui;
			srcNewAttr.SetMoving(false);
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&src.attr.ui), srcNewAttr.ui, srcAttr.ui) != srcAttr.ui);
	}

#if defined(CDBA_MORE_DEBUG)
	void MarkAsNotMovingValid(SDefragAllocChunk& src)
	{
		SDefragAllocChunkAttr srcAttr, srcNewAttr;
		do
		{
			srcAttr.ui = const_cast< u32&>(src.attr.ui);
			srcNewAttr.ui = srcAttr.ui;
			srcNewAttr.SetMoving(false);
			srcNewAttr.Validate();
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&src.attr.ui), srcNewAttr.ui, srcAttr.ui) != srcAttr.ui);
	}
#endif

	ILINE bool IsMoveableCandidate(const SDefragAllocChunkAttr& a, u32 sizeUpperBound)
	{
		return a.IsBusy() && !a.IsPinned() && !a.IsMoving() && (0 < a.GetSize()) && (a.GetSize() <= sizeUpperBound);
	}

	bool TryMarkAsMoving(SDefragAllocChunk& src, u32 sizeUpperBound)
	{
		SDefragAllocChunkAttr srcAttr, srcNewAttr;
		do
		{
			srcAttr.ui = const_cast< u32&>(src.attr.ui);
			srcNewAttr.ui = srcAttr.ui;
			if (!IsMoveableCandidate(srcAttr, sizeUpperBound))
				return false;
			srcNewAttr.SetMoving(true);
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&src.attr.ui), srcNewAttr.ui, srcAttr.ui) != srcAttr.ui);
		return true;
	}

	bool TryScheduleCopy(SDefragAllocChunk& srcChunk, SDefragAllocChunk& dstChunk, PendingMove* pPM, bool bLowHalf)
	{
		UINT_PTR dstChunkBase = dstChunk.ptr;
		UINT_PTR dstChunkEnd = dstChunkBase + dstChunk.attr.GetSize();
#if (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) || DRX_PLATFORM_DURANGO
		UINT_PTR allocAlign = BIT64(srcChunk.logAlign);
#else
		UINT_PTR allocAlign = BIT(srcChunk.logAlign);
#endif
		UINT_PTR allocSize = srcChunk.attr.GetSize();
		UINT_PTR dstAllocBase = bLowHalf
		                        ? Align(dstChunkBase, allocAlign)
		                        : ((dstChunkEnd - allocSize) & ~(allocAlign - 1));

		u32 userId = m_policy.pDefragPolicy->BeginCopy(
		  srcChunk.pContext,
		  dstAllocBase << m_logMinAlignment,
		  srcChunk.ptr << m_logMinAlignment,
		  allocSize << m_logMinAlignment,
		  &pPM->notify);
		pPM->userMoveId = userId;

		return userId != 0;
	}

	Hdl         Allocate_Locked(size_t sz, size_t alignment, tukk source, uk pContext);
	SplitResult SplitFreeBlock(Index fbId, size_t sz, size_t alignment, bool allocateInLowHalf);
	Index       MergeFreeBlock(Index fbId);

#ifdef CDBA_MORE_DEBUG
	void Defrag_ValidateFreeBlockIteration();
	void Tick_Validation_Locked();
#endif

	Index        BestFit_FindFreeBlockForSegment(size_t sz, size_t alignment, u32 nSegment);
	Index        BestFit_FindFreeBlockFor(size_t sz, size_t alignment, UINT_PTR addressMin, UINT_PTR addressMax, bool allocateInLowHalf);
	Index        FirstFit_FindFreeBlockFor(size_t sz, size_t alignment, UINT_PTR addressMin, UINT_PTR addressMax, bool allocateInLowHalf);

	size_t       Defrag_FindMovesBwd(PendingMove** pMoves, size_t maxMoves, size_t& curAmount, size_t maxAmount);
	size_t       Defrag_FindMovesFwd(PendingMove** pMoves, size_t maxMoves, size_t& curAmount, size_t maxAmount);
	bool         Defrag_CompletePendingMoves();
	Index        Defrag_Bwd_FindFreeBlockFor(size_t sz, size_t alignment, UINT_PTR addressLimit);
	Index        Defrag_Fwd_FindFreeBlockFor(size_t sz, size_t alignment, UINT_PTR addressLimit);

	PendingMove* AllocPendingMove();
	void         FreePendingMove(PendingMove* pMove);

	void         CancelMove(Index srcChunkIdx, bool bIsContentNeeded);
	void         CancelMove_Locked(Index srcChunkIdx, bool bIsContentNeeded);
	void         Relocate(u32 userMoveId, Index srcChunkIdx, Index dstChunkIdx);

	void         SyncMoveSegment(u32 seg);

	void         RebuildFreeLists();
	void         ValidateAddressChain();
	void         ValidateFreeLists();

	i32          BucketForSize(size_t sz) const
	{
		return sz > 0
		       ? static_cast<i32>(IntegerLog2(sz))
		       : 0;
	}

private:
	bool                           m_isThreadSafe;
	bool                           m_chunksAreFixed;

	DrxCriticalSection             m_lock;

	u32                         m_capacity;
	u32                         m_available;
	Index                          m_numAllocs;

	u32                         m_minAlignment;
	u32                         m_logMinAlignment;

	Index                          m_freeBuckets[NumBuckets];

	std::vector<SDefragAllocChunk> m_chunks;
	std::vector<Index>             m_unusedChunks;
	PendingMoveVec                 m_pendingMoves;
	SegmentVec                     m_segments;

	u32                         m_nCancelledMoves;

	Policy                         m_policy;

#ifdef CDBA_MORE_DEBUG
	u32                         m_nLastCheckedChunk;
#endif
};

#endif
