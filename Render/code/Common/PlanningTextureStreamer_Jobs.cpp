// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/PlanningTextureStreamer.h>

#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

DECLARE_JOB("TexStrmUpdate", TTexStrmUpdateJob, CPlanningTextureStreamer::Job_UpdateEntry)

//#define PLAN_TEXSTRM_DEBUG
#if defined(PLAN_TEXSTRM_DEBUG)
	#pragma optimize("",off)
#endif

static i32 GetTexReqStreamSizePreClamped(const SPlanningTextureOrderKey& key, i32 reqMip)
{
	i32 nTotalSize = 0;

	u8 nFormatCode = key.nFormatCode;
	if (nFormatCode)
	{
		const SStreamFormatCode& code = CTexture::s_formatCodes[nFormatCode];

		u32 nCodeMip = reqMip + (SStreamFormatCode::MaxMips - key.nMips);
		u32 nSlices = key.nSlicesMinus1 + 1;
		u32 nPoTSlices = key.nSlicesPotMinus1 + 1;

		u32 n1SliceSize = code.sizes[nCodeMip].size;
		u32 nFormatSlices = code.sizes[nCodeMip].alignSlices ? nPoTSlices : nSlices;
		nTotalSize = n1SliceSize * nFormatSlices;

#if defined(PLAN_TEXSTRM_DEBUG)
		i32 nTotalSizeTest = key.pTexture->StreamComputeSysDataSize(reqMip);

		if (nTotalSizeTest != nTotalSize)
			__debugbreak();
#endif
	}
	else
	{
		nTotalSize = key.pTexture->StreamComputeSysDataSize(reqMip);
	}

	return nTotalSize - key.nPersistentSize;
}

struct CTextureStreamSize
{
	CTextureStreamSize(i32 nMinMip, i32 nBias)
		: m_nMinMip(nMinMip)
		, m_nBias(nBias)
	{
	}

	ptrdiff_t operator()(const SPlanningTextureOrderKey& key) const
	{
		i32 numMips = key.nMips;
		i32 numPersMips = key.nMipsPersistent;
		i32 persMip = numMips - numPersMips;
		i32 reqMip = min(max(static_cast<i32>(key.GetFpMinMipCur() + m_nBias) >> 8, m_nMinMip), persMip);
		return GetTexReqStreamSizePreClamped(key, reqMip);
	}

	i32 m_nMinMip;
	i32 m_nBias;
};

struct CTexturePrecachedPred
{
	bool operator()(const SPlanningTextureOrderKey& key) const
	{
		return key.IsPrecached();
	}
};

struct CTextureOnScreenPred
{
	bool operator()(const SPlanningTextureOrderKey& key) const
	{
		return key.IsVisible();
	}
};

struct SPlanningTextureOrder
{
	explicit SPlanningTextureOrder(u32 nKeyMask)
	{
		m_nKeyMask = nKeyMask;
	}

	bool operator()(const SPlanningTextureOrderKey& a, const SPlanningTextureOrderKey& b) const
	{
		u32 nMask = m_nKeyMask;
		u32 nAKeyMasked = a.nKey & nMask;
		u32 nBKeyMasked = b.nKey & nMask;
		if (nAKeyMasked != nBKeyMasked)
			return nAKeyMasked < nBKeyMasked;

		return a.pTexture < b.pTexture;
	}

	u32 m_nKeyMask;
};

template<typename T, typename Pred>
static void QuickSelectMedianOf3(T* table, size_t a, size_t b, size_t c, const Pred& p)
{
	using std::swap;

	if (p(table[b], table[a]))
		swap(table[b], table[a]);
	if (p(table[c], table[b]))
		swap(table[c], table[b]);
	if (p(table[b], table[a]))
		swap(table[b], table[a]);
}

template<typename T, typename Pred>
static void QuickSelectMedian(T* table, size_t a, size_t m, size_t b, const Pred& p)
{
	if ((b - a) >= 40)
	{
		size_t step = (b - a + 1) / 8;
		QuickSelectMedianOf3(table, a, a + step, a + step * 2, p);
		QuickSelectMedianOf3(table, m - step, m, m + step, p);
		QuickSelectMedianOf3(table, b - step * 2, b - step, b, p);
		QuickSelectMedianOf3(table, a + step, m, b - step, p);
	}
	else
	{
		QuickSelectMedianOf3(table, a, m, b, p);
	}
}

template<typename T, typename LessFunc, typename SizeFunc>
static T* QuickSelectSum(T* p, size_t mn, size_t mx, ptrdiff_t targetSum, const LessFunc& less, const SizeFunc& size)
{
	using std::swap;

	size_t rangeMin = mn, rangeMax = mx;

	do
	{
		size_t pivotIdx = rangeMin + (rangeMax - rangeMin) / 2;
		QuickSelectMedian(p, rangeMin, pivotIdx, rangeMax - 1, less);

		ptrdiff_t leftSum = 0;

		// Move pivot out of the way, so it can be placed at the right location at the end
		T pivotVal = p[pivotIdx];
		swap(p[rangeMax - 1], p[pivotIdx]);

		// - 1 to account for the moved pivot
		size_t left = rangeMin, right = rangeMax - 1;
		while (left < right)
		{
			// Advance left edge past all vals <= pivot
			while (left < right && !less(pivotVal, p[left]))
			{
				leftSum += size(p[left]);
				++left;
			}

			// Advance right edge past all vals >= pivot
			while (right > left && !less(p[right - 1], pivotVal))
			{
				--right;
			}

			if (right > left)
			{
				// Swap the conflicting values
				swap(p[left], p[right - 1]);
			}
		}

		// Put the pivot back
		swap(p[rangeMax - 1], p[left]);

		if (leftSum >= targetSum)
			rangeMax = left;
		else if (leftSum < targetSum)
		{
			rangeMin = right;
			targetSum -= leftSum;
		}

		if (rangeMax - rangeMin <= 1)
		{
			// Handle the case where the leftSum is under budget (such as the pivot being at the far right edge of the range)
			if (leftSum < targetSum)
			{
				ptrdiff_t pivotSize = size(p[left]);
				if (leftSum + pivotSize <= targetSum)
					++left;
			}
			return &p[left];
		}
	}
	while (1);
}

void CPlanningTextureStreamer::Job_UpdateEntry()
{
	FUNCTION_PROFILER_RENDERER();

	m_state = S_Updating;

#ifdef TEXSTRM_DEFER_UMR
	u32 nList = m_nJobList;
	UpdateMipRequestVec& umrv = m_updateMipRequests[nList];

	for (UpdateMipRequestVec::iterator it = umrv.begin(), itEnd = umrv.end(); it != itEnd; ++it)
		Job_UpdateMip(it->pTexture, it->fMipFactor, it->nFlags, it->nUpdateId);
#endif

	Job_Sort();
	Job_ConfigureSchedule();

	m_state = S_QueuedForSync;
}

void CPlanningTextureStreamer::StartUpdateJob()
{
	TTexStrmUpdateJob job;
	job.RegisterJobState(&m_JobState);
	job.SetClassInstance(this);
	job.SetPriorityLevel(JobUpr::eRegularPriority);
	job.Run();
}

void CPlanningTextureStreamer::Job_UpdateMip(CTexture* pTexture, const float fMipFactor, i32k nFlags, i32k nUpdateId)
{
	assert(fMipFactor >= 0.f);

	i32k nZoneId = (nFlags & FPR_SINGLE_FRAME_PRIORITY_UPDATE) ? 0 : 1;

	STexStreamingInfo* const pStrmInfo = pTexture->m_pFileTexMips;

	if (pStrmInfo && (fMipFactor <= pStrmInfo->m_fMinMipFactor))
	{
		STexStreamZoneInfo& rZoneInfo = pStrmInfo->m_arrSPInfo[nZoneId];
		STexStreamRoundInfo& rRoundInfo = pTexture->m_streamRounds[nZoneId];

		i32k nCurrentRoundUpdateId = m_umrState.arrRoundIds[nZoneId];

		if (rRoundInfo.nRoundUpdateId != nUpdateId)
		{
			STATIC_CHECK(MAX_STREAM_PREDICTION_ZONES == 2, THIS_CODE_IS_OPTIMISED_ASSUMING_THIS_EQUALS_2);

			// reset mip factor and save the accumulated value into history and compute final mip factor
			float fFinalMipFactor = fMipFactor;
			i32 nRoundUpdateId = (i32)rRoundInfo.nRoundUpdateId;
			if (nRoundUpdateId >= 0 && (nRoundUpdateId > nCurrentRoundUpdateId - 2))
				fFinalMipFactor = min(fFinalMipFactor, rZoneInfo.fMinMipFactor);

			// if the min mip factor is at its default value, initialise the entire history with the new mip factor
			rZoneInfo.fLastMinMipFactor = (float)__fsel(rZoneInfo.fMinMipFactor - 1000000.0f, fMipFactor, rZoneInfo.fMinMipFactor);
			rZoneInfo.fMinMipFactor = fMipFactor;

			// reset the high prio flags
			rRoundInfo.bLastHighPriority = rRoundInfo.bHighPriority;
			rRoundInfo.bHighPriority = false;

			// update round id
			rRoundInfo.nRoundUpdateId = max(nUpdateId, nCurrentRoundUpdateId);

			// consider the alternate zone mip factor as well
			i32k nOtherZoneId = nZoneId ^ 1;
			const STexStreamZoneInfo& rOtherZoneInfo = pStrmInfo->m_arrSPInfo[nOtherZoneId];
			const STexStreamRoundInfo& rOtherRoundInfo = pTexture->m_streamRounds[nOtherZoneId];
			i32 nOtherRoundUpdateId = (i32)rOtherRoundInfo.nRoundUpdateId;
			if (nOtherRoundUpdateId >= 0 && (nOtherRoundUpdateId > m_umrState.arrRoundIds[nOtherZoneId] - 2))
				fFinalMipFactor = min(fFinalMipFactor, rOtherZoneInfo.fLastMinMipFactor);

			Job_CheckEnqueueForStreaming(pTexture, fFinalMipFactor, rRoundInfo.bLastHighPriority);
		}

		rZoneInfo.fMinMipFactor = min(rZoneInfo.fMinMipFactor, fMipFactor);
		rRoundInfo.bHighPriority = (rRoundInfo.bHighPriority || nFlags & FPR_HIGHPRIORITY);

#if !defined (_RELEASE)
		CTexture::s_TextureUpdates += 1;
#endif
	}
}

i32 CPlanningTextureStreamer::Job_Bias(SPlanningSortState& sortState, SPlanningTextureOrderKey* pKeys, size_t nNumPrecachedTexs, size_t nStreamLimit)
{
	FUNCTION_PROFILER_RENDERER();

	i32 fpSortStateBias = sortState.nBias;
	i32k fpSortStateMinBias = sortState.fpMinBias;
	i32k fpSortStateMaxBias = sortState.fpMaxBias;
	i32k fpSortStateMinMip = sortState.fpMinMip;

	fpSortStateBias = clamp_tpl(fpSortStateBias, fpSortStateMinBias, fpSortStateMaxBias);

	i32 fpMipBiasLow = fpSortStateMinBias;
	i32 fpMipBiasHigh = max(fpSortStateMinBias, fpSortStateMaxBias);

	i32k nMaxBiasSteps = 8;

	for (i32 nBiasStep = 0; nBiasStep < nMaxBiasSteps && (fpMipBiasHigh - fpMipBiasLow) > 1; ++nBiasStep)
	{
		i32 nMipBiasTest = (fpMipBiasLow + fpMipBiasHigh) / 2;

		size_t nBiasedListStreamSize = 0;
		for (size_t texIdx = 0, texCount = nNumPrecachedTexs; texIdx != texCount && nBiasedListStreamSize < nStreamLimit; ++texIdx)
		{
			const SPlanningTextureOrderKey& key = pKeys[texIdx];

			i32 nMips = key.nMips;
			i32 fpReqMip = max(fpSortStateMinMip, key.GetFpMinMipCur() + nMipBiasTest);
			i32 nReqMip = min(fpReqMip >> 8, nMips - 1);

			nBiasedListStreamSize += GetTexReqStreamSizePreClamped(key, nReqMip);
		}

		if (nBiasedListStreamSize < nStreamLimit)
			fpMipBiasHigh = nMipBiasTest;
		else
			fpMipBiasLow = nMipBiasTest;
	}

	i32 nProspBias = (fpMipBiasLow + fpMipBiasHigh) / 2;
	if (abs(fpSortStateBias - nProspBias) > 8)
		fpSortStateBias = nProspBias;

	return fpSortStateBias;
}

size_t CPlanningTextureStreamer::Job_Plan(SPlanningSortState& sortState, const SPlanningTextureOrderKey* pKeys, size_t nTextures, size_t nNumPrecachedTexs, size_t nBalancePoint, i32 nMinMip, i32 fpSortStateBias)
{
	FUNCTION_PROFILER_RENDERER();

	static_assert(MAX_PREDICTION_ZONES == 2, "Invalid maximum prediction zone value!");

	size_t nListSize = 0;

	i32k minTransferSize = CRenderer::CV_r_texturesstreamingMinReadSizeKB * 1024;
	enum { MaxRequests = 16384 };
	SPlanningRequestIdent requests[MaxRequests];
	size_t nRequests = 0;

	for (size_t texIdx = 0, texCount = nBalancePoint; texIdx != texCount; ++texIdx)
	{
		const SPlanningTextureOrderKey& key = pKeys[texIdx];

		i32 nMips = key.nMips;
		i32 haveMip = key.nCurMip;
		i32 persMip = nMips - key.nMipsPersistent;
		i32 wantKey;

		if (texIdx < nNumPrecachedTexs)
		{
			i16 fpMinMipCur = key.GetFpMinMipCur();

			wantKey = max(fpMinMipCur + fpSortStateBias, nMinMip << 8);
			wantKey = min(wantKey, persMip << 8);
		}
		else
		{
			wantKey = persMip << 8;
		}

		i32 deltaMip = (wantKey >> 8) - haveMip;
		i32 deltaKey = wantKey - (haveMip << 8);
		i32 wantMip  = haveMip + max(deltaMip, -CRenderer::CV_r_TexturesStreamingMaxUpdateRate);
		i32 reqstMip = haveMip + deltaMip;
		i32 prevMipSize = GetTexReqStreamSizePreClamped(key, haveMip);
		i32 cacheMipSize = GetTexReqStreamSizePreClamped(key, wantMip);
		i32 transferSize = cacheMipSize - prevMipSize;

		while ((transferSize < minTransferSize) && (reqstMip < wantMip))
		{
			cacheMipSize = GetTexReqStreamSizePreClamped(key, wantMip - 1);
			transferSize = cacheMipSize - prevMipSize;

			wantMip -= (transferSize < minTransferSize);
		}

		// more priority for streaming requests which are very far away from their target
		i32 sortKey = key.GetFpMinMipCur() - (haveMip << 8) + SPlanningTextureOrderKey::PackedFpBias;

		nListSize += cacheMipSize;

		if (!key.bIsStreaming)
		{
			if (wantMip > haveMip)
			{
				sortState.pTrimmableList->push_back(key.pTexture);
			}
			else if (haveMip > wantMip)
			{
				if (nRequests < MaxRequests)
				{
					bool bOnlyNeedsTopMip = wantMip == 0 && haveMip == 1;

					u32 nSortKey =
					  (1 << 31)
					  | ((i32)(haveMip < (max(0, key.GetFpMinMipCur()) >> 8)) << 30)
					  | ((i32)!key.IsHighPriority() << 29)
					  | ((i32)bOnlyNeedsTopMip << 28)
					  | ((i32)!key.IsVisible() << 27)
					  | ((i32)(7 - key.nStreamPrio) << 19)
					  | ((i32)!key.IsInZone(0) << 18)
					  | ((i32)!key.IsInZone(1) << 17)
					  | ((sortKey) << 0)
					;

					requests[nRequests++] = SPlanningRequestIdent(nSortKey, (i32)texIdx, wantMip);
				}
			}
		}
		else
		{
			u32 nStreamSlot = key.pTexture->m_nStreamSlot;
			if (!(nStreamSlot & (CTexture::StreamOutMask | CTexture::StreamPrepMask)))
			{
				STexStreamInState* pStreamInState = CTexture::s_StreamInTasks.GetPtrFromIdx(nStreamSlot & CTexture::StreamIdxMask);
				if (wantMip > pStreamInState->m_nLowerUploadedMip)
				{
					sortState.pActionList->push_back(SPlanningAction(SPlanningAction::Abort, texIdx));
				}
			}
		}
	}

	for (size_t texIdx = nBalancePoint, texCount = nTextures; texIdx != texCount; ++texIdx)
	{
		assert(nBalancePoint < nTextures);
		PREFAST_ASSUME(nBalancePoint < nTextures);
		const SPlanningTextureOrderKey& key = pKeys[texIdx];

		i32 nMips = key.nMips;
		i32 haveMip = key.nCurMip;
		i32 persMip = nMips - key.nMipsPersistent;
		i32 wantKey;

		if (texIdx < nNumPrecachedTexs)
		{
			i16 fpMinMipCur = key.GetFpMinMipCur();

			wantKey = max(fpMinMipCur + fpSortStateBias, nMinMip << 8);
			wantKey = min(wantKey, persMip << 8);
		}
		else
		{
			wantKey = persMip << 8;
		}

		i32 deltaMip = (wantKey >> 8) - haveMip;
		i32 deltaKey = wantKey - (haveMip << 8);
		i32 wantMip = haveMip + max(deltaMip, -CRenderer::CV_r_TexturesStreamingMaxUpdateRate);
		i32 reqstMip = haveMip + deltaMip;
		i32 prevMipSize = GetTexReqStreamSizePreClamped(key, haveMip);
		i32 cacheMipSize = GetTexReqStreamSizePreClamped(key, wantMip);
		i32 transferSize = cacheMipSize - prevMipSize;

		while ((transferSize < minTransferSize) && (reqstMip < wantMip))
		{
			cacheMipSize = GetTexReqStreamSizePreClamped(key, wantMip - 1);
			transferSize = cacheMipSize - prevMipSize;

			wantMip -= (transferSize < minTransferSize);
		}

		// more priority for streaming requests which are very far away from their target
		i32 sortKey = key.GetFpMinMipCur() - (haveMip << 8) + SPlanningTextureOrderKey::PackedFpBias;

		nListSize += cacheMipSize;

		if (!key.bIsStreaming && !key.bUnloaded)
		{
			if (wantMip > haveMip)
			{
				sortState.pTrimmableList->push_back(key.pTexture);
			}
			else if (haveMip > persMip)
			{
				if (nRequests < MaxRequests)  // Persistent mips should always be present - needed in case stream unload occurred
				{
					u32 nSortKey =
					  (1 << 31)
					  | ((i32)(haveMip < (max(0, key.GetFpMinMipCur()) >> 8)) << 30)
					  | ((i32)!key.IsHighPriority() << 29)
					  | ((i32)!key.IsVisible() << 27)
					  | ((i32)(7 - key.nStreamPrio) << 19)
					  | ((i32)!key.IsInZone(0) << 18)
					  | ((i32)!key.IsInZone(1) << 17)
					  | ((sortKey) << 0)
					;

					requests[nRequests++] = SPlanningRequestIdent(nSortKey, (i32)texIdx, persMip);
				}
			}
			else if (!key.IsPrecached() && haveMip == persMip)
			{
				sortState.pUnlinkList->push_back(key.pTexture);
			}
		}
	}

	if (nRequests > 0)
	{
		// TODO - only sort the part of the list that can actually be submitted this update
		SPlanningTextureRequestOrder sort_op;
		std::sort(&requests[0], &requests[nRequests], sort_op);

		for (size_t iRequest = 0; iRequest < nRequests; ++iRequest)
		{
			i32 nRequestMip = requests[iRequest].nMip;
			sortState.pRequestList->push_back(std::make_pair(pKeys[requests[iRequest].nKey].pTexture, nRequestMip));
		}
	}

	return nListSize;
}

void CPlanningTextureStreamer::Job_Sort()
{
	FUNCTION_PROFILER_RENDERER();

	SPlanningSortState& sortState = m_sortState;

	CTexture** pTextures = &(*sortState.pTextures)[0];
	size_t nTextures = sortState.nTextures;

	i32 nOnScreenFrameId = sortState.nFrameId - 8;
	i32 nMinMip = sortState.fpMinMip >> 8;

	i32 const nStreamLimit = sortState.nStreamLimit;

	m_keys.resize(nTextures);
	SPlanningTextureOrderKey* pKeys = &m_keys[0];

	i32 const nFrameId = sortState.nFrameId;
	i32 const nZoneIds[] = { sortState.arrRoundIds[0], sortState.arrRoundIds[1] };
	Job_InitKeys(pKeys, pTextures, nTextures, nFrameId - 8, nZoneIds);

	SPlanningTextureOrderKey* pLastPrecachedKey = std::partition(pKeys, pKeys + nTextures, CTexturePrecachedPred());
	size_t nNumPrecachedTexs = std::distance(pKeys, pLastPrecachedKey);

	i32 fpSortStateBias = Job_Bias(sortState, pKeys, nNumPrecachedTexs, nStreamLimit);

	// Don't allow greedy grab of the texture pool memory (negative bias over-commits!)
	if (fpSortStateBias < CRenderer::CV_r_TexturesStreamingLowestPrefetchBias)
		fpSortStateBias = CRenderer::CV_r_TexturesStreamingLowestPrefetchBias;

	SPlanningTextureOrderKey* pBalanceKey = pKeys + nNumPrecachedTexs;
	if (fpSortStateBias >= 0 && nNumPrecachedTexs > 0)
	{
		SPlanningTextureOrder order(SPlanningTextureOrderKey::OverBudgetMask);
		pBalanceKey = QuickSelectSum(pKeys, 0, nNumPrecachedTexs, nStreamLimit, order, CTextureStreamSize(nMinMip, fpSortStateBias));
	}

	size_t nBalancePoint = std::distance(pKeys, pBalanceKey);

	SPlanningTextureOrderKey* pFirstOffScreenKey = std::partition(pKeys, pKeys + nBalancePoint, CTextureOnScreenPred());
	size_t nOnScreenPoint = std::distance(pKeys, pFirstOffScreenKey);

	size_t nListSize = Job_Plan(sortState, pKeys, nTextures, nNumPrecachedTexs, nBalancePoint, nMinMip, fpSortStateBias);

	Job_CommitKeys(pTextures, pKeys, nTextures);

	sortState.nListSize = nListSize;
	sortState.nBalancePoint = nBalancePoint;
	sortState.nOnScreenPoint = nOnScreenPoint;
	sortState.nPrecachedTexs = nNumPrecachedTexs;
	sortState.nBias = fpSortStateBias;
}

void CPlanningTextureStreamer::Job_InitKeys(SPlanningTextureOrderKey* pKeys, CTexture** pTexs, size_t nTextures, i32 nFrameId, i32k nZoneIds[])
{
	FUNCTION_PROFILER_RENDERER();

	for (size_t i = 0; i < nTextures; ++i)
	{
#if DRX_PLATFORM_DURANGO || (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) || DRX_PLATFORM_ORBIS
		if ((i + 32) < nTextures)
			_mm_prefetch((tuk)(pTexs[i + 32]) + 0x40, 0);
#endif
		new(&pKeys[i])SPlanningTextureOrderKey(pTexs[i], nFrameId, nZoneIds);
	}
}

void CPlanningTextureStreamer::Job_CommitKeys(CTexture** pTextures, SPlanningTextureOrderKey* pKeys, size_t nTextures)
{
	for (size_t i = 0; i < nTextures; ++i)
		pTextures[i] = pKeys[i].pTexture;
}

void CPlanningTextureStreamer::Job_CheckEnqueueForStreaming(CTexture* pTexture, const float fMipFactor, bool bHighPriority)
{
	STexStreamingInfo* pStrmInfo = pTexture->m_pFileTexMips;

	// calculate the new lod value
	i32 fpMipIdSigned = pTexture->StreamCalculateMipsSignedFP(fMipFactor);
	i32k fpNewMip = max(0, fpMipIdSigned);
	i32k nNewMip = fpNewMip >> 8;
	i32k nMipIdSigned = fpMipIdSigned >> 8;

	if ((CRenderer::CV_r_TexturesStreamingDebug == 2) && (pTexture->GetRequiredMip() != nNewMip))
		iLog->Log("Updating mips: %s - Current: %i, Previous: %i", pTexture->m_SrcName.c_str(), pTexture->GetRequiredMip(), nNewMip);

#if defined(ENABLE_TEXTURE_STREAM_LISTENER)
	if (pTexture->GetRequiredMip() != nNewMip)
	{
		ITextureStreamListener* pListener = CTexture::s_pStreamListener;
		if (pListener)
			pListener->OnTextureWantsMip(pTexture, min(nNewMip, (i32)pTexture->m_nMips));
	}
#endif

	// update required mip for streaming
	pTexture->m_fpMinMipCur = fpMipIdSigned;

	// update high priority flag
	pTexture->m_bStreamHighPriority |= bHighPriority ? 1 : 0;

#if defined(TEXSTRM_DEFER_UMR)
	__debugbreak();
#else
	pTexture->Relink();
#endif
}

void CPlanningTextureStreamer::Job_ConfigureSchedule()
{
	SPlanningScheduleState& schedule = m_schedule;
	SPlanningSortState& sortState = m_sortState;

	schedule.nFrameId = sortState.nFrameId;
	schedule.nBias = sortState.nBias;
	schedule.memState = sortState.memState;
	schedule.nBalancePoint = sortState.nBalancePoint;
	schedule.nOnScreenPoint = sortState.nOnScreenPoint;
}
