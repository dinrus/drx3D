// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ItemModel.h"

#include "Common.h"

#include <ModelUtils.h>
#include <DragDrop.h>
#include <FilePathUtil.h>
#include <QtUtil.h>

namespace ACE
{
namespace Impl
{
namespace Wwise
{
//////////////////////////////////////////////////////////////////////////
CItemModelAttribute* GetAttributeForColumn(CItemModel::EColumns const column)
{
	CItemModelAttribute* pAttribute = nullptr;

	switch (column)
	{
	case CItemModel::EColumns::Notification:
		pAttribute = &ModelUtils::s_notificationAttribute;
		break;
	case CItemModel::EColumns::Connected:
		pAttribute = &ModelUtils::s_connectedAttribute;
		break;
	case CItemModel::EColumns::Localized:
		pAttribute = &ModelUtils::s_localizedAttribute;
		break;
	case CItemModel::EColumns::Name:
		pAttribute = &Attributes::s_nameAttribute;
		break;
	default:
		pAttribute = nullptr;
		break;
	}

	return pAttribute;
}

//////////////////////////////////////////////////////////////////////////
CItemModel::CItemModel(CItem const& rootItem, QObject* const pParent)
	: QAbstractItemModel(pParent)
	, m_rootItem(rootItem)
{
	InitIcons();
	ModelUtils::InitIcons();
}

//////////////////////////////////////////////////////////////////////////
void CItemModel::Reset()
{
	beginResetModel();
	endResetModel();
}

//////////////////////////////////////////////////////////////////////////
i32 CItemModel::rowCount(QModelIndex const& parent) const
{
	i32 rowCount = 0;
	CItem const* pItem = ItemFromIndex(parent);

	if (pItem == nullptr)
	{
		// If not valid it must be a top level item, so get root.
		pItem = &m_rootItem;
	}

	rowCount = static_cast<i32>(pItem->GetNumChildren());

	return rowCount;
}

//////////////////////////////////////////////////////////////////////////
i32 CItemModel::columnCount(QModelIndex const& parent) const
{
	return static_cast<i32>(EColumns::Count);
}

//////////////////////////////////////////////////////////////////////////
QVariant CItemModel::data(QModelIndex const& index, i32 role) const
{
	QVariant variant;

	if (index.isValid())
	{
		CItem const* const pItem = ItemFromIndex(index);

		if (pItem != nullptr)
		{
			if (role == static_cast<i32>(ModelUtils::ERoles::Name))
			{
				variant = QtUtil::ToQString(pItem->GetName());
			}
			else
			{
				EItemFlags const flags = pItem->GetFlags();

				switch (index.column())
				{
				case static_cast<i32>(EColumns::Notification):
					{
						switch (role)
						{
						case Qt::DecorationRole:
							if ((flags & (EItemFlags::IsConnected | EItemFlags::IsContainer)) == 0)
							{
								variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::NoConnection);
							}
							else if ((flags& EItemFlags::IsLocalized) != 0)
							{
								variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::Localized);
							}
							break;
						case Qt::ToolTipRole:
							if ((flags & (EItemFlags::IsConnected | EItemFlags::IsContainer)) == 0)
							{
								variant = TypeToString(pItem->GetType()) + tr(" is not connected to any audio system control");
							}
							else if ((flags& EItemFlags::IsLocalized) != 0)
							{
								variant = TypeToString(pItem->GetType()) + tr(" is localized");
							}
							break;
						case static_cast<i32>(ModelUtils::ERoles::Id):
							variant = pItem->GetId();
							break;
						default:
							break;
						}
					}
					break;
				case static_cast<i32>(EColumns::Connected):
					if ((role == Qt::CheckStateRole) && ((flags& EItemFlags::IsContainer) == 0))
					{
						variant = ((flags& EItemFlags::IsConnected) != 0) ? Qt::Checked : Qt::Unchecked;
					}
					break;
				case static_cast<i32>(EColumns::Localized):
					if ((role == Qt::CheckStateRole) && ((flags& EItemFlags::IsContainer) == 0))
					{
						variant = ((flags& EItemFlags::IsLocalized) != 0) ? Qt::Checked : Qt::Unchecked;
					}
					break;
				case static_cast<i32>(EColumns::Name):
					{
						switch (role)
						{
						case Qt::DecorationRole:
							variant = GetTypeIcon(pItem->GetType());
							break;
						case Qt::DisplayRole:
						case Qt::ToolTipRole:
							variant = QtUtil::ToQString(pItem->GetName());
							break;
						case static_cast<i32>(ModelUtils::ERoles::Id):
							variant = pItem->GetId();
							break;
						case static_cast<i32>(ModelUtils::ERoles::SortPriority):
							variant = static_cast<i32>(pItem->GetType());
							break;
						case static_cast<i32>(ModelUtils::ERoles::IsPlaceholder):
							variant = (flags& EItemFlags::IsPlaceHolder) != 0;
							break;
						default:
							break;
						}
					}
					break;
				}
			}
		}
	}

	return variant;
}

//////////////////////////////////////////////////////////////////////////
QVariant CItemModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	QVariant variant;

	if (orientation == Qt::Horizontal)
	{
		auto const pAttribute = GetAttributeForColumn(static_cast<EColumns>(section));

		if (pAttribute != nullptr)
		{
			switch (role)
			{
			case Qt::DecorationRole:
				if (section == static_cast<i32>(EColumns::Notification))
				{
					variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::NotificationHeader);
				}
				break;
			case Qt::DisplayRole:
				// For the notification header an icon is used instead of text.
				if (section != static_cast<i32>(EColumns::Notification))
				{
					variant = pAttribute->GetName();
				}
				break;
			case Qt::ToolTipRole:
				variant = pAttribute->GetName();
				break;
			case Attributes::s_getAttributeRole:
				variant = QVariant::fromValue(pAttribute);
				break;
			default:
				break;
			}
		}
	}

	return variant;
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags CItemModel::flags(QModelIndex const& index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	if (index.isValid())
	{
		CItem const* const pItem = ItemFromIndex(index);

		if ((pItem != nullptr) && ((pItem->GetFlags() & EItemFlags::IsPlaceHolder) == 0))
		{
			flags |= Qt::ItemIsDragEnabled;
		}
	}

	return flags;
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CItemModel::index(i32 row, i32 column, QModelIndex const& parent /*= QModelIndex()*/) const
{
	QModelIndex modelIndex = QModelIndex();

	if ((row >= 0) && (column >= 0))
	{
		CItem const* pParent = ItemFromIndex(parent);

		if (pParent == nullptr)
		{
			pParent = &m_rootItem;
		}

		if ((pParent != nullptr) && (static_cast<i32>(pParent->GetNumChildren()) > row))
		{
			auto const pItem = static_cast<CItem const*>(pParent->GetChildAt(row));

			if (pItem != nullptr)
			{
				modelIndex = createIndex(row, column, reinterpret_cast<quintptr>(pItem));
			}
		}
	}

	return modelIndex;
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CItemModel::parent(QModelIndex const& index) const
{
	QModelIndex modelIndex = QModelIndex();

	if (index.isValid())
	{
		CItem const* const pItem = ItemFromIndex(index);

		if (pItem != nullptr)
		{
			modelIndex = IndexFromItem(static_cast<CItem*>(pItem->GetParent()));
		}
	}

	return modelIndex;

}

//////////////////////////////////////////////////////////////////////////
bool CItemModel::canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CItemModel::dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
Qt::DropActions CItemModel::supportedDragActions() const
{
	return Qt::CopyAction;
}

//////////////////////////////////////////////////////////////////////////
QStringList CItemModel::mimeTypes() const
{
	QStringList types = QAbstractItemModel::mimeTypes();
	types << CDragDropData::GetMimeFormatForType(ModelUtils::s_szImplMimeType);
	return types;
}

//////////////////////////////////////////////////////////////////////////
QMimeData* CItemModel::mimeData(QModelIndexList const& indexes) const
{
	auto const pDragDropData = new CDragDropData();
	QByteArray byteArray;
	QDataStream stream(&byteArray, QIODevice::ReadWrite);

	QModelIndexList nameIndexes;

	for (QModelIndex const& index : indexes)
	{
		nameIndexes.append(index.sibling(index.row(), static_cast<i32>(EColumns::Name)));
	}

	nameIndexes.erase(std::unique(nameIndexes.begin(), nameIndexes.end()), nameIndexes.end());

	for (auto const& index : nameIndexes)
	{
		CItem const* const pItem = ItemFromIndex(index);

		if (pItem != nullptr)
		{
			stream << pItem->GetId();
		}
	}

	pDragDropData->SetCustomData(ModelUtils::s_szImplMimeType, byteArray);
	return pDragDropData;
}

//////////////////////////////////////////////////////////////////////////
CItem* CItemModel::ItemFromIndex(QModelIndex const& index) const
{
	CItem* pItem = nullptr;

	if (index.isValid())
	{
		pItem = static_cast<CItem*>(index.internalPointer());
	}

	return pItem;
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CItemModel::IndexFromItem(CItem const* const pItem) const
{
	QModelIndex modelIndex = QModelIndex();

	if (pItem != nullptr)
	{
		auto pParent = static_cast<CItem const*>(pItem->GetParent());

		if (pParent == nullptr)
		{
			pParent = &m_rootItem;
		}

		if (pParent != nullptr)
		{
			size_t const numChildren = pParent->GetNumChildren();

			for (size_t i = 0; i < numChildren; ++i)
			{
				if (pParent->GetChildAt(i) == pItem)
				{
					modelIndex = createIndex(static_cast<i32>(i), 0, reinterpret_cast<quintptr>(pItem));
					break;
				}
			}
		}
	}

	return modelIndex;
}
} //endns Wwise
} //endns Impl
} //endns ACE
