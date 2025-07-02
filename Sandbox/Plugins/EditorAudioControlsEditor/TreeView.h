// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAdvancedTreeView.h>

namespace ACE
{
class CTreeView final : public QAdvancedTreeView
{
	Q_OBJECT

public:

	explicit CTreeView(QWidget* const pParent, QAdvancedTreeView::BehaviorFlags const flags = QAdvancedTreeView::BehaviorFlags(UseItemModelAttribute));

	CTreeView() = delete;

	bool IsEditing() const { return state() == QAbstractItemView::EditingState; }

	void ExpandSelection();
	void CollapseSelection();

	void BackupExpanded();
	void RestoreExpanded();

	void BackupSelection();
	void RestoreSelection();

	void SetNameRole(i32 const nameRole)     { m_nameRole = nameRole; }
	void SetNameColumn(i32 const nameColumn) { m_nameColumn = nameColumn; }

private:

	u32 GetItemId(QModelIndex const& index) const;

	void   ExpandSelectionRecursively(QModelIndexList const& indexList);
	void   CollapseSelectionRecursively(QModelIndexList const& indexList);

	void   BackupExpandedRecursively(QModelIndex const& index);
	void   RestoreExpandedRecursively(QModelIndex const& index);
	void   RestoreSelectionRecursively(QModelIndex const& index);

	i32          m_nameRole = 0;
	i32          m_nameColumn = 0;
	QSet<u32> m_expandedBackup;
	QSet<u32> m_selectionBackup;
};
} //endns ACE

