// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ResourceLibraryModel.h"

#include "ResourceSourceModel.h"
#include "Library.h"
#include "AssetIcons.h"

#include <ModelUtils.h>
#include <QtUtil.h>

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
i32 CResourceLibraryModel::columnCount(QModelIndex const& parent) const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
QVariant CResourceLibraryModel::data(QModelIndex const& index, i32 role) const
{
	QVariant variant;

	if (index.isValid())
	{
		CAsset const* const pAsset = static_cast<CAsset*>(index.internalPointer());

		if (pAsset != nullptr)
		{
			EAssetType const assetType = pAsset->GetType();

			switch (role)
			{
			case Qt::DisplayRole:
				variant = QtUtil::ToQStringSafe(pAsset->GetName());
				break;
			case Qt::ToolTipRole:
				if (!pAsset->GetDescription().IsEmpty())
				{
					variant = QtUtil::ToQStringSafe(pAsset->GetName() + ": " + pAsset->GetDescription());
				}
				else
				{
					variant = QtUtil::ToQStringSafe(pAsset->GetName());
				}
				break;
			case Qt::DecorationRole:
				variant = GetAssetIcon(assetType);
				break;
			case static_cast<i32>(ModelUtils::ERoles::SortPriority):
				variant = static_cast<i32>(assetType);
				break;
			case static_cast<i32>(ModelUtils::ERoles::InternalPointer):
				variant = reinterpret_cast<intptr_t>(pAsset);
				break;
			default:
				break;
			}
		}
	}

	return variant;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceLibraryModel::setData(QModelIndex const& index, QVariant const& value, i32 role)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
QVariant CResourceLibraryModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	return CResourceSourceModel::GetHeaderData(section, orientation, role);
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags CResourceLibraryModel::flags(QModelIndex const& index) const
{
	Qt::ItemFlags flags = Qt::NoItemFlags;

	if (index.isValid())
	{
		flags = QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
	}

	return flags;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceLibraryModel::canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CResourceLibraryModel::dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
Qt::DropActions CResourceLibraryModel::supportedDropActions() const
{
	return Qt::IgnoreAction;
}
} //endns ACE
