// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>

#include <SharedData.h>
#include <FileImportInfo.h>
#include <FileDialogs/ExtensionFilter.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
class CImpl;
class CItem;

class CItemModel final : public QAbstractItemModel
{
public:

	enum class EColumns
	{
		Notification,
		Connected,
		PakStatus,
		InPak,
		OnDisk,
		Name,
		Count,
	};

	explicit CItemModel(CImpl const& impl, CItem const& rootItem, QObject* const pParent);

	CItemModel() = delete;

	QString GetTargetFolderName(QModelIndex const& index) const;
	void    Reset();

protected:

	// QAbstractItemModel
	virtual i32             rowCount(QModelIndex const& parent) const override;
	virtual i32             columnCount(QModelIndex const& parent) const override;
	virtual QVariant        data(QModelIndex const& index, i32 role) const override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(QModelIndex const& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, QModelIndex const& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(QModelIndex const& index) const override;
	virtual bool            canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const override;
	virtual bool            dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual QStringList     mimeTypes() const override;
	virtual QMimeData*      mimeData(QModelIndexList const& indexes) const override;
	// ~QAbstractItemModel

private:

	CItem*      ItemFromIndex(QModelIndex const& index) const;
	QModelIndex IndexFromItem(CItem const* const pItem) const;

	CImpl const& m_impl;
	CItem const& m_rootItem;
};
} //endns PortAudio
} //endns Impl
} //endns ACE

