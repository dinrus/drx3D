// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "FileLoader.h"

#include "FileWriter.h"
#include "AudioControlsEditorPlugin.h"

#include <DrxSystem/File/DrxFile.h>
#include <QtUtil.h>
#include <DrxGame/IGameFramework.h>
#include <ILevelSystem.h>

#include <QRegularExpression>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
EAssetType TagToType(char const* const szTag)
{
	EAssetType type = EAssetType::None;

	if (_stricmp(szTag, DrxAudio::s_szSwitchTag) == 0)
	{
		type = EAssetType::Switch;
	}
	else if (_stricmp(szTag, DrxAudio::s_szStateTag) == 0)
	{
		type = EAssetType::State;
	}
	else if (_stricmp(szTag, DrxAudio::s_szEnvironmentTag) == 0)
	{
		type = EAssetType::Environment;
	}
	else if (_stricmp(szTag, DrxAudio::s_szParameterTag) == 0)
	{
		type = EAssetType::Parameter;
	}
	else if (_stricmp(szTag, DrxAudio::s_szTriggerTag) == 0)
	{
		type = EAssetType::Trigger;
	}
	else if (_stricmp(szTag, DrxAudio::s_szPreloadRequestTag) == 0)
	{
		type = EAssetType::Preload;
	}
	else
	{
		type = EAssetType::None;
	}

	return type;
}

//////////////////////////////////////////////////////////////////////////
CFileLoader::CFileLoader()
	: m_errorCodeMask(EErrorCode::None)
{}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadAll()
{
	LoadScopes();
	LoadControls();
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadControls()
{
	// load the global controls
	LoadAllLibrariesInFolder(g_assetsManager.GetConfigFolderPath(), "");

	// load the level specific controls
	_finddata_t fd;
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	intptr_t handle = pDrxPak->FindFirst(g_assetsManager.GetConfigFolderPath() + DrxAudio::s_szLevelsFolderName + "/*.*", &fd);

	if (handle != -1)
	{
		do
		{
			if (fd.attrib & _A_SUBDIR)
			{
				string name = fd.name;

				if ((name != ".") && (name != ".."))
				{
					LoadAllLibrariesInFolder(g_assetsManager.GetConfigFolderPath(), name);

					if (!g_assetsManager.ScopeExists(fd.name))
					{
						// if the control doesn't exist it
						// means it is not a real level in the
						// project so it is flagged as LocalOnly
						g_assetsManager.AddScope(fd.name, true);
					}
				}
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}

	CreateDefaultControls();
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadAllLibrariesInFolder(string const& folderPath, string const& level)
{
	string path = folderPath;

	if (!level.empty())
	{
		path = path + DrxAudio::s_szLevelsFolderName + "/" + level + "/";
	}

	string const searchPath = path + "*.xml";
	IDrxPak* const pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;
	intptr_t const handle = pDrxPak->FindFirst(searchPath, &fd);

	if (handle != -1)
	{
		do
		{
			string filename = path + fd.name;
			XmlNodeRef const root = GetISystem()->LoadXmlFromFile(filename);

			if (root != nullptr)
			{
				if (_stricmp(root->getTag(), DrxAudio::s_szRootNodeTag) == 0)
				{
					m_loadedFilenames.insert(filename.MakeLower());
					string file = fd.name;

					if (root->haveAttr(DrxAudio::s_szNameAttribute))
					{
						file = root->getAttr(DrxAudio::s_szNameAttribute);
					}

					i32 version = 1;
					root->getAttr(DrxAudio::s_szVersionAttribute, version);
					PathUtil::RemoveExtension(file);
					LoadControlsLibrary(root, filename, level, file, version);
				}
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "[Audio Controls Editor] Failed parsing audio system data file %s", filename);
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}
}

//////////////////////////////////////////////////////////////////////////
CAsset* CFileLoader::AddUniqueFolderPath(CAsset* pParent, QString const& path)
{
	QStringList const folderNames = path.split(QRegularExpression(R"((\\|\/))"), QString::SkipEmptyParts);

	i32 const numFolders = folderNames.length();

	for (i32 i = 0; i < numFolders; ++i)
	{
		if (!folderNames[i].isEmpty())
		{
			CAsset* const pChild = g_assetsManager.CreateFolder(QtUtil::ToString(folderNames[i]), pParent);

			if (pChild != nullptr)
			{
				pParent = pChild;
			}
		}
	}

	return pParent;
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadControlsLibrary(XmlNodeRef const pRoot, string const& filepath, string const& level, string const& filename, u32 const version)
{
	// Always create a library file, even if no proper formatting is present.
	CLibrary* const pLibrary = g_assetsManager.CreateLibrary(filename);

	if (pLibrary != nullptr)
	{
		pLibrary->SetPakStatus(EPakStatus::InPak, gEnv->pDrxPak->IsFileExist(filepath.c_str(), IDrxPak::eFileLocation_InPak));
		pLibrary->SetPakStatus(EPakStatus::OnDisk, gEnv->pDrxPak->IsFileExist(filepath.c_str(), IDrxPak::eFileLocation_OnDisk));

		i32 const controlTypeCount = pRoot->getChildCount();

		for (i32 i = 0; i < controlTypeCount; ++i)
		{
			XmlNodeRef const pNode = pRoot->getChild(i);

			if (pNode != nullptr)
			{
				if (pNode->isTag(DrxAudio::s_szEditorDataTag))
				{
					LoadEditorData(pNode, *pLibrary);
				}
				else
				{
					Scope const scope = level.empty() ? GlobalScopeId : g_assetsManager.GetScope(level);
					i32 const numControls = pNode->getChildCount();

					for (i32 j = 0; j < numControls; ++j)
					{
						LoadControl(pNode->getChild(j), scope, version, pLibrary);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CControl* CFileLoader::LoadControl(XmlNodeRef const pNode, Scope const scope, u32 const version, CAsset* const pParentItem)
{
	CControl* pControl = nullptr;

	if (pNode != nullptr)
	{
		CAsset* const pFolderItem = AddUniqueFolderPath(pParentItem, QtUtil::ToQString(pNode->getAttr(s_szPathAttribute)));

		if (pFolderItem != nullptr)
		{
			string const name = pNode->getAttr(DrxAudio::s_szNameAttribute);
			EAssetType const controlType = TagToType(pNode->getTag());

			pControl = g_assetsManager.CreateControl(name, controlType, pFolderItem);

			if (pControl != nullptr)
			{
				switch (controlType)
				{
				case EAssetType::Trigger:
					{
						float radius = 0.0f;
						pNode->getAttr(DrxAudio::s_szRadiusAttribute, radius);
						pControl->SetRadius(radius);
						LoadConnections(pNode, pControl);
					}
					break;
				case EAssetType::Switch:
					{
						i32 const stateCount = pNode->getChildCount();

						for (i32 i = 0; i < stateCount; ++i)
						{
							LoadControl(pNode->getChild(i), scope, version, pControl);
						}
					}
					break;
				case EAssetType::Preload:
					LoadPreloadConnections(pNode, pControl, version);
					break;
				default:
					LoadConnections(pNode, pControl);
					break;
				}

				pControl->SetScope(scope);
			}
		}
	}

	return pControl;
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadScopes()
{
	ILevelSystem* const pLevelSystem = gEnv->pGameFramework->GetILevelSystem();

	if (pLevelSystem != nullptr)
	{
		i32 const levelCount = pLevelSystem->GetLevelCount();

		for (i32 i = 0; i < levelCount; ++i)
		{
			string const& name = pLevelSystem->GetLevelInfo(i)->GetName();
			g_assetsManager.AddScope(name);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
FileNames CFileLoader::GetLoadedFilenamesList()
{
	return m_loadedFilenames;
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::CreateInternalControls()
{
	// Create internal default controls.
	// These controls are hidden in the ACE and don't get written to XML!
	CAsset* const pLibrary = static_cast<CAsset*>(g_assetsManager.CreateLibrary(DrxAudio::s_szDefaultLibraryName));

	DRX_ASSERT_MESSAGE(pLibrary != nullptr, "Default library could not get created.");

	if (pLibrary != nullptr)
	{
		pLibrary->SetDescription("Contains all engine default controls.");
		pLibrary->SetFlags(pLibrary->GetFlags() | EAssetFlags::IsDefaultControl);

		g_assetsManager.CreateDefaultControl(DrxAudio::s_szDoNothingTriggerName, EAssetType::Trigger, pLibrary, true, "Used to bypass the default stop behavior of the audio system.");

		SwitchStates const occlStates {
			DrxAudio::s_szIgnoreStateName, DrxAudio::s_szAdaptiveStateName, DrxAudio::s_szLowStateName, DrxAudio::s_szMediumStateName, DrxAudio::s_szHighStateName
		};
		CreateInternalSwitch(pLibrary, DrxAudio::s_szOcclCalcSwitchName, occlStates, "Set the occlusion type of the object.");

		SwitchStates const onOffStates {
			DrxAudio::s_szOnStateName, DrxAudio::s_szOffStateName
		};

		string description;
		description.Format(R"(If enabled on an object, its "%s" parameter gets updated.)", DrxAudio::s_szAbsoluteVelocityParameterName);
		CreateInternalSwitch(pLibrary, DrxAudio::s_szAbsoluteVelocityTrackingSwitchName, onOffStates, description.c_str());

		description.Format(R"(If enabled on an object, its "%s" parameter gets updated.)", DrxAudio::s_szRelativeVelocityParameterName);
		CreateInternalSwitch(pLibrary, DrxAudio::s_szRelativeVelocityTrackingSwitchName, onOffStates, description.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::CreateInternalSwitch(CAsset* const pLibrary, char const* const szSwitchName, SwitchStates const& stateNames, char const* const szDescription)
{
	CControl* const pSwitch = g_assetsManager.CreateDefaultControl(szSwitchName, EAssetType::Switch, pLibrary, true, szDescription);

	if (pSwitch != nullptr)
	{
		for (auto const& szStateName : stateNames)
		{
			g_assetsManager.CreateDefaultControl(szStateName, EAssetType::State, pSwitch, true, szDescription);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::CreateDefaultControls()
{
	// Create default controls if they don't exist.
	// These controls need to always exist in your project!
	CAsset* const pLibrary = static_cast<CAsset*>(g_assetsManager.CreateLibrary(DrxAudio::s_szDefaultLibraryName));

	if (pLibrary != nullptr)
	{
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szGetFocusTriggerName, EAssetType::Trigger, pLibrary, false, "Unmutes all audio. Gets triggered when the editor window gets focus.");
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szLoseFocusTriggerName, EAssetType::Trigger, pLibrary, false, "Mutes all audio. Gets triggered when the editor window loses focus.");
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szMuteAllTriggerName, EAssetType::Trigger, pLibrary, false, "Mutes all audio. Gets triggered when the editor mute action is used.");
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szUnmuteAllTriggerName, EAssetType::Trigger, pLibrary, false, "Unmutes all audio. Gets triggered when the editor unmute action is used.");
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szPauseAllTriggerName, EAssetType::Trigger, pLibrary, false, "Pauses playback of all audio.");
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szResumeAllTriggerName, EAssetType::Trigger, pLibrary, false, "Resumes playback of all audio.");

		string description;
		description.Format(R"(Updates the absolute velocity of an object, if its "%s" switch is enabled.)", DrxAudio::s_szAbsoluteVelocityTrackingSwitchName);
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szAbsoluteVelocityParameterName, EAssetType::Parameter, pLibrary, false, description.c_str());
		description.Format(R"(Updates the absolute velocity of an object, if its "%s" switch is enabled.)", DrxAudio::s_szRelativeVelocityTrackingSwitchName);
		g_assetsManager.CreateDefaultControl(DrxAudio::s_szRelativeVelocityParameterName, EAssetType::Parameter, pLibrary, false, description.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadConnections(XmlNodeRef const pRoot, CControl* const pControl)
{
	// The radius might change because of the attenuation matching option
	// so we check here to inform the user if their data is outdated.
	float const radius = pControl->GetRadius();
	i32 const numChildren = pRoot->getChildCount();

	for (i32 i = 0; i < numChildren; ++i)
	{
		XmlNodeRef const pNode = pRoot->getChild(i);
		pControl->LoadConnectionFromXML(pNode);
	}

	if (radius != pControl->GetRadius())
	{
		m_errorCodeMask |= EErrorCode::NonMatchedActivityRadius;
		pControl->SetModified(true, true);
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadPreloadConnections(XmlNodeRef const pNode, CControl* const pControl, u32 const version)
{
	if (_stricmp(pNode->getAttr(DrxAudio::s_szTypeAttribute), DrxAudio::s_szDataLoadType) == 0)
	{
		pControl->SetAutoLoad(true);
	}
	else
	{
		pControl->SetAutoLoad(false);
	}

	i32 const numChildren = pNode->getChildCount();

	for (i32 i = 0; i < numChildren; ++i)
	{
		// Skip unused data from previous format
		XmlNodeRef const pPlatformNode = pNode->getChild(i);

		if (_stricmp(pPlatformNode->getTag(), DrxAudio::s_szPlatformTag) == 0)
		{
			// Get the index for that platform name
			i32 platformIndex = -1;
			bool foundPlatform = false;
			char const* const szPlatformName = pPlatformNode->getAttr(DrxAudio::s_szNameAttribute);

			for (auto const szPlatform : g_platforms)
			{
				++platformIndex;

				if (_stricmp(szPlatformName, szPlatform) == 0)
				{
					foundPlatform = true;
					break;
				}
			}

			if (!foundPlatform)
			{
				m_errorCodeMask |= EErrorCode::UnkownPlatform;
				pControl->SetModified(true, true);
			}

			i32 const numConnections = pPlatformNode->getChildCount();

			for (i32 j = 0; j < numConnections; ++j)
			{
				XmlNodeRef const pConnectionNode = pPlatformNode->getChild(j);

				if (pConnectionNode != nullptr)
				{
					pControl->LoadConnectionFromXML(pConnectionNode, platformIndex);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadEditorData(XmlNodeRef const pEditorDataNode, CAsset& library)
{
	i32 const numChildren = pEditorDataNode->getChildCount();

	for (i32 i = 0; i < numChildren; ++i)
	{
		XmlNodeRef const pChild = pEditorDataNode->getChild(i);

		if (pChild->isTag(s_szLibraryNodeTag))
		{
			LoadLibraryEditorData(pChild, library);
		}
		else if (pChild->isTag(s_szFoldersNodeTag))
		{
			LoadAllFolders(pChild, library);
		}
		else if (pChild->isTag(s_szControlsNodeTag))
		{
			LoadAllControlsEditorData(pChild);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadLibraryEditorData(XmlNodeRef const pLibraryNode, CAsset& library)
{
	string description = "";
	pLibraryNode->getAttr(s_szDescriptionAttribute, description);

	if (!description.IsEmpty())
	{
		library.SetDescription(description);
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadAllFolders(XmlNodeRef const pFoldersNode, CAsset& library)
{
	if (pFoldersNode != nullptr)
	{
		i32 const numChildren = pFoldersNode->getChildCount();

		for (i32 i = 0; i < numChildren; ++i)
		{
			LoadFolderData(pFoldersNode->getChild(i), library);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadFolderData(XmlNodeRef const pFolderNode, CAsset& parentAsset)
{
	CAsset* const pAsset = AddUniqueFolderPath(&parentAsset, pFolderNode->getAttr(DrxAudio::s_szNameAttribute));

	if (pAsset != nullptr)
	{
		string description = "";
		pFolderNode->getAttr(s_szDescriptionAttribute, description);

		if (!description.IsEmpty())
		{
			pAsset->SetDescription(description);
		}

		i32 const numChildren = pFolderNode->getChildCount();

		for (i32 i = 0; i < numChildren; ++i)
		{
			LoadFolderData(pFolderNode->getChild(i), *pAsset);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadAllControlsEditorData(XmlNodeRef const pControlsNode)
{
	if (pControlsNode != nullptr)
	{
		i32 const numChildren = pControlsNode->getChildCount();

		for (i32 i = 0; i < numChildren; ++i)
		{
			LoadControlsEditorData(pControlsNode->getChild(i));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFileLoader::LoadControlsEditorData(XmlNodeRef const pParentNode)
{
	if (pParentNode != nullptr)
	{
		EAssetType const controlType = TagToType(pParentNode->getTag());
		string description = "";
		pParentNode->getAttr(s_szDescriptionAttribute, description);

		if ((controlType != EAssetType::None) && !description.IsEmpty())
		{
			CControl* const pControl = g_assetsManager.FindControl(pParentNode->getAttr(DrxAudio::s_szNameAttribute), controlType);

			if (pControl != nullptr)
			{
				pControl->SetDescription(description);
			}
		}
	}
}
} //endns ACE

