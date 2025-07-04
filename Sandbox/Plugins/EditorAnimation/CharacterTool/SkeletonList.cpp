// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "SkeletonList.h"
#include "Serialization.h"
#include <DrxSystem/XML/IXml.h>
#include <DrxSystem/ISystem.h>
#include <IEditor.h>
#include <DrxSystem/File/IFileChangeMonitor.h>
#include <DrxSystem/File/DrxFile.h> // for PathUtil
#include "../Shared/AnimSettings.h"

namespace CharacterTool
{

static tukk ANIMATIONS_SKELETON_LIST_PATH = "Animations/SkeletonList.xml";

SkeletonList::SkeletonList()
{
	GetIEditor()->GetFileMonitor()->RegisterListener(this, ANIMATIONS_SKELETON_LIST_PATH);
	OutputDebugString("CSkeletonList created\n");
}

SkeletonList::~SkeletonList()
{
	GetIEditor()->GetFileMonitor()->UnregisterListener(this);
	OutputDebugString("CSkeletonList destroyed\n");
}

void SkeletonList::OnFileChange(tukk filename, EChangeType eType)
{
	Load();
}

void SkeletonList::Reset()
{
	m_skeletonList.clear();
}

bool SkeletonList::Load()
{
	std::set<string> loadedAliases;

	XmlNodeRef xmlRoot = GetISystem()->LoadXmlFromFile(ANIMATIONS_SKELETON_LIST_PATH);
	if (!xmlRoot)
	{
		return false;
	}

	m_skeletonList.clear();
	m_skeletonNames.clear();
	m_skeletonNames.push_back(string()); // service name that we use for UI

	i32k childCount = xmlRoot->getChildCount();
	for (i32 i = 0; i < childCount; ++i)
	{
		XmlNodeRef xmlEntry = xmlRoot->getChild(i);

		tukk name = xmlEntry->getAttr("name");
		;
		tukk file = xmlEntry->getAttr("file");
		if (file[0] != '\0' && name[0] != '\0')
		{
			const bool foundSkeletonName = loadedAliases.find(name) != loadedAliases.end();
			if (foundSkeletonName)
			{
				DrxLog("Found duplicate skeleton name '%s' in file '%s' while parsing line '%d'. Skipping this entry.", name, ANIMATIONS_SKELETON_LIST_PATH, xmlEntry->getLine());
			}
			else
			{
				SEntry entry;
				entry.alias = name;
				entry.path = file;
				m_skeletonList.push_back(entry);
				m_skeletonNames.push_back(name);
			}
		}
		else
		{
			DrxLog("Missing 'name' or 'file' attribute in skeleton entry in file '%s' while parsing line '%d'. Skipping this entry.", ANIMATIONS_SKELETON_LIST_PATH, xmlEntry->getLine());
		}
	}

	std::sort(m_skeletonNames.begin(), m_skeletonNames.end());
	SignalSkeletonListModified();
	return true;
}

bool SkeletonList::Save()
{
	XmlNodeRef xmlRoot = GetISystem()->CreateXmlNode("SkeletonList");

	for (size_t i = 0; i < m_skeletonList.size(); ++i)
	{
		const SEntry& entry = m_skeletonList[i];

		XmlNodeRef xmlEntry = xmlRoot->createNode("Skeleton");
		xmlRoot->addChild(xmlEntry);

		xmlEntry->setAttr("name", entry.alias.c_str());
		xmlEntry->setAttr("file", entry.path.c_str());
	}

	const bool saveSuccess = xmlRoot->saveToFile(ANIMATIONS_SKELETON_LIST_PATH);
	return saveSuccess;
}

bool SkeletonList::HasSkeletonName(tukk skeletonName) const
{
	for (size_t i = 0; i < m_skeletonList.size(); ++i)
	{
		if (m_skeletonList[i].alias == skeletonName)
			return true;
	}
	return false;
}

string SkeletonList::FindSkeletonNameByPath(tukk path) const
{
	for (size_t i = 0; i < m_skeletonList.size(); ++i)
	{
		const SEntry& entry = m_skeletonList[i];
		if (stricmp(entry.path.c_str(), path) == 0)
			return entry.alias;
	}

	return string();
}

string SkeletonList::FindSkeletonPathByName(tukk name) const
{
	for (size_t i = 0; i < m_skeletonList.size(); ++i)
	{
		const SEntry& entry = m_skeletonList[i];
		if (stricmp(entry.alias.c_str(), name) == 0)
			return entry.path;
	}

	return string();
}

void SkeletonList::Serialize(Serialization::IArchive& ar)
{
	ar(m_skeletonList, "aliases", "Aliases");

	if (ar.isInput())
	{
		m_skeletonNames.clear();
		m_skeletonNames.push_back(string());
		for (size_t i = 0; i < m_skeletonList.size(); ++i)
			m_skeletonNames.push_back(m_skeletonList[i].alias);
		std::sort(m_skeletonNames.begin() + 1, m_skeletonNames.end());
	}
}

void SkeletonList::SEntry::Serialize(Serialization::IArchive& ar)
{
	ar(alias, "alias", "^");
	ar(ResourceFilePath(path, "Skeleton (skel, chr, cga)|*.skel;*.chr;*.cga", "Skeleton"), "path", "^");
	if (ar.isEdit() && ar.isInput() && alias.empty() && !path.empty())
		alias = PathUtil::GetFileName(path);
}

}

// ---------------------------------------------------------------------------

bool Serialize(Serialization::IArchive& ar, SkeletonAlias& value, tukk name, tukk label)
{
	using CharacterTool::SkeletonList;
	if (ar.isEdit())
	{
		assert(ar.context<SkeletonList>());
		const Serialization::StringList& skeletons = ar.context<SkeletonList>()->GetSkeletonNames();

		Serialization::StringListValue stringListValue(skeletons, skeletons.find(value.alias), &value.alias, Serialization::TypeID::get<string>());
		ar(stringListValue, name, label);

		if (ar.isInput())
			value.alias = stringListValue.c_str();

		if (!value.alias.empty() && skeletons.find(value.alias) == skeletons.npos)
			ar.error(stringListValue, "Skeleton Alias '%s' does not exist. Please choose another one or create '%s' in (Assets -> Compression -> Skeleton List).", value.alias.c_str(), value.alias.c_str());

		return true;
	}
	else
	{
		return ar(value.alias, name, label);
	}
}
