// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "SequenceEventsModel.h"

#include <DrxMovie/IMovieSystem.h>

i32 CSequenceEventsModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return m_pSequence->GetTrackEventsCount();
	}
	return 0;
}

i32 CSequenceEventsModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant CSequenceEventsModel::data(const QModelIndex& index, i32 role) const
{
	if (index.isValid() && index.row() < m_pSequence->GetTrackEventsCount())
	{
		switch (role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch (index.column())
			{
			case 0:
				return m_pSequence->GetTrackEvent(index.row());
			}
			break;
		case Qt::DecorationRole:
		default:
			break;
		}
	}

	return QVariant();
}

QVariant CSequenceEventsModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case 0:
			return QObject::tr("Event Name");
		default:
			break;
		}
	}
	return QVariant();
}

QModelIndex CSequenceEventsModel::index(i32 row, i32 column, const QModelIndex& parent) const
{
	if (row >= 0 && row < m_pSequence->GetTrackEventsCount())
	{
		return QAbstractItemModel::createIndex(row, column, reinterpret_cast<quintptr>(m_pSequence->GetTrackEvent(row)));
	}
	return QModelIndex();
}

QModelIndex CSequenceEventsModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

