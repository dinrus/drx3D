// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/Shadow_Renderer.h>

#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/D3D_SVO.h>

#include <drx3D/Render/RenderView.h>

void ShadowMapFrustum::SortRenderItemsForFrustumAsync(i32 side, SRendItem* pFirst, size_t nNumRendItems)
{
	FUNCTION_PROFILER_RENDERER();

	if (nNumRendItems > 0)
	{
		// If any render items on this frustum side, update shadow gen side mask
		MarkShadowGenMaskForSide(side);
	}

	SRendItem::mfSortByLight(pFirst, nNumRendItems, true, false, false);
}

//////////////////////////////////////////////////////////////////////////
CRenderView* ShadowMapFrustum::GetNextAvailableShadowsView(CRenderView* pMainRenderView)
{
	CRenderView* pShadowsView = gcpRendD3D->GetOrCreateRenderView(CRenderView::eViewType_Shadow);
	DRX_ASSERT(pShadowsView->GetUsageMode() == IRenderView::eUsageModeUndefined || pShadowsView->GetUsageMode() == IRenderView::eUsageModeReadingDone);

	pShadowsView->SetParentView(pMainRenderView);
	pShadowsView->SetFrameId(pMainRenderView->GetFrameId());
	pShadowsView->SetSkinningDataPools(pMainRenderView->GetSkinningDataPools());

	DRX_ASSERT_MESSAGE(!((CRenderView*)pShadowsView)->CheckPermanentRenderObjects(), "There shouldn't be any permanent render-object be present in the view!");

	return pShadowsView;
}

IRenderView* CRenderer::GetNextAvailableShadowsView(IRenderView* pMainRenderView, ShadowMapFrustum* pOwnerFrustum)
{
	return pOwnerFrustum->GetNextAvailableShadowsView((CRenderView*)pMainRenderView);
}

//////////////////////////////////////////////////////////////////////////
std::bitset<6> ShadowMapFrustum::GenerateTimeSlicedUpdateCacheMask(u32 frameID) const
{
	const auto frameID8 = static_cast<u8>(frameID & 255);
	const auto shadowPoolUpdateRate = min<u8>(CRenderer::CV_r_ShadowPoolMaxFrames, nShadowPoolUpdateRate);

	// For shadow pools, respect CV_r_ShadowPoolMaxTimeslicedUpdatesPerFrame and nShadowPoolUpdateRate
	std::bitset<6> frustumSideCacheMask;
	if (bUseShadowsPool && shadowPoolUpdateRate != 0)
	{
		for (auto s = 0; s<GetNumSides(); ++s)
		{
			const auto frameDifference = static_cast<int8>(nSideDrawnOnFrame[s] - frameID8);
			if (abs(frameDifference) < (shadowPoolUpdateRate))
				frustumSideCacheMask.set(s);
		}
	}
	
	return frustumSideCacheMask;
}

void CRenderer::PrepareShadowFrustumForShadowPool(ShadowMapFrustum* pFrustum, u32 frameID, const SRenderLight& light, u32 *timeSlicedShadowsUpdated)
{
#if defined(ENABLE_PROFILING_CODE)
	u32& numShadowPoolAllocsThisFrame = m_frameRenderStats[m_nFillThreadID].m_NumShadowPoolAllocsThisFrame;
#else
	u32  numShadowPoolAllocsThisFrame = 0;
#endif

	pFrustum->PrepareForShadowPool(
		frameID,
		numShadowPoolAllocsThisFrame,
		CDeferredShading::Instance().m_blockPack, 
		CDeferredShading::Instance().m_shadowPoolAlloc, 
		light, 
		static_cast<u32>(CRenderer::CV_r_ShadowPoolMaxTimeslicedUpdatesPerFrame),
		timeSlicedShadowsUpdated);
}

//////////////////////////////////////////////////////////////////////////
void ShadowMapFrustum::PrepareForShadowPool(u32 frameID, u32& numShadowPoolAllocsThisFrame, CPowerOf2BlockPacker& blockPacker, TArray<SShadowAllocData>& shadowPoolAlloc, const SRenderLight& light, u32 timeSlicedShadowUpdatesLimit, u32 *timeSlicedShadowsUpdated)
{
	if (!bUseShadowsPool)
	{
		nSideCacheMask = 0;
		return;
	}

	// Mask of which sides can be cached
	auto cacheHintMask = GenerateTimeSlicedUpdateCacheMask(frameID);

	const auto lightID = light.m_nEntityId;
	i32 nBlockW = nTexSize >> TEX_POOL_BLOCKLOGSIZE;
	i32 nBlockH = nTexSize >> TEX_POOL_BLOCKLOGSIZE;
	i32 nLogBlockW = IntegerLog2((u32)nBlockW);
	i32 nLogBlockH = nLogBlockW;

	bool bNeedsUpdate = false;

	DRX_ASSERT(lightID != (u32)-1);

	// Reserve a shadowpool slot for each side
	for (uint nSide = 0; nSide<GetNumSides(); ++nSide)
	{
		// If side is not being cached, we should update to defragment the pool block packer.
		bNeedsUpdate = !cacheHintMask[nSide];

		SShadowAllocData* pAlloc = nullptr;
		for (SShadowAllocData& e : shadowPoolAlloc)
		{
			if (e.m_lightID == lightID && e.m_side == nSide)
			{
				pAlloc = &e;
				break;
			}
		}

		if (pAlloc)
		{
			TRect_tpl<u32> poolPack;
			const auto nID = blockPacker.GetBlockInfo(pAlloc->m_blockID, poolPack.Min.x, poolPack.Min.y, poolPack.Max.x, poolPack.Max.y);

			if (nID == 0xFFFFFFFF)
				bNeedsUpdate = true;

			if (!bNeedsUpdate)
			{
				if (poolPack.Min.x != 0xFFFFFFFF && nBlockW == poolPack.Max.x - poolPack.Min.x)   // ignore Y, is square
				{
					blockPacker.GetBlockInfo(nID, poolPack.Min.x, poolPack.Min.y, poolPack.Max.x, poolPack.Max.y);
					poolPack.Min.x <<= TEX_POOL_BLOCKLOGSIZE;
					poolPack.Min.y <<= TEX_POOL_BLOCKLOGSIZE;
					poolPack.Max.x <<= TEX_POOL_BLOCKLOGSIZE;
					poolPack.Max.y <<= TEX_POOL_BLOCKLOGSIZE;
					if (shadowPoolPack[nSide] != poolPack)
						InvalidateSide(nSide);
					shadowPoolPack[nSide] = poolPack;

					continue; // All currently valid, skip
				}
			}

			if (nID != 0xFFFFFFFF && poolPack.Min.x != 0xFFFFFFFF) // Valid block, realloc
			{
				blockPacker.RemoveBlock(nID);
				pAlloc->Clear();
			}
		}

		// Try to allocate new block, always invalidate
		InvalidateSide(nSide);

		const auto nID = blockPacker.AddBlock(nLogBlockW, nLogBlockH);
#if defined(ENABLE_PROFILING_CODE)
		++numShadowPoolAllocsThisFrame;
#endif
		if (nID != 0xFFFFFFFF)
		{
			if (!pAlloc)
			{
				// Attempt to find a free block
				for (SShadowAllocData& e : shadowPoolAlloc)
				{
					if (e.isFree())
					{
						pAlloc = &e;
						break;
					}
				}
				// Otherwise create a new one
				if (!pAlloc)
					pAlloc = shadowPoolAlloc.AddIndex(1);
			}

			pAlloc->m_blockID = nID;
			pAlloc->m_lightID = lightID;
			pAlloc->m_side = nSide;

			TRect_tpl<u32> poolPack;
			blockPacker.GetBlockInfo(nID, poolPack.Min.x, poolPack.Min.y, poolPack.Max.x, poolPack.Max.y);
			poolPack.Min.x <<= TEX_POOL_BLOCKLOGSIZE;
			poolPack.Min.y <<= TEX_POOL_BLOCKLOGSIZE;
			poolPack.Max.x <<= TEX_POOL_BLOCKLOGSIZE;
			poolPack.Max.y <<= TEX_POOL_BLOCKLOGSIZE;
			shadowPoolPack[nSide] = poolPack;
		}
		else
		{

#if defined(ENABLE_PROFILING_CODE)
			if (CRenderer::CV_r_ShadowPoolMaxFrames != 0 || CRenderer::CV_r_DeferredShadingTiled > 1)
				numShadowPoolAllocsThisFrame |= 0x80000000;
#endif
		}

		// Limit time-sliced updates
		if (timeSlicedShadowsUpdated && this->nShadowPoolUpdateRate > 0)
		{
			if (this->nSideInvalidatedMask[nSide] || !cacheHintMask[nSide])
				continue;

			if (*timeSlicedShadowsUpdated >= timeSlicedShadowUpdatesLimit)
				cacheHintMask[nSide] = true;
			else
				++*timeSlicedShadowsUpdated;
		}
	}

	// Cache non-invalidated sides
	this->nSideCacheMask = cacheHintMask & ~this->nSideInvalidatedMask;
}

//////////////////////////////////////////////////////////////////////////
void SShadowRenderer::FinishRenderFrustumsToView(CRenderView* pRenderView)
	{
	FUNCTION_PROFILER_RENDERER();

	auto& frustumsToRender = pRenderView->GetFrustumsToRender();

	for (SShadowFrustumToRender& rFrustumToRender : frustumsToRender)
	{
		auto* pCurFrustum = rFrustumToRender.pFrustum.get();
		if (pCurFrustum->pOnePassShadowView)
			continue; // already processed in 3dengine

		for (i32 side = 0; side < pCurFrustum->GetNumSides(); ++side)
		{
			if (pCurFrustum->ShouldCacheSideHint(side))
			{
				pCurFrustum->MarkShadowGenMaskForSide(side);
			}
		}
	}

	// Switch all shadow views WritingDone
	for (SShadowFrustumToRender& rFrustumToRender : frustumsToRender)
	{
		auto* pCurFrustum = rFrustumToRender.pFrustum.get();
		if (pCurFrustum->pOnePassShadowView)
			continue; // already processed in 3dengine

		rFrustumToRender.pShadowsView->SwitchUsageMode(IRenderView::eUsageModeWritingDone);
	}

	pRenderView->PostWriteShadowViews();
}
