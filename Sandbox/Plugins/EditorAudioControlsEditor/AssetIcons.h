// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <SharedData.h>
#include <DrxIcon.h>

namespace ACE
{
static DrxIcon s_errorIcon;
static DrxIcon s_environmentIcon;
static DrxIcon s_folderIcon;
static DrxIcon s_libraryIcon;
static DrxIcon s_parameterIcon;
static DrxIcon s_preloadIcon;
static DrxIcon s_stateIcon;
static DrxIcon s_switchIcon;
static DrxIcon s_triggerIcon;

//////////////////////////////////////////////////////////////////////////
inline void InitAssetIcons()
{
	s_errorIcon = DrxIcon("icons:Dialogs/dialog-error.ico");
	s_environmentIcon = DrxIcon("icons:audio/assets/environment.ico");
	s_folderIcon = DrxIcon("icons:General/Folder.ico");
	s_libraryIcon = DrxIcon("icons:General/File.ico");
	s_parameterIcon = DrxIcon("icons:audio/assets/parameter.ico");
	s_preloadIcon = DrxIcon("icons:audio/assets/preload.ico");
	s_stateIcon = DrxIcon("icons:audio/assets/state.ico");
	s_switchIcon = DrxIcon("icons:audio/assets/switch.ico");
	s_triggerIcon = DrxIcon("icons:audio/assets/trigger.ico");
}

//////////////////////////////////////////////////////////////////////////
inline DrxIcon const& GetAssetIcon(EAssetType const type)
{
	switch (type)
	{
	case EAssetType::Trigger:
		return s_triggerIcon;
		break;
	case EAssetType::Parameter:
		return s_parameterIcon;
		break;
	case EAssetType::Switch:
		return s_switchIcon;
		break;
	case EAssetType::State:
		return s_stateIcon;
		break;
	case EAssetType::Environment:
		return s_environmentIcon;
		break;
	case EAssetType::Preload:
		return s_preloadIcon;
		break;
	case EAssetType::Folder:
		return s_folderIcon;
		break;
	case EAssetType::Library:
		return s_libraryIcon;
		break;
	default:
		return s_errorIcon;
		break;
	}
}
} //endns ACE