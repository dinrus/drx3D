// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "ExportDataTypes.h"
#include "IEditorMaterial.h"

namespace Private_ExportDataTypes
{

static void SetTexture(Export::TPath& outName, const IRenderShaderResources* pRes, i32 nSlot)
{
	SEfResTexture* pTex = pRes->GetTexture(nSlot);
	if (pTex)
	{
		IDrxPak* const pPak = GetISystem()->GetIPak();
		char path[IDrxPak::g_nMaxPath] = {};
		tukk szPath = pPak->AdjustFileName(pTex->m_Name.c_str(), path, IDrxPak::FLAGS_PATH_REAL | IDrxPak::FLAGS_FOR_WRITING);
		drx_strcat(outName, szPath);
	}
}

}

//////////////////////////////////////////////////////////
// SExportMesh
SExportMesh::SExportMesh()
{
	::ZeroMemory(&material, sizeof(material));
	material.opacity = 1.0f;
}

void SExportMesh::SetMaterial(IEditorMaterial* pMtl, CBaseObject* pBaseObj)
{
	using namespace Private_ExportDataTypes;

	if (!pMtl)
	{
		drx_strcpy(material.name, pBaseObj->GetName());
		return;
	}

	drx_strcpy(material.name, pMtl->GetFullName());

	IMaterial* matInfo = pMtl->GetMatInfo();
	IRenderShaderResources* pRes = matInfo->GetShaderItem().m_pShaderResources;
	if (!pRes)
		return;

	const ColorF& difColor = pRes->GetColorValue(EFTT_DIFFUSE);
	material.diffuse.r = difColor.r;
	material.diffuse.g = difColor.g;
	material.diffuse.b = difColor.b;
	material.diffuse.a = difColor.a;

	const ColorF& specColor = pRes->GetColorValue(EFTT_SPECULAR);
	material.specular.r = specColor.r;
	material.specular.g = specColor.g;
	material.specular.b = specColor.b;
	material.specular.a = specColor.a;

	material.opacity = pRes->GetStrengthValue(EFTT_OPACITY);
	material.smoothness = pRes->GetStrengthValue(EFTT_SMOOTHNESS);

	SetTexture(material.mapDiffuse, pRes, EFTT_DIFFUSE);
	SetTexture(material.mapSpecular, pRes, EFTT_SPECULAR);
	SetTexture(material.mapOpacity, pRes, EFTT_OPACITY);
	SetTexture(material.mapNormals, pRes, EFTT_NORMALS);
	SetTexture(material.mapDecal, pRes, EFTT_DECAL_OVERLAY);
	SetTexture(material.mapDisplacement, pRes, EFTT_HEIGHT);
}

//////////////////////////////////////////////////////////
// CExportObject
SExportObject::SExportObject(tukk pName) :
	m_MeshHash(0)
{
	pos.x = pos.y = pos.z = 0;
	rot.v.x = rot.v.y = rot.v.z = 0;
	rot.w = 1.0f;
	scale.x = scale.y = scale.z = 1.0f;

	nParent = -1;

	drx_strcpy(name, pName);

	materialName[0] = '\0';

	entityType = Export::eEntity;

	cameraTargetNodeName[0] = '\0';

	m_pLastObject = 0;
}

void SExportObject::SetMaterialName(tukk pName)
{
	drx_strcpy(materialName, pName);
}

// SExportData
SExportObject* SExportData::AddObject(tukk objectName)
{
	for (size_t objectID = 0; objectID < m_objects.size(); ++objectID)
	{
		if (strcmp(m_objects[objectID]->name, objectName) == 0)
		{
			return m_objects[objectID];
		}
	}

	SExportObject* pObj = new SExportObject(objectName);
	m_objects.push_back(pObj);
	return m_objects[m_objects.size() - 1];
}

SExportObject* SExportData::AddObject(SExportObject* pObject)
{
	if (pObject)
	{
		m_objects.push_back(pObject);
		return pObject;
	}

	return nullptr;
}

void SExportData::Clear()
{
	m_objects.clear();
}

