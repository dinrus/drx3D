// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Item.h"
#include <DrxIcon.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
static DrxIcon s_errorIcon;
static DrxIcon s_bankIcon;
static DrxIcon s_editorFolderIcon;
static DrxIcon s_eventIcon;
static DrxIcon s_folderIcon;
static DrxIcon s_mixerGroupIcon;
static DrxIcon s_ParameterIcon;
static DrxIcon s_returnIcon;
static DrxIcon s_snapshotIcon;
static DrxIcon s_vcaIcon;

static QString const s_emptyTypeName("");
static QString const s_bankTypeName("Bank");
static QString const s_editorFolderTypeName("Editor Folder");
static QString const s_eventTypeName("Event");
static QString const s_folderTypeName("Folder");
static QString const s_mixerGroupTypeName("Mixer Group");
static QString const s_parameterTypeName("Parameter");
static QString const s_returnTypeName("Return");
static QString const s_snapshotTypeName("Snapshot");
static QString const s_vcaTypeName("VCA");

//////////////////////////////////////////////////////////////////////////
inline void InitIcons()
{
	s_errorIcon = DrxIcon("icons:Dialogs/dialog-error.ico");
	s_bankIcon = DrxIcon("icons:audio/impl/fmod/bank.ico");
	s_editorFolderIcon = DrxIcon("icons:General/Folder.ico");
	s_eventIcon = DrxIcon("icons:audio/impl/fmod/event.ico");
	s_folderIcon = DrxIcon("icons:audio/impl/fmod/folder_closed.ico");
	s_mixerGroupIcon = DrxIcon("icons:audio/impl/fmod/group.ico");
	s_ParameterIcon = DrxIcon("icons:audio/impl/fmod/tag.ico");
	s_returnIcon = DrxIcon("icons:audio/impl/fmod/return.ico");
	s_snapshotIcon = DrxIcon("icons:audio/impl/fmod/snapshot.ico");
	s_vcaIcon = DrxIcon("icons:audio/impl/fmod/vca.ico");
}

//////////////////////////////////////////////////////////////////////////
inline DrxIcon const& GetTypeIcon(EItemType const type)
{
	switch (type)
	{
	case EItemType::Bank:
		return s_bankIcon;
		break;
	case EItemType::EditorFolder:
		return s_editorFolderIcon;
		break;
	case EItemType::Event:
		return s_eventIcon;
		break;
	case EItemType::Folder:
		return s_folderIcon;
		break;
	case EItemType::MixerGroup:
		return s_mixerGroupIcon;
		break;
	case EItemType::Parameter:
		return s_ParameterIcon;
		break;
	case EItemType::Return:
		return s_returnIcon;
		break;
	case EItemType::Snapshot:
		return s_snapshotIcon;
		break;
	case EItemType::VCA:
		return s_vcaIcon;
		break;
	default:
		return s_errorIcon;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
inline QString const& TypeToString(EItemType const type)
{
	switch (type)
	{
	case EItemType::Bank:
		return s_bankTypeName;
		break;
	case EItemType::EditorFolder:
		return s_editorFolderTypeName;
		break;
	case EItemType::Event:
		return s_eventTypeName;
		break;
	case EItemType::Folder:
		return s_folderTypeName;
		break;
	case EItemType::MixerGroup:
		return s_mixerGroupTypeName;
		break;
	case EItemType::Parameter:
		return s_parameterTypeName;
		break;
	case EItemType::Return:
		return s_returnTypeName;
		break;
	case EItemType::Snapshot:
		return s_snapshotTypeName;
		break;
	case EItemType::VCA:
		return s_vcaTypeName;
		break;
	default:
		return s_emptyTypeName;
		break;
	}
}
} //endns Fmod
} //endns Impl
} //endns ACE
