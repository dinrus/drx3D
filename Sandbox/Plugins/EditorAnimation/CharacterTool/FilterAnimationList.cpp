// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <drx3D/CoreX/Platform/platform.h>
#include <DrxSystem/File/IDrxPak.h>
#include <IEditor.h>
#include <DrxSystem/File/IFileChangeMonitor.h>
#include "FilterAnimationList.h"
#include "../Shared/AnimSettings.h"

namespace CharacterTool
{

FilterAnimationList::FilterAnimationList()
	: m_revision(0)
{
	GetIEditor()->GetFileMonitor()->RegisterListener(this, "", "animsettings");
}

FilterAnimationList::~FilterAnimationList()
{
	GetIEditor()->GetFileMonitor()->UnregisterListener(this);
}

void FilterAnimationList::Populate()
{
	m_items.clear();

	std::vector<string> filenames;

	SDirectoryEnumeratorHelper dirHelper;
	dirHelper.ScanDirectoryRecursive("", "", "*.animsettings", filenames);

	for (size_t i = 0; i < filenames.size(); ++i)
	{
		tukk filename = filenames[i];
		SAnimSettings settings;

		string animSettingsFilename = SAnimSettings::GetAnimSettingsFilename(filename);
		if (settings.Load(animSettingsFilename.c_str(), std::vector<string>(), 0, 0))
		{
			SAnimationFilterItem filterItem;
			filterItem.tags = settings.build.tags;
			filterItem.skeletonAlias = settings.build.skeletonAlias;
			filterItem.path = PathUtil::ReplaceExtension(PathUtil::ToUnixPath(filename), "caf");
			m_items.push_back(filterItem);
		}
	}
}

void FilterAnimationList::UpdateItem(tukk filename)
{
	string animationPath = PathUtil::ReplaceExtension(PathUtil::ToUnixPath(filename), "caf");
	i32 numItems = m_items.size();
	bool updated = false;
	for (i32 i = 0; i < numItems; ++i)
	{
		if (stricmp(m_items[i].path.c_str(), animationPath) == 0)
		{
			SAnimationFilterItem& filterItem = m_items[i];
			SAnimSettings settings;
			string animSettingsFilename = SAnimSettings::GetAnimSettingsFilename(filename);
			if (settings.Load(animSettingsFilename.c_str(), std::vector<string>(), 0, 0))
			{
				filterItem.tags = settings.build.tags;
			}
			else
			{
				i -= RemoveItem(filename);
			}
			updated = true;
		}
	}

	if (!updated)
	{
		SAnimSettings settings;
		string animSettingsFilename = SAnimSettings::GetAnimSettingsFilename(filename);
		if (settings.Load(animSettingsFilename.c_str(), std::vector<string>(), 0, 0))
		{
			SAnimationFilterItem filterItem;
			filterItem.tags = settings.build.tags;
			filterItem.path = animationPath;
			m_items.push_back(filterItem);
		}
	}
	++m_revision;
}

i32 FilterAnimationList::RemoveItem(tukk _filename)
{
	string filename = PathUtil::ReplaceExtension(PathUtil::ToUnixPath(_filename), "caf");

	i32 itemsRemoved = 0;

	i32 numItems = m_items.size();
	for (i32 i = 0; i < numItems; ++i)
	{
		if (stricmp(m_items[i].path.c_str(), filename) == 0)
		{
			m_items.erase(m_items.begin() + i);
			--numItems;
			++itemsRemoved;
			--i;
		}
	}
	++m_revision;
	return itemsRemoved;
}

void FilterAnimationList::OnFileChange(tukk filename, EChangeType eType)
{
	const string originalExt = PathUtil::GetExt(filename);
	if (originalExt == "animsettings")
	{
		if (gEnv->pDrxPak->IsFileExist(filename, IDrxPak::eFileLocation_OnDisk))
		{
			UpdateItem(filename);
		}
		else
		{
			RemoveItem(filename);
		}
	}
}

}

