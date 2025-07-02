// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "GizmoSink.h"
#include <QObject>

class QPropertyTree;
#include <drx3D/CoreX/Serialization/Forward.h>

namespace CharacterTool
{

using std::vector;
using std::unique_ptr;

using namespace Explorer;

struct System;

class CharacterGizmoManager : public QObject
{
	Q_OBJECT
public:
	typedef vector<ukk> SelectionHandles;
	CharacterGizmoManager(System* system);
	QPropertyTree*          Tree(GizmoLayer layer);

	void                    SetSubselection(GizmoLayer layer, const SelectionHandles& handles);
	const SelectionHandles& Subselection(GizmoLayer layer) const;
	void                    ReadGizmos();

signals:
	void SignalSubselectionChanged(i32 layer);
	void SignalGizmoChanged();

private slots:
	void OnActiveCharacterChanged();
	void OnActiveAnimationSwitched();

	void OnTreeAboutToSerialize(Serialization::IArchive& ar);
	void OnTreeSerialized(Serialization::IArchive& ar);
	void OnExplorerEntryModified(ExplorerEntryModifyEvent& ev);
	void OnExplorerBeginRemoveEntry(ExplorerEntry* entry);
	void OnSceneChanged();

private:
	vector<unique_ptr<QPropertyTree>>       m_trees;
	unique_ptr<Serialization::CContextList> m_contextList;
	vector<SelectionHandles>                m_subselections;
	ExplorerEntry*                          m_attachedAnimationEntry;

	System* m_system;
};

}

