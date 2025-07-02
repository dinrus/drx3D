// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class IControllerRelocatableChain;

class CControllerDefragHdl
{
public:
	CControllerDefragHdl()
		: m_hdlAndFlags(0)
	{
	}

	explicit CControllerDefragHdl(uk pFixed)
		: m_hdlAndFlags((UINT_PTR)pFixed)
	{
	}

	explicit CControllerDefragHdl(IDefragAllocator::Hdl hdl)
		: m_hdlAndFlags((hdl << 1) | IsHandleMask)
	{
	}

	bool                  IsFixed() const { return (m_hdlAndFlags & IsHandleMask) == 0; }
	IDefragAllocator::Hdl AsHdl() const   { return (IDefragAllocator::Hdl)(m_hdlAndFlags >> 1); }
	uk                 AsFixed() const { return (uk )m_hdlAndFlags; }

	bool                  IsValid() const { return m_hdlAndFlags != 0; }

	friend bool           operator==(CControllerDefragHdl a, CControllerDefragHdl b)
	{
		return a.m_hdlAndFlags == b.m_hdlAndFlags;
	}

	friend bool operator!=(CControllerDefragHdl a, CControllerDefragHdl b)
	{
		return a.m_hdlAndFlags != b.m_hdlAndFlags;
	}

private:
	enum
	{
		IsHandleMask = 0x1
	};

private:
	UINT_PTR m_hdlAndFlags;
};

class CControllerDefragHeap : private IDefragAllocatorPolicy
{
public:
	struct Stats
	{
		IDefragAllocatorStats defragStats;
		size_t                bytesInFixedAllocs;
	};

public:
	CControllerDefragHeap();
	~CControllerDefragHeap();

	void                 Init(size_t capacity);
	bool                 IsInitialised() const { return m_pAddressRange != NULL; }

	Stats                GetStats();

	void                 Update();

	CControllerDefragHdl AllocPinned(size_t sz, IControllerRelocatableChain* pContext);
	void                 Free(CControllerDefragHdl hdl);

	void                 ChangeContext(CControllerDefragHdl hdl, IControllerRelocatableChain* pContext)
	{
		if (!hdl.IsFixed())
			m_pAllocator->ChangeContext(hdl.AsHdl(), pContext);
	}

	size_t UsableSize(CControllerDefragHdl hdl);

	uk  WeakPin(CControllerDefragHdl hdl)
	{
		if (!hdl.IsFixed())
			return m_pBaseAddress + m_pAllocator->WeakPin(hdl.AsHdl());
		else
			return hdl.AsFixed();
	}

	uk Pin(CControllerDefragHdl hdl)
	{
		if (!hdl.IsFixed())
			return m_pBaseAddress + m_pAllocator->Pin(hdl.AsHdl());
		else
			return hdl.AsFixed();
	}

	void Unpin(CControllerDefragHdl hdl)
	{
		if (!hdl.IsFixed())
			m_pAllocator->Unpin(hdl.AsHdl());
	}

private:
	union FixedHdr
	{
		char pad[16];
		struct
		{
			u32 size;
			bool   bFromGPH;
		};
	};

	struct Copy
	{
		Copy()
		{
			memset(this, 0, sizeof(*this));
		}

		Copy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pNotification)
			: inUse(true)
			, pContext(pContext)
			, dstOffset(dstOffset)
			, srcOffset(srcOffset)
			, size(size)
			, pNotification(pNotification)
			, jobState(-1)
			, relocateFrameId(0)
			, cancelled(false)
			, relocated(false)
		{
		}

		bool                              inUse;

		uk                             pContext;
		UINT_PTR                          dstOffset;
		UINT_PTR                          srcOffset;
		UINT_PTR                          size;
		IDefragAllocatorCopyNotification* pNotification;

		 i32                      jobState;
		u32                            relocateFrameId;
		bool                              cancelled;
		bool                              relocated;
	};

	enum
	{
		CompletionLatencyFrames     = 2,
		MaxInFlightCopies           = 32,
		MaxScheduledCopiesPerUpdate = 4,
		MaxScheduledBytesPerUpdate  = 128 * 1024,
		MinJobCopySize              = 4096,
		MinFixedAllocSize           = 384 * 1024,
		MinAlignment                = 16,
		MaxAllocs                   = 4 * 1024,
		AllowGPHFallback            = 1,
	};

private:
	virtual u32 BeginCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pNotification);
	virtual void   Relocate(u32 userMoveId, uk pContext, UINT_PTR newOffset, UINT_PTR oldOffset, UINT_PTR size);
	virtual void   CancelCopy(u32 userMoveId, uk pContext, bool bSync);

	virtual void   SyncCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size);

private:
	CControllerDefragHeap(const CControllerDefragHeap&);
	CControllerDefragHeap& operator=(const CControllerDefragHeap&);

private:
	bool  IncreaseRange(UINT_PTR offs, UINT_PTR sz);
	void  DecreaseRange(UINT_PTR offs, UINT_PTR sz);
	void  DecreaseRange_Locked(UINT_PTR offs, UINT_PTR sz);

	uk FixedAlloc(size_t sz, bool bFromGPH);
	void  FixedFree(uk p);

	void  UpdateInflight(i32 frameId);

private:
	IMemoryAddressRange*           m_pAddressRange;
	IDefragAllocator*              m_pAllocator;

	tuk                          m_pBaseAddress;
	u32                         m_nPageSize;
	u32                         m_nLogPageSize;

	DrxCriticalSectionNonRecursive m_pagesLock;
	std::vector<i32>               m_numAllocsPerPage;

	 i32                   m_nBytesInFixedAllocs;

	Copy                           m_copiesInFlight[MaxInFlightCopies];
	size_t                         m_numAvailableCopiesInFlight;
	size_t                         m_bytesInFlight;
	i32                            m_tickId;
	u32                         m_nLastCopyIdx;

	DrxCriticalSectionNonRecursive m_queuedCancelLock;
	i32                            m_numQueuedCancels;
	u32                         m_queuedCancels[MaxInFlightCopies];
};
