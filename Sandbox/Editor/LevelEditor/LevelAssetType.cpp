// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include "LevelAssetType.h"
#include "IEditorImpl.h"
#include "LevelEditor.h" // CLevelEditor::tr
#include "LevelIndependentFileMan.h"
#include "LevelEditor/NewLevelDialog.h"
#include "LevelFileUtils.h"
#include "DrxEdit.h"
#include "DrxEditDoc.h"
#include "GameExporter.h"
#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/DependencyTracker.h"
#include "ThreadingUtils.h"
#include <Preferences/GeneralPreferences.h>
#include <DrxSandbox/ScopedVariableSetter.h>
#include <drx3D/CoreX/String/DrxPath.h>
#include "QtUtil.h"
#include <QDirIterator> 
#include <FilePathUtil.h>

REGISTER_ASSET_TYPE(CLevelType)

namespace Private_LevelAssetType
{

std::vector<SAssetDependencyInfo> GetDependencies(const CAsset& asset)
{
	DRX_ASSERT(asset.GetFilesCount());

	const CAssetManager* const pAssetManager = GetIEditorImpl()->GetAssetManager();
	std::vector<CAssetType*> types(pAssetManager->GetAssetTypes());
	std::sort(types.begin(), types.end(), [](const auto a, const auto b)
{
		return strcmp(a->GetFileExtension(), b->GetFileExtension()) < 0;
	});

	std::vector<SAssetDependencyInfo> dependencies;
	dependencies.reserve(32);

	const string levelFolder(PathUtil::GetPathWithoutFilename(asset.GetFile(0)));
	const string gameFolder(PathUtil::AddSlash(gEnv->pDrxPak->GetGameFolder()));

	IResourceList* pResList = gEnv->pDrxPak->GetResourceList(IDrxPak::RFOM_Level);
	for (tukk szFilename = pResList->GetFirst(); szFilename; szFilename = pResList->GetNext())
	{
		// Make relative to the game folder
		if (strnicmp(szFilename, gameFolder.c_str(), gameFolder.size()) == 0)
		{
			szFilename += gameFolder.size();
		}

		// Ignore file types that are not registered as assets.
		tukk const szExt = PathUtil::GetExt(szFilename);
		const auto it = std::lower_bound(types.begin(), types.end(), szExt, [](const auto a, const auto b)
		{
			return stricmp(a->GetFileExtension(), b) < 0;
		});
		if (it == types.end() || stricmp((*it)->GetFileExtension(), szExt))
		{
			continue;
		}

		// Ignore content of the level folder
		if (strnicmp(szFilename, levelFolder.c_str(), levelFolder.size()) == 0)
		{
			continue;
		}

		dependencies.emplace_back(szFilename,0);
	}

	// Filter sub-dependencies out. 

	std::sort(dependencies.begin(), dependencies.end(), [](const auto& a, const auto& b) 
	{
		return a.path < b.path;
	});

	for (size_t i = 0, N = dependencies.size(); i < N; ++i)
	{
		if (dependencies[i].path.empty())
		{
			continue;
		}

		const CAsset* pSubAsset = pAssetManager->FindAssetForFile(dependencies[i].path);
		if (!pSubAsset)
		{
			continue;
		}

		const std::vector<SAssetDependencyInfo>& subDependencies = pSubAsset->GetDependencies();
		for (const auto& item : subDependencies)
		{
			tukk szFilename = item.path.c_str();
			const auto it = std::lower_bound(dependencies.begin(), dependencies.end(), szFilename, [](const auto a, const auto b)
			{
				return stricmp(a.path.c_str(), b) < 0;
			});

			if (it != dependencies.end() && it->path.Compare(szFilename) == 0)
			{
				it->path.clear();
			}
		}
	}

	dependencies.erase(std::remove_if(dependencies.begin(), dependencies.end(), [](const auto x) 
	{
		return x.path.empty();
	}), dependencies.end());

	return dependencies;
}

}

CAssetEditor* CLevelType::Edit(CAsset* pAsset) const
{
	// Editing the level type presents a special case, as it does not return an asset editor.
	// Instead we load the level.
	auto levelFullPath = PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), pAsset->GetFile(0));
	CDrxEditApp::GetInstance()->LoadLevel(PathUtil::AbsolutePathToDrxPakPath(levelFullPath));

	return nullptr;
}

bool CLevelType::DeleteAssetFiles(const CAsset& asset, bool bDeleteSourceFile, size_t& numberOfFilesDeleted) const
{
	numberOfFilesDeleted = 0;
	const string activeLevelPath = PathUtil::GetDirectory(GetIEditorImpl()->GetDocument()->GetPathName());
	const string levelPath = PathUtil::Make(PathUtil::GetGameFolder(), PathUtil::GetDirectory(asset.GetFile(0)));
	if (activeLevelPath.CompareNoCase(levelPath) == 0)
	{
		GetIEditorImpl()->GetSystem()->GetIPak()->ClosePacks(PathUtil::Make(activeLevelPath, "*.*"));
	}

	if (!CAssetType::DeleteAssetFiles(asset, bDeleteSourceFile, numberOfFilesDeleted))
	{
		return false;
	}

	// Make sure none of the level files are read only.
	QDirIterator iterator(QtUtil::ToQString(levelPath), QDirIterator::Subdirectories);
	while (iterator.hasNext())
	{
		QFileInfo fileInfo(iterator.next());
		if (!fileInfo.isWritable())
		{
			DrxWarning(EValidatorModule::VALIDATOR_MODULE_EDITOR, EValidatorSeverity::VALIDATOR_WARNING, "File is read-only: %s", QtUtil::ToString(fileInfo.absoluteFilePath()));
			return false;
		}
	}

	// Asynchronously delete the level directory. 
	// A new level can not be created while the old one is still being deleted. We keep the most recent future result to be able to wait for deletion requests to complete.
	// see CLevelType::OnCreate 
	m_asyncAction = ThreadingUtils::AsyncQueue([levelPath]()
	{
		return QDir(QtUtil::ToQString(levelPath)).removeRecursively();
	});
	return true;
}

DrxIcon CLevelType::GetIconInternal() const
{
	return DrxIcon("icons:FileType/Level.ico");
}

void CLevelType::UpdateDependencies(CEditableAsset& editAsset)
{
	std::vector<SAssetDependencyInfo> dependencies = Private_LevelAssetType::GetDependencies(editAsset.GetAsset());
	if (dependencies.size())
	{
		editAsset.SetDependencies(std::move(dependencies));
	}
}

bool CLevelType::OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const
{
	SCreateParams params;

	if (pTypeSpecificParameter)
	{
		params = *static_cast<const SCreateParams*>(pTypeSpecificParameter);
	}
	else
	{
		bool isProceed = GetIEditorImpl()->GetDocument()->CanClose();
		if (!isProceed)
		{
			return false;
		}

		if (!GetIEditorImpl()->GetLevelIndependentFileMan()->PromptChangedFiles())
		{
			return false;
		}

		CNewLevelDialog dialog;
		if (dialog.exec() != QDialog::Accepted)
		{
			return false;
		}
		params = dialog.GetResult();
	}

	// Waiting for levels removal to complete.
	// see CLevelType::DeleteAssetFiles
	if (m_asyncAction.valid())
	{
		m_asyncAction.get();
	}

	string levelFolder = PathUtil::RemoveExtension(PathUtil::RemoveExtension(editAsset.GetAsset().GetMetadataFile()));
	
	auto createResult = CDrxEditApp::GetInstance()->CreateLevel(levelFolder.c_str(), params.resolution, params.unitSize, params.bUseTerrain);

	switch (createResult)
	{
	case CDrxEditApp::ECLR_OK:
		break;

	case CDrxEditApp::ECLR_ALREADY_EXISTS:
		CQuestionDialog::SWarning(CLevelEditor::tr("New Level failed"), CLevelEditor::tr("Level with this name already exists, please choose another name."));
		return false;

	case CDrxEditApp::ECLR_DIR_CREATION_FAILED:
			{
			const auto errorCode = GetLastError();
			const QString errorText = qt_error_string(errorCode);
			const QString dir = QtUtil::ToQString(levelFolder);
			const QString messageText = CLevelEditor::tr("Failed to create level directory: %1\nError: %1").arg(dir).arg(errorText);

			CQuestionDialog::SWarning(CLevelEditor::tr("New Level failed"), messageText);
			return false;
			}
	default:
		CQuestionDialog::SWarning(CLevelEditor::tr("New Level failed"), CLevelEditor::tr("Unknown error"));
		return false;
	}

	if (params.bUseTerrain)
	{
		// Temporarily disable auto backup.
		CScopedVariableSetter<bool> autoBackupEnabledChange(gEditorFilePreferences.autoSaveEnabled, false);

		SGameExporterSettings settings;
		settings.iExportTexWidth = params.texture.resolution;
		settings.bHiQualityCompression = params.texture.bHighQualityCompression;
		settings.fBrMultiplier = params.texture.colorMultiplier;
		settings.eExportEndian = eLittleEndian;

		CGameExporter gameExporter(settings);

		auto exportFlags = eExp_SurfaceTexture | eExp_ReloadTerrain;
		gameExporter.Export(exportFlags, ".");
	}

	editAsset.AddFile(string().Format("%s/%s.%s", levelFolder.c_str(), PathUtil::GetFile(levelFolder).c_str(), editAsset.GetAsset().GetType()->GetFileExtension()));
	UpdateDependencies(editAsset);
	return true;
}


