// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include "CSharpAssetType.h"

#include <AssetSystem/Loader/AssetLoaderHelpers.h>
#include <AssetSystem/Asset.h>

#include <DrxSystem/IProjectManager.h>

#include <cctype> 
#include <clocale>

REGISTER_ASSET_TYPE(CSharpSourcefileAssetType)

DrxIcon CSharpSourcefileAssetType::GetIconInternal() const
{
	return DrxIcon("icons:csharp/assets_csharp.ico");
}

string CSharpSourcefileAssetType::GetCleanName(const string& name) const
{
	string cleanName;
	cleanName.reserve(name.size());
	for (i32 c : name)
	{
		if (std::isalpha(c) != 0)
		{
			cleanName += c;
		}
		else if (std::isdigit(c) != 0)
		{
			cleanName += c;
		}
	}
	return cleanName;
}

CAssetEditor* CSharpSourcefileAssetType::Edit(CAsset* pAsset) const
{
#if DRX_PLATFORM_WINDOWS
	if (pAsset->GetFilesCount() > 0)
	{
		string filePath = PathUtil::Make(gEnv->pSystem->GetIProjectManager()->GetCurrentAssetDirectoryAbsolute(), pAsset->GetFile(0));
		CCSharpEditorPlugin::GetInstance()->OpenCSharpFile(filePath, 10);
	}
#endif

	return nullptr;
}

bool CSharpSourcefileAssetType::OnCreate(CEditableAsset& editAsset, ukk pCreateParams) const
{
	const string basePath = PathUtil::RemoveExtension(PathUtil::RemoveExtension(editAsset.GetAsset().GetMetadataFile()));
	const string csFilePath = basePath + ".cs";
	const string assetName = PathUtil::GetFileName(basePath);

	string projectName = gEnv->pSystem->GetIProjectManager()->GetCurrentProjectName();

	string cleanProjectName = GetCleanName(projectName);
	string cleanAssetName = GetCleanName(assetName);
	DrxGUID guid = DrxGUID::Create();

	CDrxFile assetFile(csFilePath.c_str(), "wb", IDrxPak::FLAGS_NO_LOWCASE);
	if (assetFile.GetHandle() != nullptr)
	{
		string assetContents = gEnv->pSystem->GetIProjectManager()->LoadTemplateFile("%ENGINE%/EngineAssets/Templates/ManagedAsset.cs.txt", [this, cleanProjectName, cleanAssetName, guid](tukk szAlias) -> string
		{
			if (!strcmp(szAlias, "namespace"))
			{
				return cleanProjectName;
			}
			else if (!strcmp(szAlias, "guid"))
			{
				return guid.ToString();
			}
			else if (!strcmp(szAlias, "class_name"))
			{
				return cleanAssetName;
			}

			DRX_ASSERT_MESSAGE(false, "Unhandled alias!");
			return "";
		});

		if (assetFile.Write(assetContents.data(), assetContents.size()))
		{
			editAsset.SetFiles("", { csFilePath });
		}
	}

	return true;
}

