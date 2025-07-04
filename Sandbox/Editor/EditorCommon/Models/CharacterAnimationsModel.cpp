// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "CharacterAnimationsModel.h"

#include <DrxAnimation/IDrxAnimation.h>

i32 CCharacterAnimationsModel::rowCount(const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		return m_animationSet.GetAnimationCount();
	}
	return 0;
}

i32 CCharacterAnimationsModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant CCharacterAnimationsModel::data(const QModelIndex& index, i32 role) const
{
	if (index.isValid() && index.row() < m_animationSet.GetAnimationCount())
	{
		switch (role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch (index.column())
			{
			case 0:
				return m_animationSet.GetNameByAnimID(index.row());
			}
			break;
		case Qt::DecorationRole:
		default:
			break;
		}
	}

	return QVariant();
}

QVariant CCharacterAnimationsModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case 0:
			return QObject::tr("Animation Name");
		default:
			break;
		}
	}
	return QVariant();
}

QModelIndex CCharacterAnimationsModel::index(i32 row, i32 column, const QModelIndex& parent) const
{
	if (row >= 0 && row < m_animationSet.GetAnimationCount())
	{
		return QAbstractItemModel::createIndex(row, column, reinterpret_cast<quintptr>(m_animationSet.GetNameByAnimID(row)));
	}
	return QModelIndex();
}

QModelIndex CCharacterAnimationsModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

