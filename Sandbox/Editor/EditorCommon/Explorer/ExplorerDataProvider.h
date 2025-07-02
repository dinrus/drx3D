// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QObject>
#include <vector>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "Serialization.h"

namespace Explorer
{

using std::vector;
struct ExplorerEntry;
struct ExplorerAction;
struct ActionOutput;
class ExplorerData;

struct EDITOR_COMMON_API ExplorerEntryId
{
	i32  subtree;
	uint id;

	ExplorerEntryId(i32 subtree, uint id) : subtree(subtree), id(id)  {}
	ExplorerEntryId() : subtree(-1), id(0)  {}

	bool operator==(const ExplorerEntryId& rhs) const { return subtree == rhs.subtree && id == rhs.id; }

	void Serialize(Serialization::IArchive& ar);
};

struct EntryModifiedEvent
{
	i32            subtree;
	uint           id;
	tukk    reason;
	bool           contentChanged;
	bool           continuousChange;
	DynArray<char> previousContent;

	EntryModifiedEvent()
		: subtree(-1)
		, id(0)
		, reason("")
		, contentChanged(false)
		, continuousChange(false)
	{
	}
};

class EDITOR_COMMON_API IExplorerEntryProvider : public QObject
{
	Q_OBJECT

public:
	IExplorerEntryProvider()
		: m_subtree(-1) {}

	virtual void        UpdateEntry(ExplorerEntry* entry) = 0;
	virtual bool        GetEntrySerializer(Serialization::SStruct* out, uint id) const = 0;
	virtual void        GetEntryActions(vector<ExplorerAction>* actions, uint id, ExplorerData* explorerData) = 0;

	virtual i32         GetEntryCount() const = 0;
	virtual uint        GetEntryIdByIndex(i32 index) const = 0;
	virtual bool        LoadOrGetChangedEntry(uint id) { return true; }
	virtual void        CheckIfModified(uint id, tukk reason, bool continuous) = 0;
	virtual bool        HasBackgroundLoading() const   { return false; }

	virtual bool        SaveEntry(ActionOutput* output, uint id) = 0;
	virtual string      GetSaveFilename(uint id) = 0;
	virtual void        RevertEntry(uint id) = 0;
	virtual bool        SaveAll(ActionOutput* output) = 0;
	virtual void        SetExplorerData(ExplorerData* explorerData, i32 subtree) {}
	virtual tukk CanonicalExtension() const                               { return ""; }
	virtual void        GetDependencies(vector<string>* paths, uint id)          {}

	void                SetSubtree(i32 subtree)                                  { m_subtree = subtree; }
	bool                OwnsEntry(const ExplorerEntry* entry) const;
	bool                OwnsAssetEntry(const ExplorerEntry* entry) const;

	string              GetCanonicalPath(tukk path) const;

signals:
	void SignalSubtreeReset(i32 subtree);
	void SignalEntryModified(EntryModifiedEvent& ev);
	void SignalEntryAdded(i32 subtree, uint id);
	void SignalEntryRemoved(i32 subtree, uint id);
	void SignalEntrySavedAs(tukk oldName, tukk newName);
	void SignalEntry(i32 subtree, uint id);
	void SignalSubtreeLoadingFinished(i32 subtree);
	void SignalBeginBatchChange(i32 subtree);
	void SignalEndBatchChange(i32 subtree);

protected:
	i32 m_subtree;
};

}

