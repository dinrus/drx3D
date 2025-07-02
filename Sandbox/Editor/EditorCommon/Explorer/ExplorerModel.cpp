// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "ExplorerModel.h"
#include "Explorer.h"
#include "Expected.h"
#include <QFont>
#include "DrxIcon.h"

#if 0
	#define TRACE_MODEL(format, ...) { char buf[1024]; drx_sprintf(buf, "0x%p: " format "\n", this, __VA_ARGS__); OutputDebugString(buf); }
#else
	#define TRACE_MODEL(...)
#endif

namespace Explorer
{

ExplorerModel::ExplorerModel(ExplorerData* explorerData, QObject* parent)
	: QAbstractItemModel(parent)
	, m_explorerData(explorerData)
	, m_rootIndex(0)
	, m_rootSubtree(-1)
	, m_addWithinActiveRoot(false)
	, m_removeWithinActiveRoot(false)
{
	EXPECTED(connect(m_explorerData, SIGNAL(SignalEntryModified(ExplorerEntryModifyEvent &)), this, SLOT(OnEntryModified(ExplorerEntryModifyEvent &))));
	EXPECTED(connect(m_explorerData, SIGNAL(SignalBeginAddEntry(ExplorerEntry*)), this, SLOT(OnBeginAddEntry(ExplorerEntry*))));
	EXPECTED(connect(m_explorerData, SIGNAL(SignalEndAddEntry()), this, SLOT(OnEndAddEntry())));
	EXPECTED(connect(m_explorerData, SIGNAL(SignalBeginRemoveEntry(ExplorerEntry*)), this, SLOT(OnBeginRemoveEntry(ExplorerEntry*))));
	EXPECTED(connect(m_explorerData, SIGNAL(SignalEndRemoveEntry()), this, SLOT(OnEndRemoveEntry())));
}

void ExplorerModel::SetRootByIndex(i32 index)
{
	if (m_rootIndex != index)
	{
		beginResetModel();
		m_rootIndex = index;
		if (index == 0)
			m_rootSubtree = -1;
		else
			m_rootSubtree = GetActiveRoot()->subtree;
		endResetModel();
	}
}

ExplorerEntry* ExplorerModel::GetEntry(const QModelIndex& index)
{
	return ((ExplorerEntry*)index.internalPointer());
}

ExplorerEntry* ExplorerModel::GetActiveRoot() const
{
	if (m_rootIndex < 1 || m_rootIndex > m_explorerData->GetRoot()->children.size())
		return m_explorerData->GetRoot();
	else
	{
		return m_explorerData->GetRoot()->children[m_rootIndex - 1];
	}
}

i32 ExplorerModel::GetRootIndex() const
{
	return m_rootIndex;
}

QModelIndex ExplorerModel::index(i32 row, i32 column, const QModelIndex& parent) const
{
	if (row < 0 || column < 0)
		return QModelIndex();
	ExplorerEntry* entry = GetEntry(parent);
	if (!entry)
		entry = GetActiveRoot();
	if (size_t(row) >= entry->children.size())
		return QModelIndex();
	return createIndex(row, column, entry->children[row]);
}

i32 ExplorerModel::rowCount(const QModelIndex& parent) const
{
	if (parent.column() > 0)
		return 0;
	ExplorerEntry* entry = GetEntry(parent);
	if (!entry)
		entry = GetActiveRoot();
	return entry->children.size();
}

i32 ExplorerModel::columnCount(const QModelIndex& parent) const
{
	return m_explorerData->GetColumnCount();
}

QVariant ExplorerModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	if (orientation == Qt::Horizontal)
	{
		if (const ExplorerColumn* column = m_explorerData->GetColumn(section))
		{
			if (role == Qt::DisplayRole)
			{
				return QString(column->label);
			}
			else if (role == Qt::SizeHintRole)
			{
				switch (column->format)
				{
				case ExplorerColumn::TEXT:
					return QSize(200, 20);
				case ExplorerColumn::INTEGER:
				case ExplorerColumn::KILOBYTES:
					return QSize(50, 20);
				case ExplorerColumn::ICON:
					return QSize(20, 20);
				}
			}
		}
	}

	return QVariant();
}

bool ExplorerModel::hasChildren(const QModelIndex& parent) const
{
	ExplorerEntry* entry = GetEntry(parent);
	if (!entry)
		entry = GetActiveRoot();
	return !entry->children.empty();
}

QVariant ExplorerModel::data(const QModelIndex& index, i32 role) const
{
	if (!index.isValid())
		return QVariant();
	ExplorerEntry* entry = GetEntry(index);
	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (entry)
			{
				if (index.column() == 0)
				{
					if (entry->modified)
						return QString("*") + QString(entry->name.c_str());
					else
						return QString(entry->name.c_str());
				}
				else
				{
					const ExplorerColumn* column = m_explorerData->GetColumn(index.column());
					if (column && size_t(index.column()) < entry->columnValues.size())
					{
						if (column->format == ExplorerColumn::INTEGER)
						{
							return QVariant(entry->columnValues[index.column()]);
						}
						else if (column->format == ExplorerColumn::KILOBYTES)
						{
							uint value = entry->columnValues[index.column()];
							if (value != 0)
							{
								QString result;
								result.sprintf("%i KB", value / 1024);
								return result;
							}
						}
					}
				}
			}
			return QVariant();
		}
	case Qt::FontRole:
		{
			if (entry)
			{
				if (entry->type == ENTRY_SUBTREE_ROOT)
				{
					QFont font;
					font.setBold(true);
					return font;
				}
			}
			return QVariant();
		}
	case Qt::DecorationRole:
		{
			if (entry)
			{
				if (index.column() == 0)
					return DrxIcon(m_explorerData->IconForEntry(entry));
				else
				{
					const ExplorerColumn* column = m_explorerData->GetColumn(index.column());
					if (column && size_t(index.column()) < entry->columnValues.size())
					{
						i32 iconIndex = entry->columnValues[index.column()];
						if (iconIndex < column->values.size())
							return DrxIcon(column->values[iconIndex].icon);
						else
							return DrxIcon();
					}
				}
			}
			return QVariant();
		}
	case Qt::TextAlignmentRole:
	case Qt::SizeHintRole:
		{
			return headerData(index.column(), Qt::Horizontal, role);
		}
	case Qt::UserRole: // we use UserRole for sorting
		{
			if (index.column() == 0 && (m_rootIndex == 0 && index.parent() == QModelIndex()))
			{
				// compare first level by subtree index
				return QVariant(i32(entry ? entry->subtree : 0));
			}
			else if (index.column() == 0)
			{
				// and nested items by name
				return QVariant(QString(entry->name.c_str()).toLower());
			}
			else
			{
				i32 column = index.column();
				if (size_t(column) < entry->columnValues.size())
					return QVariant((i32)entry->columnValues[column]);
				else
					return QVariant();
			}
			return QVariant();
		}
	}
	return QVariant();
}

QModelIndex ExplorerModel::ModelIndexFromEntry(ExplorerEntry* entry, i32 column) const
{
	if (entry == GetActiveRoot())
		return QModelIndex();
	if (!EXPECTED(entry != m_explorerData->GetRoot()))
		return QModelIndex();
	if (!EXPECTED(entry != 0))
		return QModelIndex();
	if (entry->parent == 0) // FIXME
		return QModelIndex();

	for (size_t i = 0; i < entry->parent->children.size(); ++i)
		if (entry->parent->children[i] == entry)
			return createIndex(i, column, entry);

	DRX_ASSERT(0);
	return createIndex(0, column, entry);
}

QModelIndex ExplorerModel::NewModelIndexFromEntry(ExplorerEntry* entry, i32 column) const
{
	if (entry == GetActiveRoot())
		return QModelIndex();
	if (!EXPECTED(entry != m_explorerData->GetRoot()))
		return QModelIndex();

	i32 i = (i32)entry->parent->children.size();
	return createIndex(i, column, entry);
}

QModelIndex ExplorerModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();
	ExplorerEntry* entry = GetEntry(index);
	if (entry == GetActiveRoot())
		return QModelIndex();
	if (!EXPECTED(entry != m_explorerData->GetRoot()))
		return QModelIndex();

	ExplorerEntry* parent = entry->parent;
	if (!EXPECTED(parent != 0))
		return QModelIndex();
	if (parent == GetActiveRoot())
		return QModelIndex();
	if (!EXPECTED(parent != m_explorerData->GetRoot()))
		return QModelIndex();
	return ModelIndexFromEntry(parent, 0);
}

void ExplorerModel::OnEntryModified(ExplorerEntryModifyEvent& ev)
{
	if (ev.continuousChange)
		return;
	if (m_rootIndex != 0 && ev.entry->subtree != m_rootSubtree)
		return;

	i32k columnCount = m_explorerData->GetColumnCount();
	if (columnCount > 0)
	{
		i32k rangeStartColumn = (ev.entryParts & ENTRY_PART_CONTENT ? 0 : 1);
		i32k rangeEndColumn = (ev.entryParts & ENTRY_PART_STATUS_COLUMNS ? columnCount - 1 : 0);

		if (rangeStartColumn < columnCount && rangeEndColumn < columnCount)
		{
			const QModelIndex rangeStart = ModelIndexFromEntry(ev.entry, rangeStartColumn);
			const QModelIndex rangeEnd = ModelIndexFromEntry(ev.entry, rangeEndColumn);

			dataChanged(rangeStart, rangeEnd);
		}
	}
}

void ExplorerModel::OnBeginAddEntry(ExplorerEntry* entry)
{
	if (!EXPECTED(entry->parent != 0))
		return;
	if (m_rootIndex != 0 && entry->subtree != m_rootSubtree)
	{
		m_addWithinActiveRoot = false;
		TRACE_MODEL("skip add: %s", entry->path.c_str());
	}
	else
	{
		m_addWithinActiveRoot = true;

		QModelIndex parent = entry == GetActiveRoot() ? QModelIndex() : ModelIndexFromEntry(entry->parent, 0);
		i32 newRowIndex = entry->parent->children.size();
		beginInsertRows(parent, newRowIndex, newRowIndex);
		TRACE_MODEL("add: %s", entry->path.c_str());
	}
}

void ExplorerModel::OnEndAddEntry()
{
	if (m_addWithinActiveRoot)
		endInsertRows();
}

void ExplorerModel::OnBeginRemoveEntry(ExplorerEntry* entry)
{
	if (m_rootIndex != 0 && entry->subtree != m_rootSubtree)
	{
		m_removeWithinActiveRoot = false;
		TRACE_MODEL("skip remove: %s", entry->path.c_str());
	}
	else
	{
		m_removeWithinActiveRoot = true;

		QModelIndex parent = ModelIndexFromEntry(entry->parent, 0);
		QModelIndex index = ModelIndexFromEntry(entry, 0);

		TRACE_MODEL("begin remove row: (%i, %i, 0x%p)", index.row(), index.column(), index.internalPointer());
		ExplorerEntry* removedEntry = GetEntry(index);
		EXPECTED(removedEntry == entry);
		beginRemoveRows(parent, index.row(), index.row());
		TRACE_MODEL("remove: %s", entry->path.c_str());
	}
}

void ExplorerModel::OnEndRemoveEntry()
{
	if (m_removeWithinActiveRoot)
	{
		TRACE_MODEL("end remove row");
		endRemoveRows();
	}
}

}

