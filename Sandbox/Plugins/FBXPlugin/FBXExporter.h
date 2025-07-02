// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <IExportManager.h>
#include "fbxsdk.h"

enum EAxis
{
	eAxis_Y,
	eAxis_Z
};

struct SFBXSettings
{
	bool  bCopyTextures;
	bool  bEmbedded;
	bool  bAsciiFormat;
	EAxis axis;
};

class CFBXExporter : public IExporter
{
public:
	CFBXExporter();

	virtual tukk GetExtension() const;
	virtual tukk GetShortDescription() const;
	virtual bool        ExportToFile(tukk filename, const SExportData* pData);
	virtual bool        ImportFromFile(tukk filename, SExportData* pData);
	virtual void        Release();

private:
	FbxMesh*            CreateFBXMesh(const SExportObject* pObj);
	FbxFileTexture*     CreateFBXTexture(tukk pTypeName, tukk pName);
	FbxSurfaceMaterial* CreateFBXMaterial(const STxt& name, const SExportMesh* pMesh);
	FbxNode*            CreateFBXNode(const SExportObject* pObj);
	FbxNode*            CreateFBXAnimNode(FbxScene* pScene, FbxAnimLayer* pCameraAnimBaseLayer, const SExportObject* pObj);
	void                FillAnimationData(SExportObject* pObject, FbxAnimLayer* pAnimLayer, FbxAnimCurve* pCurve, EAnimParamType paramType);

	FbxManager*                                m_pFBXManager;
	SFBXSettings                               m_settings;
	FbxScene*                                  m_pFBXScene;
	STxt                                m_path;
	std::vector<FbxNode*>                      m_nodes;
	std::map<STxt, FbxSurfaceMaterial*> m_materials;
	std::map<const SExportMesh*, i32>          m_meshMaterialIndices;
};

