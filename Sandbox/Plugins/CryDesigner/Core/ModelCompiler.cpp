// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ModelCompiler.h"
#include "Model.h"
#include <Preferences/ViewportPreferences.h>
#include "Objects/DesignerObject.h"
#include "Tools/BaseTool.h"
#include "Core/SmoothingGroupManager.h"
#include "Subdivision.h"
#include "Material/Material.h"
#include "Helper.h"

#include <Objects/IObjectLayer.h>

namespace Designer
{

ModelCompiler::ModelCompiler(i32 nCompilerFlag) :
	m_nCompilerFlag(nCompilerFlag),
	m_viewDistRatio(100)
{
	m_RenderFlags = CheckFlags(eCompiler_CastShadow) ? ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS : 0;
	for (i32 shelfID = eShelf_Base; shelfID < cShelfMax; ++shelfID)
	{
		m_pStatObj[shelfID] = NULL;
		m_pRenderNode[shelfID] = NULL;
	}
}

ModelCompiler::ModelCompiler(const ModelCompiler& compiler)
{
	for (i32 shelfID = eShelf_Base; shelfID < cShelfMax; ++shelfID)
	{
		m_pStatObj[shelfID] = NULL;
		m_pRenderNode[shelfID] = NULL;
	}
	m_RenderFlags = compiler.m_RenderFlags;
	m_viewDistRatio = compiler.m_viewDistRatio;
	m_nCompilerFlag = compiler.m_nCompilerFlag;
}

ModelCompiler::~ModelCompiler()
{
	for (i32 i = eShelf_Base; i < cShelfMax; ++i)
		RemoveStatObj(static_cast<ShelfID>(i));
	DeleteAllRenderNodes();
}

IMaterial* ModelCompiler::GetMaterialFromBaseObj(CBaseObject* pObj) const
{
	IMaterial* pMaterial = NULL;
	if (pObj)
	{
		string materialName(pObj->GetMaterialName());
		CMaterial* pObjMaterial = (CMaterial*)pObj->GetMaterial();
		if (pObjMaterial)
			pMaterial = pObjMaterial->GetMatInfo();
	}
	return pMaterial;
}

void ModelCompiler::RemoveStatObj(ShelfID nShelf)
{
	if (m_pStatObj[nShelf])
	{
		m_pStatObj[nShelf]->Release();
		m_pStatObj[nShelf] = NULL;
	}
}

void ModelCompiler::CreateStatObj(ShelfID nShelf)
{
	RemoveStatObj(nShelf);
	if (!m_pStatObj[nShelf])
	{
		m_pStatObj[nShelf] = GetIEditor()->Get3DEngine()->CreateStatObj();
		m_pStatObj[nShelf]->AddRef();
	}
}

void ModelCompiler::InvalidateStatObj(IStatObj* pStatObj, bool bPhysics)
{
	if (!pStatObj)
		return;
	IIndexedMesh* pIndexedMesh = pStatObj->GetIndexedMesh();
	if (pIndexedMesh)
	{
		CMesh* pMesh = pIndexedMesh->GetMesh();
		if (!pMesh->m_pIndices)
			bPhysics = false;
	}
	pStatObj->Invalidate(bPhysics);
	i32 nSubObjCount = pStatObj->GetSubObjectCount();
	for (i32 i = 0; i < nSubObjCount; ++i)
	{
		IStatObj::SSubObject* pSubObject = pStatObj->GetSubObject(i);
		if (pSubObject && pSubObject->pStatObj)
			pSubObject->pStatObj->Invalidate(bPhysics);
	}
}

void ModelCompiler::Compile(CBaseObject* pBaseObject, Model* pModel, ShelfID shelfID, bool bUpdateOnlyRenderNode)
{
	MODEL_SHELF_RECONSTRUCTOR(pModel);

	i32 nSubdivisionLevel = pModel->GetSubdivisionLevel();
	if (nSubdivisionLevel == 0)
	{
		if (shelfID == eShelf_Any)
		{
			for (i32 i = eShelf_Base; i < cShelfMax; ++i)
			{
				pModel->SetShelf(static_cast<ShelfID>(i));
				if (!bUpdateOnlyRenderNode)
					UpdateMesh(pBaseObject, pModel, static_cast<ShelfID>(i));
				UpdateRenderNode(pBaseObject, static_cast<ShelfID>(i));
			}
		}
		else
		{
			pModel->SetShelf(shelfID);
			if (!bUpdateOnlyRenderNode)
				UpdateMesh(pBaseObject, pModel, shelfID);
			UpdateRenderNode(pBaseObject, shelfID);
		}
		pModel->SetSubdivisionResult(NULL);
	}
	else
	{
		if (!bUpdateOnlyRenderNode)
		{
			CreateStatObj(eShelf_Base);

			Subdivision subdivisionModifier;
			SubdivisionContext sc = subdivisionModifier.CreateSubdividedMesh(pModel, nSubdivisionLevel);

			bool bDisplayBackface = pModel->GetFlag() & eModelFlag_DisplayBackFace;
			IMaterial* pMaterial = GetMaterialFromBaseObj(pBaseObject);

			std::vector<FlexibleMeshPtr> meshes;
			sc.fullPatches->CreateMeshFaces(meshes, bDisplayBackface, pModel->IsSmoothingSurfaceForSubdividedMeshEnabled());

			pModel->SetSubdivisionResult(sc.fullPatches);

			if (meshes.size() == 1)
			{
				IIndexedMesh* pMesh = m_pStatObj[0]->GetIndexedMesh();
				meshes[0]->FillIndexedMesh(pMesh);
				m_pStatObj[0]->SetMaterial(pMaterial);
				pMesh->Optimize();
				InvalidateStatObj(m_pStatObj[0], CheckFlags(eCompiler_Physicalize));
			}
			else
			{
				for (i32 i = 0, iSubObjCount(meshes.size()); i < iSubObjCount; ++i)
					m_pStatObj[0]->AddSubObject(GetIEditor()->Get3DEngine()->CreateStatObj());

				for (i32 i = 0, iSubObjCount(m_pStatObj[0]->GetSubObjectCount()); i < iSubObjCount; ++i)
				{
					IStatObj* pSubObj = m_pStatObj[0]->GetSubObject(i)->pStatObj;
					pSubObj->SetMaterial(pMaterial);
					IIndexedMesh* pMesh = pSubObj->GetIndexedMesh();
					meshes[i]->FillIndexedMesh(pMesh);
					pMesh->Optimize();
					InvalidateStatObj(pSubObj, CheckFlags(eCompiler_Physicalize));
					pSubObj->m_eStreamingStatus = ecss_Ready;
				}

				AABB aabb = pModel->GetBoundBox();
				m_pStatObj[0]->SetBBoxMin(aabb.min);
				m_pStatObj[0]->SetBBoxMax(aabb.max);
			}
		}
		UpdateRenderNode(pBaseObject, eShelf_Base);
	}
}

bool ModelCompiler::UpdateMesh(CBaseObject* pBaseObject, Model* pModel, ShelfID nShelf)
{
	if (pModel == NULL)
		return false;

	if (!pModel->HasClosedPolygon(nShelf))
	{
		RemoveStatObj(nShelf);
		DeleteRenderNode(nShelf);
		pBaseObject->GetLayer()->SetModified(true);
		return true;
	}

	CreateStatObj(nShelf);

	IMaterial* pMaterial = GetMaterialFromBaseObj(pBaseObject);

	i32 prevSubObjCount = m_pStatObj[nShelf]->GetSubObjectCount();
	bool bCreateBackFaces = pModel->GetFlag() & eModelFlag_DisplayBackFace;

	m_pStatObj[nShelf]->m_eStreamingStatus = ecss_Ready;
	i32 nStatObjFlag = m_pStatObj[nShelf]->GetFlags();
	m_pStatObj[nShelf]->SetFlags(nStatObjFlag & (~STATIC_OBJECT_COMPOUND));
	m_pStatObj[nShelf]->SetMaterial(pMaterial);

	IIndexedMesh* pMesh = m_pStatObj[nShelf]->GetIndexedMesh();

	CreateIndexdMesh(pModel, pMesh, bCreateBackFaces);
	pMesh->Optimize();

	InvalidateStatObj(m_pStatObj[nShelf], CheckFlags(eCompiler_Physicalize));
	m_pStatObj[nShelf]->m_eStreamingStatus = ecss_Ready;
	pBaseObject->GetLayer()->SetModified(true);

	return true;
}

bool ModelCompiler::CreateIndexdMesh(Model* pModel, IIndexedMesh* pMesh, bool bCreateBackFaces)
{
	if (!pModel)
		return false;

	FlexibleMesh mesh;

	SmoothingGroupManager* pSmoothingGroupMgr = pModel->GetSmoothingGroupMgr();
	for (i32 i = 0, iPolygonCount(pModel->GetPolygonCount()); i < iPolygonCount; ++i)
	{
		PolygonPtr pPolygon = pModel->GetPolygon(i);
		if (pSmoothingGroupMgr->GetSmoothingGroupID(pPolygon) ||
		    pPolygon->CheckFlags(ePolyFlag_Hidden))
		{
			continue;
		}
		mesh.Join(*pPolygon->GetTriangles(bCreateBackFaces));
	}

	std::vector<std::pair<string, SmoothingGroupPtr>> smoothingGroupList = pSmoothingGroupMgr->GetSmoothingGroupList();
	for (i32 i = 0, iSmoothingGroupCount(smoothingGroupList.size()); i < iSmoothingGroupCount; ++i)
	{
		SmoothingGroupPtr pSmoothingGroup = smoothingGroupList[i].second;
		mesh.Join(pSmoothingGroup->GetFlexibleMesh(pModel, bCreateBackFaces));
	}

	if (mesh.IsValid())
	{
		mesh.FillIndexedMesh(pMesh);
	}
	else
	{
		pMesh->FreeStreams();
		return false;
	}

	return true;
}

void ModelCompiler::SetStaticObjFlags(i32 nStaticObjFlag)
{
	if (!m_pStatObj[0])
		CreateStatObj(eShelf_Base);
	m_pStatObj[0]->SetFlags(nStaticObjFlag);
}

i32 ModelCompiler::GetStaticObjFlags()
{
	if (!m_pStatObj[0])
		CreateStatObj(eShelf_Base);
	return m_pStatObj[0]->GetFlags();
}

void ModelCompiler::DeleteAllRenderNodes()
{
	for (i32 shelfID = 0; shelfID < cShelfMax; ++shelfID)
		DeleteRenderNode(static_cast<ShelfID>(shelfID));
}

void ModelCompiler::UpdateHighlightPassState(bool bSelected, bool bHighlighted)
{
	DesignerEditor* pDesigner = GetDesigner();
	// disable highlights while editing
	if (pDesigner && pDesigner->GetCurrentTool())
	{
		bSelected = bHighlighted = false;
	}

	for (i32 shelfID = 0; shelfID < cShelfMax; ++shelfID)
	{
		if (m_pRenderNode[shelfID])
		{
			m_pRenderNode[shelfID]->SetEditorObjectInfo(bSelected, bHighlighted);
		}
	}
}

void ModelCompiler::DeleteRenderNode(ShelfID shelfID)
{
	if (m_pRenderNode[shelfID])
	{
		GetIEditor()->Get3DEngine()->DeleteRenderNode(m_pRenderNode[shelfID]);
		m_pRenderNode[shelfID] = NULL;
	}
}

void ModelCompiler::UpdateRenderNode(CBaseObject* pBaseObject, ShelfID nShelf)
{
	if (m_pStatObj[nShelf] == NULL)
	{
		DeleteRenderNode(nShelf);
		return;
	}

	if (!m_pRenderNode[nShelf])
	{
		m_pRenderNode[nShelf] = GetIEditor()->Get3DEngine()->CreateRenderNode(eERType_Brush);
		if (pBaseObject)
		{
			m_pRenderNode[nShelf]->SetEditorObjectId(pBaseObject->GetId().hipart >> 32);
		}
	}

	i32 renderFlags = m_RenderFlags;

	m_pRenderNode[nShelf]->SetRndFlags(renderFlags);
	m_pRenderNode[nShelf]->SetViewDistRatio(m_viewDistRatio);
	m_pRenderNode[nShelf]->SetMinSpec(pBaseObject->GetMinSpec());
	m_pRenderNode[nShelf]->SetMaterialLayers(pBaseObject->GetMaterialLayersMask());

	if (pBaseObject && pBaseObject->IsKindOf(RUNTIME_CLASS(DesignerObject)))
	{
		string generatedFilename;
		static_cast<DesignerObject*>(pBaseObject)->GenerateGameFilename(generatedFilename);
		m_pStatObj[nShelf]->SetFilePath(generatedFilename);
	}

	m_pStatObj[nShelf]->SetMaterial(GetMaterialFromBaseObj(pBaseObject));
	m_pRenderNode[nShelf]->SetEntityStatObj(m_pStatObj[nShelf], &pBaseObject->GetWorldTM());
	m_pRenderNode[nShelf]->SetMaterial(m_pStatObj[nShelf]->GetMaterial());

	if (m_pRenderNode[nShelf])
	{
		if (CheckFlags(eCompiler_Physicalize))
			m_pRenderNode[nShelf]->Physicalize();
		else
			m_pRenderNode[nShelf]->Dephysicalize();
	}
}

bool ModelCompiler::GetIStatObj(_smart_ptr<IStatObj>* pOutStatObj)
{
	if (!m_pStatObj[0])
		return false;

	if (pOutStatObj)
		*pOutStatObj = m_pStatObj[0];

	return IsValid();
}

void ModelCompiler::SaveToCgf(tukk filename)
{
	_smart_ptr<IStatObj> pObj;
	if (GetIStatObj(&pObj))
		pObj->SaveToCGF(filename, NULL, true);
}

void ModelCompiler::SaveMesh(CArchive& ar, CBaseObject* pObj, Model* pModel)
{
	if (!m_pStatObj[0])
		return;

	i32 nSubObjectCount = m_pStatObj[0]->GetSubObjectCount();

	std::vector<IStatObj*> pStatObjs;
	if (nSubObjectCount == 0)
	{
		pStatObjs.push_back(m_pStatObj[0]);
	}
	else if (nSubObjectCount > 0)
	{
		for (i32 i = 0; i < nSubObjectCount; ++i)
			pStatObjs.push_back(m_pStatObj[0]->GetSubObject(i)->pStatObj);
	}

	ar.Write(&nSubObjectCount, sizeof(nSubObjectCount));

	for (i32 k = 0, iStatObjCount(pStatObjs.size()); k < iStatObjCount; ++k)
	{
		IIndexedMesh* pMesh = pStatObjs[k]->GetIndexedMesh();
		if (!pMesh)
			return;

		i32 nPositionCount = pMesh->GetVertexCount();
		Vec3* const positions = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::POSITIONS);
		Vec3* const normals = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::NORMALS);

		i32 nTexCoordCount = pMesh->GetTexCoordCount();
		SMeshTexCoord* const texcoords = pMesh->GetMesh()->GetStreamPtr<SMeshTexCoord>(CMesh::TEXCOORDS);

		i32 nFaceCount = pMesh->GetFaceCount();
		SMeshFace* faces = pMesh->GetMesh()->GetStreamPtr<SMeshFace>(CMesh::FACES);
		if (nFaceCount == 0 || faces == NULL)
		{
			pMesh->RestoreFacesFromIndices();
			faces = pMesh->GetMesh()->GetStreamPtr<SMeshFace>(CMesh::FACES);
			nFaceCount = pMesh->GetFaceCount();
		}

		i32 nIndexCount = pMesh->GetIndexCount();
		vtx_idx* const indices = pMesh->GetMesh()->GetStreamPtr<vtx_idx>(CMesh::INDICES);

		i32 nSubsetCount = pMesh->GetSubSetCount();

		ar.Write(&nPositionCount, sizeof(i32));
		ar.Write(&nTexCoordCount, sizeof(i32));
		ar.Write(&nFaceCount, sizeof(i32));
		ar.Write(&nSubsetCount, sizeof(i32));

		ar.Write(positions, sizeof(Vec3) * nPositionCount);
		ar.Write(normals, sizeof(Vec3) * nPositionCount);
		ar.Write(texcoords, sizeof(SMeshTexCoord) * nTexCoordCount);

		for (i32 i = 0; i < nSubsetCount; ++i)
		{
			const SMeshSubset& subset = pMesh->GetSubSet(i);
			for (i32 k = 0; k < subset.nNumIndices / 3; ++k)
				faces[(subset.nFirstIndexId / 3) + k].nSubset = i;
		}
		ar.Write(faces, sizeof(SMeshFace) * nFaceCount);

		for (i32 i = 0; i < nSubsetCount; ++i)
		{
			const SMeshSubset& subset = pMesh->GetSubSet(i);
			ar.Write(&subset.vCenter, sizeof(Vec3));
			ar.Write(&subset.fRadius, sizeof(float));
			ar.Write(&subset.fTexelDensity, sizeof(float));
			ar.Write(&subset.nFirstIndexId, sizeof(i32));
			ar.Write(&subset.nNumIndices, sizeof(i32));
			ar.Write(&subset.nFirstVertId, sizeof(i32));
			ar.Write(&subset.nNumVerts, sizeof(i32));
			ar.Write(&subset.nMatID, sizeof(i32));
			ar.Write(&subset.nMatFlags, sizeof(i32));
			ar.Write(&subset.nPhysicalizeType, sizeof(i32));
		}
	}
}

bool ModelCompiler::LoadMesh(CArchive& ar, CBaseObject* pObj, Model* pModel)
{
	i32 nSubObjectCount = 0;
	ar.Read(&nSubObjectCount, sizeof(nSubObjectCount));

	if (!m_pStatObj[0])
	{
		m_pStatObj[0] = GetIEditor()->Get3DEngine()->CreateStatObj();
		m_pStatObj[0]->AddRef();
	}

	std::vector<IStatObj*> statObjList;
	if (nSubObjectCount == 2)
	{
		m_pStatObj[0]->AddSubObject(GetIEditor()->Get3DEngine()->CreateStatObj());
		m_pStatObj[0]->AddSubObject(GetIEditor()->Get3DEngine()->CreateStatObj());
		m_pStatObj[0]->GetIndexedMesh()->FreeStreams();
		statObjList.push_back(m_pStatObj[0]->GetSubObject(0)->pStatObj);
		statObjList.push_back(m_pStatObj[0]->GetSubObject(1)->pStatObj);
	}
	else
	{
		statObjList.push_back(m_pStatObj[0]);
	}

	IMaterial* pMaterial = GetMaterialFromBaseObj(pObj);
	m_pStatObj[0]->SetMaterial(pMaterial);

	for (i32 k = 0, iCount(statObjList.size()); k < iCount; ++k)
	{
		i32 nPositionCount = 0;
		i32 nTexCoordCount = 0;
		i32 nFaceCount = 0;
		ar.Read(&nPositionCount, sizeof(i32));
		ar.Read(&nTexCoordCount, sizeof(i32));
		ar.Read(&nFaceCount, sizeof(i32));
		if (nPositionCount <= 0 || nTexCoordCount <= 0 || nFaceCount <= 0)
		{
			assert(0);
			return false;
		}

		i32 nSubsetCount = 0;
		ar.Read(&nSubsetCount, sizeof(i32));

		IIndexedMesh* pMesh = statObjList[k]->GetIndexedMesh();
		if (!pMesh)
		{
			assert(0);
			return false;
		}

		pMesh->FreeStreams();
		pMesh->SetVertexCount(nPositionCount);
		pMesh->SetFaceCount(nFaceCount);
		pMesh->SetIndexCount(0);
		pMesh->SetTexCoordCount(nTexCoordCount);

		Vec3* const positions = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::POSITIONS);
		Vec3* const normals = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::NORMALS);
		SMeshTexCoord* const texcoords = pMesh->GetMesh()->GetStreamPtr<SMeshTexCoord>(CMesh::TEXCOORDS);
		SMeshFace* const faces = pMesh->GetMesh()->GetStreamPtr<SMeshFace>(CMesh::FACES);

		ar.Read(positions, sizeof(Vec3) * nPositionCount);
		ar.Read(normals, sizeof(Vec3) * nPositionCount);
		ar.Read(texcoords, sizeof(SMeshTexCoord) * nTexCoordCount);
		ar.Read(faces, sizeof(SMeshFace) * nFaceCount);

		pMesh->SetSubSetCount(nSubsetCount);
		for (i32 i = 0; i < nSubsetCount; ++i)
		{
			SMeshSubset subset;
			ar.Read(&subset.vCenter, sizeof(Vec3));
			ar.Read(&subset.fRadius, sizeof(float));
			ar.Read(&subset.fTexelDensity, sizeof(float));
			ar.Read(&subset.nFirstIndexId, sizeof(i32));
			ar.Read(&subset.nNumIndices, sizeof(i32));
			ar.Read(&subset.nFirstVertId, sizeof(i32));
			ar.Read(&subset.nNumVerts, sizeof(i32));
			ar.Read(&subset.nMatID, sizeof(i32));
			ar.Read(&subset.nMatFlags, sizeof(i32));
			ar.Read(&subset.nPhysicalizeType, sizeof(i32));

			pMesh->SetSubsetBounds(i, subset.vCenter, subset.fRadius);
			pMesh->SetSubsetIndexVertexRanges(i, subset.nFirstIndexId, subset.nNumIndices, subset.nFirstVertId, subset.nNumVerts);
			pMesh->SetSubsetMaterialId(i, subset.nMatID == -1 ? 0 : subset.nMatID);
			pMesh->SetSubsetMaterialProperties(i, subset.nMatFlags, subset.nPhysicalizeType);
		}

		pMesh->Optimize();

		statObjList[k]->SetMaterial(pMaterial);
		InvalidateStatObj(statObjList[k], CheckFlags(eCompiler_Physicalize));
	}

	UpdateRenderNode(pObj, eShelf_Base);

	return true;
}

bool ModelCompiler::SaveMesh(i32 nVersion, std::vector<char>& buffer, CBaseObject* pObj, Model* pModel)
{
	if (!m_pStatObj[0])
		return false;

	i32 nSubObjectCount = (i32)m_pStatObj[0]->GetSubObjectCount();

	std::vector<IStatObj*> pStatObjs;
	if (nSubObjectCount == 0)
	{
		pStatObjs.push_back(m_pStatObj[0]);
	}
	else if (nSubObjectCount > 0)
	{
		for (i32 i = 0; i < nSubObjectCount; ++i)
			pStatObjs.push_back(m_pStatObj[0]->GetSubObject(i)->pStatObj);
	}

	Write2Buffer(buffer, &nSubObjectCount, sizeof(i32));

	if (nVersion == 2)
	{
		i32 nStaticObjFlags = (i32)m_pStatObj[0]->GetFlags();
		Write2Buffer(buffer, &nStaticObjFlags, sizeof(i32));
	}

	for (i32 k = 0, iStatObjCount(pStatObjs.size()); k < iStatObjCount; ++k)
	{
		IIndexedMesh* pMesh = pStatObjs[k]->GetIndexedMesh();
		if (!pMesh)
			return false;
		i32 nIndexCount = pMesh->GetIndexCount();
		vtx_idx* const indices = pMesh->GetMesh()->GetStreamPtr<vtx_idx>(CMesh::INDICES);
		if (nIndexCount == 0 || indices == 0)
		{
			pMesh->Optimize();
			break;
		}
	}

	for (i32 k = 0, iStatObjCount(pStatObjs.size()); k < iStatObjCount; ++k)
	{
		IIndexedMesh* pMesh = pStatObjs[k]->GetIndexedMesh();
		if (!pMesh)
			return false;

		i32 nPositionCount = (i32)pMesh->GetVertexCount();
		i32 nTexCoordCount = (i32)pMesh->GetTexCoordCount();
		i32 nFaceCount = (i32)pMesh->GetFaceCount();
		i32 nTangentCount = (i32)pMesh->GetTangentCount();
		i32 nSubsetCount = (i32)pMesh->GetSubSetCount();
		i32 nIndexCount = (i32)pMesh->GetIndexCount();

		Write2Buffer(buffer, &nPositionCount, sizeof(i32));
		Write2Buffer(buffer, &nTexCoordCount, sizeof(i32));
		if (nVersion == 0)
		{
			Write2Buffer(buffer, &nFaceCount, sizeof(i32));
		}
		else if (nVersion >= 1)
		{
			if (sizeof(vtx_idx) == sizeof(u16) && nIndexCount >= 0xFFFFFFFF)
				return false;
			Write2Buffer(buffer, &nIndexCount, sizeof(i32));
			Write2Buffer(buffer, &nTangentCount, sizeof(i32));
		}
		Write2Buffer(buffer, &nSubsetCount, sizeof(i32));

		Vec3* const positions = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::POSITIONS);
		Vec3* const normals = pMesh->GetMesh()->GetStreamPtr<Vec3>(CMesh::NORMALS);
		SMeshTexCoord* const texcoords = pMesh->GetMesh()->GetStreamPtr<SMeshTexCoord>(CMesh::TEXCOORDS);
		SMeshFace* faces = pMesh->GetMesh()->GetStreamPtr<SMeshFace>(CMesh::FACES);
		if (nVersion == 0 && (nFaceCount == 0 || faces == NULL))
		{
			pMesh->RestoreFacesFromIndices();
			faces = pMesh->GetMesh()->GetStreamPtr<SMeshFace>(CMesh::FACES);
			nFaceCount = pMesh->GetFaceCount();
		}
		vtx_idx* const indices = pMesh->GetMesh()->GetStreamPtr<vtx_idx>(CMesh::INDICES);
		u16* const indices16 = nVersion >= 1 ? (sizeof(vtx_idx) == sizeof(u16) ? (u16*)indices : new u16[nIndexCount]) : NULL;
		if (nVersion >= 1 && (u16*)indices != indices16)
		{
			for (i32 i = 0; i < nIndexCount; ++i)
				indices16[i] = (u16)indices[i];
		}

		Write2Buffer(buffer, positions, sizeof(Vec3) * nPositionCount);
		Write2Buffer(buffer, normals, sizeof(Vec3) * nPositionCount);
		Write2Buffer(buffer, texcoords, sizeof(SMeshTexCoord) * nTexCoordCount);
		if (nVersion == 0)
		{
			for (i32 i = 0; i < nSubsetCount; ++i)
			{
				const SMeshSubset& subset = pMesh->GetSubSet(i);
				for (i32 k = 0; k < subset.nNumIndices / 3; ++k)
					faces[(subset.nFirstIndexId / 3) + k].nSubset = i;
			}
			Write2Buffer(buffer, faces, sizeof(SMeshFace) * nFaceCount);
		}
		else if (nVersion >= 1)
		{
			Write2Buffer(buffer, indices16, sizeof(u16) * nIndexCount);
			if (nTangentCount > 0)
			{
				SMeshTangents* tangents = pMesh->GetMesh()->GetStreamPtr<SMeshTangents>(CMesh::TANGENTS);
				Write2Buffer(buffer, tangents, sizeof(SMeshTangents) * nTangentCount);
			}
		}
		// optimization for designer bbox loading, turn off for now
#if 0
		if (nVersion >= 3)
		{
			Vec3 bbmin = pMesh->GetBBox().min;
			Vec3 bbmax = pMesh->GetBBox().max;

			Write2Buffer(buffer, &bbmin, sizeof(Vec3));
			Write2Buffer(buffer, &bbmax, sizeof(Vec3));
		}
#endif
		for (i32 i = 0; i < nSubsetCount; ++i)
		{
			const SMeshSubset& subset = pMesh->GetSubSet(i);
			Write2Buffer(buffer, &subset.vCenter, sizeof(Vec3));
			Write2Buffer(buffer, &subset.fRadius, sizeof(float));
			Write2Buffer(buffer, &subset.fTexelDensity, sizeof(float));

			i32 nFirstIndexId = (i32)subset.nFirstIndexId;
			i32 nNumIndices = (i32)subset.nNumIndices;
			i32 nFirstVertId = (i32)subset.nFirstVertId;
			i32 nNumVerts = (i32)subset.nNumVerts;
			i32 nMatID = (i32)subset.nMatID;
			i32 nMatFlags = (i32)subset.nMatFlags;
			i32 nPhysicalizeType = (i32)subset.nPhysicalizeType;

			Write2Buffer(buffer, &nFirstIndexId, sizeof(i32));
			Write2Buffer(buffer, &nNumIndices, sizeof(i32));
			Write2Buffer(buffer, &nFirstVertId, sizeof(i32));
			Write2Buffer(buffer, &nNumVerts, sizeof(i32));
			Write2Buffer(buffer, &nMatID, sizeof(i32));
			Write2Buffer(buffer, &nMatFlags, sizeof(i32));
			Write2Buffer(buffer, &nPhysicalizeType, sizeof(i32));
		}

		if (indices16 && (u16*)indices != indices16)
			delete[] indices16;
	}

	return true;
}

bool ModelCompiler::LoadMesh(i32 nVersion, std::vector<char>& buffer, CBaseObject* pObj, Model* pModel)
{
	IStatObj* pStatObj = GetIEditor()->Get3DEngine()->LoadDesignerObject(nVersion, &buffer[0], buffer.size());
	if (pStatObj)
	{
		RemoveStatObj(eShelf_Base);
		pStatObj->AddRef();
		m_pStatObj[0] = pStatObj;

		IMaterial* pMaterial = GetMaterialFromBaseObj(pObj);
		m_pStatObj[0]->SetMaterial(pMaterial);
		m_pStatObj[0]->m_eStreamingStatus = ecss_Ready;
		UpdateRenderNode(pObj, eShelf_Base);
		return true;
	}
	return false;
}

bool ModelCompiler::IsValid() const
{
	return m_pStatObj[0] || m_pStatObj[1];
}
};

