// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <StdAfx.h>
#include "ParticleAssetType.h"

#include <AssetSystem/Loader/AssetLoaderHelpers.h>
#include <AssetSystem/EditableAsset.h>
#include <FileDialogs/EngineFileDialog.h>
#include <FilePathUtil.h>

#include <DrxParticleSystem/IParticlesPfx2.h>

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

// #TODO Move this somewhere else.
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.inl>
#include <drx3D/CoreX/ToolsHelpers/SettingsManagerHelpers.inl>
#include <drx3D/CoreX/ToolsHelpers/EngineSettingsManager.inl>

REGISTER_ASSET_TYPE(CParticlesType)

namespace Private_ParticleAssetType
{

static string ShowSaveDialog()
{
	CEngineFileDialog::RunParams runParams;
	runParams.title = QStringLiteral("Save Particle Effects");
	runParams.extensionFilters << CExtensionFilter(QStringLiteral("Particle Effects (pfx)"), "pfx");

	const QString filePath = CEngineFileDialog::RunGameSave(runParams, nullptr);
	return QtUtil::ToString(filePath);
}

static bool MakeNewComponent(pfx2::IParticleEffectPfx2* pEffect)
{
	pfx2::IParticleComponent* const pComp = pEffect->AddComponent();
	if (!pComp)
	{
		return false;
	}

	const string templateName = "%ENGINE%/EngineAssets/Particles/Default.pfxp";

	return Serialization::LoadJsonFile(*pComp, templateName);
}

static string CreateAssetMetaData(const string& pfxFilePath)
{
	const string options = "/refresh /overwriteextension=cryasset";
	const CResourceCompilerHelper::ERcExePath path = CResourceCompilerHelper::eRcExePath_editor;
	const auto result = CResourceCompilerHelper::CallResourceCompiler(
	  pfxFilePath.c_str(),
	  options.c_str(),
	  nullptr,
	  false, // may show window?
	  path,
	  true,  // silent?
	  true); // no user dialog?
	if (result != CResourceCompilerHelper::eRcCallResult_success)
	{
		return string();
	}
	return pfxFilePath + ".cryasset";
}

} //endns Private_ParticleAssetType

struct CParticlesType::SCreateParams
{
	bool bUseExistingEffect;

	SCreateParams()
		: bUseExistingEffect(false)
	{
	}
};

DrxIcon CParticlesType::GetIconInternal() const
{
	return DrxIcon("icons:common/assets_particles.ico");
}

CAssetEditor* CParticlesType::Edit(CAsset* asset) const
{
	return CAssetEditor::OpenAssetForEdit("Particle Editor", asset);
}

bool CParticlesType::CreateForExistingEffect(tukk szFilePath) const
{
	SCreateParams params;
	params.bUseExistingEffect = true;
	return Create(szFilePath, &params);
}

bool CParticlesType::OnCreate(CEditableAsset& editAsset, ukk pCreateParams) const
{
	using namespace Private_ParticleAssetType;

	const string basePath = PathUtil::RemoveExtension(PathUtil::RemoveExtension(editAsset.GetAsset().GetMetadataFile()));

	const string pfxFilePath = basePath + ".pfx";

	std::shared_ptr<pfx2::IParticleSystem> pParticleSystem = pfx2::GetIParticleSystem();
	if (!pParticleSystem)
	{
		return false;
	}

	const bool bCreatePfxFile = !(pCreateParams && ((SCreateParams*)pCreateParams)->bUseExistingEffect);

	if (bCreatePfxFile)
	{
		pfx2::PParticleEffect pEffect = pParticleSystem->CreateEffect();
		pParticleSystem->RenameEffect(pEffect, pfxFilePath.c_str());
		MakeNewComponent(pEffect.get());

		if (!Serialization::SaveJsonFile(pfxFilePath.c_str(), *pEffect))
		{
			return false;
		}
	}

	editAsset.SetFiles("", { pfxFilePath });

	return true;
}

