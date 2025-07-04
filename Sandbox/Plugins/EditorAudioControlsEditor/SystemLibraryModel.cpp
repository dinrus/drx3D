// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SystemLibraryModel.h"

#include "SystemSourceModel.h"
#include "AudioControlsEditorPlugin.h"
#include "ImplementationManager.h"
#include "AssetIcons.h"
#include "AssetUtils.h"

#include <ModelUtils.h>
#include <IItem.h>
#include <QtUtil.h>
#include <DragDrop.h>

#include <QApplication>

namespace ACE
{
/////////////////////////////////////////////////////////////////////////////////////////
CSystemLibraryModel::CSystemLibraryModel(CLibrary* const pLibrary, QObject* const pParent)
	: QAbstractItemModel(pParent)
	, m_pLibrary(pLibrary)
	, m_nameColumn(static_cast<i32>(CSystemSourceModel::EColumns::Name))
{
	DRX_ASSERT_MESSAGE(pLibrary != nullptr, "Library is null pointer.");
	ConnectSignals();
}

//////////////////////////////////////////////////////////////////////////
CSystemLibraryModel::~CSystemLibraryModel()
{
	DisconnectSignals();
}

//////////////////////////////////////////////////////////////////////////
void CSystemLibraryModel::ConnectSignals()
{
	g_assetsManager.SignalAssetAboutToBeAdded.Connect([this](CAsset* const pAsset)
		{
			if (AssetUtils::GetParentLibrary(pAsset) == m_pLibrary)
			{
			  i32 const row = static_cast<i32>(pAsset->ChildCount());

			  if (pAsset->GetType() == EAssetType::Library)
			  {
			    DRX_ASSERT_MESSAGE(pAsset == m_pLibrary, "Parent is not the current library.");
			    beginInsertRows(QModelIndex(), row, row);
			  }
			  else
			  {
			    QModelIndex const& parent = IndexFromItem(pAsset);
			    beginInsertRows(parent, row, row);
			  }
			}
	  }, reinterpret_cast<uintptr_t>(this));

	g_assetsManager.SignalAssetAdded.Connect([this](CAsset* const pAsset)
		{
			if (AssetUtils::GetParentLibrary(pAsset) == m_pLibrary)
			{
			  endInsertRows();
			}
	  }, reinterpret_cast<uintptr_t>(this));

	g_assetsManager.SignalAssetAboutToBeRemoved.Connect([this](CAsset* const pAsset)
		{
			if (AssetUtils::GetParentLibrary(pAsset) == m_pLibrary)
			{
			  CAsset const* const pParent = pAsset->GetParent();

			  if (pParent != nullptr)
			  {
			    size_t const childCount = pParent->ChildCount();

			    for (size_t i = 0; i < childCount; ++i)
			    {
			      if (pParent->GetChild(i) == pAsset)
			      {
			        i32 const index = static_cast<i32>(i);

			        if (pParent->GetType() == EAssetType::Library)
			        {
			          DRX_ASSERT_MESSAGE(pParent == m_pLibrary, "Parent is not the current library.");
			          beginRemoveRows(QModelIndex(), index, index);
			        }
			        else
			        {
			          QModelIndex const& parent = IndexFromItem(pParent);
			          beginRemoveRows(parent, index, index);
			        }

			        break;
			      }
			    }
			  }
			}
	  }, reinterpret_cast<uintptr_t>(this));

	g_assetsManager.SignalAssetRemoved.Connect([this](CAsset* const pParent, CAsset* const pAsset)
		{
			if (AssetUtils::GetParentLibrary(pParent) == m_pLibrary)
			{
			  endRemoveRows();
			}
	  }, reinterpret_cast<uintptr_t>(this));
}

//////////////////////////////////////////////////////////////////////////
void CSystemLibraryModel::DisconnectSignals()
{
	g_assetsManager.SignalAssetAboutToBeAdded.DisconnectById(reinterpret_cast<uintptr_t>(this));
	g_assetsManager.SignalAssetAdded.DisconnectById(reinterpret_cast<uintptr_t>(this));
	g_assetsManager.SignalAssetAboutToBeRemoved.DisconnectById(reinterpret_cast<uintptr_t>(this));
	g_assetsManager.SignalAssetRemoved.DisconnectById(reinterpret_cast<uintptr_t>(this));
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CSystemLibraryModel::IndexFromItem(CAsset const* const pAsset) const
{
	QModelIndex modelIndex = QModelIndex();

	if (pAsset != nullptr)
	{
		CAsset const* const pParent = pAsset->GetParent();

		if (pParent != nullptr)
		{
			size_t const numChildren = pParent->ChildCount();

			for (size_t i = 0; i < numChildren; ++i)
			{
				if (pParent->GetChild(i) == pAsset)
				{
					modelIndex = createIndex(static_cast<i32>(i), 0, reinterpret_cast<quintptr>(pAsset));
					break;
				}
			}
		}
	}

	return modelIndex;
}

//////////////////////////////////////////////////////////////////////////
i32 CSystemLibraryModel::rowCount(QModelIndex const& parent) const
{
	size_t rowCount = 0;

	if (!parent.isValid())
	{
		rowCount = m_pLibrary->ChildCount();
	}
	else
	{
		CAsset const* const pAsset = static_cast<CAsset*>(parent.internalPointer());

		if (pAsset != nullptr)
		{
			rowCount = pAsset->ChildCount();
		}
	}

	return static_cast<i32>(rowCount);
}

//////////////////////////////////////////////////////////////////////////
i32 CSystemLibraryModel::columnCount(QModelIndex const& parent) const
{
	return static_cast<i32>(CSystemSourceModel::EColumns::Count);
}

//////////////////////////////////////////////////////////////////////////
QVariant CSystemLibraryModel::data(QModelIndex const& index, i32 role) const
{
	QVariant variant;

	if (index.isValid())
	{
		auto const pAsset = static_cast<CAsset*>(index.internalPointer());

		if (pAsset != nullptr)
		{
			if (role == static_cast<i32>(ModelUtils::ERoles::IsDefaultControl))
			{
				variant = (pAsset->GetFlags() & EAssetFlags::IsDefaultControl) != 0;
			}
			else if (role == static_cast<i32>(ModelUtils::ERoles::Name))
			{
				variant = QtUtil::ToQString(pAsset->GetName());
			}
			else
			{
				EAssetType const assetType = pAsset->GetType();
				EAssetFlags const flags = pAsset->GetFlags();

				switch (index.column())
				{
				case static_cast<i32>(CSystemSourceModel::EColumns::Notification):
					{
						switch (role)
						{
						case Qt::DecorationRole:
							if ((flags& EAssetFlags::HasPlaceholderConnection) != 0)
							{
								variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::Placeholder);
							}
							else if ((flags& EAssetFlags::HasConnection) == 0)
							{
								variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::NoConnection);
							}
							else if (((assetType == EAssetType::Folder) || (assetType == EAssetType::Switch)) && ((flags& EAssetFlags::HasControl) == 0))
							{
								variant = ModelUtils::GetItemNotificationIcon(ModelUtils::EItemStatus::NoControl);
							}
							break;
						case Qt::ToolTipRole:
							if ((flags& EAssetFlags::HasPlaceholderConnection) != 0)
							{
								if ((assetType == EAssetType::Folder) || (assetType == EAssetType::Switch))
								{
									variant = tr("Contains item whose connection was not found in middleware project");
								}
								else
								{
									variant = tr("Item connection was not found in middleware project");
								}
							}
							else if ((flags& EAssetFlags::HasConnection) == 0)
							{
								if ((assetType == EAssetType::Folder) || (assetType == EAssetType::Switch))
								{
									variant = tr("Contains item that is not connected to any middleware control");
								}
								else
								{
									variant = tr("Item is not connected to any middleware control");
								}
							}
							else if ((flags& EAssetFlags::HasControl) == 0)
							{
								if (assetType == EAssetType::Folder)
								{
									variant = tr("Contains no audio control");
								}
								else if (assetType == EAssetType::Switch)
								{
									variant = tr("Contains no state");
								}
							}
							break;
						case static_cast<i32>(ModelUtils::ERoles::Id):
							if (assetType != EAssetType::Folder)
							{
								CControl const* const pControl = static_cast<CControl*>(pAsset);

								if (pControl != nullptr)
								{
									variant = pControl->GetId();
								}
							}
							break;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::Type):
					{
						if ((role == Qt::DisplayRole) && (assetType != EAssetType::Folder))
						{
							variant = QString(AssetUtils::GetTypeName(assetType));
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::Placeholder):
					{
						if (role == Qt::CheckStateRole)
						{
							variant = ((flags& EAssetFlags::HasPlaceholderConnection) == 0) ? Qt::Checked : Qt::Unchecked;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::NoConnection):
					{
						if (role == Qt::CheckStateRole)
						{
							variant = ((flags& EAssetFlags::HasConnection) != 0) ? Qt::Checked : Qt::Unchecked;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::NoControl):
					{
						if ((role == Qt::CheckStateRole) && ((assetType == EAssetType::Folder) || (assetType == EAssetType::Switch)))
						{
							variant = ((flags& EAssetFlags::HasControl) == 0) ? Qt::Checked : Qt::Unchecked;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::PakStatus):
					{
						switch (role)
						{
						case Qt::DecorationRole:
							variant = ModelUtils::GetPakStatusIcon(m_pLibrary->GetPakStatus());
							break;
						case Qt::ToolTipRole:
							{
								EPakStatus const pakStatus = m_pLibrary->GetPakStatus();

								if (pakStatus == (EPakStatus::InPak | EPakStatus::OnDisk))
								{
									variant = tr("Parent library is in pak and on disk");
								}
								else if ((pakStatus& EPakStatus::InPak) != 0)
								{
									variant = tr("Parent library is only in pak file");
								}
								else if ((pakStatus& EPakStatus::OnDisk) != 0)
								{
									variant = tr("Parent library is only on disk");
								}
								else
								{
									variant = tr("Parent library does not exist on disk. Save to write file.");
								}
							}
							break;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::InPak):
					{
						if (role == Qt::CheckStateRole)
						{
							variant = ((m_pLibrary->GetPakStatus() & EPakStatus::InPak) != 0) ? Qt::Checked : Qt::Unchecked;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::OnDisk):
					{
						if (role == Qt::CheckStateRole)
						{
							variant = ((m_pLibrary->GetPakStatus() & EPakStatus::OnDisk) != 0) ? Qt::Checked : Qt::Unchecked;
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::Scope):
					if ((role == Qt::DisplayRole) && (assetType != EAssetType::Folder))
					{
						CControl const* const pControl = static_cast<CControl*>(pAsset);

						if (pControl != nullptr)
						{
							variant = QtUtil::ToQString(g_assetsManager.GetScopeInfo(pControl->GetScope()).name);
						}
					}
					break;
				case static_cast<i32>(CSystemSourceModel::EColumns::Name):
					{
						switch (role)
						{
						case Qt::DecorationRole:
							variant = GetAssetIcon(assetType);
							break;
						case Qt::DisplayRole:
							if ((pAsset->GetFlags() & EAssetFlags::IsModified) != 0)
							{
								variant = QtUtil::ToQString(pAsset->GetName() + " *");
							}
							else
							{
								variant = QtUtil::ToQString(pAsset->GetName());
							}
							break;
						case Qt::EditRole:
							variant = QtUtil::ToQString(pAsset->GetName());
							break;
						case Qt::ToolTipRole:
							if (!pAsset->GetDescription().IsEmpty())
							{
								variant = QtUtil::ToQString(pAsset->GetName() + ": " + pAsset->GetDescription());
							}
							else
							{
								variant = QtUtil::ToQString(pAsset->GetName());
							}
							break;
						case static_cast<i32>(ModelUtils::ERoles::SortPriority):
							variant = static_cast<i32>(assetType);
							break;
						case static_cast<i32>(ModelUtils::ERoles::InternalPointer):
							variant = reinterpret_cast<intptr_t>(pAsset);
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
bool CSystemLibraryModel::setData(QModelIndex const& index, QVariant const& value, i32 role)
{
	bool wasDataChanged = false;

	if (index.isValid() && (index.column() == m_nameColumn))
	{
		auto const pAsset = static_cast<CAsset*>(index.internalPointer());

		if (pAsset != nullptr)
		{
			switch (role)
			{
			case Qt::EditRole:
				{
					if (value.canConvert<QString>())
					{
						string const& oldName = pAsset->GetName();
						string const& newName = QtUtil::ToString(value.toString());

						if (!newName.empty() && newName.compareNoCase(oldName) != 0)
						{
							EAssetType const assetType = pAsset->GetType();

							switch (assetType)
							{
							case EAssetType::Preload:
							case EAssetType::Parameter:
							case EAssetType::Switch:
							case EAssetType::Trigger:
							case EAssetType::Environment:
								pAsset->SetName(AssetUtils::GenerateUniqueControlName(newName, assetType));
								break;
							case EAssetType::State:
							case EAssetType::Folder:
								pAsset->SetName(AssetUtils::GenerateUniqueName(newName, assetType, pAsset->GetParent()));
								break;
							default:
								DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, R"([Audio Controls Editor] The item type '%d' is not handled!)", assetType);
								break;
							}
						}

						wasDataChanged = true;
					}
				}
				break;
			default:
				DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, R"([Audio Controls Editor] The role '%d' is not handled!)", role);
				break;
			}
		}
	}

	return wasDataChanged;
}

//////////////////////////////////////////////////////////////////////////
QVariant CSystemLibraryModel::headerData(i32 section, Qt::Orientation orientation, i32 role) const
{
	return CSystemSourceModel::GetHeaderData(section, orientation, role);
}

//////////////////////////////////////////////////////////////////////////
Qt::ItemFlags CSystemLibraryModel::flags(QModelIndex const& index) const
{
	Qt::ItemFlags flags = Qt::NoItemFlags;

	if (index.isValid())
	{
		if (index.data(static_cast<i32>(ModelUtils::ERoles::IsDefaultControl)).toBool())
		{
			flags = QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
		}
		else if (index.column() == m_nameColumn)
		{
			flags = QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
		}
		else
		{
			flags = QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
		}
	}

	return flags;
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CSystemLibraryModel::index(i32 row, i32 column, QModelIndex const& parent) const
{
	QModelIndex modelIndex = QModelIndex();

	if ((row >= 0) && (column >= 0))
	{
		auto const childIndex = static_cast<size_t>(row);

		if (parent.isValid())
		{
			auto const pParent = static_cast<CAsset const* const>(parent.internalPointer());

			if ((pParent != nullptr) && (childIndex < pParent->ChildCount()))
			{
				CAsset const* const pChild = pParent->GetChild(childIndex);

				if (pChild != nullptr)
				{
					modelIndex = createIndex(row, column, reinterpret_cast<quintptr>(pChild));
				}
			}
		}
		else if (childIndex < m_pLibrary->ChildCount())
		{
			modelIndex = createIndex(row, column, reinterpret_cast<quintptr>(m_pLibrary->GetChild(childIndex)));
		}
	}

	return modelIndex;
}

//////////////////////////////////////////////////////////////////////////
QModelIndex CSystemLibraryModel::parent(QModelIndex const& index) const
{
	QModelIndex modelIndex = QModelIndex();

	if (index.isValid())
	{
		CAsset const* const pAsset = static_cast<CAsset*>(index.internalPointer());

		if (pAsset != nullptr)
		{
			CAsset const* const pParent = pAsset->GetParent();

			if ((pParent != nullptr) && (pParent->GetType() != EAssetType::Library))
			{
				modelIndex = IndexFromItem(pParent);
			}
		}
	}

	return modelIndex;
}

//////////////////////////////////////////////////////////////////////////
bool CSystemLibraryModel::canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const
{
	bool canDrop = false;
	QString dragText = tr("Invalid operation");

	if (parent.isValid())
	{
		CAsset const* const pParent = static_cast<CAsset*>(parent.internalPointer());
		DRX_ASSERT_MESSAGE(pParent != nullptr, "Parent is null pointer.");

		if (pParent != nullptr)
		{
			canDrop = CSystemSourceModel::CanDropData(pData, *pParent);

			if (canDrop)
			{
				dragText = tr("Add to ") + QtUtil::ToQString(pParent->GetName());
			}
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(m_pLibrary != nullptr, "Library is null pointer.");

		if (m_pLibrary != nullptr)
		{
			canDrop = ((m_pLibrary->GetFlags() & EAssetFlags::IsDefaultControl) == 0) && CSystemSourceModel::CanDropData(pData, *m_pLibrary);

			if (canDrop)
			{
				dragText = tr("Add to ") + QtUtil::ToQString(m_pLibrary->GetName());
			}
			else if ((m_pLibrary->GetFlags() & EAssetFlags::IsDefaultControl) != 0)
			{
				dragText = tr("Cannot add items to the default controls library.");
			}
		}
	}

	CDragDropData::ShowDragText(qApp->widgetAt(QCursor::pos()), dragText);
	return canDrop;
}

//////////////////////////////////////////////////////////////////////////
bool CSystemLibraryModel::dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent)
{
	bool wasDropped = false;

	if (parent.isValid())
	{
		auto const pAsset = static_cast<CAsset*>(parent.internalPointer());
		DRX_ASSERT_MESSAGE(pAsset != nullptr, "Asset is null pointer.");
		wasDropped = CSystemSourceModel::DropData(pData, pAsset);
	}
	else
	{
		DRX_ASSERT_MESSAGE(m_pLibrary != nullptr, "Library is null pointer.");
		wasDropped = CSystemSourceModel::DropData(pData, m_pLibrary);
	}

	return wasDropped;
}

//////////////////////////////////////////////////////////////////////////
QMimeData* CSystemLibraryModel::mimeData(QModelIndexList const& indexes) const
{
	return CSystemSourceModel::GetDragDropData(indexes);
}

//////////////////////////////////////////////////////////////////////////
Qt::DropActions CSystemLibraryModel::supportedDropActions() const
{
	return static_cast<Qt::DropActions>(Qt::MoveAction | Qt::CopyAction);
}

//////////////////////////////////////////////////////////////////////////
QStringList CSystemLibraryModel::mimeTypes() const
{
	QStringList types = QAbstractItemModel::mimeTypes();
	types << CDragDropData::GetMimeFormatForType(ModelUtils::s_szSystemMimeType);
	types << CDragDropData::GetMimeFormatForType(ModelUtils::s_szImplMimeType);
	return types;
}
} //endns ACE
