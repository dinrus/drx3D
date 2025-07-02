// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Item.h"

#include <DrxSystem/XML/IXml.h>

namespace ACE
{
namespace Impl
{
namespace Wwise
{
class CProjectLoader final
{
public:

	explicit CProjectLoader(string const& projectPath, string const& soundbanksPath, CItem& rootItem, ItemCache& itemCache);

	CProjectLoader() = delete;

private:

	void   LoadSoundBanks(string const& folderPath, bool const isLocalized, CItem& parent);
	void   LoadFolder(string const& folderPath, string const& folderName, CItem& parent);
	void   LoadWorkUnitFile(string const& filePath, CItem& parent, EPakStatus const pakStatus);
	void   LoadXml(XmlNodeRef const root, CItem& parent, EPakStatus const pakStatus);
	CItem* CreateItem(string const& name, EItemType const type, CItem& pParent, EPakStatus const pakStatus);
	void   LoadEventsMetadata(string const& soundbanksPath);

	void   BuildFileCache(string const& folderPath);

private:

	struct SEventInfo
	{
		float maxRadius;
	};

	using EventsInfoMap = std::map<u32, SEventInfo>;
	using FilesCache = std::map<u32, string>;
	using Items = std::map<u32, CItem*>;

	EventsInfoMap m_eventsInfoMap;
	CItem&        m_rootItem;
	ItemCache&    m_itemCache;
	string const  m_projectPath;

	// This maps holds the items with the internal IDs given in the Wwise files.
	Items m_items;

	// Cache with the file path to each work unit file
	FilesCache m_filesCache;

	// List of already loaded work unit files
	std::set<u32> m_filesLoaded;
};
} //endns Wwise
} //endns Impl
} //endns ACE

