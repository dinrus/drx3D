// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <drx3D/CoreX/String/DrxString.h>

#include <memory>

namespace MeshImporter
{

class CSceneManager;

} //endns MeshImporter

class CCreateMaterialTask;
class CTextureManager;
struct ITaskHost;

class CMaterial;

class CMaterialGenerator
{
public:
	CMaterialGenerator();

	void SetTaskHost(ITaskHost* pTaskHost);
	void SetSceneManager(MeshImporter::CSceneManager* pSceneManager);
	void SetTextureManager(const std::shared_ptr<CTextureManager>& pTextureManager);

	void GenerateMaterial(uk pUserData = nullptr);
	void GenerateMaterial(const string& mtlFilePath, uk pUserData = nullptr);
	void GenerateTemporaryMaterial(uk pUserData = nullptr);

	CDrxSignal<void(CMaterial* pMaterial, uk pUserData)> sigMaterialGenerated;
private:
	void GenerateMaterialInternal(const string& mtlFilePath, uk pUserData);
	void OnMaterialCreated(CCreateMaterialTask* pTask);

private:
	ITaskHost* m_pTaskHost;
	MeshImporter::CSceneManager* m_pSceneManager;
	std::shared_ptr<CTextureManager> m_pTextureManager;
};
