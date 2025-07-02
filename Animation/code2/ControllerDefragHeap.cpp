// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/ControllerDefragHeap.h>

#include <drx3D/Animation/ControllerPQ.h>

CControllerDefragHeap::CControllerDefragHeap()
	: m_pAddressRange(NULL)
	, m_pAllocator(NULL)
	, m_pBaseAddress(NULL)
	, m_nPageSize(0)
	, m_nLogPageSize(0)
	, m_nBytesInFixedAllocs(0)
	, m_numAvailableCopiesInFlight(MaxInFlightCopies)
	, m_bytesInFlight(0)
	, m_tickId(0)
	, m_nLastCopyIdx(0)
	, m_numQueuedCancels(0)
{
}

CControllerDefragHeap::~CControllerDefragHeap()
{
	if (m_pAllocator)
	{
		// Release with discard because controller heap is global, and torn down with engine. This may
		// occur even though the character manager hasn't been destroyed, so allocations may still be active.
		// The engine is lost though, so it doesn't matter...
		m_pAllocator->Release(true);
		m_pAllocator = NULL;
	}
}

void CControllerDefragHeap::Init(size_t capacity)
{
	assert(!m_pAllocator);

	m_pAddressRange = DrxGetIMemoryUpr()->ReserveAddressRange(capacity, "Anim Defrag Heap");
	m_pBaseAddress = m_pAddressRange->GetBaseAddress();
	m_nPageSize = m_pAddressRange->GetPageSize();
	m_nLogPageSize = IntegerLog2(m_nPageSize);

	capacity = Align(capacity, m_nPageSize);

	m_pAllocator = DrxGetIMemoryUpr()->CreateDefragAllocator();

	IDefragAllocator::Policy pol;
	pol.pDefragPolicy = this;
	pol.maxAllocs = MaxAllocs;
	m_pAllocator->Init(capacity, MinAlignment, pol);

	m_numAllocsPerPage.resize(capacity >> m_nLogPageSize);
}

CControllerDefragHeap::Stats CControllerDefragHeap::GetStats()
{
	Stats stats;

	stats.defragStats = m_pAllocator->GetStats();
	stats.bytesInFixedAllocs = (size_t)m_nBytesInFixedAllocs;

	return stats;
}

void CControllerDefragHeap::Update()
{
	DRX_PROFILE_FUNCTION(PROFILE_ANIMATION);

	UpdateInflight(m_tickId);

	if (m_pAllocator != nullptr)
	{
		m_pAllocator->DefragmentTick(
			(size_t)min((i32)MaxScheduledCopiesPerUpdate, (i32)m_numAvailableCopiesInFlight),
			(size_t)min((i32)max(0, (i32)MinFixedAllocSize - (i32)m_bytesInFlight), (i32)MaxScheduledBytesPerUpdate));
	}

	++m_tickId;
}

CControllerDefragHdl CControllerDefragHeap::AllocPinned(size_t sz, IControllerRelocatableChain* pContext)
{
	CControllerDefragHdl ret;

	if (sz < MinFixedAllocSize)
	{
		IDefragAllocator::AllocatePinnedResult apr = m_pAllocator->AllocatePinned(sz, "", pContext);

		if (apr.hdl != IDefragAllocator::InvalidHdl)
		{
			// Ensure that the requisite pages are mapped
			if (IncreaseRange(apr.offs, apr.usableSize))
			{
				ret = CControllerDefragHdl(apr.hdl);

				MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
				MEMREPLAY_SCOPE_ALLOC(m_pBaseAddress + apr.offs, apr.usableSize, MinAlignment);
			}
			else
			{
				m_pAllocator->Free(apr.hdl);
			}
		}
		else if (AllowGPHFallback)
		{
			MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

			ret = CControllerDefragHdl(FixedAlloc(sz, true));

			if (ret.IsValid())
			{
				MEMREPLAY_SCOPE_ALLOC(ret.AsFixed(), UsableSize(ret), MinAlignment);
			}
		}
	}
	else
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		ret = CControllerDefragHdl(FixedAlloc(sz, false));

		if (ret.IsValid())
		{
			MEMREPLAY_SCOPE_ALLOC(ret.AsFixed(), UsableSize(ret), MinAlignment);
		}
	}

	return ret;
}

void CControllerDefragHeap::Free(CControllerDefragHdl hdl)
{
	if (!hdl.IsFixed())
	{
		UINT_PTR offs = m_pAllocator->Pin(hdl.AsHdl());

		{
			MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
			MEMREPLAY_SCOPE_FREE(m_pBaseAddress + offs);
		}

		assert(hdl.IsValid());

		// Ensure that pages are unmapped
		UINT_PTR sz = m_pAllocator->UsableSize(hdl.AsHdl());

		DecreaseRange(offs, sz);

		m_pAllocator->Free(hdl.AsHdl());
	}
	else
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		FixedFree(hdl.AsFixed());

		MEMREPLAY_SCOPE_FREE(hdl.AsFixed());
	}
}

size_t CControllerDefragHeap::UsableSize(CControllerDefragHdl hdl)
{
	assert(hdl.IsValid());
	if (!hdl.IsFixed())
	{
		return m_pAllocator->UsableSize(hdl.AsHdl());
	}
	else
	{
		FixedHdr* pHdr = ((FixedHdr*)hdl.AsFixed()) - 1;
		return pHdr->size;
	}
}

u32 CControllerDefragHeap::BeginCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size, IDefragAllocatorCopyNotification* pNotification)
{
	// Try and boost the destination range
	if (IncreaseRange(dstOffset, size))
	{
		u32 nLastId = m_nLastCopyIdx;
		u32 nIdx = (u32) - 1;

		for (u32 i = 0; i < MaxInFlightCopies; ++i)
		{
			if (!m_copiesInFlight[(nLastId + i) % MaxInFlightCopies].inUse)
			{
				nIdx = (nLastId + i) % MaxInFlightCopies;
				break;
			}
		}

		if (nIdx != (u32) - 1)
		{
			assert(nIdx >= 0 && nIdx < DRX_ARRAY_COUNT(m_copiesInFlight));
			Copy& cp = m_copiesInFlight[nIdx];
			stl::reconstruct(cp, pContext, dstOffset, srcOffset, size, pNotification);

			if (cp.size >= MinJobCopySize)
			{
				// Boost the src region so it will remain valid during the copy
				IncreaseRange(srcOffset, size);

				cp.jobState = 1;
				drxAsyncMemcpy(m_pBaseAddress + cp.dstOffset, m_pBaseAddress + cp.srcOffset, cp.size, MC_CPU_TO_CPU, &cp.jobState);
			}
			else
			{
				memcpy(m_pBaseAddress + cp.dstOffset, m_pBaseAddress + cp.srcOffset, cp.size);
				cp.pNotification->bDstIsValid = true;
			}

			--m_numAvailableCopiesInFlight;
			m_bytesInFlight += cp.size;
			m_nLastCopyIdx = nIdx;

			return nIdx + 1;
		}
	}

	return 0;
}

void CControllerDefragHeap::Relocate(u32 userMoveId, uk pContext, UINT_PTR newOffset, UINT_PTR oldOffset, UINT_PTR size)
{
	Copy& cp = m_copiesInFlight[userMoveId - 1];
	if (cp.inUse && (cp.pContext == pContext))
	{
#ifndef _RELEASE
		if (cp.relocated)
			__debugbreak();
#endif

		tuk pNewBase = m_pBaseAddress + newOffset;
		tuk pOldBase = m_pBaseAddress + oldOffset;

		for (IControllerRelocatableChain* rbc = (IControllerRelocatableChain*)pContext; rbc; rbc = rbc->GetNext())
			rbc->Relocate(pNewBase, pOldBase);

		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
		MEMREPLAY_SCOPE_REALLOC(m_pBaseAddress + oldOffset, m_pBaseAddress + newOffset, size, MinAlignment);

		cp.relocateFrameId = m_tickId;
		cp.relocated = true;
		return;
	}

#ifndef _RELEASE
	__debugbreak();
#endif
}

void CControllerDefragHeap::CancelCopy(u32 userMoveId, uk pContext, bool bSync)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_queuedCancelLock);
#ifndef _RELEASE
	if (m_numQueuedCancels == DRX_ARRAY_COUNT(m_queuedCancels))
		__debugbreak();
#endif
	assert(m_numQueuedCancels < DRX_ARRAY_COUNT(m_queuedCancels));
	m_queuedCancels[m_numQueuedCancels++] = userMoveId;
}

void CControllerDefragHeap::SyncCopy(uk pContext, UINT_PTR dstOffset, UINT_PTR srcOffset, UINT_PTR size)
{
	// Not supported - but shouldn't be needed.
	__debugbreak();
}

bool CControllerDefragHeap::IncreaseRange(UINT_PTR offs, UINT_PTR sz)
{
	UINT_PTR basePageIdx = offs >> m_nLogPageSize;
	UINT_PTR lastPageIdx = (offs + sz + m_nPageSize - 1) >> m_nLogPageSize;

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_pagesLock);

	for (UINT_PTR pageIdx = basePageIdx; pageIdx != lastPageIdx; ++pageIdx)
	{
		if ((++m_numAllocsPerPage[pageIdx]) == 1)
		{
			if (!m_pAddressRange->MapPage(pageIdx))
			{
				// Failed to allocate. Unwind mapping.
				DecreaseRange_Locked(offs, (pageIdx - basePageIdx) * m_nPageSize);
				return false;
			}
		}
	}

	return true;
}

void CControllerDefragHeap::DecreaseRange(UINT_PTR offs, UINT_PTR sz)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_pagesLock);
	DecreaseRange_Locked(offs, sz);
}

void CControllerDefragHeap::DecreaseRange_Locked(UINT_PTR offs, UINT_PTR sz)
{
	UINT_PTR basePageIdx = offs >> m_nLogPageSize;
	UINT_PTR lastPageIdx = (offs + sz + m_nPageSize - 1) >> m_nLogPageSize;

	for (UINT_PTR pageIdx = basePageIdx; pageIdx != lastPageIdx; ++pageIdx)
	{
		if (!(--m_numAllocsPerPage[pageIdx]))
			m_pAddressRange->UnmapPage(pageIdx);
	}
}

uk CControllerDefragHeap::FixedAlloc(size_t sz, bool bFromGPH)
{
	PREFAST_SUPPRESS_WARNING(6240) // did not intend to use bitwise-and, its just a bool check
	if (bFromGPH && AllowGPHFallback)
	{
		FixedHdr* p = (FixedHdr*)DrxModuleMalloc(sz + sizeof(FixedHdr));
		if (p)
		{
			DrxInterlockedAdd(&m_nBytesInFixedAllocs, (i32)sz);

			p->size = sz;
			p->bFromGPH = true;
			return p + 1;
		}
	}

	FixedHdr* p = (FixedHdr*)DrxGetIMemoryUpr()->AllocPages(sz + sizeof(FixedHdr));
	if (p)
	{
		DrxInterlockedAdd(&m_nBytesInFixedAllocs, (i32)sz);

		p->size = sz;
		p->bFromGPH = false;
		return p + 1;
	}
	else
	{
		return NULL;
	}
}

void CControllerDefragHeap::FixedFree(uk p)
{
	FixedHdr* fh = ((FixedHdr*)p) - 1;
	DrxInterlockedAdd(&m_nBytesInFixedAllocs, -(i32)fh->size);

	PREFAST_SUPPRESS_WARNING(6239) // did not intend to use bitwise-and, its just a bool check
	if (AllowGPHFallback && fh->bFromGPH)
		DrxModuleFree(fh);
	else
		DrxGetIMemoryUpr()->FreePages(fh, (size_t)fh->size);
}

void CControllerDefragHeap::UpdateInflight(i32 frameId)
{
	if (m_numAvailableCopiesInFlight != MaxInFlightCopies)
	{
		u32 pQueuedCancels[MaxInFlightCopies];
		i32 nQueuedCancels = 0;

		{
			DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_queuedCancelLock);
			memcpy(pQueuedCancels, m_queuedCancels, m_numQueuedCancels * sizeof(pQueuedCancels[0]));
			nQueuedCancels = m_numQueuedCancels;
			m_numQueuedCancels = 0;
		}

		std::sort(pQueuedCancels, pQueuedCancels + nQueuedCancels);
		nQueuedCancels = (i32)std::distance(pQueuedCancels, std::unique(pQueuedCancels, pQueuedCancels + nQueuedCancels));

		for (i32 i = 0; i < MaxInFlightCopies; ++i)
		{
			Copy& cp = m_copiesInFlight[i];

			if (cp.inUse)
			{
				if (std::binary_search(pQueuedCancels, pQueuedCancels + nQueuedCancels, (u32)(i + 1)))
					cp.cancelled = true;

				bool bDone = false;

				if (cp.jobState >= 0)
				{
					if (cp.jobState == 0)
					{
						// Undo the boost for the copy
						DecreaseRange(cp.srcOffset, cp.size);
						cp.pNotification->bDstIsValid = true;
						cp.jobState = -1;
					}
				}
				else if (cp.relocateFrameId)
				{
					if ((frameId - cp.relocateFrameId) >= CompletionLatencyFrames)
					{
						cp.pNotification->bSrcIsUnneeded = true;
						cp.relocateFrameId = 0;
						bDone = true;
					}
				}
				else if (cp.cancelled)
				{
					cp.pNotification->bSrcIsUnneeded = true;
					bDone = true;
				}

				if (bDone)
				{
					if (cp.cancelled && !cp.relocated)
						DecreaseRange(cp.dstOffset, cp.size);
					else
						DecreaseRange(cp.srcOffset, cp.size);

					cp.inUse = false;
					++m_numAvailableCopiesInFlight;
					m_bytesInFlight -= cp.size;
				}
			}
		}
	}
}
