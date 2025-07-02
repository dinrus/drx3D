// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "SequenceCamerasModel.h"

#include <DrxMovie/IMovieSystem.h>

i32 CSequenceCamerasModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return m_cameraNodes.size();
	}
	return 0;
}

i32 CSequenceCamerasModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant CSequenceCamerasModel::data(const QModelIndex& index, i32 role) const
{
	if (index.isValid() && index.row() < m_cameraNodes.size())
	{
		switch (role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch (index.column())
			{
			case 0:
				return m_cameraNodes[index.row()].c_str();
			}
			break;
		case Qt::DecorationRole:
		default:
			break;
		}
	}

	return QVariant();
}

QVariant CSequenceCamerasModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case 0:
			return QObject::tr("Camera Name");
		default:
			break;
		}
	}
	return QVariant();
}

QModelIndex CSequenceCamerasModel::index(i32 row, i32 column, const QModelIndex& parent) const
{
	if (row >= 0 && row < m_cameraNodes.size())
	{
		return QAbstractItemModel::createIndex(row, column, reinterpret_cast<quintptr>(&m_cameraNodes[row]));
	}
	return QModelIndex();
}

QModelIndex CSequenceCamerasModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

void CSequenceCamerasModel::Load()
{
	for (i32 i = m_pSequence->GetNodeCount(); i--; )
	{
		IAnimNode* pNode = m_pSequence->GetNode(i);
		if (pNode && pNode->GetType() == eAnimNodeType_Camera)
		{
			m_cameraNodes.push_back(pNode->GetName());
		}
	}
}

