// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAdvancedTreeView.h>
#include <SharedData.h>

namespace ACE
{
namespace Impl
{
namespace Fmod
{
class CTreeView final : public QAdvancedTreeView
{
public:

	explicit CTreeView(QWidget* const pParent, QAdvancedTreeView::BehaviorFlags const flags = QAdvancedTreeView::BehaviorFlags(UseItemModelAttribute));

	CTreeView() = delete;

	void ExpandSelection();
	void CollapseSelection();

	void BackupExpanded();
	void RestoreExpanded();

	void BackupSelection();
	void RestoreSelection();

	void SetNameRole(i32 const nameRole) { m_nameRole = nameRole; }
	void SetNameColumn(i32 const nameColumn) { m_nameColumn = nameColumn; }

private:

	ControlId GetItemId(QModelIndex const& index) const;

	void      ExpandSelectionRecursively(QModelIndexList const& indexList);
	void      CollapseSelectionRecursively(QModelIndexList const& indexList);

	void      BackupExpandedRecursively(QModelIndex const& index);
	void      RestoreExpandedRecursively(QModelIndex const& index);
	void      RestoreSelectionRecursively(QModelIndex const& index);

	i32             m_nameRole = 0;
	i32             m_nameColumn = 0;
	QSet<ControlId> m_expandedBackup;
	QSet<ControlId> m_selectionBackup;
};
} //endns Fmod
} //endns Impl
} //endns ACE

