// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../Shared/CompressionPresetTable.h"
#include <drx3D/CoreX/Serialization/Decorators/TagList.h>
#include <QObject>

namespace CharacterTool
{

class FilterAnimationList;

struct EditorCompressionPreset
{
	SCompressionPresetEntry entry;
	vector<string>          matchingAnimations;

	void                    Serialize(Serialization::IArchive& ar);
	EditorCompressionPreset() {}
};

class EditorCompressionPresetTable : public QObject
{
	Q_OBJECT
public:

	EditorCompressionPresetTable() : m_filterAnimationList(0) {}
	void                     SetFilterAnimationList(const FilterAnimationList* filterAnimationList) { m_filterAnimationList = filterAnimationList; }

	void                     Reset();
	bool                     Load();
	bool                     Save();

	EditorCompressionPreset* FindPresetForAnimation(const SAnimationFilterItem& item);

	void                     Serialize(Serialization::IArchive& ar);
	void                     FindTags(std::vector<std::pair<string, string>>* tags) const;
signals:
	void                     SignalChanged();
private:
	void                     UpdateMatchingAnimations();

	const FilterAnimationList*           m_filterAnimationList;
	std::vector<EditorCompressionPreset> m_entries;
};

}

