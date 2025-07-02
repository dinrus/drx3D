// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_sector.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Инициализация сектора, отображение объектов.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjMan.h>

i32 CTerrainNode::GetMML(i32 nDist, i32 mmMin, i32 mmMax)
{
	i32k nStep = 48;

	for (i32 i = mmMin; i < mmMax; i++)
		if (nStep << i > nDist)
			return i;

	return mmMax;
}

void CTerrainNode::FillSectorHeightMapTextureData(Array2d<float> &arrHmData)
{
	FUNCTION_PROFILER_3DENGINE;

	i32 nTexSize = m_pTerrain->m_texCache[2].m_nDim;
	float fBoxSize = GetBBox().GetSize().x;
	arrHmData.Allocate(nTexSize);
	for (i32 x = 0; x < nTexSize; x++) for (i32 y = 0; y < nTexSize; y++)
	{
		arrHmData[x][y] = m_pTerrain->GetZApr(
			(float)m_nOriginX + fBoxSize * float(x) / nTexSize * (1.f + 1.f / (float)nTexSize),
			(float)m_nOriginY + fBoxSize * float(y) / nTexSize * (1.f + 1.f / (float)nTexSize));
	}
}

void CTerrainNode::SetLOD(const SRenderingPassInfo& passInfo)
{
	const float fDist = m_arrfDistance[passInfo.GetRecursiveLevel()];

	// Calculate Texture LOD
	if (passInfo.IsGeneralPass())
		m_cNodeNewTexMML = GetTextureLOD(fDist, passInfo);
}

u8 CTerrainNode::GetTextureLOD(float fDistance, const SRenderingPassInfo& passInfo)
{
	i32 nDiffTexDim = GetTerrain()->m_arrBaseTexInfos.m_TerrainTextureLayer[0].nSectorSizePixels;

	float fTexSizeK = nDiffTexDim ? float(nDiffTexDim) / float(GetTerrain()->GetTerrainTextureNodeSizeMeters()) : 1.f;

	i32 nMinLod = 0;

	if (m_pTerrain->m_texCache[0].m_eTexFormat == eTF_R8G8B8A8)
	{
		nMinLod++; // limit amount of texture data if in fall-back mode
	}

	u8 cNodeNewTexMML = GetMML(i32(fTexSizeK * 0.05f * (fDistance * passInfo.GetZoomFactor()) * GetFloatCVar(e_TerrainTextureLodRatio)), nMinLod, GetTerrain()->GetParentNode()->m_nTreeLevel);

	return cNodeNewTexMML;
}

i32 CTerrainNode::GetSecIndex()
{
	i32 nSectorSize = CTerrain::GetSectorSize() << m_nTreeLevel;
	i32 nSectorsTableSize = CTerrain::GetSectorsTableSize() >> m_nTreeLevel;
	return (m_nOriginX / nSectorSize) * nSectorsTableSize + (m_nOriginY / nSectorSize);
}