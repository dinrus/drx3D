// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Item.h"

#include <DrxIcon.h>
#include <FileDialogs/ExtensionFilter.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
static DrxIcon s_errorIcon;
static DrxIcon s_eventIcon;
static DrxIcon s_folderIcon;

static QString const s_emptyTypeName("");
static QString const s_eventTypeName("Audio File");
static QString const s_folderTypeName("Folder");

static QStringList const s_supportedFileTypes {
	"ogg", "wav"
};

static ExtensionFilterVector const s_extensionFilters(
			{
				CExtensionFilter("Ogg Vorbis", "ogg"),
				CExtensionFilter("Wave (Microsoft)", "wav")
      });

//////////////////////////////////////////////////////////////////////////
inline void InitIcons()
{
	s_errorIcon = DrxIcon("icons:Dialogs/dialog-error.ico");
	s_eventIcon = DrxIcon("icons:audio/impl/portaudio/event.ico");
	s_folderIcon = DrxIcon("icons:General/Folder.ico");
}

//////////////////////////////////////////////////////////////////////////
inline DrxIcon const& GetTypeIcon(EItemType const type)
{
	switch (type)
	{
	case EItemType::Event:
		return s_eventIcon;
		break;
	case EItemType::Folder:
		return s_folderIcon;
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
	case EItemType::Event:
		return s_eventTypeName;
		break;
	case EItemType::Folder:
		return s_folderTypeName;
		break;
	default:
		return s_emptyTypeName;
		break;
	}
}
} //endns PortAudio
} //endns Impl
} //endns ACE
