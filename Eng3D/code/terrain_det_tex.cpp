// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_det_tex.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Текстуры деталей ландшафта.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/3dEngine.h>

void CTerrain::SetDetailLayerProperties(i32 nId, float fScaleX, float fScaleY,
                                        u8 ucProjAxis, tukk szSurfName,
                                        const PodArray<i32>& lstnVegetationGroups, IMaterial* pMat)
{
	if (nId >= 0 && nId < SRangeInfo::e_max_surface_types)
	{
		i32k nNameLength = strlen(szSurfName);
		i32k nDestNameLength = sizeof(m_SSurfaceType[nId].szName) - 1;
		if (nNameLength > nDestNameLength)
			Error("CTerrain::SetDetailLayerProperties: attempt to assign too long surface type name (%s)", szSurfName);
		drx_strcpy(m_SSurfaceType[nId].szName, szSurfName);
		m_SSurfaceType[nId].fScale = fScaleX;
		m_SSurfaceType[nId].ucDefProjAxis = ucProjAxis;
		m_SSurfaceType[nId].ucThisSurfaceTypeId = nId;
		m_SSurfaceType[nId].lstnVegetationGroups.Reset();
		m_SSurfaceType[nId].lstnVegetationGroups.AddList(lstnVegetationGroups);
		m_SSurfaceType[nId].pLayerMat = (CMatInfo*)pMat;
#ifndef SW_STRIP_LOADING_MSG
		if (m_SSurfaceType[nId].pLayerMat && !m_bEditor)
			CTerrain::Get3DEngine()->PrintMessage("  Layer %d - %s has material %s", nId, szSurfName, pMat->GetName());
#endif
	}
	else
	{
		Warning("CTerrain::SetDetailTextures: LayerId is out of range: %d: %s", nId, szSurfName);
		assert(!"CTerrain::SetDetailTextures: LayerId is out of range");
	}
}

void CTerrain::LoadSurfaceTypesFromXML(XmlNodeRef pDetTexTagList)
{
	LOADING_TIME_PROFILE_SECTION;

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Terrain, 0, "Surface types");

#ifndef SW_STRIP_LOADING_MSG
	CTerrain::Get3DEngine()->PrintMessage("Loading terrain layers ...");
#endif

	if (!pDetTexTagList)
		return;

	m_bProcVegetationInUse = false;

	for (i32 nId = 0; nId < pDetTexTagList->getChildCount() && nId < SRangeInfo::e_max_surface_types; nId++)
	{
		XmlNodeRef pDetLayer = pDetTexTagList->getChild(nId);
		IMaterialUpr* pMatMan = Get3DEngine()->GetMaterialUpr();
		tukk pMatName = pDetLayer->getAttr("DetailMaterial");
		_smart_ptr<IMaterial> pMat = pMatName[0] ? pMatMan->LoadMaterial(pMatName) : NULL;

		// material diffuse texture may be needed to generate terrain base texture (on CPU)
		if (gEnv->IsEditor() && pMat)
			pMat->SetKeepLowResSysCopyForDiffTex();

		float fScaleX = 1.f;
		pDetLayer->getAttr("DetailScaleX", fScaleX);
		float fScaleY = 1.f;
		pDetLayer->getAttr("DetailScaleY", fScaleY);
		u8 projAxis = pDetLayer->getAttr("ProjAxis")[0];

		if (!pMat || pMat == pMatMan->GetDefaultMaterial())
		{
			Error("CTerrain::LoadSurfaceTypesFromXML: Error loading material: %s", pMatName);
			pMat = pMatMan->GetDefaultTerrainLayerMaterial();
		}

		if (CMatInfo* pMatInfo = static_cast<CMatInfo*>(pMat.get()))
		{
			pMatInfo->m_fDefautMappingScale = fScaleX;
			Get3DEngine()->InitMaterialDefautMappingAxis(pMatInfo);
		}

		PodArray<i32> lstnVegetationGroups;
		for (i32 nGroup = 0; nGroup < pDetLayer->getChildCount(); nGroup++)
		{
			XmlNodeRef pVegetationGroup = pDetLayer->getChild(nGroup);
			i32 nVegetationGroupId = -1;
			if (pVegetationGroup->isTag("VegetationGroup") && pVegetationGroup->getAttr("Id", nVegetationGroupId))
			{
				lstnVegetationGroups.Add(nVegetationGroupId);
				m_bProcVegetationInUse = true;
			}
		}

		SetDetailLayerProperties(nId, fScaleX, fScaleY, projAxis, pDetLayer->getAttr("Name"), lstnVegetationGroups, pMat);
	}
}

void CTerrain::UpdateSurfaceTypes()
{
	if (GetParentNode())
		GetParentNode()->UpdateDetailLayersInfo(true);
}
