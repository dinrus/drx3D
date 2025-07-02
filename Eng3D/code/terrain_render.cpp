// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_render.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Отрисовка видимых секторов.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/terrain_sector.h>

i32 CTerrain::GetDetailTextureMaterials(IMaterial* materials[])
{
	i32 materialNumber = 0;

	uchar szProj[] = "XYZ";

	for (i32 s = 0; s < SRangeInfo::e_hole; s++)
	{
		SSurfaceType* pSurf = &m_SSurfaceType[s];

		if (pSurf->HasMaterial())
		{
			for (i32 p = 0; p < 3; p++)
			{
				if (IMaterial* pMat = pSurf->GetMaterialOfProjection(szProj[p]))
				{
					if (materials)
					{
						materials[materialNumber] = pMat;
					}
					materialNumber++;
				}
			}
		}
	}

	return materialNumber;
}

void CTerrain::DrawVisibleSectors(const SRenderingPassInfo& passInfo)
{
	FUNCTION_PROFILER_3DENGINE;

	if (!passInfo.RenderTerrain() || !Get3DEngine()->m_bShowTerrainSurface)
		return;

	for (STerrainVisItem node : m_lstVisSectors)
	{
		node.first->RenderNodeHeightmap(passInfo, node.second);
	}
}

void CTerrain::UpdateSectorMeshes(const SRenderingPassInfo& passInfo)
{
	FUNCTION_PROFILER_3DENGINE;

	m_pTerrainUpdateDispatcher->SyncAllJobs(false, passInfo);
}
