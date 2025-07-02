// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   statobjmanshadows.cpp
//  Version:     v1.00
//  Created:     2/6/2002 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Отношения отбрасывателей/получателей теней.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/VisAreas.h>
#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/CoreX/Math/AABBSV.h>
#include <drx3D/Eng3D/Vegetation.h>
#include <drx3D/Eng3D/Brush.h>
#include <drx3D/Eng3D/LightEntity.h>
#include <drx3D/Eng3D/ObjectsTree.h>

#pragma warning(push)
#pragma warning(disable: 4244)

bool IsAABBInsideHull(const SPlaneObject* pHullPlanes, i32 nPlanesNum, const AABB& aabbBox);

void CObjUpr::MakeShadowCastersList(CVisArea* pArea, const AABB& aabbReceiver, i32 dwAllowedTypes, i32 nRenderNodeFlags, Vec3 vLightPos, SRenderLight* pLight, ShadowMapFrustum* pFr, PodArray<SPlaneObject>* pShadowHull, const SRenderingPassInfo& passInfo)
{
	FUNCTION_PROFILER_3DENGINE;

	assert(pLight && vLightPos.len() > 1); // world space pos required

	pFr->ResetCasterLists();

	assert(CLightEntity::IsOnePassTraversalFrustum(pFr));
}

i32 CObjUpr::MakeStaticShadowCastersList(IRenderNode* pIgnoreNode, ShadowMapFrustum* pFrustum, const PodArray<struct SPlaneObject>* pShadowHull, i32 renderNodeExcludeFlags, i32 nMaxNodes, const SRenderingPassInfo& passInfo)
{
	i32k curLevel = 0;
	i32 nRemainingNodes = nMaxNodes;

	i32 nNumTrees = Get3DEngine()->m_pObjectsTree ? 1 : 0;
	if (CVisAreaUpr* pVisAreaUpr = GetVisAreaUpr())
		nNumTrees += pVisAreaUpr->m_lstVisAreas.size() + pVisAreaUpr->m_lstPortals.size();

	// objects tree first
	if (pFrustum->pShadowCacheData->mOctreePath[curLevel] == 0)
	{
		if (!Get3DEngine()->m_pObjectsTree->GetShadowCastersTimeSliced(pIgnoreNode, pFrustum, pShadowHull, renderNodeExcludeFlags, nRemainingNodes, curLevel + 1, passInfo))
			return nRemainingNodes;

		++pFrustum->pShadowCacheData->mOctreePath[curLevel];
	}

	// Vis Areas
	if (CVisAreaUpr* pVisAreaUpr = GetVisAreaUpr())
	{
		auto gatherCastersForVisAreas = [=, &nRemainingNodes](PodArray<CVisArea*>& visAreas)
		{
			for (i32 i = pFrustum->pShadowCacheData->mOctreePath[curLevel] - 1; i < visAreas.Count(); ++i, ++pFrustum->pShadowCacheData->mOctreePath[curLevel])
			{
				if (visAreas[i]->IsAffectedByOutLights() && visAreas[i]->IsObjectsTreeValid())
				{
					if (pFrustum->aabbCasters.IsReset() || Overlap::AABB_AABB(pFrustum->aabbCasters, *visAreas[i]->GetAABBox()))
					{
						if (!visAreas[i]->GetObjectsTree()->GetShadowCastersTimeSliced(pIgnoreNode, pFrustum, pShadowHull, renderNodeExcludeFlags, nRemainingNodes, curLevel + 1, passInfo))
							return false;
					}
				}
			}

			return true;
		};

		if (!gatherCastersForVisAreas(pVisAreaUpr->m_lstVisAreas))
			return nRemainingNodes;

		if (!gatherCastersForVisAreas(pVisAreaUpr->m_lstPortals))
			return nRemainingNodes;
	}

	// if we got here we processed every tree, so reset tree index
	pFrustum->pShadowCacheData->mOctreePath[curLevel] = 0;

	return nRemainingNodes;
}

#pragma warning(pop)
