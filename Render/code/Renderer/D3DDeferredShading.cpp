// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Render/D3DPostProcess.h>
#include <drx3D/Render/Textures/TextureUpr.h>
#include <drx3D/Render/Textures/TextureHelpers.h>
#include <drx3D/Render/RendElements/FlareSoftOcclusionQuery.h>
#include <drx3D/Render/ReverseDepth.h>
#include <drx3D/Render/D3D/GraphicsPipeline/TiledLightVolumes.h>
#include <drx3D/Render/D3D/GraphicsPipeline/ClipVolumes.h>
#include <drx3D/Render/D3D/GraphicsPipeline/DeferredDecals.h>
#include <drx3D/Render/D3D/GraphicsPipeline/ShadowMask.h>
#include <drx3D/Render/D3D/GraphicsPipeline/Water.h>
#if defined(FEATURE_SVO_GI)
	#include <drx3D/Render/D3D_SVO.h>
#endif

#include <drx3D/Render/RenderView.h>
#include <drx3D/Render/RendererResources.h>

#define MAX_VIS_AREAS 32

// MSAA potential optimizations todo:
//  - long term: port all functionality to compute, including all extra effects passes.

// About MSAA:
// - Please be careful when accessing or rendering into msaa'ed targets. When adding new techniques please make sure to test
// - For post process technique to be MSAA friendly, do either:
//    - Use compute. Single pass and as efficient as gets. Context switches might be problematic, until all lighting pipeline done like this.
//    - For non compute, require 2 passes. One at pixel frequency, other at sub sample frequency.
//				 - Reuse existing sample frequency regions on stencil via stencilread/write mask:
//						- If not possible, tag pixel frequency regions using stencil + m_pMSAAMaskRT
//						- Alternative poor man version, do clip in shader.

#pragma warning(push)
#pragma warning(disable: 4244)

CDeferredShading* CDeferredShading::m_pInstance = NULL;

#define RT_LIGHTSMASK                     g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3] | g_HWSR_MaskBit[HWSR_SAMPLE4] | g_HWSR_MaskBit[HWSR_SAMPLE5] | g_HWSR_MaskBit[HWSR_LIGHT_TEX_PROJ] | g_HWSR_MaskBit[HWSR_CUBEMAP0]
#define RT_LIGHTPASS_RESETMASK            g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3] | g_HWSR_MaskBit[HWSR_SAMPLE5] | g_HWSR_MaskBit[HWSR_LIGHT_TEX_PROJ] | g_HWSR_MaskBit[HWSR_CUBEMAP0]
#define RT_DEBUGMASK                      g_HWSR_MaskBit[HWSR_DEBUG0] | g_HWSR_MaskBit[HWSR_DEBUG1] | g_HWSR_MaskBit[HWSR_DEBUG2] | g_HWSR_MaskBit[HWSR_DEBUG3]
#define RT_TEX_PROJECT                    g_HWSR_MaskBit[HWSR_SAMPLE0]
#define RT_GLOBAL_CUBEMAP                 g_HWSR_MaskBit[HWSR_SAMPLE0]
#define RT_SPECULAR_CUBEMAP               g_HWSR_MaskBit[HWSR_SAMPLE1]
#define RT_AMBIENT_LIGHT                  g_HWSR_MaskBit[HWSR_SAMPLE5]
#define RT_GLOBAL_CUBEMAP_IGNORE_VISAREAS g_HWSR_MaskBit[HWSR_SAMPLE4]
#define RT_AREALIGHT                      g_HWSR_MaskBit[HWSR_SAMPLE1]
#define RT_OVERDRAW_DEBUG                 g_HWSR_MaskBit[HWSR_DEBUG0]
#define RT_BOX_PROJECTION                 g_HWSR_MaskBit[HWSR_SAMPLE3]
#define RT_CLIPVOLUME_ID                  g_HWSR_MaskBit[HWSR_LIGHTVOLUME0]

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::Init(i32 nSize)
{
	m_nSize = nSize;
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::Clear()
{
	memset(&m_arrAllocatedBlocks[0], 0, MAX_BLOCKS * sizeof(m_arrAllocatedBlocks[0]));
#ifdef _DEBUG
	m_nTotalWaste = 0;
	m_arrDebugBlocks.clear();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::FreeMemory()
{
#ifdef _DEBUG
	stl::free_container(m_arrDebugBlocks);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CTexPoolAtlas::AllocateGroup(i32* pOffsetX, i32* pOffsetY, i32 nSizeX, i32 nSizeY)
{
	i32 nBorder = 2;
	nSizeX += nBorder << 1;
	nSizeY += nBorder << 1;

	if (nSizeX > m_nSize || nSizeY > m_nSize)
		return false;

	u16 nBestX = 0;
	u16 nBestY = 0;
	u16 nBestID = 0;
	u32 nBestWaste = ~0;

	// outer loop over all relevant allocated blocks (Y dimension)
	i32 nCurrY = 0;
	for (i32 nCurrBlockID = 0;
	     m_arrAllocatedBlocks[max(nCurrBlockID - 1, 0)] > 0 && nCurrY <= m_nSize - nSizeY && nBestWaste > 0;
	     ++nCurrBlockID)
	{
		u32k nCurrBlock = m_arrAllocatedBlocks[nCurrBlockID];
		u16k nCurrBlockWidth = LOWORD(nCurrBlock);
		u16k nCurrBlockHeight = HIWORD(nCurrBlock);

		// get max X for intersected blocks
		i32 nCurrX = nCurrBlockWidth;
		i32 nNextY = nCurrBlockHeight;
		for (u16 nNextBlockID = nCurrBlockID + 1;
		     m_arrAllocatedBlocks[nNextBlockID] > 0 && nNextY < nSizeY;
		     ++nNextBlockID)
		{
			u32k nNextBlock = m_arrAllocatedBlocks[nNextBlockID];
			u16k nNextBlockWidth = LOWORD(nNextBlock);
			u16k nNextBlockHeight = HIWORD(nNextBlock);
			nCurrX = max(nCurrX, i32(nNextBlockWidth));
			nNextY += nNextBlockHeight;
		}

		// can fit next to them?
		if (nSizeX <= m_nSize - nCurrX)
		{
			// compute waste
			u32 nWaste = 0;
			nNextY = nCurrY;
			for (u16 nNextBlockID = nCurrBlockID;
			     m_arrAllocatedBlocks[nNextBlockID] > 0 && nNextY < nCurrY + nSizeY;
			     ++nNextBlockID)
			{
				u32k nNextBlock = m_arrAllocatedBlocks[nNextBlockID];
				u16k nNextBlockWidth = LOWORD(nNextBlock);
				u16k nNextBlockHeight = HIWORD(nNextBlock);
				nWaste += (nCurrX - nNextBlockWidth) * (min(nCurrY + nSizeY, nNextY + nNextBlockHeight) - max(nCurrY, nNextY));
				nNextY += nNextBlockHeight;
			}
			nWaste += max(nCurrY + nSizeY - nNextY, 0) * nCurrX;

			// right spot?
			if (nWaste < nBestWaste)
			{
				nBestX = nCurrX;
				nBestY = nCurrY;
				nBestID = nCurrBlockID;
				nBestWaste = nWaste;
			}
		}

		nCurrY += nCurrBlockHeight;
	}

	if ((nBestX | nBestY) || nCurrY <= m_nSize - nSizeY)
	{
		assert(nBestID < MAX_BLOCKS - 1);
		if (nBestID >= MAX_BLOCKS - 1)
			return false;

		*pOffsetX = nBestX + nBorder;
		*pOffsetY = nBestY + nBorder;

		// block to be added, update block info
		u32 nBlockData = m_arrAllocatedBlocks[nBestID];
		u16k nReplacedHeight = HIWORD(nBlockData);
		if (nSizeY < nReplacedHeight)
		{
			nBlockData = MAKELONG(nBlockData, nReplacedHeight - nSizeY);
			// shift by 1
			for (u16 nID = nBestID + 1; nBlockData > 0; ++nID)
				std::swap(m_arrAllocatedBlocks[nID], nBlockData);
		}
		else if (nSizeY > nReplacedHeight)
		{
			u16 nCoveredHeight = nReplacedHeight;
			u16 nBlocksToSkip = 0;
			nBlockData = m_arrAllocatedBlocks[nBestID + 1];
			for (u16 nID = nBestID + 1; nBlockData > 0; nBlockData = m_arrAllocatedBlocks[++nID])
			{
				u16k nCurrHeight = HIWORD(nBlockData);
				nCoveredHeight += nCurrHeight;
				if (nSizeY >= nCoveredHeight)
					nBlocksToSkip++;
				else
				{
					m_arrAllocatedBlocks[nID] = MAKELONG(nBlockData, nCoveredHeight - nSizeY);
					break;
				}
			}
			// shift by nBlocksToSkip
			u16 nID = nBestID + nBlocksToSkip + 1;
			nBlockData = m_arrAllocatedBlocks[nID];
			for (; nBlockData > 0; nBlockData = m_arrAllocatedBlocks[++nID])
				m_arrAllocatedBlocks[nID - nBlocksToSkip] = nBlockData;
		}
		m_arrAllocatedBlocks[nBestID] = MAKELONG(nBestX + nSizeX, nSizeY);

#ifdef _DEBUG
		if (m_arrDebugBlocks.size())
			m_nTotalWaste += nBestWaste;
		_AddDebugBlock(nBestX, nBestY, nSizeX, nSizeY);
#endif

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CTexPoolAtlas::_AddDebugBlock(i32 x, i32 y, i32 sizeX, i32 sizeY)
{
	SShadowMapBlock block;
	block.m_nX1 = x;
	block.m_nX2 = x + sizeX;
	block.m_nY1 = y;
	block.m_nY2 = y + sizeY;
	assert(block.m_nX2 <= m_nSize && block.m_nY2 <= m_nSize);
	for (std::vector<SShadowMapBlock>::const_iterator it = m_arrDebugBlocks.begin();
	     it != m_arrDebugBlocks.end(); it++)
	{
		assert(!block.Intersects(*it));
	}
	m_arrDebugBlocks.push_back(block);
}

float CTexPoolAtlas::_GetDebugUsage() const
{
	u32 nUsed = 0;
	for (std::vector<SShadowMapBlock>::const_iterator it = m_arrDebugBlocks.begin();
	     it != m_arrDebugBlocks.end(); it++)
	{
		nUsed += (it->m_nX2 - it->m_nX1) * (it->m_nY2 - it->m_nY1);
	}

	return 100.f * nUsed / (m_nSize * m_nSize);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::GetClipVolumeParams(const Vec4*& pParams)
{
	pParams = m_vClipVolumeParams;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::ReleaseData()
{
	auto* tiledLights = gcpRendD3D.GetGraphicsPipeline().GetTiledLightVolumesStage();

	tiledLights->Clear();

	for (u32 iThread = 0; iThread < 2; ++iThread)
	{
		for (u32 nRecurseLevel = 0; nRecurseLevel < MAX_REND_RECURSION_LEVELS; ++nRecurseLevel)
		{
			m_vecGIClipVolumes[iThread][nRecurseLevel].clear();
		}
	}

	m_shadowPoolAlloc.SetUse(0);
	stl::free_container(m_shadowPoolAlloc);

	m_blockPack.FreeContainers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::AddGIClipVolume(IRenderMesh* pClipVolume, const Matrix34& mxTransform)
{
	u32 nThreadID = gRenDev->GetMainThreadID();

	TArray<SClipShape>& vecGIClipVolumes = m_vecGIClipVolumes[nThreadID][0];
	vecGIClipVolumes.push_back(SClipShape(pClipVolume, mxTransform));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SetupPasses(CRenderView* pRenderView)
{
	m_nThreadID = gRenDev->GetRenderThreadID();

	m_pNormalsRT = CRendererResources::s_ptexSceneNormalsMap;
	m_pDepthRT = CRendererResources::s_ptexLinearDepth;
	m_pResolvedStencilRT = CRendererResources::s_ptexVelocity;
	m_pDiffuseRT = CRendererResources::s_ptexSceneDiffuse;
	m_pSpecularRT = CRendererResources::s_ptexSceneSpecular;
	m_pMSAAMaskRT = CRendererResources::s_ptexBackBuffer;

	const auto& viewInfo = pRenderView->GetViewInfo(CCamera::eEye_Left);


	m_pCamFront = viewInfo.cameraVZ;
	m_pCamFront.Normalize();
	m_pCamPos = viewInfo.cameraOrigin;

	m_fCamFar = viewInfo.farClipPlane;
	m_fCamNear = viewInfo.nearClipPlane;

	m_pView = viewInfo.viewMatrix;
	m_pView.Transpose();

	m_mViewProj = viewInfo.cameraProjMatrix;
	m_pViewProjI = m_mViewProj.GetInverted();

	m_mViewProj.Transpose();

	m_pShader = CShaderMan::s_shDeferredShading;

	//gcpRendD3D->SetCullMode(R_CULL_BACK);

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest <= 1)
		m_nRenderState |= GS_NODEPTHTEST;
	else
		m_nRenderState &= ~GS_NODEPTHTEST;

	m_nCurTargetWidth = m_pDiffuseRT->GetWidth();
	m_nCurTargetHeight = m_pDiffuseRT->GetHeight();

	SetupGlobalConsts(pRenderView);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SetupGlobalConsts(CRenderView* pRenderView)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	//set world basis
	float maskRTWidth = float(m_nCurTargetWidth);
	float maskRTHeight = float(m_nCurTargetHeight);
	Vec4r vWBasisX, vWBasisY, vWBasisZ, vCamPos;
	Vec4 vParamValue, vMag;

	CShadowUtils::ProjectScreenToWorldExpansionBasis(rd->m_IdentityMatrix, pRenderView->GetCamera(CCamera::eEye_Left), pRenderView->m_vProjMatrixSubPixoffset, maskRTWidth, maskRTHeight, vWBasisX, vWBasisY, vWBasisZ, vCamPos, true);

	const auto& downscaleFactor = gRenDev->GetRenderQuality().downscaleFactor;
	vWorldBasisX = vWBasisX / downscaleFactor.x;
	vWorldBasisY = vWBasisY / downscaleFactor.y;
	vWorldBasisZ = vWBasisZ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::GetLightRenderSettings(const CRenderView* pRenderView, const SRenderLight* const __restrict pDL, bool& bStencilMask, bool& bUseLightVolumes, EShapeMeshType& meshType)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	const bool bAreaLight = (pDL->m_Flags & DLF_AREA_LIGHT) && pDL->m_fAreaWidth && pDL->m_fAreaHeight && pDL->m_fLightFrustumAngle;

	if (CRenderer::CV_r_deferredshadingLightVolumes)
	{
		if (bAreaLight)
		{
			// area lights use non-uniform box volume
			// need to do more complex box intersection test
			float fExpensionRadius = pDL->m_fRadius * 1.08f;
			Vec3 vScale(fExpensionRadius, fExpensionRadius, fExpensionRadius);

			Matrix34 mObjInv = CShadowUtils::GetAreaLightMatrix(pDL, vScale);
			mObjInv.Invert();

			// check if volumes bounding box intersects the near clipping plane
			const CCamera& camera = pRenderView->GetCamera(CCamera::eEye_Left);
			const Plane* pNearPlane(camera.GetFrustumPlane(FR_PLANE_NEAR));
			Vec3 pntOnNearPlane(camera.GetPosition() - pNearPlane->DistFromPlane(camera.GetPosition()) * pNearPlane->n);
			Vec3 pntOnNearPlaneOS(mObjInv.TransformPoint(pntOnNearPlane));

			Vec3 nearPlaneOS_n(mObjInv.TransformVector(pNearPlane->n));
			f32 nearPlaneOS_d(-nearPlaneOS_n.Dot(pntOnNearPlaneOS));

			// get extreme lengths
			float t(fabsf(nearPlaneOS_n.x) + fabsf(nearPlaneOS_n.y) + fabsf(nearPlaneOS_n.z));

			float t0 = t + nearPlaneOS_d;
			float t1 = -t + nearPlaneOS_d;

			if (t0 * t1 > 0.0f)
				bUseLightVolumes = true;
			else
				bStencilMask = true;
		}
		else
		{
			const float kDLRadius = pDL->m_fRadius;
			const float fSmallLightBias = 0.5f;
			// the light mesh tessellation and near clipping plane require some bias when testing if inside sphere
			// higher bias for low radius lights
			float fSqLightRadius = kDLRadius * max(-0.1f * kDLRadius + 1.5f, 1.22f);
			fSqLightRadius = max(kDLRadius + fSmallLightBias, fSqLightRadius); //always add on a minimum bias, for very small light's sake
			fSqLightRadius *= fSqLightRadius;
			if (fSqLightRadius < pDL->m_Origin.GetSquaredDistance(m_pCamPos))
				bUseLightVolumes = true;
			else
				bStencilMask = true;
		}
	}

	Vec4 pLightRect = Vec4(pDL->m_sX, pDL->m_sY, pDL->m_sWidth, pDL->m_sHeight);

	float fCurTargetWidth = (float)(m_nCurTargetWidth);
	float fCurTargetHeight = (float)(m_nCurTargetHeight);

	if (!iszero(CRenderer::CV_r_DeferredShadingLightLodRatio))
	{
		if (CRenderer::CV_r_DeferredShadingLightStencilRatio > 0.01f)
		{
			const float fLightLodRatioScale = CRenderer::CV_r_DeferredShadingLightLodRatio;
			float fLightArea = pLightRect.z * pLightRect.w;
			float fScreenArea = fCurTargetHeight * fCurTargetWidth;
			float fLightRatio = fLightLodRatioScale * (fLightArea / fScreenArea);

			const float fDrawVolumeThres = 0.005f;
			if (fLightRatio < fDrawVolumeThres)
			{
				bUseLightVolumes = false;
			}

			if (fLightRatio > 4 * CRenderer::CV_r_DeferredShadingLightStencilRatio)
				meshType = SHAPE_PROJECTOR2;
			else if (fLightRatio > 2 * CRenderer::CV_r_DeferredShadingLightStencilRatio)
				meshType = SHAPE_PROJECTOR1;
		}
		else
		{
			const float fLightLodRatioScale = CRenderer::CV_r_DeferredShadingLightLodRatio;
			float fLightArea = pLightRect.z * pLightRect.w;
			float fScreenArea = fCurTargetHeight * fCurTargetWidth;
			float fLightRatio = fLightLodRatioScale * (fLightArea / fScreenArea);

			const float fDrawVolumeThres = 0.005f;
			if (fLightRatio < fDrawVolumeThres)
			{
				bUseLightVolumes = false;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CubemapsCompare
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		// Cubes sort by: Sort priority first, light radius, lastly by entity id (insertion order every frame is not guaranteed)
		if (l0.m_nSortPriority != l1.m_nSortPriority)
			return l0.m_nSortPriority < l1.m_nSortPriority;

		if (l0.m_fRadius == l1.m_fRadius)
			return l0.m_nEntityId < l1.m_nEntityId;

		return l0.m_fRadius > l1.m_fRadius;
	}
};

struct CubemapsCompareInv
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		// Cubes sort by: Sort priority first, light radius, lastly by entity id (insertion order every frame is not guaranteed)
		if (l0.m_nSortPriority != l1.m_nSortPriority)
			return l0.m_nSortPriority > l1.m_nSortPriority;

		if (l0.m_fRadius == l1.m_fRadius)
			return l0.m_nEntityId > l1.m_nEntityId;

		return l0.m_fRadius < l1.m_fRadius;
	}
};

struct LightsCompare
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		if (!(l0.m_Flags & DLF_CASTSHADOW_MAPS) && (l1.m_Flags & DLF_CASTSHADOW_MAPS))
		{
			return true;
		}
		else
			return false;
	}
};

struct DeffDecalSort
{
	bool operator()(const SDeferredDecal& decal0, const SDeferredDecal& decal1) const
	{
		uint bBump0 = (decal0.nFlags & DECAL_HAS_NORMAL_MAP);
		uint bBump1 = (decal1.nFlags & DECAL_HAS_NORMAL_MAP);
		//bump-mapped decals first
		if (bBump0 != bBump1)
			return (bBump0 < bBump1);
		return
		  (decal0.nSortOrder < decal1.nSortOrder);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::Release()
{
	CRendererResources::DestroyDeferredMaps();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderer::EF_ReleaseDeferredData()
{
	if (CDeferredShading::IsValid())
		CDeferredShading::Instance().ReleaseData();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderer::Ef_AddDeferredGIClipVolume(const IRenderMesh* pClipVolume, const Matrix34& mxTransform)
{
	if (pClipVolume)
		CDeferredShading::Instance().AddGIClipVolume(const_cast<IRenderMesh*>(pClipVolume), mxTransform);
}
