// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_node.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Узел ландшафта.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/VisAreas.h>

#define TERRAIN_NODE_CHUNK_VERSION 8

i32 CTerrainNode::Load(u8*& f, i32& nDataSize, EEndian eEndian, bool bSectorPalettes, SHotUpdateInfo* pExportInfo) { return Load_T(f, nDataSize, eEndian, bSectorPalettes, pExportInfo); }
i32 CTerrainNode::Load(FILE*& f, i32& nDataSize, EEndian eEndian, bool bSectorPalettes, SHotUpdateInfo* pExportInfo)  { return Load_T(f, nDataSize, eEndian, bSectorPalettes, pExportInfo); }

i32 CTerrainNode::FTell(u8*& f)                                                                                    { return -1; }
i32 CTerrainNode::FTell(FILE*& f)                                                                                     { return GetPak()->FTell(f); }

template<class T>
i32 CTerrainNode::Load_T(T*& f, i32& nDataSize, EEndian eEndian, bool bSectorPalettes, SHotUpdateInfo* pExportInfo)
{
	const AABB* pAreaBox = (pExportInfo && !pExportInfo->areaBox.IsReset()) ? &pExportInfo->areaBox : NULL;

	if (pAreaBox && !Overlap::AABB_AABB(GetBBox(), *pAreaBox))
		return 0;

	// set node data
	STerrainNodeChunk chunk;
	if (!CTerrain::LoadDataFromFile(&chunk, 1, f, nDataSize, eEndian))
		return 0;
	assert(chunk.nChunkVersion == TERRAIN_NODE_CHUNK_VERSION);
	if (chunk.nChunkVersion != TERRAIN_NODE_CHUNK_VERSION)
		return 0;

	// set error levels, bounding boxes and some flags
	m_boxHeigtmapLocal = chunk.boxHeightmap;
	m_boxHeigtmapLocal.max.z = max(m_boxHeigtmapLocal.max.z, GetTerrain()->GetWaterLevel());
	m_bHasHoles = chunk.bHasHoles;
	m_rangeInfo.fOffset = chunk.fOffset;
	m_rangeInfo.fRange = chunk.fRange;
	m_rangeInfo.nSize = chunk.nSize;
	SAFE_DELETE_ARRAY(m_rangeInfo.pHMData);
	m_rangeInfo.pSTPalette = NULL;
	m_rangeInfo.UpdateBitShift(GetTerrain()->m_nUnitsToSectorBitShift);
	m_rangeInfo.nModified = false;

	m_nNodeHMDataOffset = -1;
	if (m_rangeInfo.nSize)
	{
		m_rangeInfo.pHMData = new SHeightMapItem[m_rangeInfo.nSize * m_rangeInfo.nSize];
		m_nNodeHMDataOffset = FTell(f);
		if (!CTerrain::LoadDataFromFile(m_rangeInfo.pHMData, m_rangeInfo.nSize * m_rangeInfo.nSize, f, nDataSize, eEndian))
			return 0;

		CTerrain::LoadDataFromFile_FixAllignemt(f, nDataSize);
	}

	// load sector error metric, we load more floats for back compatibility with old levels
	// TODO: switch to single float on next file format change
	float* pTmp = new float[GetTerrain()->m_nUnitsToSectorBitShift];
	if (!CTerrain::LoadDataFromFile(pTmp, GetTerrain()->m_nUnitsToSectorBitShift, f, nDataSize, eEndian))
		return 0;
	m_geomError = pTmp[0];
	SAFE_DELETE_ARRAY(pTmp);

	// load used surf types
	for (i32 i = 0; i < m_lstSurfaceTypeInfo.Count(); i++)
		m_lstSurfaceTypeInfo[i].DeleteRenderMeshes(GetRenderer());
	m_lstSurfaceTypeInfo.Clear();

	m_lstSurfaceTypeInfo.PreAllocate(chunk.nSurfaceTypesNum, chunk.nSurfaceTypesNum);

	if (chunk.nSurfaceTypesNum)
	{
		u8* pTypes = new u8[chunk.nSurfaceTypesNum];  // TODO: avoid temporary allocations

		if (!CTerrain::LoadDataFromFile(pTypes, chunk.nSurfaceTypesNum, f, nDataSize, eEndian))
			return 0;

		for (i32 i = 0; i < m_lstSurfaceTypeInfo.Count() && i < SRangeInfo::e_hole; i++)
			m_lstSurfaceTypeInfo[i].pSurfaceType = &GetTerrain()->m_SSurfaceType[min((i32)pTypes[i], (i32)SRangeInfo::e_undefined)];

		if (!m_pChilds)
		{
			// For leaves, we reconstruct the sector's surface type palette from the surface type array.
			m_rangeInfo.pSTPalette = new uchar[SRangeInfo::e_palette_size];
			for (i32 i = 0; i < SRangeInfo::e_index_hole; i++)
				m_rangeInfo.pSTPalette[i] = SRangeInfo::e_undefined;

			m_rangeInfo.pSTPalette[SRangeInfo::e_index_hole] = SRangeInfo::e_hole;

			// Check which palette entries are actually used in the sector. We need to map all of them to global IDs.
			i32 nDataCount = m_rangeInfo.nSize * (m_rangeInfo.nSize - 1);
			i32 nUsedPaletteEntriesCount = 0;
			bool UsedPaletteEntries[SRangeInfo::e_palette_size];
			memset(UsedPaletteEntries, 0, sizeof(UsedPaletteEntries));
			if (m_rangeInfo.pHMData)
			{
				for (i32 i = 0; i < nDataCount; i++)
				{
					if ((i + 1) % m_rangeInfo.nSize)
					{
						SSurfaceTypeLocal si;
						si.DecodeFromUint32(m_rangeInfo.pHMData[i].surface, si);

						for (i32 s = 0; s < SSurfaceTypeLocal::kMaxSurfaceTypesNum; s++)
						{
							if(si.we[s])
							{
								UsedPaletteEntries[si.ty[s]] = true;
							}
						}
					}
				}
			}

			for (i32 i = 0; i < SRangeInfo::e_index_undefined; i++)
			{
				if (UsedPaletteEntries[i])
					nUsedPaletteEntriesCount++;
			}

			// Get the first nUsedPaletteEntriesCount entries from m_lstSurfaceTypeInfo and put them into the used entries
			// of the palette, skipping the unused entries.
			i32 nCurrentListEntry = 0;
			assert(nUsedPaletteEntriesCount <= m_lstSurfaceTypeInfo.Count());
			assert(m_rangeInfo.pHMData && bSectorPalettes);

			for (i32 i = 0; i < SRangeInfo::e_index_undefined; i++)
			{
				if (UsedPaletteEntries[i])
				{
					m_rangeInfo.pSTPalette[i] = pTypes[nCurrentListEntry];
					//!!! shielding a problem
					if (nCurrentListEntry + 1 < chunk.nSurfaceTypesNum)
						++nCurrentListEntry;
				}
			}
		}

		delete[] pTypes;

		CTerrain::LoadDataFromFile_FixAllignemt(f, nDataSize);
	}

	// count number of nodes saved
	i32 nNodesNum = 1;

	// process childs
	for (i32 i = 0; m_pChilds && i < 4; i++)
		nNodesNum += m_pChilds[i].Load_T(f, nDataSize, eEndian, bSectorPalettes, pExportInfo);

	return nNodesNum;
}

void CTerrainNode::ReleaseHoleNodes()
{
	if (!m_pChilds)
		return;

	if (m_bHasHoles == 2)
	{
		SAFE_DELETE_ARRAY(m_pChilds);
	}
	else
	{
		for (i32 i = 0; i < 4; i++)
			m_pChilds[i].ReleaseHoleNodes();
	}
}

i32 CTerrainNode::ReloadModifiedHMData(FILE* f)
{
	if (m_rangeInfo.nSize && m_nNodeHMDataOffset >= 0 && m_rangeInfo.nModified)
	{
		m_rangeInfo.nModified = false;

		GetPak()->FSeek(f, m_nNodeHMDataOffset, SEEK_SET);
		i32 nDataSize = m_rangeInfo.nSize * m_rangeInfo.nSize * sizeof(m_rangeInfo.pHMData[0]);
		if (!CTerrain::LoadDataFromFile(m_rangeInfo.pHMData, m_rangeInfo.nSize * m_rangeInfo.nSize, f, nDataSize, m_pTerrain->m_eEndianOfTexture))
			return 0;
	}

	// process childs
	for (i32 i = 0; m_pChilds && i < 4; i++)
		m_pChilds[i].ReloadModifiedHMData(f);

	return 1;
}

i32 CTerrainNode::GetData(byte*& pData, i32& nDataSize, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
	const AABB* pAreaBox = (pExportInfo && !pExportInfo->areaBox.IsReset()) ? &pExportInfo->areaBox : NULL;

	AABB boxWS = GetBBox();

	if (pAreaBox && !Overlap::AABB_AABB(boxWS, *pAreaBox))
		return 0;

	if (pData)
	{
		// get node data
		STerrainNodeChunk* pCunk = (STerrainNodeChunk*)pData;
		pCunk->nChunkVersion = TERRAIN_NODE_CHUNK_VERSION;
		pCunk->boxHeightmap = boxWS;
		pCunk->bHasHoles = m_bHasHoles;
		pCunk->fOffset = m_rangeInfo.fOffset;
		pCunk->fRange = m_rangeInfo.fRange;
		pCunk->nSize = m_rangeInfo.nSize;
		pCunk->nSurfaceTypesNum = m_lstSurfaceTypeInfo.Count();

		SwapEndian(*pCunk, eEndian);
		UPDATE_PTR_AND_SIZE(pData, nDataSize, sizeof(STerrainNodeChunk));

		// get heightmap data
		AddToPtr(pData, nDataSize, m_rangeInfo.pHMData, m_rangeInfo.nSize * m_rangeInfo.nSize, eEndian, true);

		// get heightmap error data, we store more floats for back compatibility
		// TODO: switch to single float on next file format change
		float* pTmp = new float[GetTerrain()->m_nUnitsToSectorBitShift];
		memset(pTmp, 0, sizeof(pTmp[0]) * GetTerrain()->m_nUnitsToSectorBitShift);
		pTmp[0] = m_geomError;
		AddToPtr(pData, nDataSize, pTmp, GetTerrain()->m_nUnitsToSectorBitShift, eEndian);
		SAFE_DELETE_ARRAY(pTmp);

		// get used surf types
		DRX_ASSERT(m_lstSurfaceTypeInfo.Count() <= SRangeInfo::e_max_surface_types);
		u8 arrTmp[SRangeInfo::e_max_surface_types] = { SRangeInfo::e_index_undefined };
		for (i32 i = 0; i < SRangeInfo::e_max_surface_types && i < m_lstSurfaceTypeInfo.Count(); i++)
			arrTmp[i] = m_lstSurfaceTypeInfo[i].pSurfaceType->ucThisSurfaceTypeId;

		// For a leaf sector, store its surface type palette.
		if (m_rangeInfo.pSTPalette)
		{
			assert(m_rangeInfo.pSTPalette[SRangeInfo::e_index_hole] == SRangeInfo::e_hole);

			// To ensure correct reconstruction of the sector's palette on loading, all entries present in the palette
			// must be *in the beginning* of arrTmp, *in the same order as in the palette*. The palette should not contain
			// any entries that are not included in arrTmp.

			// Check which palette entries are actually used in the sector. Editing can leave unused palette entries.
			// The check omits the last row and the last column of the sector (m_lstSurfaceTypeInfo does not include them).
			i32 nDataCount = m_rangeInfo.nSize * (m_rangeInfo.nSize - 1);
			i32 nUsedPaletteEntrisCount = 0;
			bool UsedPaletteEntries[SRangeInfo::e_palette_size];
			memset(UsedPaletteEntries, 0, sizeof(UsedPaletteEntries));
			for (i32 i = 0; i < nDataCount; i++)
			{
				if ((i + 1) % m_rangeInfo.nSize) // Ignore sector's last column
				{
					SSurfaceTypeLocal si;
					si.DecodeFromUint32(m_rangeInfo.pHMData[i].surface, si);

					for (i32 s = 0; s < SSurfaceTypeLocal::kMaxSurfaceTypesNum; s++)
					{
						if (si.we[s])
						{
							UsedPaletteEntries[si.ty[s]] = true;
						}
					}
				}
			}

			// Clear any palette entries whose indices do not occur in the data (set them to 127). They are not present in
			// m_lstSurfaceTypeInfo/arrTmp, so they cannot be stored (and there is no reason to store them anyway).
			for (i32 i = 0; i < SRangeInfo::e_index_undefined; i++)
			{
				if (!UsedPaletteEntries[i])
					m_rangeInfo.pSTPalette[i] = SRangeInfo::e_undefined;
				else
				{
					assert(m_rangeInfo.pSTPalette[i] != SRangeInfo::e_hole);
					nUsedPaletteEntrisCount++;
				}
			}

			assert(nUsedPaletteEntrisCount <= m_lstSurfaceTypeInfo.Count());

			// Reorder arrTmp so that the palette's entries are in the correct order in the beginning of the list.
			i32 nTargetListEntry = 0;
			for (i32 i = 0; i < SRangeInfo::e_index_undefined; i++)
			{
				if (UsedPaletteEntries[i])
				{
					assert(m_rangeInfo.pSTPalette[i] != SRangeInfo::e_hole);
					// Take the current palette entry and find the corresponding entry in arrTmp.
					i32 nCurrentListEntry = -1;
					for (i32 j = 0; nCurrentListEntry < 0 && j < SRangeInfo::e_index_undefined && j < m_lstSurfaceTypeInfo.Count(); j++)
					{
						if (arrTmp[j] == m_rangeInfo.pSTPalette[i])
							nCurrentListEntry = j;
					}

					// Swap the entries in arrTmp to move the entry where we want it.
					assert(nCurrentListEntry >= 0);
					PREFAST_ASSUME(nCurrentListEntry >= 0);
					if (nCurrentListEntry != nTargetListEntry)
						std::swap(arrTmp[nCurrentListEntry], arrTmp[nTargetListEntry]);
					nTargetListEntry++;
				}
			}
		}

		AddToPtr(pData, nDataSize, arrTmp, m_lstSurfaceTypeInfo.Count(), eEndian, true);
		//		memcpy(pData, arrTmp, m_lstSurfaceTypeInfo.Count()*sizeof(u8));
		//	UPDATE_PTR_AND_SIZE(pData,nDataSize,m_lstSurfaceTypeInfo.Count()*sizeof(u8));
	}
	else // just count size
	{
		nDataSize += sizeof(STerrainNodeChunk);
		nDataSize += m_rangeInfo.nSize * m_rangeInfo.nSize * sizeof(m_rangeInfo.pHMData[0]);
		while (nDataSize & 3)
			nDataSize++;
		nDataSize += GetTerrain()->m_nUnitsToSectorBitShift * sizeof(float);
		nDataSize += m_lstSurfaceTypeInfo.Count() * sizeof(u8);
		while (nDataSize & 3)
			nDataSize++;
	}

	// count number of nodes saved
	i32 nNodesNum = 1;

	// process childs
	for (i32 i = 0; m_pChilds && i < 4; i++)
		nNodesNum += m_pChilds[i].GetData(pData, nDataSize, eEndian, pExportInfo);

	return nNodesNum;
}

void CTerrainNode::UpdateGeomError()
{
	FUNCTION_PROFILER_3DENGINE;

	// all (x,y) values are INT in hm units

	i32k hmSize = i32(CTerrain::GetTerrainSize() * CTerrain::GetHeightMapUnitSizeInverted());
	i32k sectorSize = i32((CTerrain::GetSectorSize() << m_nTreeLevel) * CTerrain::GetHeightMapUnitSizeInverted());
	i32k meshDim = i32(float(CTerrain::GetSectorSize()) / CTerrain::GetHeightMapUnitSize());
	i32k x1 = i32(m_nOriginX * CTerrain::GetHeightMapUnitSizeInverted());
	i32k x2 = x1 + sectorSize;
	i32k y1 = i32(m_nOriginY * CTerrain::GetHeightMapUnitSizeInverted());
	i32k y2 = y1 + sectorSize;
	i32k halfStep = sectorSize / meshDim / 2;

	m_geomError = 0;

	if (!halfStep)
		return; // leaf node = no error

	i32 mapBorder = 8;

	for (i32 X = x1; X < x2; X += halfStep)
	{
		for (i32 Y = y1; Y < y2; Y += halfStep)
		{
			float z1 = .5f * GetTerrain()->GetZfromUnits(X - halfStep, Y - halfStep) + .5f * GetTerrain()->GetZfromUnits(X + halfStep, Y - halfStep);
			float z2 = .5f * GetTerrain()->GetZfromUnits(X - halfStep, Y + halfStep) + .5f * GetTerrain()->GetZfromUnits(X + halfStep, Y + halfStep);

			// skip map borders
			if (X < mapBorder || X > (hmSize - mapBorder) || Y < mapBorder || Y > (hmSize - mapBorder))
				continue;

			float interpolatedZ = .5f * z1 + .5f * z2;
			float realZ = GetTerrain()->GetZfromUnits(X, Y);
			float zDiff = fabs(realZ - interpolatedZ);

			if (zDiff > m_geomError)
				m_geomError = zDiff;
		}
	}
}
