// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   VisAreaCompile.cpp
//  Version:     v1.00
//  Created:     28/4/2005 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Загрузка/сохранение узла видимой области.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/VisAreas.h>

#define VISAREA_NODE_CHUNK_VERSION       2

#define VISAREA_FLAG_OCEAN_VISIBLE       BIT(0)
#define VISAREA_FLAG_IGNORE_SKY_COLOR    BIT(1)
#define VISAREA_FLAG_AFFECTEDBYOUTLIGHTS BIT(2)
#define VISAREA_FLAG_SKYONLY             BIT(3)
#define VISAREA_FLAG_DOUBLESIDE          BIT(4)
#define VISAREA_FLAG_USEININDOORS        BIT(5)
#define VISAREA_FLAG_IGNORE_GI           BIT(6)
#define VISAREA_FLAG_IGNORE_OUTDOOR_AO   BIT(7)

#define MAX_VIS_AREA_CONNECTIONS_NUM     30

struct SVisAreaChunk
{
	// cppcheck-suppress unusedStructMember
	i32    nChunkVersion;
	AABB   boxArea, boxStatics;
	// cppcheck-suppress unusedStructMember
	char   sName[32];
	// cppcheck-suppress unusedStructMember
	i32    nObjectsBlockSize;
	// cppcheck-suppress unusedStructMember
	i32    arrConnectionsId[MAX_VIS_AREA_CONNECTIONS_NUM];
	u32 dwFlags;
	float  fPortalBlending;
	Vec3   vConnNormals[2];
	// cppcheck-suppress unusedStructMember
	float  fHeight;
	Vec3   vAmbColor;
	// cppcheck-suppress unusedStructMember
	float  fViewDistRatio;

	AUTO_STRUCT_INFO_LOCAL;
};

#if ENGINE_ENABLE_COMPILATION
i32 CVisArea::GetData(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
	if (IsObjectsTreeValid())
	{
		GetObjectsTree()->CleanUpTree();
	}

	if (pData)
	{
		byte* pHead = pData;
		SaveHeader(pData, nDataSize);

		// save shape points num
		i32 nPointsCount = m_lstShapePoints.Count();
		SwapEndian(nPointsCount, eEndian);
		memcpy(pData, &nPointsCount, sizeof(nPointsCount));
		UPDATE_PTR_AND_SIZE(pData, nDataSize, sizeof(nPointsCount));

		// save shape points
		memcpy(pData, m_lstShapePoints.GetElements(), m_lstShapePoints.GetDataSize());
		SwapEndian((Vec3*)pData, m_lstShapePoints.Count(), eEndian);
		UPDATE_PTR_AND_SIZE(pData, nDataSize, m_lstShapePoints.GetDataSize());

		SaveObjetsTree(pData, nDataSize, pStatObjTable, pMatTable, pStatInstGroupTable, eEndian, pExportInfo, pHead);
	}
	else // just count size
	{
		nDataSize += sizeof(SVisAreaChunk);

		nDataSize += sizeof(i32);
		nDataSize += m_lstShapePoints.GetDataSize();

		if (IsObjectsTreeValid())
		{
			GetObjectsTree()->GetData(pData, nDataSize, NULL, NULL, NULL, eEndian, pExportInfo);
		}
	}
	return true;
}
#endif

i32 CVisArea::Load(byte*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
	return Load_T(f, nDataSizeLeft, pStatObjTable, pMatTable, eEndian, pExportInfo);
}

i32 CVisArea::Load(FILE*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
	return Load_T(f, nDataSizeLeft, pStatObjTable, pMatTable, eEndian, pExportInfo);
}

template<class T>
i32 CVisArea::Load_T(T*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
	i32 objBlockSize = 0;
	if (!LoadHeader_T(f, nDataSizeLeft, eEndian, objBlockSize))
		return 0;

	{
		// get shape points
		i32 nPointsCount = 0;
		if (!CTerrain::LoadDataFromFile(&nPointsCount, 1, f, nDataSizeLeft, eEndian))
			return 0;

		// get shape points
		m_lstShapePoints.PreAllocate(nPointsCount, nPointsCount);
		if (!CTerrain::LoadDataFromFile(m_lstShapePoints.GetElements(), nPointsCount, f, nDataSizeLeft, eEndian))
			return 0;

		UpdateClipVolume();
	}

	if (!LoadObjectsTree_T(f, nDataSizeLeft, pStatObjTable, pMatTable, eEndian, pExportInfo, objBlockSize))
		return 0;

	return true;
}

const AABB* CVisArea::GetStaticObjectAABBox() const
{
	return &m_boxStatics;
}
#if ENGINE_ENABLE_COMPILATION
i32 CVisArea::SaveHeader(byte*& pData, i32& nDataSize)
{
	// save node info
	SVisAreaChunk* pCunk = (SVisAreaChunk*)pData;
	pCunk->nChunkVersion = VISAREA_NODE_CHUNK_VERSION;
	pCunk->boxArea = m_boxArea;
	UpdateGeometryBBox();
	pCunk->boxStatics = m_boxStatics;
	memset(pCunk->sName, 0, sizeof(pCunk->sName));
	drx_strcpy(pCunk->sName, m_pVisAreaColdData->m_sName);
	memcpy(pCunk->vConnNormals, m_vConnNormals, sizeof(pCunk->vConnNormals));
	pCunk->fHeight = m_fHeight;
	pCunk->vAmbColor = m_vAmbientColor;
	pCunk->fViewDistRatio = m_fViewDistRatio;
	pCunk->fPortalBlending = m_fPortalBlending;

	pCunk->dwFlags = 0;
	if (m_bOceanVisible)
		pCunk->dwFlags |= VISAREA_FLAG_OCEAN_VISIBLE;
	if (m_bIgnoreSky)
		pCunk->dwFlags |= VISAREA_FLAG_IGNORE_SKY_COLOR;
	if (m_bAffectedByOutLights)
		pCunk->dwFlags |= VISAREA_FLAG_AFFECTEDBYOUTLIGHTS;
	if (m_bSkyOnly)
		pCunk->dwFlags |= VISAREA_FLAG_SKYONLY;
	if (m_bDoubleSide)
		pCunk->dwFlags |= VISAREA_FLAG_DOUBLESIDE;
	if (m_bUseInIndoors)
		pCunk->dwFlags |= VISAREA_FLAG_USEININDOORS;
	if (m_bIgnoreGI)
		pCunk->dwFlags |= VISAREA_FLAG_IGNORE_GI;
	if (m_bIgnoreOutdoorAO)
		pCunk->dwFlags |= VISAREA_FLAG_IGNORE_OUTDOOR_AO;

	// transform connections id into pointers
	PodArray<CVisArea*>& rAreas = IsPortal() ? GetVisAreaUpr()->m_lstVisAreas : GetVisAreaUpr()->m_lstPortals;
	for (i32 i = 0; i < MAX_VIS_AREA_CONNECTIONS_NUM; i++)
		pCunk->arrConnectionsId[i] = -1;
	for (i32 i = 0; i < m_lstConnections.Count() && i < MAX_VIS_AREA_CONNECTIONS_NUM; i++)
	{
		IVisArea* pArea = m_lstConnections[i];
		i32 nId;
		for (nId = 0; nId < rAreas.Count(); nId++)
		{
			if (pArea == rAreas[nId])
				break;
		}

		if (nId < rAreas.Count())
			pCunk->arrConnectionsId[i] = nId;
		else
		{
			pCunk->arrConnectionsId[i] = -1;
			assert(!"Undefined connction");
		}
	}

	UPDATE_PTR_AND_SIZE(pData, nDataSize, sizeof(SVisAreaChunk));

	return true;
}

i32 CVisArea::SaveObjetsTree(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo, byte* pHead)
{
	SVisAreaChunk* pCunk = (SVisAreaChunk*)pHead;

	// save objects
	pCunk->nObjectsBlockSize = 0;

	// get data from objects tree
	if (IsObjectsTreeValid())
	{
		byte* pTmp = NULL;
		GetObjectsTree()->GetData(pTmp, pCunk->nObjectsBlockSize, NULL, NULL, NULL, eEndian, pExportInfo);
		GetObjectsTree()->GetData(pData, nDataSize, pStatObjTable, pMatTable, pStatInstGroupTable, eEndian, pExportInfo); // UPDATE_PTR_AND_SIZE is inside
	}

	SwapEndian(*pCunk, eEndian);

	return true;
}
#endif
template<class T>
i32 CVisArea::LoadHeader_T(T*& f, i32& nDataSizeLeft, EEndian eEndian, i32& objBlockSize)
{
	SVisAreaChunk chunk;
	if (!CTerrain::LoadDataFromFile(&chunk, 1, f, nDataSizeLeft, eEndian))
		return 0;

	assert(chunk.nChunkVersion == VISAREA_NODE_CHUNK_VERSION);
	if (chunk.nChunkVersion != VISAREA_NODE_CHUNK_VERSION)
		return 0;

	// get area info
	m_boxArea = chunk.boxArea;
	m_boxStatics = chunk.boxStatics;
	chunk.sName[sizeof(chunk.sName) - 1] = 0;
	drx_strcpy(m_pVisAreaColdData->m_sName, chunk.sName);
	m_bThisIsPortal = strstr(m_pVisAreaColdData->m_sName, "portal") != 0;
	m_bIgnoreSky = (strstr(m_pVisAreaColdData->m_sName, "ignoresky") != 0) || ((chunk.dwFlags & VISAREA_FLAG_IGNORE_SKY_COLOR) != 0);
	memcpy(m_vConnNormals, chunk.vConnNormals, sizeof(m_vConnNormals));
	m_fHeight = chunk.fHeight;
	m_vAmbientColor = chunk.vAmbColor;
	m_fViewDistRatio = chunk.fViewDistRatio;
	m_fPortalBlending = chunk.fPortalBlending;

	if (chunk.dwFlags == u32(-1))
		chunk.dwFlags = 0;
	m_bOceanVisible = (chunk.dwFlags & VISAREA_FLAG_OCEAN_VISIBLE) != 0;

	m_bAffectedByOutLights = (chunk.dwFlags & VISAREA_FLAG_AFFECTEDBYOUTLIGHTS) != 0;
	m_bSkyOnly = (chunk.dwFlags & VISAREA_FLAG_SKYONLY) != 0;
	m_bDoubleSide = (chunk.dwFlags & VISAREA_FLAG_DOUBLESIDE) != 0;
	m_bUseInIndoors = (chunk.dwFlags & VISAREA_FLAG_USEININDOORS) != 0;
	m_bIgnoreGI = (chunk.dwFlags & VISAREA_FLAG_IGNORE_GI) != 0;
	m_bIgnoreOutdoorAO = (chunk.dwFlags & VISAREA_FLAG_IGNORE_OUTDOOR_AO) != 0;

	objBlockSize = chunk.nObjectsBlockSize;

	// convert connections id into pointers
	PodArray<CVisArea*>& rAreas = IsPortal() ? GetVisAreaUpr()->m_lstVisAreas : GetVisAreaUpr()->m_lstPortals;
	for (i32 i = 0; i < MAX_VIS_AREA_CONNECTIONS_NUM && rAreas.Count(); i++)
	{
		assert(chunk.arrConnectionsId[i] < rAreas.Count());
		if (chunk.arrConnectionsId[i] >= 0)
			m_lstConnections.Add(rAreas[chunk.arrConnectionsId[i]]);
	}

	return true;
}

template<class T>
i32 CVisArea::LoadObjectsTree_T(T*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo, i32k objBlockSize)
{
	// mark tree as invalid since new visarea was just added
	SAFE_DELETE(GetVisAreaUpr()->m_pAABBTree);

	AABB* pBox = (pExportInfo && !pExportInfo->areaBox.IsReset()) ? &pExportInfo->areaBox : NULL;

	// load content of objects tree
	if (!m_bEditor && objBlockSize > 4)
	{
		i32 nCurDataSize = nDataSizeLeft;
		if (nCurDataSize > 0)
		{
			if (!IsObjectsTreeValid())
			{
				SetObjectsTree( COctreeNode::Create(m_boxArea, this) );
			}

			if (pExportInfo != NULL && pExportInfo->pVisibleLayerMask != NULL && pExportInfo->pLayerIdTranslation)
			{
				SLayerVisibility visInfo;
				visInfo.pLayerVisibilityMask = pExportInfo->pVisibleLayerMask;
				visInfo.pLayerIdTranslation = pExportInfo->pLayerIdTranslation;
				GetObjectsTree()->Load(f, nDataSizeLeft, pStatObjTable, pMatTable, eEndian, pBox, &visInfo);
			}
			else
			{
				GetObjectsTree()->Load(f, nDataSizeLeft, pStatObjTable, pMatTable, eEndian, pBox, NULL);
			}

			assert(nDataSizeLeft == (nCurDataSize - objBlockSize));
		}
	}
	else if (objBlockSize > 0)
	{
		CTerrain::LoadDataFromFile_Seek(objBlockSize, f, nDataSizeLeft, eEndian);
	}

	UpdateOcclusionFlagInTerrain();

	return true;
}

VisAreaGUID CVisArea::GetGUIDFromFile(byte* f, EEndian eEndian)
{
	SVisAreaChunk* pChunk = (SVisAreaChunk*)f;
	SwapEndian(pChunk, sizeof(SVisAreaChunk), eEndian);

	assert(pChunk->nChunkVersion == VISAREA_NODE_CHUNK_VERSION);
	if (pChunk->nChunkVersion != VISAREA_NODE_CHUNK_VERSION)
		return 0;

	VisAreaGUID guid = *(VisAreaGUID*)(f + sizeof(SVisAreaChunk));
	SwapEndian(&guid, sizeof(VisAreaGUID), eEndian);
	return guid;
}

#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Eng3D/VisAreaCompile_info.h>
