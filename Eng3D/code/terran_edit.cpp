// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terran_edit.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Добавить/удалить статические объекты, изменить hmap
//              (используется редактором).
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/StatObj.h>
#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/Eng3D/Vegetation.h>
#include <drx3D/Eng3D/PolygonClipContext.h>
#include <drx3D/Eng3D/RoadRenderNode.h>
#include <drx3D/Eng3D/MergedMeshRenderNode.h>
#include <drx3D/Eng3D/MergedMeshGeometry.h>
#include <drx3D/Eng3D/Brush.h>
#include <drx3D/Eng3D/DecalRenderNode.h>
#include <drx3D/Eng3D/WaterVolumeRenderNode.h>
#include <drx3D/Entity/IEntitySystem.h>

#define RAD2BYTE(x) ((x)* 255.0f / float(g_PI2))
#define BYTE2RAD(x) ((x)* float(g_PI2) / 255.0f)

//////////////////////////////////////////////////////////////////////////
IRenderNode* CTerrain::AddVegetationInstance(i32 nStaticGroupIndex, const Vec3& vPos, const float fScale, u8 ucBright,
                                             u8 angle, u8 angleX, u8 angleY)
{
	if (vPos.x <= 0 || vPos.y <= 0 || vPos.x >= CTerrain::GetTerrainSize() || vPos.y >= CTerrain::GetTerrainSize() || fScale * VEGETATION_CONV_FACTOR < 1.f)
		return 0;
	IRenderNode* renderNode = NULL;

	if (nStaticGroupIndex < 0 || nStaticGroupIndex >= GetObjUpr()->m_lstStaticTypes.Count())
	{
		return 0;
	}

	StatInstGroup& group = GetObjUpr()->m_lstStaticTypes[nStaticGroupIndex];
	if (!group.GetStatObj())
	{
		Warning("I3DEngine::AddStaticObject: Attempt to add object of undefined type");
		return 0;
	}

	bool bValidForMerging = group.GetStatObj()->GetRenderTrisCount() < GetCVars()->e_MergedMeshesMaxTriangles;

	if (!group.bAutoMerged || !bValidForMerging)
	{
		CVegetation* pEnt = (CVegetation*)Get3DEngine()->CreateRenderNode(eERType_Vegetation);
		pEnt->SetScale(fScale);
		pEnt->m_vPos = vPos;
		pEnt->SetStatObjGroupIndex(nStaticGroupIndex);
		pEnt->m_ucAngle = angle;
		pEnt->m_ucAngleX = angleX;
		pEnt->m_ucAngleY = angleY;
		pEnt->CalcBBox();

		float fEntLengthSquared = pEnt->GetBBox().GetSize().GetLengthSquared();
		if (fEntLengthSquared > MAX_VALID_OBJECT_VOLUME || !_finite(fEntLengthSquared) || fEntLengthSquared <= 0)
		{
			Warning("CTerrain::AddVegetationInstance: Object has invalid bbox: %s,%s, GetRadius() = %.2f",
			        pEnt->GetName(), pEnt->GetEntityClassName(), sqrt_tpl(fEntLengthSquared) * 0.5f);
		}

		if (group.bAutoMerged && !bValidForMerging)
		{
			static i32 nLastFrameId = 0;
			if (nLastFrameId != gEnv->pRenderer->GetFrameID()) // log spam prevention
			{
				Warning("%s: Vegetation object is not suitable for merging because of too many polygons: %s (%d triangles)", __FUNCTION__, group.GetStatObj()->GetFilePath(), group.GetStatObj()->GetRenderTrisCount());
				nLastFrameId = gEnv->pRenderer->GetFrameID();
			}
		}

		pEnt->Physicalize();
		Get3DEngine()->RegisterEntity(pEnt);
		renderNode = pEnt;
	}
	else
	{
		SProcVegSample sample;
		sample.InstGroupId = nStaticGroupIndex;
		sample.pos = vPos;
		sample.scale = (u8)SATURATEB(fScale * VEGETATION_CONV_FACTOR);
		Matrix33 mr = Matrix33::CreateRotationXYZ(Ang3(BYTE2RAD(angleX), BYTE2RAD(angleY), BYTE2RAD(angle)));

		if (group.GetAlignToTerrainAmount() != 0.f)
		{
			Matrix33 m33;
			GetTerrain()->GetTerrainAlignmentMatrix(vPos, group.GetAlignToTerrainAmount(), m33);
			sample.q = Quat(m33) * Quat(mr);
		}
		else
		{
			sample.q = Quat(mr);
		}
		sample.q.NormalizeSafe();
		renderNode = m_pMergedMeshesUpr->AddInstance(sample);
	}

	return renderNode;
}

void CTerrain::RemoveAllStaticObjects()
{
	if (!Get3DEngine()->m_pObjectsTree)
		return;

	PodArray<SRNInfo> lstObjects;
	Get3DEngine()->m_pObjectsTree->MoveObjectsIntoList(&lstObjects, NULL);

	for (i32 i = 0; i < lstObjects.Count(); i++)
	{
		IRenderNode* pNode = lstObjects.GetAt(i).pNode;
		switch (pNode->GetRenderNodeType())
		{
		case eERType_Vegetation:
			if (!(pNode->GetRndFlags() & ERF_PROCEDURAL))
				pNode->ReleaseNode();
			break;
		case eERType_MergedMesh:
			pNode->ReleaseNode();
			break;
		}
	}
}

void CTerrain::HighlightTerrain(i32 x1, i32 y1, i32 x2, i32 y2)
{
	// Input dimensions are in units.
	float unitSize = GetHeightMapUnitSize();
	float x1u = (float)x1 * unitSize, x2u = (float)x2 * unitSize;
	float y1u = (float)y1 * unitSize, y2u = (float)y2 * unitSize;

	ColorB clrRed(255, 0, 0);
	IRenderAuxGeom* prag = GetISystem()->GetIRenderer()->GetIRenderAuxGeom();

	for (i32 x = x1; x < x2; x += 1)
	{
		float xu = (float)x * unitSize;
		prag->DrawLine(Vec3(y1u, xu, GetZfromUnits(y1, x) + 0.5f), clrRed, Vec3(y1u, xu + unitSize, GetZfromUnits(y1, x + 1) + 0.5f), clrRed, 5.0f);
		prag->DrawLine(Vec3(y2u, xu, GetZfromUnits(y2, x) + 0.5f), clrRed, Vec3(y2u, xu + unitSize, GetZfromUnits(y2, x + 1) + 0.5f), clrRed, 5.0f);
	}
	for (i32 y = y1; y < y2; y += 1)
	{
		float yu = (float)y * unitSize;
		prag->DrawLine(Vec3(yu, x1u, GetZfromUnits(y, x1) + 0.5f), clrRed, Vec3(yu + unitSize, x1u, GetZfromUnits(y + 1, x1) + 0.5f), clrRed, 5.0f);
		prag->DrawLine(Vec3(yu, x2u, GetZfromUnits(y, x2) + 0.5f), clrRed, Vec3(yu + unitSize, x2u, GetZfromUnits(y + 1, x2) + 0.5f), clrRed, 5.0f);
	}
}

void CTerrain::SetTerrainElevation(i32 X1, i32 Y1, i32 nSizeX, i32 nSizeY, float* pTerrainBlock,
                                   SSurfaceTypeItem* pSurfaceData, i32 nSurfOrgX, i32 nSurfOrgY, i32 nSurfSizeX, i32 nSurfSizeY,
                                   u32* pResolMap, i32 nResolMapSizeX, i32 nResolMapSizeY)
{
#ifndef _RELEASE

	//LOADING_TIME_PROFILE_SECTION;
	FUNCTION_PROFILER_3DENGINE;

	float fStartTime = GetCurAsyncTimeSec();
	float unitSize = CTerrain::GetHeightMapUnitSize();
	i32 nHmapSize = i32(CTerrain::GetTerrainSize() / unitSize);

	ResetHeightMapCache();

	// everything is in units in this function

	assert(nSizeX == nSizeY);
	assert(X1 == ((X1 >> m_nUnitsToSectorBitShift) << m_nUnitsToSectorBitShift));
	assert(Y1 == ((Y1 >> m_nUnitsToSectorBitShift) << m_nUnitsToSectorBitShift));
	assert(nSizeX == ((nSizeX >> m_nUnitsToSectorBitShift) << m_nUnitsToSectorBitShift));
	assert(nSizeY == ((nSizeY >> m_nUnitsToSectorBitShift) << m_nUnitsToSectorBitShift));

	if (X1 < 0 || Y1 < 0 || X1 + nSizeX > nHmapSize || Y1 + nSizeY > nHmapSize)
	{
		Warning("CTerrain::SetTerrainHeightMapBlock: (X1,Y1) values out of range");
		return;
	}

	AABB aabb = Get3DEngine()->m_pObjectsTree->GetNodeBox();

	i32 x0 = (i32)(aabb.min.x * m_fInvUnitSize);
	i32 y0 = (i32)(aabb.min.y * m_fInvUnitSize);

	if (!GetParentNode())
		BuildSectorsTree(false);

	Array2d<struct CTerrainNode*>& sectorLayer = m_arrSecInfoPyramid[0];

	i32 rangeX1 = max(0, X1 >> m_nUnitsToSectorBitShift);
	i32 rangeY1 = max(0, Y1 >> m_nUnitsToSectorBitShift);
	i32 rangeX2 = min(sectorLayer.GetSize(), (X1 + nSizeX) >> m_nUnitsToSectorBitShift);
	i32 rangeY2 = min(sectorLayer.GetSize(), (Y1 + nSizeY) >> m_nUnitsToSectorBitShift);

	std::vector<float> rawHeightmap;

	AABB modifiedArea;
	modifiedArea.Reset();

	for (i32 rangeX = rangeX1; rangeX < rangeX2; rangeX++)
	{
		for (i32 rangeY = rangeY1; rangeY < rangeY2; rangeY++)
		{
			CTerrainNode* pTerrainNode = sectorLayer[rangeX][rangeY];

			i32 x1 = x0 + (rangeX << m_nUnitsToSectorBitShift);
			i32 y1 = y0 + (rangeY << m_nUnitsToSectorBitShift);
			i32 x2 = x0 + ((rangeX + 1) << m_nUnitsToSectorBitShift);
			i32 y2 = y0 + ((rangeY + 1) << m_nUnitsToSectorBitShift);

			float fMaxTexelSizeMeters = -1;

			if (pResolMap)
			{
				// get max allowed texture resolution here
				i32 nResMapX = (nResolMapSizeX * (x1 / 2 + x2 / 2) / nHmapSize);
				i32 nResMapY = (nResolMapSizeY * (y1 / 2 + y2 / 2) / nHmapSize);
				nResMapX = CLAMP(nResMapX, 0, nResolMapSizeX - 1);
				nResMapY = CLAMP(nResMapY, 0, nResolMapSizeY - 1);
				i32 nTexRes = pResolMap[nResMapY + nResMapX * nResolMapSizeY];
				i32 nResTileSizeMeters = GetTerrainSize() / nResolMapSizeX;
				fMaxTexelSizeMeters = (float)nResTileSizeMeters / (float)nTexRes;
			}

			i32 nStep = 1 << 0 /*nGeomMML*/;
			i32 nMaxStep = 1 << m_nUnitsToSectorBitShift;

			SRangeInfo& ri = pTerrainNode->m_rangeInfo;

			if (ri.nSize != nMaxStep / nStep + 1)
			{
				delete[] ri.pHMData;
				ri.nSize = nMaxStep / nStep + 1;
				ri.pHMData = new SHeightMapItem[ri.nSize * ri.nSize];
				ri.UpdateBitShift(m_nUnitsToSectorBitShift);
			}

			assert(ri.pHMData);

			if (rawHeightmap.size() < (size_t)ri.nSize * ri.nSize)
			{
				rawHeightmap.resize(ri.nSize * ri.nSize);
			}

			// find min/max
			float fMin = pTerrainBlock[CLAMP(x1, 1, nHmapSize - 1) * nHmapSize + CLAMP(y1, 1, nHmapSize - 1)];
			float fMax = fMin;

			// fill height map data array in terrain node, all in units
			for (i32 x = x1; x <= x2; x += nStep)
			{
				for (i32 y = y1; y <= y2; y += nStep)
				{
					i32 ix = min(nHmapSize - 1, x);
					i32 iy = min(nHmapSize - 1, y);

					float fHeight = pTerrainBlock[ix * nHmapSize + iy];

					if (fHeight > fMax) fMax = fHeight;
					if (fHeight < fMin) fMin = fHeight;

					// TODO: add proper fp16/fp32 support

					i32 x_local = (x - x1) / nStep;
					i32 y_local = (y - y1) / nStep;

					rawHeightmap[x_local * ri.nSize + y_local] = fHeight;
				}
			}

			// reserve some space for in-game deformations
			fMin = max(0.f, fMin - TERRAIN_DEFORMATION_MAX_DEPTH);

			pTerrainNode->m_bHMDataIsModified = (ri.fOffset != fMin);

			ri.fOffset = fMin;
			ri.fRange = (fMax - fMin) / float(0x0FFF);

			pTerrainNode->m_boxHeigtmapLocal.min.Set((float)((x1 - x0) * unitSize), (float)((y1 - y0) * unitSize), fMin);
			pTerrainNode->m_boxHeigtmapLocal.max.Set((float)((x2 - x0) * unitSize), (float)((y2 - y0) * unitSize), max(fMax, GetWaterLevel()));

			for (i32 x = x1; x <= x2; x += nStep)
			{
				for (i32 y = y1; y <= y2; y += nStep)
				{
					i32 xlocal = (x - x1) / nStep;
					i32 ylocal = (y - y1) / nStep;
					i32 nCellLocal = xlocal * ri.nSize + ylocal;

					u32 height = ri.fRange ? i32((rawHeightmap[nCellLocal] - fMin) / ri.fRange) : 0;

					assert(x >= x0 + X1 && y >= y0 + Y1 && x <= x0 + X1 + nSurfSizeX && y <= y0 + Y1 + nSurfSizeY && pSurfaceData);

					i32 nSurfX = (x - nSurfOrgX);
					i32 nSurfY = (y - nSurfOrgY);

					SSurfaceTypeLocal dst;

					assert(nSurfX >= 0 && nSurfY >= 0 && pSurfaceData);

					nSurfX = min(nSurfX, nSurfSizeX - 1);
					nSurfY = min(nSurfY, nSurfSizeY - 1);

					{
						i32 nSurfCell = nSurfX * nSurfSizeY + nSurfY;
						assert(nSurfCell >= 0 && nSurfCell < nSurfSizeX * nSurfSizeY);

						const SSurfaceTypeItem& src = pSurfaceData[nSurfCell];

						if (src.GetHole())
						{
							dst = SRangeInfo::e_index_hole;
						}
						else
						{
							// read all 3 types, remap to local
							for (i32 i = 0; i < SSurfaceTypeLocal::kMaxSurfaceTypesNum; i++)
							{
								dst.we[i] = src.we[i] / 16;

								if (src.we[i] || !i)
								{
									dst.ty[i] = (byte)ri.GetLocalSurfaceTypeID(src.ty[i]);
								}
							}

							dst.we[0] = SATURATEB(15 - dst.we[1] - dst.we[2]);
						}
					}

					SHeightMapItem nNewValue;
					nNewValue.height = height;

					// pack SSurfTypeItem into u32
					u32 surface = 0;
					SSurfaceTypeLocal::EncodeIntoUint32(dst, surface);
					nNewValue.surface = surface;

					if (nNewValue != ri.pHMData[nCellLocal])
					{
						ri.pHMData[nCellLocal] = nNewValue;

						pTerrainNode->m_bHMDataIsModified = true;
					}
				}
			}
		}
	}

	for (i32 rangeX = rangeX1; rangeX < rangeX2; rangeX++)
	{
		for (i32 rangeY = rangeY1; rangeY < rangeY2; rangeY++)
		{
			CTerrainNode* pTerrainNode = sectorLayer[rangeX][rangeY];

			// re-init surface types info and update vert buffers in entire brunch
			if (GetParentNode())
			{
				CTerrainNode* pNode = pTerrainNode;
				while (pNode)
				{
					pNode->m_geomError = kGeomErrorNotSet;

					pNode->ReleaseHeightMapGeometry();
					pNode->RemoveProcObjects(false, false);
					pNode->UpdateDetailLayersInfo(false);

					// propagate bounding boxes and error metrics to parents
					if (pNode != pTerrainNode)
					{
						pNode->m_boxHeigtmapLocal.min = SetMaxBB();
						pNode->m_boxHeigtmapLocal.max = SetMinBB();

						for (i32 nChild = 0; nChild < 4; nChild++)
						{
							pNode->m_boxHeigtmapLocal.min.CheckMin(pNode->m_pChilds[nChild].m_boxHeigtmapLocal.min);
							pNode->m_boxHeigtmapLocal.max.CheckMax(pNode->m_pChilds[nChild].m_boxHeigtmapLocal.max);
						}
					}

					// request elevation texture update
					if (pNode->m_nNodeTexSet.nSlot0 != 0xffff && pNode->m_nNodeTexSet.nSlot0 < m_pTerrain->m_texCache[2].GetPoolSize())
					{
						if (pTerrainNode->m_bHMDataIsModified)
							pNode->m_eElevTexEditingState = eTES_SectorIsModified_AtlasIsDirty;
					}

					pNode = pNode->m_pParent;
				}
			}

			modifiedArea.Add(pTerrainNode->GetBBox());
		}
	}

	if (Get3DEngine()->m_pObjectsTree)
		Get3DEngine()->m_pObjectsTree->UpdateTerrainNodes();

	// update roads
	if (Get3DEngine()->m_pObjectsTree && (GetCVars()->e_TerrainDeformations || m_bEditor))
	{
		PodArray<IRenderNode*> lstRoads;

		aabb = AABB(
		  Vec3((float)(x0 + X1) * (float)unitSize, (float)(y0 + Y1) * (float)unitSize, 0.f),
		  Vec3((float)(x0 + X1) * (float)unitSize + (float)nSizeX * (float)unitSize, (float)(y0 + Y1) * (float)unitSize + (float)nSizeY * (float)unitSize, 1024.f));

		Get3DEngine()->m_pObjectsTree->GetObjectsByType(lstRoads, eERType_Road, &aabb);
		for (i32 i = 0; i < lstRoads.Count(); i++)
		{
			CRoadRenderNode* pRoad = (CRoadRenderNode*)lstRoads[i];
			pRoad->OnTerrainChanged();
		}
	}

	if (GetParentNode())
		GetParentNode()->UpdateRangeInfoShift();

	if (!modifiedArea.IsReset())
	{
		modifiedArea.Expand(Vec3(2.f * GetHeightMapUnitSize()));
		ResetTerrainVertBuffers(&modifiedArea);
	}

	m_bHeightMapModified = 0;

	m_terrainPaintingFrameId = GetRenderer()->GetFrameID(false);

#endif // _RELEASE
}

void CTerrain::SetTerrainSectorTexture(i32 nTexSectorX, i32 nTexSectorY, u32 textureId, bool bMergeNotAllowed)
{
	i32 nDiffTexTreeLevelOffset = 0;

	if (nTexSectorX < 0 ||
	    nTexSectorY < 0 ||
	    nTexSectorX >= CTerrain::GetSectorsTableSize() >> nDiffTexTreeLevelOffset ||
	    nTexSectorY >= CTerrain::GetSectorsTableSize() >> nDiffTexTreeLevelOffset)
	{
		Warning("CTerrain::LockSectorTexture: (nTexSectorX, nTexSectorY) values out of range");
		return;
	}

	CTerrainNode* pNode = m_arrSecInfoPyramid[nDiffTexTreeLevelOffset][nTexSectorX][nTexSectorY];

	while (pNode)
	{
		pNode->EnableTextureEditingMode(textureId);
		pNode = pNode->m_pParent;
	}
}

void CTerrain::ResetTerrainVertBuffers(const AABB* pBox)
{
	if (GetParentNode())
		GetParentNode()->ReleaseHeightMapGeometry(true, pBox);
}

void CTerrain::SetOceanWaterLevel(float oceanWaterLevel)
{
	SetWaterLevel(oceanWaterLevel);
	pe_params_buoyancy pb;
	pb.waterPlane.origin.Set(0, 0, oceanWaterLevel);
	if (gEnv->pPhysicalWorld)
		gEnv->pPhysicalWorld->AddGlobalArea()->SetParams(&pb);
	extern float g_oceanStep;
	g_oceanStep = -1; // if e_PhysOceanCell is used, make it re-apply the params on Update
}

bool CTerrain::CanPaintSurfaceType(i32 x, i32 y, i32 r, u16 usGlobalSurfaceType)
{
	// Checks if the palettes of all sectors touching the given brush square can accept usGlobalSurfaceType.

	if (usGlobalSurfaceType == SRangeInfo::e_hole)
		return true;

	i32 nTerrainSizeSectors = CTerrain::GetTerrainSize() / CTerrain::GetSectorSize();

	i32 rangeX1 = max(0, (x - r) >> m_nUnitsToSectorBitShift);
	i32 rangeY1 = max(0, (y - r) >> m_nUnitsToSectorBitShift);
	i32 rangeX2 = min(nTerrainSizeSectors, ((x + r) >> m_nUnitsToSectorBitShift) + 1);
	i32 rangeY2 = min(nTerrainSizeSectors, ((y + r) >> m_nUnitsToSectorBitShift) + 1);

	float unitSize = (float)CTerrain::GetSectorSize();
	i32 nSignedBitShift = IntegerLog2((u32)CTerrain::GetSectorSize());
	while (unitSize > CTerrain::GetHeightMapUnitSize())
	{
		unitSize *= 0.5f;
		nSignedBitShift--;
	}

	// Can we fit this surface type into the palettes of all sectors involved?
	for (i32 rangeX = rangeX1; rangeX < rangeX2; rangeX++)
		for (i32 rangeY = rangeY1; rangeY < rangeY2; rangeY++)
		{
			i32 rx = rangeX, ry = rangeY;

			Array2d<struct CTerrainNode*>& sectorLayer = m_arrSecInfoPyramid[0];
			CTerrainNode* pTerrainNode = sectorLayer[ry][rx];
			SRangeInfo& ri = pTerrainNode->m_rangeInfo;

			if (ri.GetLocalSurfaceTypeID(usGlobalSurfaceType) == SRangeInfo::e_index_hole)
			{
				IRenderAuxText::DrawLabel(Vec3((float)y, (float)x, GetZfromUnits(y, x) + 1.0f), 2.0f, "SECTOR PALETTE FULL!");
				HighlightTerrain(
				  rangeX << m_nUnitsToSectorBitShift, rangeY << m_nUnitsToSectorBitShift,
				  (rangeX + 1) << m_nUnitsToSectorBitShift, (rangeY + 1) << m_nUnitsToSectorBitShift);
				return false;
			}
		}

	return true;
}

ILINE bool CTerrain::IsRenderNodeIncluded(IRenderNode* pNode, const AABB& region, u16k* pIncludeLayers, i32 numIncludeLayers)
{
	EERType type = pNode->GetRenderNodeType();

	switch (type)
	{
	case eERType_Brush:
	case eERType_Vegetation:
	case eERType_Decal:
	case eERType_WaterVolume:
	case eERType_MergedMesh:
		break;

	default:
		DRX_ASSERT_TRACE(0, ("Need to support cloning terrain object type %d", type));
		return false;
	}

	if (type == eERType_MergedMesh)
	{
		CMergedMeshRenderNode* pMergedMesh = (CMergedMeshRenderNode*)pNode;

		// The bounding box for a merged mesh can slop over into a neighboring grid
		// chunk, so only clone it if the center is contained in the region we're cloning.
		if (!region.IsContainPoint(pMergedMesh->m_pos))
			return false;
	}

	if (numIncludeLayers == 0)
		return true;

	u16k layerId = pNode->GetLayerId();

	// Some terrain objects don't have layer ids (like vegetation), so we have to include
	// them even if we're filtering.
	if (layerId == 0)
	{
		return true;
	}
	else
	{
		for (i32 e = 0; e < numIncludeLayers; e++)
		{
			if (pIncludeLayers[e] == layerId)
			{
				return true;
			}
		}
	}

	// If we made it to this point none of the include layers matched, so it's not excluded
	return false;
}

void CTerrain::MarkAndOffsetCloneRegion(const AABB& region, const Vec3& offset)
{
	C3DEngine* p3DEngine = Get3DEngine();
	i32 numObjects = p3DEngine->GetObjectsInBox(region, NULL);

	if (numObjects == 0)
		return;

	PodArray<IRenderNode*> objects;
	objects.resize(numObjects);
	p3DEngine->GetObjectsInBox(region, &objects[0]);

	i32 skyLayer = gEnv->pEntitySystem->GetLayerId("Sky");

	for (i32 i = 0; i < numObjects; i++)
	{
		IRenderNode* pNode = objects[i];
		EERType type = pNode->GetRenderNodeType();

		// HACK: Need a way for certain layers to not get offset, like the skybox
		if (type == eERType_Brush && skyLayer != -1)
		{
			CBrush* pBrush = (CBrush*)pNode;
			if (pBrush->GetLayerId() == skyLayer)
				continue;
		}

		if (IsRenderNodeIncluded(pNode, region, NULL, 0))
		{
			DRX_ASSERT_MESSAGE((pNode->GetRndFlags() & ERF_CLONE_SOURCE) == 0, "Marking already marked node, is an object overlapping multiple regions?");

			pNode->SetRndFlags(ERF_CLONE_SOURCE, true);

			pNode->OffsetPosition(offset);
			p3DEngine->RegisterEntity(pNode);
		}
	}
}

void CTerrain::CloneRegion(const AABB& region, const Vec3& offset, float zRotation, u16k* pIncludeLayers, i32 numIncludeLayers)
{
	C3DEngine* p3DEngine = Get3DEngine();
	i32 numObjects = p3DEngine->GetObjectsInBox(region, NULL);

	if (numObjects == 0)
		return;

	PodArray<IRenderNode*> objects;
	objects.resize(numObjects);
	p3DEngine->GetObjectsInBox(region, &objects[0]);

	Vec3 localOrigin = region.GetCenter();
	Matrix34 l2w(Matrix33::CreateRotationZ(zRotation));
	l2w.SetTranslation(offset);

	for (i32 i = 0; i < numObjects; i++)
	{
		IRenderNode* pNode = objects[i];

		// If this wasn't flagged as a clone source, don't include it
		if ((pNode->GetRndFlags() & ERF_CLONE_SOURCE) == 0)
			continue;

		if (!IsRenderNodeIncluded(pNode, region, pIncludeLayers, numIncludeLayers))
			continue;

		IRenderNode* pNodeClone = NULL;

		EERType type = pNode->GetRenderNodeType();
		if (type == eERType_Brush)
		{
			CBrush* pSrcBrush = (CBrush*)pNode;

			pNodeClone = pSrcBrush->Clone();
			CBrush* pBrush = (CBrush*)pNodeClone;

			pBrush->m_Matrix.SetTranslation(pBrush->m_Matrix.GetTranslation() - localOrigin);
			pBrush->m_Matrix = l2w * pBrush->m_Matrix;

			pBrush->CalcBBox();

			pBrush->m_pOcNode = NULL;

			// to get correct indirect lighting the registration must be done before checking if this object is inside a VisArea
			Get3DEngine()->RegisterEntity(pBrush);

			//if (gEnv->IsEditorGameMode())
			{
				//pBrush->Physicalize();
			}
			//else
			{
				// keep everything deactivated, game will activate it later
				//if(Get3DEngine()->IsAreaActivationInUse())
				pBrush->SetRndFlags(ERF_HIDDEN, false);

				if (!(Get3DEngine()->IsAreaActivationInUse() && GetCVars()->e_ObjectLayersActivationPhysics == 1))// && !(pChunk->m_dwRndFlags&ERF_NO_PHYSICS))
					pBrush->Physicalize();
			}
		}
		else if (type == eERType_Vegetation)
		{
			pNodeClone = pNode->Clone();
			CVegetation* pVeg = (CVegetation*)pNodeClone;

			Matrix34 matrix;
			matrix.SetRotationZ(pVeg->GetZAngle(), pVeg->m_vPos - localOrigin);

			matrix = l2w * matrix;

			pVeg->m_vPos = matrix.GetTranslation();

			// Vegetation stores rotation as a byte representing degrees, so we have to remap it back
			DrxQuat rot(matrix);
			float zRot = rot.GetRotZ();
			pVeg->m_ucAngle = (byte)((RAD2DEG(zRot) / 360.0f) * 255.0f);

			pVeg->CalcBBox();

			pVeg->Physicalize();

			Get3DEngine()->RegisterEntity(pVeg);
		}
		else if (type == eERType_Decal)
		{
			pNodeClone = pNode->Clone();
			CDecalRenderNode* pDecal = (CDecalRenderNode*)pNodeClone;

			Matrix34 mat = pDecal->GetMatrix();
			mat.SetTranslation(mat.GetTranslation() - localOrigin);
			mat = l2w * mat;

			pDecal->SetMatrixFull(mat);

			Get3DEngine()->RegisterEntity(pDecal);
			GetObjUpr()->m_decalsToPrecreate.push_back(pDecal);
		}
		else if (type == eERType_WaterVolume)
		{
			pNodeClone = pNode->Clone();
			CWaterVolumeRenderNode* pWaterVol = (CWaterVolumeRenderNode*)pNodeClone;

			pWaterVol->Transform(localOrigin, l2w);

			pWaterVol->Physicalize();

			Get3DEngine()->RegisterEntity(pWaterVol);
		}
		else if (type == eERType_MergedMesh)
		{
			CMergedMeshRenderNode* pSrcMergedMesh = (CMergedMeshRenderNode*)pNode;

			// Transform the position of the source merged mesh to our destination location
			Vec3 dstPos;
			{
				Matrix34 dst;
				dst.SetTranslationMat(pSrcMergedMesh->m_pos - localOrigin);
				dst = l2w * dst;
				dstPos = dst.GetTranslation();
			}

			// Get the merged mesh node for this coordinate.
			CMergedMeshRenderNode* pDstMergedMesh = m_pMergedMeshesUpr->GetNode(dstPos);
			DRX_ASSERT_MESSAGE(pDstMergedMesh->m_pos == dstPos, "Cloned destination position isn't on grid");
			DRX_ASSERT_MESSAGE(pDstMergedMesh->m_nGroups == 0, "Cloning into a used chunk");

			if (pDstMergedMesh->m_nGroups == 0)
			{
				pDstMergedMesh->m_initPos = pSrcMergedMesh->m_initPos;
				pDstMergedMesh->m_zRotation = zRotation;

				// Transform the AABB's
				AABB visibleAABB = pSrcMergedMesh->m_visibleAABB;
				visibleAABB.Move(-localOrigin);
				visibleAABB.SetTransformedAABB(l2w, visibleAABB);
				pDstMergedMesh->m_visibleAABB = visibleAABB;

				AABB internalAABB = pSrcMergedMesh->m_internalAABB;
				internalAABB.Move(-localOrigin);
				internalAABB.SetTransformedAABB(l2w, internalAABB);
				pDstMergedMesh->m_internalAABB = internalAABB;

				pSrcMergedMesh->CopyIRenderNodeData(pDstMergedMesh);

				if (!gEnv->IsDedicated())
				{
					// In the editor the merged meshes have extra proxy data.  We don't
					// clear out the source merged meshes in the editor, so the easiest
					// solution is to just share the source groups and not delete them
					// on destruction.
					if (gEnv->IsEditorGameMode())
					{
						pDstMergedMesh->m_ownsGroups = 0;
						pDstMergedMesh->m_groups = pSrcMergedMesh->m_groups;
						pDstMergedMesh->m_nGroups = pSrcMergedMesh->m_nGroups;
					}
					else
					{
						for (size_t i = 0; i < pSrcMergedMesh->m_nGroups; ++i)
						{
							SMMRMGroupHeader& pGroup = pSrcMergedMesh->m_groups[i];
							pDstMergedMesh->AddGroup(pGroup.instGroupId, pGroup.numSamples);
						}
					}
				}

				pNodeClone = pDstMergedMesh;

				Get3DEngine()->RegisterEntity(pDstMergedMesh);
			}
		}

		if (pNodeClone)
		{
			pNodeClone->SetRndFlags(ERF_CLONE_SOURCE, false);
			pNodeClone->SetLayerId(pNode->GetLayerId());
		}
	}
}

void CTerrain::ClearCloneSources()
{
	I3DEngine* p3DEngine = Get3DEngine();

	i32 numObjects = p3DEngine->GetObjectsByFlags(ERF_CLONE_SOURCE, NULL);

	if (numObjects == 0)
		return;

	PodArray<IRenderNode*> objects;
	objects.resize(numObjects);
	p3DEngine->GetObjectsByFlags(ERF_CLONE_SOURCE, &objects[0]);

	for (i32 i = 0; i < numObjects; i++)
	{
		IRenderNode* pNode = objects[i];

		Get3DEngine()->DeleteRenderNode(pNode);
	}
}
