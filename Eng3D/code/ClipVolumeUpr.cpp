// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/ClipVolume.h>
#include <drx3D/Eng3D/ClipVolumeUpr.h>
#include <drx3D/Eng3D/LightEntity.h>
#include <drx3D/Eng3D/FogVolumeRenderNode.h>

CClipVolumeUpr::~CClipVolumeUpr()
{
	DRX_ASSERT(m_clipVolumes.empty());
	DRX_ASSERT(m_deletedClipVolumes.empty());
}

IClipVolume* CClipVolumeUpr::CreateClipVolume()
{
	m_clipVolumes.emplace_back(new CClipVolume);
	return m_clipVolumes.back().m_pVolume;
}

bool CClipVolumeUpr::DeleteClipVolume(IClipVolume* pClipVolume)
{
	auto it = std::find(m_clipVolumes.begin(), m_clipVolumes.end(), static_cast<CClipVolume*>(pClipVolume));

	if (it != m_clipVolumes.end())
	{
		m_deletedClipVolumes.push_back(*it);
		m_clipVolumes.erase(it);

		return true;
	}

	return false;
}

bool CClipVolumeUpr::UpdateClipVolume(IClipVolume* pClipVolume, _smart_ptr<IRenderMesh> pRenderMesh, IBSPTree3D* pBspTree, const Matrix34& worldTM, u8 viewDistRatio, bool bActive, u32 flags, tukk szName)
{
	auto it = std::find(m_clipVolumes.begin(), m_clipVolumes.end(), static_cast<CClipVolume*>(pClipVolume));

	if (it != m_clipVolumes.end())
	{
		it->m_pVolume->Update(pRenderMesh, pBspTree, worldTM, viewDistRatio, flags);
		it->m_pVolume->SetName(szName);
		it->m_bActive = bActive;

		AABB volumeBBox = pClipVolume->GetClipVolumeBBox();
		Get3DEngine()->m_pObjUpr->ReregisterEntitiesInArea(&volumeBBox);
		return true;
	}

	return false;
}

void CClipVolumeUpr::PrepareVolumesForRendering(const SRenderingPassInfo& passInfo)
{
	FUNCTION_PROFILER_3DENGINE;

	// free clip volumes scheduled for deletion two frames ago: 
	// 1 frame for render thread latency, 1 frame for potentially scheduled MarkRNTmpDataPoolForReset
	TrimDeletedClipVolumes(passInfo.GetFrameID() - 2);

	const CCamera& cam = passInfo.GetCamera();

	// update all clipvolumes
	for (size_t i = 0; i < m_clipVolumes.size(); ++i)
	{
		SClipVolumeInfo& volInfo = m_clipVolumes[i];
		volInfo.m_pVolume->SetStencilRef(InactiveVolumeStencilRef);
		volInfo.m_updateFrameId = passInfo.GetFrameID();
		volInfo.m_currentViewDist = std::numeric_limits<float>::max();

		if (volInfo.m_bActive)
		{
			const AABB& volumeBox = volInfo.m_pVolume->GetClipVolumeBBox();
			const float volumeDistance = sqrt_tpl(Distance::Point_AABBSq(cam.GetPosition(), volumeBox)) * passInfo.GetZoomFactor();
			const float volumeMaxViewDist = volInfo.m_pVolume->GetMaxViewDist();
			if (volumeDistance < volumeMaxViewDist && cam.IsAABBVisible_F(volumeBox))
				volInfo.m_currentViewDist = volumeDistance;
		}
	}

	// sort by view dist and add to renderview
	{
		std::sort(m_clipVolumes.begin(), m_clipVolumes.end(), [](const SClipVolumeInfo& volA, const SClipVolumeInfo& volB)
		{
			return volA.m_currentViewDist < volB.m_currentViewDist;
		});

		for (const auto& volInfo : m_clipVolumes)
		{
			if (volInfo.m_currentViewDist == std::numeric_limits<float>::max())
				break;

			u8 nStencilRef = passInfo.GetIRenderView()->AddClipVolume(volInfo.m_pVolume);
			volInfo.m_pVolume->SetStencilRef(nStencilRef);
		}
	}
}

void CClipVolumeUpr::UpdateEntityClipVolume(const Vec3& pos, IRenderNode* pRenderNode)
{
	DRX_PROFILE_REGION(PROFILE_3DENGINE, "CClipVolumeUpr::UpdateEntityClipVolume");

	if (!pRenderNode)
		return;

	const auto pTempData = pRenderNode->m_pTempData.load();
	if (!pTempData)
		return;

	if (pTempData->userData.bClipVolumeAssigned && pTempData->userData.lastClipVolumePosition == pos)
		return;

	// user assigned clip volume
	CLightEntity* pLight = static_cast<CLightEntity*>(pRenderNode);
	if (pRenderNode->GetRenderNodeType() == eERType_Light && (pLight->GetLightProperties().m_Flags & DLF_HAS_CLIP_VOLUME) != 0)
	{
		for (i32 i = 1; i >= 0; --i)
		{
			if (CClipVolume* pVolume = static_cast<CClipVolume*>(pLight->GetLightProperties().m_pClipVolumes[i]))
				pTempData->SetClipVolume(pVolume, pos);
		}
	}
	else // assign by position
	{
		// Check if entity is in same clip volume as before
		IClipVolume* pPreviousVolume = pTempData->userData.m_pClipVolume;
		if (pPreviousVolume && (pPreviousVolume->GetClipVolumeFlags() & IClipVolume::eClipVolumeIsVisArea) == 0)
		{
			CClipVolume* pVolume = static_cast<CClipVolume*>(pPreviousVolume);
			if (pVolume->IsPointInsideClipVolume(pos))
			{
				pTempData->SetClipVolume(pVolume, pos);
				return;
			}
		}

		CClipVolume* pVolume = GetClipVolumeByPos(pos, pPreviousVolume);
		pTempData->SetClipVolume(pVolume, pos); // may be nullptr
	}
}

bool CClipVolumeUpr::IsClipVolumeRequired(IRenderNode* pRenderNode) const
{
	u32k NoClipVolumeLights = DLF_SUN | DLF_ATTACH_TO_SUN;

	const bool bForwardObject = (pRenderNode->m_nInternalFlags & IRenderNode::REQUIRES_FORWARD_RENDERING) != 0;
	const EERType ertype = pRenderNode->GetRenderNodeType();
	const bool bIsValidLight = ertype == eERType_Light &&
		(static_cast<CLightEntity*>(pRenderNode)->GetLightProperties().m_Flags & NoClipVolumeLights) == 0;
	const bool bIsValidFogVolume = (ertype == eERType_FogVolume) &&
		static_cast<CFogVolumeRenderNode*>(pRenderNode)->IsAffectsThisAreaOnly();

	return bIsValidLight || bForwardObject || bIsValidFogVolume;
}

CClipVolume* CClipVolumeUpr::GetClipVolumeByPos(const Vec3& pos, const IClipVolume* pIgnoreVolume) const
{
	for (size_t i = 0; i < m_clipVolumes.size(); ++i)
	{
		const SClipVolumeInfo& volInfo = m_clipVolumes[i];

		if (volInfo.m_bActive && volInfo.m_pVolume != pIgnoreVolume && volInfo.m_pVolume->IsPointInsideClipVolume(pos))
			return m_clipVolumes[i].m_pVolume;
	}

	return NULL;
}

void CClipVolumeUpr::GetMemoryUsage(class IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	for (size_t i = 0; i < m_clipVolumes.size(); ++i)
		pSizer->AddObject(m_clipVolumes[i].m_pVolume);
}

void CClipVolumeUpr::TrimDeletedClipVolumes(i32 trimFrameId)
{
	auto itEnd = std::partition(m_deletedClipVolumes.begin(), m_deletedClipVolumes.end(),
		[=](const SClipVolumeInfo& clipVolume)
	{
		return clipVolume.m_updateFrameId >= trimFrameId;
	});
	
	for (auto it = itEnd; it != m_deletedClipVolumes.end(); ++it)
		delete it->m_pVolume;

	m_deletedClipVolumes.erase(itEnd, m_deletedClipVolumes.end());
}
