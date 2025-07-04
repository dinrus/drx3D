// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "TreeView.h"

#include <ModelUtils.h>
#include <DrxAudio/IAudioSystem.h>

#include <QHeaderView>

namespace ACE
{
namespace Impl
{
namespace Wwise
{
//////////////////////////////////////////////////////////////////////////
CTreeView::CTreeView(QWidget* const pParent, QAdvancedTreeView::BehaviorFlags const flags /*= QAdvancedTreeView::BehaviorFlags(UseItemModelAttribute)*/)
	: QAdvancedTreeView(QAdvancedTreeView::BehaviorFlags(flags), pParent)
{
	QObject::connect(header(), &QHeaderView::sortIndicatorChanged, [this]() { scrollTo(currentIndex()); });
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::ExpandSelection()
{
	ExpandSelectionRecursively(selectedIndexes());
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::ExpandSelectionRecursively(QModelIndexList const& indexList)
{
	for (auto const& index : indexList)
	{
		if (index.isValid())
		{
			i32 const childCount = index.model()->rowCount(index);

			for (i32 i = 0; i < childCount; ++i)
			{
				QModelIndexList childList;
				childList.append(index.child(i, 0));
				ExpandSelectionRecursively(childList);
			}

			expand(index);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::CollapseSelection()
{
	CollapseSelectionRecursively(selectedIndexes());
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::CollapseSelectionRecursively(QModelIndexList const& indexList)
{
	for (auto const& index : indexList)
	{
		if (index.isValid())
		{
			i32 const childCount = index.model()->rowCount(index);

			for (i32 i = 0; i < childCount; ++i)
			{
				QModelIndexList childList;
				childList.append(index.child(i, 0));
				CollapseSelectionRecursively(childList);
			}

			collapse(index);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
ControlId CTreeView::GetItemId(QModelIndex const& index) const
{
	auto itemId = static_cast<ControlId>(DrxAudio::InvalidCRC32);

	if (index.isValid())
	{
		QModelIndex const itemIndex = index.sibling(index.row(), m_nameColumn);

		if (itemIndex.isValid())
		{
			itemId = static_cast<ControlId>(itemIndex.data(static_cast<i32>(ModelUtils::ERoles::Id)).toInt());
		}
	}

	return itemId;
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::BackupExpanded()
{
	m_expandedBackup.clear();

	i32 const rowCount = model()->rowCount();

	for (i32 i = 0; i < rowCount; ++i)
	{
		BackupExpandedRecursively(model()->index(i, 0));
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::BackupExpandedRecursively(QModelIndex const& index)
{
	if (index.isValid())
	{
		i32 const rowCount = model()->rowCount(index);

		if (rowCount > 0)
		{
			for (i32 i = 0; i < rowCount; ++i)
			{
				BackupExpandedRecursively(index.child(i, 0));
			}
		}

		if (isExpanded(index))
		{
			m_expandedBackup.insert(GetItemId(index));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::RestoreExpanded()
{
	if (!m_expandedBackup.isEmpty())
	{
		i32 const rowCount = model()->rowCount();

		for (i32 i = 0; i < rowCount; ++i)
		{
			RestoreExpandedRecursively(model()->index(i, 0));
		}

		m_expandedBackup.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::RestoreExpandedRecursively(QModelIndex const& index)
{
	if (index.isValid())
	{
		i32 const rowCount = model()->rowCount(index);

		if (rowCount > 0)
		{
			for (i32 i = 0; i < rowCount; ++i)
			{
				RestoreExpandedRecursively(index.child(i, 0));
			}
		}

		if (m_expandedBackup.contains(GetItemId(index)))
		{
			expand(index);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::BackupSelection()
{
	m_selectionBackup.clear();

	QModelIndexList const& selectedList = selectionModel()->selectedRows();

	for (QModelIndex const& index : selectedList)
	{
		m_selectionBackup.insert(GetItemId(index));
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::RestoreSelection()
{
	if (!m_selectionBackup.isEmpty())
	{
		i32 const rowCount = model()->rowCount();

		for (i32 i = 0; i < rowCount; ++i)
		{
			RestoreSelectionRecursively(model()->index(i, 0));
		}
	}
	else
	{
		for (QModelIndex const& index : selectionModel()->selectedIndexes())
		{
			scrollTo(index);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTreeView::RestoreSelectionRecursively(QModelIndex const& index)
{
	if (index.isValid())
	{
		i32 const rowCount = model()->rowCount(index);

		if (rowCount > 0)
		{
			for (i32 i = 0; i < rowCount; ++i)
			{
				RestoreSelectionRecursively(index.child(i, 0));
			}
		}

		if (m_selectionBackup.contains(GetItemId(index)))
		{
			selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			scrollTo(index);
		}
	}
}
} //endns Wwise
} //endns Impl
} //endns ACE
