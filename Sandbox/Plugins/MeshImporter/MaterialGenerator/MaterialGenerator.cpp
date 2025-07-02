// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "MaterialGenerator.h"
#include "CreateMaterialTask.h"
#include "DialogCommon.h"  // CSceneManager.

#include <Drx3DEngine/I3DEngine.h>
#include <Material/Material.h>

#include <QTemporaryDir>

CMaterialGenerator::CMaterialGenerator()
	: m_pTaskHost(nullptr)
	, m_pSceneManager(nullptr)
	, m_pTextureManager(nullptr)
{
}

void CMaterialGenerator::SetTaskHost(ITaskHost* pTaskHost)
{
	m_pTaskHost = pTaskHost;
}

void CMaterialGenerator::SetSceneManager(MeshImporter::CSceneManager* pSceneManager)
{
	m_pSceneManager = pSceneManager;
}

void CMaterialGenerator::SetTextureManager(const std::shared_ptr<CTextureManager>& pTextureManager)
{
	m_pTextureManager = pTextureManager;
}

void CMaterialGenerator::GenerateMaterialInternal(const string& mtlFilePath, uk pUserData)
{
	CCreateMaterialTask* pTask = new CCreateMaterialTask();
	pTask->SetMaterialFilePath(mtlFilePath);
	pTask->SetCallback(std::bind(&CMaterialGenerator::OnMaterialCreated, this, std::placeholders::_1));
	pTask->SetDisplayScene(m_pSceneManager->GetDisplayScene());
	pTask->SetTextureManager(m_pTextureManager);
	pTask->SetUserData(pUserData);

	if (m_pTaskHost)
	{
		m_pTaskHost->RunTask(pTask);
	}
	else
	{
		CAsyncTaskBase::CallBlocking(pTask);
	}
}

void CMaterialGenerator::GenerateMaterial(uk pUserData)
{
	GenerateMaterialInternal("", pUserData);
}

void CMaterialGenerator::GenerateMaterial(const string& mtlFilePath, uk pUserData)
{
	GenerateMaterialInternal(mtlFilePath, pUserData);
}

void CMaterialGenerator::GenerateTemporaryMaterial(uk pUserData)
{
	QTemporaryDir* pTempDir = CreateTemporaryDirectory().release();

	QString materialFilePath = pTempDir->path() + "/mi_caf_material.mtl";

	GenerateMaterialInternal(QtUtil::ToString(materialFilePath), pUserData);
}

void CMaterialGenerator::OnMaterialCreated(CCreateMaterialTask* pTask)
{
	if (!pTask->Succeeded())
	{
		return;
	}

	_smart_ptr<CMaterial> pMtl = pTask->GetMaterial();

	if (!pMtl)
	{
		return;
	}

	sigMaterialGenerated(pMtl, pTask->GetUserData()); // TODO: Memory leak!
}
