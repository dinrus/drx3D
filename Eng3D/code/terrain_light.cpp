// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_light.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Генерация геометрии для проходки hmap light.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

#include <drx3D/Eng3D/terrain.h>
#include <drx3D/Eng3D/terrain_sector.h>
#include <drx3D/Eng3D/PolygonClipContext.h>
#include <drx3D/Eng3D/RoadRenderNode.h>
#include <drx3D/Eng3D/IIndexedMesh.h>

_smart_ptr<IRenderMesh> CTerrain::MakeAreaRenderMesh(const Vec3& vPos, float fRadius,
                                                     IMaterial* pMat, tukk szLSourceName,
                                                     Plane* planes)
{
	PodArray<vtx_idx> lstIndices;
	lstIndices.Clear();
	PodArray<Vec3> posBuffer;
	posBuffer.Clear();
	PodArray<SVF_P3S_C4B_T2S> vertBuffer;
	vertBuffer.Clear();
	PodArray<SPipTangents> tangBasises;
	tangBasises.Clear();

	float unitSize = GetTerrain()->GetHeightMapUnitSize();

	Vec3 vBoxMin = vPos - Vec3(fRadius, fRadius, fRadius);
	Vec3 vBoxMax = vPos + Vec3(fRadius, fRadius, fRadius);

	vBoxMin.x = floor((float)vBoxMin.x / unitSize) * unitSize;
	vBoxMin.y = floor((float)vBoxMin.y / unitSize) * unitSize;
	vBoxMin.z = floor((float)vBoxMin.z / unitSize) * unitSize;

	vBoxMax.x = floor((float)vBoxMax.x / unitSize) * unitSize + unitSize;
	vBoxMax.y = floor((float)vBoxMax.y / unitSize) * unitSize + unitSize;
	vBoxMax.z = floor((float)vBoxMax.z / unitSize) * unitSize + unitSize;

	i32 nSizeX = i32((vBoxMax.x - vBoxMin.x) / unitSize);
	i32 nSizeY = i32((vBoxMax.y - vBoxMin.y) / unitSize);

	i32 nEstimateVerts = nSizeX * nSizeY;
	nEstimateVerts = clamp_tpl(nEstimateVerts, 100, 10000);
	posBuffer.reserve(nEstimateVerts);
	lstIndices.reserve(nEstimateVerts * 6);

	const CTerrain* pTerrain = GetTerrain();
	for (float x = vBoxMin.x; x <= vBoxMax.x; x += unitSize)
	{
		for (float y = vBoxMin.y; y <= vBoxMax.y; y += unitSize)
		{
			Vec3 vTmp = Vec3((float)x, (float)y, (float)(pTerrain->GetZ(x, y)));
			posBuffer.Add(vTmp);
		}
	}

	for (i32 x = 0; x < nSizeX; x++)
	{
		for (i32 y = 0; y < nSizeY; y++)
		{
			vtx_idx id0 = (x + 0) * (nSizeY + 1) + (y + 0);
			vtx_idx id1 = (x + 1) * (nSizeY + 1) + (y + 0);
			vtx_idx id2 = (x + 0) * (nSizeY + 1) + (y + 1);
			vtx_idx id3 = (x + 1) * (nSizeY + 1) + (y + 1);

			assert((i32)id3 < posBuffer.Count());

			if (pTerrain->GetHole(vBoxMin.x + x, vBoxMin.y + y)) continue;

			if (pTerrain->IsMeshQuadFlipped(vBoxMin.x + x, vBoxMin.y + y, unitSize))
			{
				lstIndices.Add(id0);
				lstIndices.Add(id1);
				lstIndices.Add(id3);

				lstIndices.Add(id0);
				lstIndices.Add(id3);
				lstIndices.Add(id2);
			}
			else
			{
				lstIndices.Add(id0);
				lstIndices.Add(id1);
				lstIndices.Add(id2);

				lstIndices.Add(id2);
				lstIndices.Add(id1);
				lstIndices.Add(id3);
			}
		}
	}

	// clip triangles
	if (planes)
	{
		i32 nOrigCount = lstIndices.Count();
		for (i32 i = 0; i < nOrigCount; i += 3)
		{
			if (CRoadRenderNode::ClipTriangle(posBuffer, lstIndices, i, planes))
			{
				i -= 3;
				nOrigCount -= 3;
			}
		}
	}

	AABB bbox;
	bbox.Reset();
	for (i32 i = 0, nIndexCount = lstIndices.size(); i < nIndexCount; i++)
		bbox.Add(posBuffer[lstIndices[i]]);

	tangBasises.reserve(posBuffer.size());
	vertBuffer.reserve(posBuffer.size());

	for (i32 i = 0, nPosCount = posBuffer.size(); i < nPosCount; i++)
	{
		SVF_P3S_C4B_T2S vTmp;
		vTmp.xyz = posBuffer[i] - vPos;
		vTmp.color.dcolor = u32(-1);
		vTmp.st = Vec2(0, 0);
		vertBuffer.Add(vTmp);

		SPipTangents basis;

		Vec3 vTang = Vec3(0, 1, 0);
		Vec3 vNormal = GetTerrain()->GetTerrainSurfaceNormal_Int(posBuffer[i].x, posBuffer[i].y);

		// Orthonormalize Tangent Frame
		Vec3 vBitang = -vNormal.Cross(vTang);
		vBitang.Normalize();
		vTang = vNormal.Cross(vBitang);
		vTang.Normalize();

		tangBasises.Add(SPipTangents(vTang, vBitang, -1));
	}

	_smart_ptr<IRenderMesh> pMesh = GetRenderer()->CreateRenderMeshInitialized(
	  vertBuffer.GetElements(), vertBuffer.Count(), EDefaultInputLayouts::P3S_C4B_T2S,
	  lstIndices.GetElements(), lstIndices.Count(), prtTriangleList,
	  szLSourceName, szLSourceName, eRMT_Static, 1, 0, NULL, NULL, false, true, tangBasises.GetElements());

	float texelAreaDensity = 1.0f;

	pMesh->SetChunk(pMat, 0, vertBuffer.Count(), 0, lstIndices.Count(), texelAreaDensity);
	pMesh->SetBBox(bbox.min - vPos, bbox.max - vPos);
	if (pMesh->GetChunks()[0].pRE)
		pMesh->GetChunks()[0].pRE->mfUpdateFlags(FCEF_DIRTY);

	return pMesh;
}

bool CTerrain::RenderArea(Vec3 vPos, float fRadius, _smart_ptr<IRenderMesh>& pRenderMesh,
                          CRenderObject* pObj, IMaterial* pMaterial, tukk szComment, float* pCustomData,
                          Plane* planes, const SRenderingPassInfo& passInfo)
{
	if (passInfo.IsRecursivePass())
		return false;

	FUNCTION_PROFILER_3DENGINE;

	bool bREAdded = false;

	if (!pRenderMesh)
		pRenderMesh = MakeAreaRenderMesh(vPos, fRadius, pMaterial, szComment, planes);

	if (pRenderMesh && pRenderMesh->GetIndicesCount())
	{
		pRenderMesh->SetREUserData(pCustomData);
		pRenderMesh->AddRE(pMaterial, pObj, 0, passInfo, EFSLIST_GENERAL, 1);
		bREAdded = true;
	}

	return bREAdded;
}
