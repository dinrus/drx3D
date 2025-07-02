// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QObject>
#include "../Shared/DBATable.h"

namespace CharacterTool
{

class FilterAnimationList;

struct SEditorDBAEntry
{
	SDBAEntry           entry;
	std::vector<string> matchingAnimations;

	void                Serialize(Serialization::IArchive& ar);
};

class FilterAnimationList;
class EditorDBATable : public QObject
{
	Q_OBJECT
public:
	EditorDBATable() : m_filterAnimationList(0) {}
	void                   SetFilterAnimationList(const FilterAnimationList* filterAnimationList) { m_filterAnimationList = filterAnimationList; }
	void                   Serialize(Serialization::IArchive& ar);

	void                   FindTags(std::vector<std::pair<string, string>>* tags) const;
	i32                    FindDBAForAnimation(const SAnimationFilterItem& item) const;
	const SEditorDBAEntry* GetEntryByIndex(i32 index) const;

	void                   Reset();
	bool                   Load();
	bool                   Save();

signals:
	void SignalChanged();
private:
	void UpdateMatchingAnimations();

	std::vector<SEditorDBAEntry> m_entries;
	const FilterAnimationList*   m_filterAnimationList;
};

}

