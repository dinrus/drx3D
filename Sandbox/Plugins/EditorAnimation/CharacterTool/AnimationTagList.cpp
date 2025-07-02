// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "AnimationTagList.h"
#include "EditorCompressionPresetTable.h"
#include "EditorDBATable.h"

namespace CharacterTool
{

enum ETagGroup
{
	TAG_GROUP_COMPRESSION_PRESETS,
	TAG_GROUP_DBAS,
	NUM_TAG_GROUPS
};

AnimationTagList::AnimationTagList(EditorCompressionPresetTable* presets, EditorDBATable* dbaTable)
	: m_presets(presets)
	, m_dbaTable(dbaTable)
{
	m_groups.resize(NUM_TAG_GROUPS);
	m_groups[TAG_GROUP_COMPRESSION_PRESETS].name = "Compression Preset";
	m_groups[TAG_GROUP_DBAS].name = "DBA";

	connect(m_presets, SIGNAL(SignalChanged()), this, SLOT(OnCompressionPresetsChanged()));
	connect(m_dbaTable, SIGNAL(SignalChanged()), this, SLOT(OnDBATableChanged()));

	OnCompressionPresetsChanged();
	OnDBATableChanged();
}

void AnimationTagList::OnCompressionPresetsChanged()
{
	std::vector<std::pair<string, string>> tags;
	m_presets->FindTags(&tags);

	SGroup& group = m_groups[TAG_GROUP_COMPRESSION_PRESETS];
	group.entries.resize(tags.size());
	for (size_t i = 0; i < tags.size(); ++i)
	{
		group.entries[i].tag = tags[i].first;
		group.entries[i].description = tags[i].second;
	}
}

void AnimationTagList::OnDBATableChanged()
{
	std::vector<std::pair<string, string>> tags;
	m_dbaTable->FindTags(&tags);

	SGroup& group = m_groups[TAG_GROUP_DBAS];
	group.entries.resize(tags.size());
	for (size_t i = 0; i < tags.size(); ++i)
	{
		group.entries[i].tag = tags[i].first;
		group.entries[i].description = tags[i].second;
	}
}

u32 AnimationTagList::TagCount(u32 group) const
{
	if (group >= m_groups.size())
		return 0;
	return m_groups[group].entries.size();
}

tukk AnimationTagList::TagValue(u32 group, u32 index) const
{
	if (group >= m_groups.size())
		return 0;
	const SGroup& g = m_groups[group];
	if (index >= g.entries.size())
		return "";
	return g.entries[index].tag.c_str();
}

tukk AnimationTagList::TagDescription(u32 group, u32 index) const
{
	if (group >= m_groups.size())
		return 0;
	const SGroup& g = m_groups[group];
	if (index >= g.entries.size())
		return "";
	return g.entries[index].description.c_str();
}

tukk AnimationTagList::GroupName(u32 group) const
{
	if (group >= m_groups.size())
		return "";
	return m_groups[group].name.c_str();
}

u32 AnimationTagList::GroupCount() const
{
	return m_groups.size();
}

}

