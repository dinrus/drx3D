// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

#include <drx3D/Render/Textures/TextureStreamPool.h>

//===============================================================================
#ifdef TEXSTRM_ASYNC_TEXCOPY
DECLARE_JOB("Texture_StreamOutCopy", TTexture_StreamOutCopy, STexStreamOutState::CopyMips);
#endif

// checks for MT-safety of called functions
#if !defined(CHK_RENDTH)
	#define CHK_RENDTH assert(gRenDev->m_pRT->IsRenderThread(true))
#endif
#if !defined(CHK_MAINTH)
	#define CHK_MAINTH assert(gRenDev->m_pRT->IsMainThread(true))
#endif
#if !defined(CHK_MAINORRENDTH)
	#define CHK_MAINORRENDTH assert(gRenDev->m_pRT->IsMainThread(true) || gRenDev->m_pRT->IsRenderThread(true) || gRenDev->m_pRT->IsLevelLoadingThread(true))
#endif

void CTexture::InitStreamingDev()
{
#if defined(TEXSTRM_DEFERRED_UPLOAD)
	if (CRenderer::CV_r_texturesstreamingDeferred)
	{
		if (gcpRendD3D->GetDevice().IsValid() && !s_pStreamDeferredCtx)
		{
			gcpRendD3D->GetDevice().CreateDeferredContext(0, &s_pStreamDeferredCtx);
		}
	}
#endif
}

bool CTexture::IsStillUsedByGPU()
{
	// TODO: add tracked resource check
	CDeviceTexture* pDeviceTexture = m_pDevTexture;
	if (pDeviceTexture)
	{
		CHK_RENDTH;
		D3DBaseTexture* pD3DTex = pDeviceTexture->GetBaseTexture();
	}
	return false;
}

#if !(DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120))
bool CTexture::StreamPrepare_Platform()
{
	return true;
}
#endif

void CTexture::StreamExpandMip(ukk vpRawData, i32 nMip, i32 nBaseMipOffset, i32 nSideDelta)
{
	FUNCTION_PROFILER_RENDERER();

	u32k nCurMipWidth  = (m_nWidth >> (nMip + nBaseMipOffset));
	u32k nCurMipHeight = (m_nHeight >> (nMip + nBaseMipOffset));

	const STexMipHeader& mh = m_pFileTexMips->m_pMipHeader[nBaseMipOffset + nMip];

	const byte* pRawData = (const byte*)vpRawData;

	i32k nSides = StreamGetNumSlices();
	const bool bIsDXT = CTexture::IsBlockCompressed(m_eSrcFormat);
	i32k nMipAlign = bIsDXT ? 4 : 1;

	i32k nSrcSurfaceSize = CTexture::TextureDataSize(nCurMipWidth, nCurMipHeight, 1, 1, 1, m_eSrcFormat, m_eSrcTileMode);
	i32k nSrcSidePitch = nSrcSurfaceSize + nSideDelta;

	SRenderThread* pRT = gRenDev->m_pRT;

	for (i32 iSide = 0; iSide < nSides; ++iSide)
	{
		SMipData* mp = &mh.m_Mips[iSide];
		if (!mp->DataArray)
			mp->Init(mh.m_SideSize, Align(max(1, m_nWidth >> nMip), nMipAlign), Align(max(1, m_nHeight >> nMip), nMipAlign));

		const byte* pRawSideData = pRawData + nSrcSidePitch * iSide;
		CTexture::ExpandMipFromFile(&mp->DataArray[0], mh.m_SideSize, pRawSideData, nSrcSurfaceSize, m_eSrcFormat, m_eDstFormat);
	}
}

#ifdef TEXSTRM_ASYNC_TEXCOPY
void STexStreamOutState::CopyMips()
{
	CTexture* tp = m_pTexture;

	if (m_nStartMip < MAX_MIP_LEVELS)
	{
		i32k nOldMipOffset = m_nStartMip - tp->m_nMinMipVidUploaded;
		i32k nNumMips = tp->GetNumMips() - m_nStartMip;
	#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
		m_pNewPoolItem->m_pDevTexture->InitD3DTexture();
		m_copyFence = CTexture::StreamCopyMipsTexToTex_MoveEngine(tp->m_pFileTexMips->m_pPoolItem, 0 + nOldMipOffset, m_pNewPoolItem, 0, nNumMips);
	#else
		CTexture::StreamCopyMipsTexToTex(tp->m_pFileTexMips->m_pPoolItem, 0 + nOldMipOffset, m_pNewPoolItem, 0, nNumMips);
	#endif
	}
	else
	{
		// Stream unload case - pull persistent mips into local memory
		tp->StreamCopyMipsTexToMem(tp->m_nMips - tp->m_CacheFileHeader.m_nMipsPersistent, tp->m_nMips - 1, false, NULL);
	}

	m_bDone = true;
}
#endif

i32 CTexture::StreamTrim(i32 nToMip)
{
	FUNCTION_PROFILER_RENDERER();
	CHK_RENDTH;

	if (IsUnloaded() || !IsStreamed() || IsStreaming())
		return 0;

	// clamp mip level
	nToMip = max(0, min(nToMip, m_nMips - m_CacheFileHeader.m_nMipsPersistent));

	if (m_nMinMipVidUploaded >= nToMip)
		return 0;

	i32 nFreeSize = StreamComputeSysDataSize(m_nMinMipVidUploaded) - StreamComputeSysDataSize(nToMip);

#ifndef _RELEASE
	if (CRenderer::CV_r_TexturesStreamingDebug == 2)
		iLog->Log("Shrinking texture: %s - From mip: %i, To mip: %i", m_SrcName.c_str(), m_nMinMipVidUploaded, GetRequiredMip());
#endif

	STexPoolItem* pNewPoolItem = StreamGetPoolItem(nToMip, m_nMips - nToMip, false, false, true, true);
	assert(pNewPoolItem != m_pFileTexMips->m_pPoolItem);
	if (pNewPoolItem)
	{
		i32k nOldMipOffset = nToMip - m_nMinMipVidUploaded;
		i32k nNumMips = GetNumMips() - nToMip;

#ifdef TEXSTRM_ASYNC_TEXCOPY

		bool bCopying = false;

		if (CanAsyncCopy() && (TryAddRef() > 0))
		{
			STexStreamOutState* pStreamState = StreamState_AllocateOut();
			if (pStreamState)
			{
				pStreamState->m_nStartMip = nToMip;
				pStreamState->m_pNewPoolItem = pNewPoolItem;
				pStreamState->m_pTexture = this;

				SetStreamingInProgress(StreamOutMask | (u8)s_StreamOutTasks.GetIdxFromPtr(pStreamState));

				TTexture_StreamOutCopy job;
				job.SetClassInstance(pStreamState);
				job.RegisterJobState(&pStreamState->m_jobState);
				job.Run();

				bCopying = true;

	#ifdef DO_RENDERLOG
				if (gRenDev->m_LogFileStr)
					gRenDev->LogStrv(0, "Async Start SetLod '%s', Lods: [%d-%d], Time: %.3f\n", m_SrcName.c_str(), nToMip, m_nMips - 1, iTimer->GetAsyncCurTime());
	#endif
			}
			else
			{
				Release();
			}
		}

		if (!bCopying)
#endif
		{
#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
			pNewPoolItem->m_pDevTexture->InitD3DTexture();
#endif
			// it is a sync operation anyway, so we do it in the render thread
			CTexture::StreamCopyMipsTexToTex(m_pFileTexMips->m_pPoolItem, 0 + nOldMipOffset, pNewPoolItem, 0, nNumMips);
			StreamAssignPoolItem(pNewPoolItem, nToMip);
		}
	}
	else
	{
		s_pTextureStreamer->FlagOutOfMemory();
	}

	return nFreeSize;
}

i32 CTexture::StreamUnload()
{
	CHK_RENDTH;

	if (IsUnloaded() || !IsStreamed() || !CRenderer::CV_r_texturesstreaming)
		return 0;

	AbortStreamingTasks(this);
	assert(!IsStreaming());

	i32 nDevSize = m_nDevTextureSize;

#ifdef TEXSTRM_ASYNC_TEXCOPY
	bool bCopying = false;

	if (CanAsyncCopy() && (TryAddRef() > 0))
	{
		STexStreamOutState* pStreamState = StreamState_AllocateOut();
		if (pStreamState)
		{
			pStreamState->m_pTexture = this;
			pStreamState->m_nStartMip = MAX_MIP_LEVELS;

			SetStreamingInProgress(StreamOutMask | (u8)s_StreamOutTasks.GetIdxFromPtr(pStreamState));

			TTexture_StreamOutCopy job;
			job.SetClassInstance(pStreamState);
			job.RegisterJobState(&pStreamState->m_jobState);
			job.Run();

			bCopying = true;
		}
		else
		{
			Release();
		}
	}

	if (!bCopying)
#endif
	{
		// copy old mips to system memory
		StreamCopyMipsTexToMem(m_nMips - m_CacheFileHeader.m_nMipsPersistent, m_nMips - 1, false, NULL);
		ReleaseDeviceTexture(true);
		SetWasUnload(true);
	}

#ifndef _RELEASE
	if (CRenderer::CV_r_TexturesStreamingDebug == 2)
		iLog->Log("Unloading unused texture: %s", m_SrcName.c_str());
#endif

	return nDevSize;
}

void CTexture::StreamActivateLod(i32 nMinMip)
{
	FUNCTION_PROFILER_RENDERER();

	STexPoolItem* pItem = m_pFileTexMips->m_pPoolItem;
	STexPool* pPool = pItem->m_pOwner;
	i32 nMipOffset = m_nMips - pPool->m_nMips;
	i32 nDevMip = min((i32)pPool->m_nMips - 1, max(0, nMinMip - nMipOffset));

	if (pItem->m_nActiveLod != nDevMip)
	{
		pItem->m_nActiveLod = nDevMip;

		//D3DDevice* dv = gcpRendD3D->GetD3DDevice();
		//gcpRendD3D->GetDeviceContext().SetResourceMinLOD(m_pDevTexture->GetBaseTexture(), (FLOAT)nDevMip);
	}

	m_nMinMipVidActive = nMinMip;
}

void CTexture::StreamCopyMipsTexToMem(i32 nStartMip, i32 nEndMip, bool bToDevice, STexPoolItem* pNewPoolItem)
{
	PROFILE_FRAME(Texture_StreamUpload);

	HRESULT h = S_OK;
	nEndMip = min(nEndMip + 1, (i32)m_nMips) - 1;//+1 -1 needed as the compare is <=
	STexMipHeader* mh = m_pFileTexMips->m_pMipHeader;

	const bool bIsDXT = CTexture::IsBlockCompressed(m_eSrcFormat);
	i32k nMipAlign = bIsDXT ? 4 : 1;

	i32k nOldMinMipVidUploaded = m_nMinMipVidUploaded;

	if (bToDevice && !pNewPoolItem)
		SetMinLoadedMip(nStartMip);

	D3DFormat fmt = DeviceFormats::ConvertFromTexFormat(GetDstFormat());
	if (m_bIsSRGB)
		fmt = DeviceFormats::ConvertToSRGB(fmt);

	CDeviceTexture* pDevTexture = m_pDevTexture;
	u32 nTexMips = m_nMips;
	if (m_pFileTexMips->m_pPoolItem)
	{
		assert(m_pFileTexMips->m_pPoolItem->m_pDevTexture);
		assert(pDevTexture == m_pFileTexMips->m_pPoolItem->m_pDevTexture);
		nTexMips = m_pFileTexMips->m_pPoolItem->m_pOwner->m_nMips;
	}
	if (bToDevice && pNewPoolItem)
	{
		if (m_pDevTexture)
			m_pDevTexture->SetOwner(NULL);

		assert(pNewPoolItem->m_pDevTexture);
		pDevTexture = pNewPoolItem->m_pDevTexture;
		nTexMips = pNewPoolItem->m_pOwner->m_nMips;

		if (m_pDevTexture)
			m_pDevTexture->SetOwner(this);
	}

	if (!pDevTexture)
	{
		STextureLayout Layout = GetLayout();

#if DRX_PLATFORM_DURANGO && DURANGO_USE_ESRAM
		Layout.m_nESRAMOffset = SKIP_ESRAM;
#endif
		Layout.m_eFlags |= FT_STREAMED_FADEIN /* CDeviceObjectFactory::USAGE_STREAMING */;
		Layout.m_eDstFormat = CTexture::GetClosestFormatSupported(Layout.m_eDstFormat, Layout.m_pPixelFormat);

		pDevTexture = CDeviceTexture::Create(Layout, nullptr);
		assert(!!pDevTexture);

		// If a pool item was provided, it would have a valid dev texture, so we shouldn't have ended up here..
		assert(!bToDevice || !pNewPoolItem);
		SetDevTexture(pDevTexture);
		m_pDevTexture->SetOwner(this);
	}

	if (CRenderer::CV_r_texturesstreamingnoupload && bToDevice)
		return;

	i32k nMipOffset = m_nMips - nTexMips;
	i32k nSides = StreamGetNumSlices();

	D3DBaseTexture* pID3DTexture = pDevTexture->GetBaseTexture();

	i32 SizeToLoad = 0;
	for (i32 iSide = 0; iSide < nSides; ++iSide)
	{
		for (i32 nLod = nStartMip; nLod <= nEndMip; nLod++)
		{
			SMipData* mp = &mh[nLod].m_Mips[iSide];
			i32 nMipW = m_nWidth >> nLod;
			i32 nMipH = m_nHeight >> nLod;

			if (bToDevice && !mp->DataArray && s_bStreamDontKeepSystem)  // we have this mip already loaded
				continue;

			i32k nDevTexMip = nLod - nMipOffset;

			if (bToDevice)
			{
				if (mp->DataArray)
				{
					DRX_PROFILE_REGION_WAITING(PROFILE_RENDERER, "update texture");
					DrxInterlockedAdd(&CTexture::s_nTexturesDataBytesUploaded, mh[nLod].m_SideSize);					

					// TODO: batch upload (instead of loop)
					const SResourceMemoryMapping mapping =
					{
						pDevTexture->GetAlignment(nDevTexMip),                          // src alignment == hardware alignment
						{ 0, 0, 0, D3D11CalcSubresource(nDevTexMip, iSide, nTexMips) }, // dst position
						{ nMipW, nMipH, 1, 1 }                                          // dst size
					};

					GetDeviceObjectFactory().GetCoreCommandList().GetCopyInterface()->Copy(&mp->DataArray[0], pDevTexture, mapping);
				}
				else
					assert(0);
			}
			else
			{
				// TODO: the local memory layout matches the hardware layout, remove the row-wise copying
				i32k nMipSize = mh[nLod].m_SideSize;
				mp->Init(nMipSize, Align(max(1, nMipW), nMipAlign), Align(max(1, nMipH), nMipAlign));
				i32k nRowPitch = CTexture::TextureDataSize(nMipW, 1, 1, 1, 1, m_eDstFormat);
				i32k nRows = nMipSize / nRowPitch;
				assert(nMipSize % nRowPitch == 0);

				DRX_PROFILE_REGION_WAITING(PROFILE_RENDERER, "update texture");

				pDevTexture->DownloadToStagingResource(D3D11CalcSubresource(nDevTexMip, iSide, nTexMips), [&](uk pData, u32 rowPitch, u32 slicePitch)
				{
					for (i32 iRow = 0; iRow < nRows; ++iRow)
					{
					  memcpy(&mp->DataArray[iRow * nRowPitch], (byte*)pData + rowPitch * iRow, nRowPitch);
					}

					return true;
				});

				// mark as native
				mp->m_bNative = true;
			}
			SizeToLoad += m_pFileTexMips->m_pMipHeader[nLod].m_SideSize;

			if (s_bStreamDontKeepSystem && bToDevice)
				mp->Free();
		}
	}
#ifdef DO_RENDERLOG
	if (gRenDev->m_LogFileStr)
		gRenDev->LogStrv("Uploading mips '%s'. (%d[%d]), Size: %d, Time: %.3f\n", m_SrcName.c_str(), nStartMip, m_nMips, SizeToLoad, iTimer->GetAsyncCurTime());
#endif
}

#if defined(TEXSTRM_DEFERRED_UPLOAD)

ID3D11CommandList* CTexture::StreamCreateDeferred(i32 nStartMip, i32 nEndMip, STexPoolItem* pNewPoolItem, STexPoolItem* pSrcPoolItem)
{
	PROFILE_FRAME(Texture_StreamCreateDeferred);

	ID3D11CommandList* pCmdList = NULL;

	if (CTexture::s_pStreamDeferredCtx)
	{
		HRESULT h = S_OK;
		nEndMip = min(nEndMip + 1, (i32)m_nMips) - 1;//+1 -1 needed as the compare is <=
		STexMipHeader* mh = m_pFileTexMips->m_pMipHeader;

		i32k nOldMinMipVidUploaded = m_nMinMipVidUploaded;

		D3DFormat fmt = DeviceFormats::ConvertFromTexFormat(GetDstFormat());
		if (m_bIsSRGB)
			fmt = DeviceFormats::ConvertToSRGB(fmt);

		CDeviceTexture* pDevTexture = pNewPoolItem->m_pDevTexture;
		u32 nTexMips = pNewPoolItem->m_pOwner->m_nMips;

		i32k nMipOffset = m_nMips - nTexMips;
		i32k nSides = StreamGetNumSlices();

		D3DBaseTexture* pID3DTexture = pDevTexture->GetBaseTexture();

		i32 SizeToLoad = 0;
		for (i32 iSide = 0; iSide < nSides; ++iSide)
		{
			for (i32 nLod = nStartMip; nLod <= nEndMip; nLod++)
			{
				SMipData* mp = &mh[nLod].m_Mips[iSide];

				if (mp->DataArray)
				{
					i32k nDevTexMip = nLod - nMipOffset;

					DrxInterlockedAdd(&CTexture::s_nTexturesDataBytesUploaded, mh[nLod].m_SideSize);
					i32k nUSize = m_nWidth >> nLod;
					i32k nVSize = m_nHeight >> nLod;
					i32k nRowPitch = CTexture::TextureDataSize(nUSize, 1, 1, 1, 1, m_eDstFormat, m_eSrcTileMode);
					i32k nSlicePitch = CTexture::TextureDataSize(nUSize, nVSize, 1, 1, 1, m_eDstFormat, m_eSrcTileMode);
					DRX_PROFILE_REGION_WAITING(PROFILE_RENDERER, "update texture");
					{
						s_pStreamDeferredCtx->UpdateSubresource(pID3DTexture, D3D11CalcSubresource(nDevTexMip, iSide, nTexMips), NULL, &mp->DataArray[0], nRowPitch, nSlicePitch);
					}

					SizeToLoad += m_pFileTexMips->m_pMipHeader[nLod].m_SideSize;

					if (s_bStreamDontKeepSystem)
						mp->Free();
				}
			}
		}

		i32 nMipsSrc = pSrcPoolItem->m_pOwner->m_nMips;
		i32 nMipsDst = pNewPoolItem->m_pOwner->m_nMips;
		i32 nMipSrcOffset = m_nMips - nMipsSrc;
		i32 nMipDstOffset = m_nMips - nMipsDst;

		for (i32 iSide = 0; iSide < nSides; ++iSide)
		{
			for (i32 i = nEndMip + 1; i < m_nMips; ++i)
			{
				s_pStreamDeferredCtx->CopySubresourceRegion(
				  pID3DTexture,
				  D3D11CalcSubresource(i - nMipDstOffset, iSide, nMipsDst),
				  0, 0, 0, pSrcPoolItem->m_pDevTexture->GetBaseTexture(),
				  D3D11CalcSubresource(i - nMipSrcOffset, iSide, nMipsSrc),
				  NULL);
			}
		}

		s_pStreamDeferredCtx->FinishCommandList(FALSE, &pCmdList);
	}

	return pCmdList;
}

void CTexture::StreamApplyDeferred(ID3D11CommandList* pCmdList)
{
	FUNCTION_PROFILER_RENDERER();
	gcpRendD3D->GetDeviceContext().ExecuteCommandList(pCmdList, TRUE);
}

#endif

// Just remove item from the texture object and keep Item in Pool list for future use
// This function doesn't release API texture
void CTexture::StreamRemoveFromPool()
{
	CHK_MAINORRENDTH;

	if (!m_pFileTexMips || !m_pFileTexMips->m_pPoolItem)
		return;

	AUTO_LOCK(STexPoolItem::s_sSyncLock);

	ptrdiff_t nSize = (ptrdiff_t)m_nDevTextureSize;
	ptrdiff_t nPersSize = (ptrdiff_t)m_nPersistentSize;

	s_pPoolMgr->ReleaseItem(m_pFileTexMips->m_pPoolItem);

	m_pFileTexMips->m_pPoolItem = NULL;
	m_nDevTextureSize = 0;
	m_nPersistentSize = 0;
	if (m_pDevTexture)
		m_pDevTexture->SetOwner(NULL);
	m_pDevTexture = NULL;

	SetMinLoadedMip(MAX_MIP_LEVELS);
	m_nMinMipVidActive = MAX_MIP_LEVELS;

	DrxInterlockedAdd(&s_nStatsStreamPoolBoundMem, -nSize);
	DrxInterlockedAdd(&s_nStatsStreamPoolBoundPersMem, -nPersSize);
}

void CTexture::StreamAssignPoolItem(STexPoolItem* pItem, i32 nMinMip)
{
	FUNCTION_PROFILER_RENDERER();

	assert(!pItem->IsFree());
	STexPool* pItemOwner = pItem->m_pOwner;

	if (m_pFileTexMips->m_pPoolItem == pItem)
	{
		assert(m_nDevTextureSize == m_pFileTexMips->m_pPoolItem->m_pOwner->m_nDevTextureSize);
		assert(m_pFileTexMips->m_pPoolItem->m_pTex == this);
		assert(m_pDevTexture == m_pFileTexMips->m_pPoolItem->m_pDevTexture);
		return;
	}

	if (m_pFileTexMips->m_pPoolItem == NULL)
	{
		if (m_pDevTexture)
			__debugbreak();
	}

	i32 nPersMip = m_nMips - m_CacheFileHeader.m_nMipsPersistent;
	size_t nPersSize = StreamComputeSysDataSize(nPersMip);

	// Assign a new pool item
	{
		AUTO_LOCK(STexPoolItem::s_sSyncLock);
		StreamRemoveFromPool();

		m_pFileTexMips->m_pPoolItem = pItem;
		m_nDevTextureSize = pItemOwner->m_nDevTextureSize;
		m_nPersistentSize = nPersSize;
		pItem->m_pTex = this;
	}

#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	pItem->m_pDevTexture->InitD3DTexture();
#endif

	SAFE_RELEASE(m_pDevTexture);
	m_pDevTexture = pItem->m_pDevTexture;
	if (m_pDevTexture)
		m_pDevTexture->SetOwner(this);

#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	m_nDeviceAddressInvalidated = m_pDevTexture->GetBaseAddressInvalidated();
#endif

	SetMinLoadedMip(m_nMips - pItemOwner->m_nMips);
	StreamActivateLod(nMinMip);

	DrxInterlockedAdd(&s_nStatsStreamPoolBoundMem, pItem->m_nDevTextureSize);
	DrxInterlockedAdd(&s_nStatsStreamPoolBoundPersMem, nPersSize);

	InvalidateDeviceResource(this, eDeviceResourceDirty | eDeviceResourceViewDirty);
}

STexPool* CTexture::StreamGetPool(i32 nStartMip, i32 nMips)
{
	// depth is not 4x4x4 compressed, but 4x4x1
	const bool bIsBC = CTexture::IsBlockCompressed(m_eSrcFormat);
	i32k nMipAlign = bIsBC ? 4 : 1;
	i32 uSize = Align(max(1, m_nWidth  >> nStartMip), nMipAlign);
	i32 vSize = Align(max(1, m_nHeight >> nStartMip), nMipAlign);
	i32 wSize = Align(max(1, m_nDepth  >> nStartMip), 1);
	i32 nArraySize = m_nArraySize;

	STextureLayout pLayout = GetLayout();

	// Specify sub-set of sub-resources
	pLayout.m_nWidth     = uSize;
	pLayout.m_nHeight    = vSize;
	pLayout.m_nDepth     = wSize;
	pLayout.m_nArraySize = nArraySize;
	pLayout.m_nMips      = nMips;
	pLayout.m_eDstFormat     = CTexture::GetClosestFormatSupported(pLayout.m_eDstFormat, pLayout.m_pPixelFormat);

	return s_pPoolMgr->GetPool(pLayout);
}

STexPoolItem* CTexture::StreamGetPoolItem(i32 nStartMip, i32 nMips, bool bShouldBeCreated, bool bCreateFromMipData, bool bCanCreate, bool bForStreamOut)
{
	FUNCTION_PROFILER_RENDERER();

	if (!m_pFileTexMips)
		return NULL;

	assert(nStartMip < m_nMips);
	assert(!IsStreaming());

	SCOPED_RENDERER_ALLOCATION_NAME_HINT(GetSourceName());

	// depth is not 4x4x4 compressed, but 4x4x1
	const bool bIsBC = CTexture::IsBlockCompressed(m_eSrcFormat);
	i32k nMipAlign = bIsBC ? 4 : 1;
	i32 uSize = Align(max(1, m_nWidth  >> nStartMip), nMipAlign);
	i32 vSize = Align(max(1, m_nHeight >> nStartMip), nMipAlign);
	i32 wSize = Align(max(1, m_nDepth  >> nStartMip), 1);
	i32 nArraySize = m_nArraySize;

	if (m_pFileTexMips->m_pPoolItem && m_pFileTexMips->m_pPoolItem->m_pOwner)
	{
		STexPoolItem* pPoolItem = m_pFileTexMips->m_pPoolItem;
		if (pPoolItem->m_pOwner->m_nMips      == nMips &&
		    pPoolItem->m_pOwner->m_Width      == uSize &&
		    pPoolItem->m_pOwner->m_Height     == vSize &&
		//	pPoolItem->m_pOwner->m_Depth      == wSize &&
		    pPoolItem->m_pOwner->m_nArraySize == nArraySize)
		{
			return NULL;
		}
	}

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Texture, 0, "Creating Texture");
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Texture, 0, "%s %ix%ix%i %08x", m_SrcName.c_str(), m_nWidth, m_nHeight, m_nMips, m_eFlags);

	STextureLayout pLayout = GetLayout();
	STexturePayload pPayload;

	if (bCreateFromMipData)
	{
		i32k nSlices = nArraySize;

		SSubresourcePayload* InitData = new SSubresourcePayload[nMips * nSlices];

		// Contrary to hardware here the sub-resources are sorted by mips, then by slices.
		// In hardware it's sorted by slices, then by mips (so same mips are consecutive).
		for (i32 nSlice = 0, nSubresource = 0; nSlice < nSlices; nSlice++)
		{
			i32 w = uSize;
			i32 h = vSize;
			i32 d = wSize;

			for (i32 nMip = nStartMip, nEndMip = nStartMip + nMips; nMip != nEndMip; ++nMip, ++nSubresource)
			{
				if (!w) w = 1;
				if (!h) h = 1;
				if (!d) d = 1;

				STexMipHeader& mmh = m_pFileTexMips->m_pMipHeader[nMip];
				SMipData& md = mmh.m_Mips[nSlice];

				assert(nSubresource < nMips*nSlices);
				InitData[nSubresource].m_pSysMem = md.DataArray;
				InitData[nSubresource].m_sSysMemAlignment.typeStride   = CTexture::TextureDataSize(1, 1, 1, 1, 1, pLayout.m_eSrcFormat, m_eSrcTileMode);
				InitData[nSubresource].m_sSysMemAlignment.rowStride    = CTexture::TextureDataSize(w, 1, 1, 1, 1, pLayout.m_eSrcFormat, m_eSrcTileMode);
				InitData[nSubresource].m_sSysMemAlignment.planeStride  = CTexture::TextureDataSize(w, h, 1, 1, 1, pLayout.m_eSrcFormat, m_eSrcTileMode);
				InitData[nSubresource].m_sSysMemAlignment.volumeStride = CTexture::TextureDataSize(w, h, d, 1, 1, pLayout.m_eSrcFormat, m_eSrcTileMode);

				w >>= 1;
				h >>= 1;
				d >>= 1;
			}
		}

		pPayload.m_pSysMemSubresourceData = InitData;
		pPayload.m_eSysMemTileMode = m_eSrcTileMode;
	}

	// Specify sub-set of sub-resources
	pLayout.m_nWidth     = uSize;
	pLayout.m_nHeight    = vSize;
	pLayout.m_nDepth     = wSize;
	pLayout.m_nArraySize = nArraySize;
	pLayout.m_nMips      = nMips;
	pLayout.m_eDstFormat = CTexture::GetClosestFormatSupported(pLayout.m_eDstFormat, pLayout.m_pPixelFormat);

	// For end of C3, preserve existing (idle wait) console behaviour
	bool bGPIMustWaitForIdle = !bForStreamOut;

	STexPoolItem* pItem = s_pPoolMgr->GetItem(pLayout, bShouldBeCreated, m_SrcName.c_str(), bCreateFromMipData ? &pPayload : nullptr, bCanCreate, bGPIMustWaitForIdle);
	if (pItem)
		return pItem;

	s_pTextureStreamer->FlagOutOfMemory();
	return NULL;
}

void CTexture::StreamCopyMipsTexToTex(STexPoolItem* const pSrcItem, i32 nSrcMipOffset,
                                      STexPoolItem* const pDstItem, i32 nDstMipOffset, i32 nNumMips)
{
	CHK_RENDTH;

	CDeviceTexture* pSrcDevTexture = pSrcItem->m_pDevTexture;
	CDeviceTexture* pDstDevTexture = pDstItem->m_pDevTexture;

	u32k nDstNumSlices = pDstItem->m_pOwner->GetNumSlices();
	u32k nSrcNumSlices = pSrcItem->m_pOwner->GetNumSlices();
	assert(nDstNumSlices == nSrcNumSlices && "Can't stream individual slices!");

	if (0)
	{
	}
#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	else if (!gcpRendD3D->m_pRT->IsRenderThread(true))
	{
		// We can use the move engine!
		UINT64 fence = StreamCopyMipsTexToTex_MoveEngine(pSrcItem, nSrcMipOffset, pDstItem, nDstMipOffset, nNumMips);
		while (gcpRendD3D->GetPerformanceDevice().IsFencePending(fence))
			DrxSleep(1);
	}
#endif
	else
	{
		GPUPIN_DEVICE_TEXTURE(gcpRendD3D->GetPerformanceDeviceContext(), pDstDevTexture);

		for (u32 iSlice = 0; iSlice < nSrcNumSlices; ++iSlice)
			CopySliceChain(
				pDstItem->m_pDevTexture, pDstItem->m_pOwner->m_nMips, iSlice, nDstMipOffset, 
				pSrcItem->m_pDevTexture, pSrcItem->m_pOwner->m_nMips, iSlice, nSrcMipOffset,
				1, nNumMips);
	}
}

// Debug routines /////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _RELEASE
struct StreamDebugSortFunc
{
	bool operator()(const CTexture* p1, const CTexture* p2) const
	{
		if (!p1)
			return true;
		if (!p2)
			return false;
		if (p1->GetActualSize() != p2->GetActualSize())
			return p1->GetActualSize() < p2->GetActualSize();
		return p1 < p2;
	}
};

struct StreamDebugSortWantedFunc
{
	bool operator()(const CTexture* p1, const CTexture* p2) const
	{
		i32 nP1ReqMip = p1->GetRequiredMip();
		i32 nP1Size = p1->StreamComputeSysDataSize(nP1ReqMip);

		i32 nP2ReqMip = p2->GetRequiredMip();
		i32 nP2Size = p2->StreamComputeSysDataSize(nP2ReqMip);

		if (nP1Size != nP2Size)
			return nP1Size < nP2Size;

		return p1 < p2;
	}
};

void CTexture::OutputDebugInfo()
{
	i32 nX = 40;
	i32 nY = 30;

	if (CRenderer::CV_r_TexturesStreamingDebugDumpIntoLog)
	{
		Vec3i vCamPos = iSystem->GetViewCamera().GetPosition();
		DrxLogAlways("===================== Dumping textures streaming debug info for camera position (%d, %d, %d) =====================", vCamPos.x, vCamPos.y, vCamPos.z);
	}

	char szText[512];
	drx_sprintf(szText, "Size | WantedSize |  MF   | P |  Mips  |  N  | F | Z | S | Name");
	IRenderAuxText::WriteXY(nX, nY, 1.f, 1.f, 1, 1, 0, 1, "%s", szText);
	if (CRenderer::CV_r_TexturesStreamingDebugDumpIntoLog)
	{
		DrxLogAlways("%s", szText);
	}

	std::vector<CTexture*> texSorted;
	s_pTextureStreamer->StatsFetchTextures(texSorted);

	tukk const sTexFilter = CRenderer::CV_r_TexturesStreamingDebugfilter->GetString();
	const bool bNameFilter = sTexFilter != 0 && strlen(sTexFilter) > 1;

	if (!texSorted.empty())
	{
		switch (CRenderer::CV_r_TexturesStreamingDebug)
		{
		case 4:
			std::stable_sort(&texSorted[0], &texSorted[0] + texSorted.size(), StreamDebugSortFunc());
			break;

		case 5:
			std::reverse(texSorted.begin(), texSorted.end());
			break;

		case 6:
			std::stable_sort(&texSorted[0], &texSorted[0] + texSorted.size(), StreamDebugSortWantedFunc());
			break;
		}
	}

	for (i32 i = (i32)texSorted.size() - 1, nTexNum = 0; i >= 0; --i)
	{
		CTexture* tp = texSorted[i];
		if (tp == NULL)
			continue;
		// name filter
		if (bNameFilter && !strstr(tp->m_SrcName.c_str(), sTexFilter))
			continue;
		if (tp->m_nDevTextureSize / 1024 < CRenderer::CV_r_TexturesStreamingDebugMinSize)
			continue;

		ColorF color = (tp->m_nDevTextureSize / 1024 >= CRenderer::CV_r_TexturesStreamingDebugMinSize * 2) ? Col_Red : Col_Green;

		// compute final mip factor
		bool bHighPriority = false;
		float fFinalMipFactor = pow(99.99f, 2.f);
		for (i32 z = 0; z < MAX_PREDICTION_ZONES; z++)
			if (tp->m_pFileTexMips && (i32)tp->m_streamRounds[z].nRoundUpdateId > gRenDev->GetStreamZoneRoundId(z) - 2)
			{
				fFinalMipFactor = min(fFinalMipFactor, tp->m_pFileTexMips->m_arrSPInfo[z].fLastMinMipFactor);
				bHighPriority |= tp->m_streamRounds[z].bLastHighPriority;
			}

		// how many times used in area around
		assert(tp->m_pFileTexMips);

		i32 nMipIdSigned = tp->StreamCalculateMipsSigned(fFinalMipFactor);

		if (nMipIdSigned > CRenderer::CV_r_TexturesStreamingDebugMinMip)
			continue;

		i32 nPersMip = tp->m_nMips - tp->m_CacheFileHeader.m_nMipsPersistent;
		i32 nMipReq = min(tp->GetRequiredMip(), nPersMip);
		i32k nWantedSize = tp->StreamComputeSysDataSize(nMipReq);

		assert(tp->m_pFileTexMips);
		PREFAST_ASSUME(tp->m_pFileTexMips);
		drx_sprintf(szText, "%.2f | %.2f |%6.2f | %1d | %2d/%d/%d | %i/%i | %i | %s",
		            (float)tp->m_nDevTextureSize / (1024 * 1024.0f),
		            (float)nWantedSize / (1024 * 1024.0f),
		            sqrtf(fFinalMipFactor),
		            (i32)bHighPriority,
		            tp->GetNumMips() - nMipIdSigned, tp->GetNumMips() - tp->m_nMinMipVidUploaded, tp->GetNumMips(),
		            tp->m_streamRounds[0].nRoundUpdateId, tp->m_streamRounds[MAX_STREAM_PREDICTION_ZONES - 1].nRoundUpdateId,
		            tp->m_nAccessFrameID >= gRenDev->GetMainFrameID() - 8,
		            tp->m_SrcName.c_str());

		IRenderAuxText::WriteXY(nX, nY + (nTexNum + 1) * 10, 1.f, 1.f, color.r, color.g, color.b, 1, "%s", szText);
		if (CRenderer::CV_r_TexturesStreamingDebugDumpIntoLog)
		{
			DrxLogAlways("%s", szText);
		}

		++nTexNum;
		if (nTexNum > 50 && !CRenderer::CV_r_TexturesStreamingDebugDumpIntoLog)
		{
			break;
		}
	}

	if (CRenderer::CV_r_TexturesStreamingDebugDumpIntoLog)
	{
		DrxLogAlways("==============================================================================================================");
	}

	if (ICVar* pCVar = gEnv->pConsole->GetCVar("r_TexturesStreamingDebugDumpIntoLog"))
	{
		pCVar->Set(0);
	}
}
#endif
