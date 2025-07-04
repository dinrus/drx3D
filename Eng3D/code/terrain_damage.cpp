// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_damage.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Деформации ландшафта.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/Vegetation.h>

void CTerrain::MakeCrater(Vec3 vExploPos, float fExploRadius)
{
	m_StoredModifications.PushModification(vExploPos, fExploRadius);
}

bool CTerrain::RemoveObjectsInArea(Vec3 vExploPos, float fExploRadius)
{
	bool bEverythingDeleted = true;

	// get near sectors
	//  PodArray<CTerrainNode*> lstNearSecInfos;
	Vec3 vRadius(fExploRadius, fExploRadius, fExploRadius);
	//	IntersectWithBox(AABB(vExploPos-vRadius,vExploPos+vRadius),&lstNearSecInfos,false);

	PodArray<SRNInfo> lstEntities;
	const AABB cExplosionBox(vExploPos - vRadius, vExploPos + vRadius);
	Get3DEngine()->MoveObjectsIntoListGlobal(&lstEntities, &cExplosionBox, false, true, true, true);

	// remove small objects around
	//i32 s;
	//  for( s=0; s<lstNearSecInfos.Count(); s++)
	{
		//  CTerrainNode * pSecInfo = lstNearSecInfos[s];
		//		for(i32 nListId=STATIC_OBJECTS; nListId<=PROC_OBJECTS; nListId++)
		for (i32 i = 0; i < lstEntities.Count(); i++)
		{
			IRenderNode* pRenderNode = lstEntities[i].pNode;
			AABB entBox = pRenderNode->GetBBox();
			float fEntRadius = entBox.GetRadius();
			Vec3 vEntCenter = pRenderNode->GetBBox().GetCenter();
			float fDist = vExploPos.GetDistance(vEntCenter);
			if (fDist < fExploRadius + fEntRadius &&
			    Overlap::Sphere_AABB(Sphere(vExploPos, fExploRadius), entBox))
			{
				if (fDist >= fExploRadius)
				{
					//
					Matrix34A objMat;
					CStatObj* pStatObj = (CStatObj*)pRenderNode->GetEntityStatObj(0, &objMat);
					if (!pStatObj)
						continue;
					objMat.Invert();
					//assert(0);
					Vec3 vOSExploPos = objMat.TransformPoint(vExploPos);

					Vec3 vScaleTest(0, 0, 1.f);
					vScaleTest = objMat.TransformVector(vScaleTest);
					float fObjScaleInv = vScaleTest.len();

					if (!pStatObj->IsSphereOverlap(Sphere(vOSExploPos, fExploRadius * fObjScaleInv)))
						continue;
				}

				if (pRenderNode->GetRenderNodeType() == eERType_Vegetation && (fEntRadius < (fExploRadius - fDist * .5f)) &&
				    !((CVegetation*)pRenderNode)->IsBreakable())
				{
					// remove this object
					Get3DEngine()->UnRegisterEntityAsJob(pRenderNode);
					pRenderNode->Dephysicalize();

					if (!(pRenderNode->m_dwRndFlags & ERF_PROCEDURAL))
						Get3DEngine()->m_lstKilledVegetations.Add(pRenderNode);
				}
				else
				{
					// if something was impossible to destroy - disable deformation
					bEverythingDeleted = false;
				}
			}
		}
	}

	return bEverythingDeleted;
}

void CTerrain::GetObjectsAround(Vec3 vExploPos, float fExploRadius, PodArray<SRNInfo>* pEntList, bool bSkip_ERF_NO_DECALNODE_DECALS, bool bSkipDynamicObjects)
{
	assert(pEntList);

	//	if(!GetParentNode(nSID))
	//	return;

	// get intersected outdoor sectors
	//	static PodArray<CTerrainNode*> lstSecotors; lstSecotors.Clear();
	AABB aabbBox(vExploPos - Vec3(fExploRadius, fExploRadius, fExploRadius), vExploPos + Vec3(fExploRadius, fExploRadius, fExploRadius));

	Get3DEngine()->MoveObjectsIntoListGlobal(pEntList, &aabbBox, false, true, bSkip_ERF_NO_DECALNODE_DECALS, bSkipDynamicObjects);

	/*	GetParentNode(nSID)->IntersectTerrainAABB(aabbBox, lstSecotors);

	   // find static objects around
	   for( i32 s=0; s<lstSecotors.Count(); s++)
	   {
	    CTerrainNode * pSecInfo = lstSecotors[s];
	    for(i32 i=0; i<pSecInfo->m_lstEntities[STATIC_OBJECTS].Count(); i++)
	    {
	      IRenderNode * pRenderNode =  pSecInfo->m_lstEntities[STATIC_OBJECTS][i].pNode;
	      if(bSkip_ERF_NO_DECALNODE_DECALS && pRenderNode->GetRndFlags()&ERF_NO_DECALNODE_DECALS)
	        continue;

	      if(pRenderNode->GetRenderNodeType() == eERType_Decal)
	        continue;

	      const AABB & box = pRenderNode->GetBBox();
	      if(Overlap::Sphere_AABB(Sphere(vExploPos,fExploRadius), box))
	        if(pEntList->Find(pRenderNode)<0)
	          pEntList->Add(pRenderNode);
	    }
	   }*/
}

i32 CTerrain::ReloadModifiedHMData(FILE* f)
{
	ResetHeightMapCache();
	return GetParentNode() ? GetParentNode()->ReloadModifiedHMData(f) : 0;
}
