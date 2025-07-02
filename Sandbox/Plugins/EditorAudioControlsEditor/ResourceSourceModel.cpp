// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ResourceSourceModel.h"

#include "Library.h"
#include "AssetIcons.h"

#include <ModelUtils.h>
#include <QtUtil.h>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
QVariant CResourceSourceModel::GetHeaderData(i32 const section, Qt::Orientation const orientation, i32 const role)
{
	QVariant variant;

	if ((orientation == Qt::Horizontal) && ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole)))
	{
		variant = "Name";
	}

	return variant;
}

//////////////////////////////////////////////////////////////////////////
i32 CResourceSourceModel::columnCount(QModelIndex const& parent) const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
QVariant CResourceSourceModel::data(QModelIndex const& index, i32 role) const
{
	QVariant variant;

	CLibrary const* const pLibrary = static_cast<CLibrary*>(index.internalPointer());

	if (pLibrary != nullptr)
	{
		switch (role)
		{
		case Qt::DisplayRole:
			variant = QtUtil::ToQStringSafe(pLibrary->GetName());
			break;
		case Qt::ToolTipRole:
			if (!pLibrary->GetDescription().IsEmpty())
			{
				variant = QtUtil::ToQStringSafe(pLibrary->GetName() + ": " + pLibrary->GetDescription());
			}
			else
			{
				variant = QtUtil::ToQStringSafe(pLibrary->GetName());
			}
			break;
		case Qt::DecorationRole:
			variant = GetAssetIcon(EAssetType::Library);
			break;
		case static_cast<i32>(ModelUtils::ERoles::SortPriority):
			variant = static_cast<i32>(EAssetType::Library);
			break;
		case static_cast<i32>(ModelUtils::ERoles::InternalPointer):
			variant = reinterpret_cast<intptr_t>(pLibrary);
			break;
		default:
			break;
		}
	}

	return variant;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceSourceModel::setData(QModelIndex const& index, QVariant const& value, i32 role)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
QVariant CResourceSourceModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	return GetHeaderData(section, orientation, role);
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags CResourceSourceModel::flags(QModelIndex const& index) const
{
	return index.isValid() ? (QAbstractItemModel::flags(index) | Qt::ItemIsSelectable) : Qt::NoItemFlags;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceSourceModel::canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceSourceModel::dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
Qt::DropActions CResourceSourceModel::supportedDropActions() const
{
	return Qt::IgnoreAction;
}
} //endns ACE
