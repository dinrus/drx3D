// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>

namespace ACE
{
class CAsset;
class CLibrary;

class CSystemLibraryModel : public QAbstractItemModel
{
public:

	CSystemLibraryModel(CLibrary* const pLibrary, QObject* const pParent);
	virtual ~CSystemLibraryModel() override;

	void DisconnectSignals();

protected:

	// QAbstractItemModel
	virtual i32             rowCount(QModelIndex const& parent) const override;
	virtual i32             columnCount(QModelIndex const& parent) const override;
	virtual QVariant        data(QModelIndex const& index, i32 role) const override;
	virtual bool            setData(QModelIndex const& index, QVariant const& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(QModelIndex const& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, QModelIndex const& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(QModelIndex const& index) const override;
	virtual bool            canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const override;
	virtual bool            dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) override;
	virtual QMimeData*      mimeData(QModelIndexList const& indexes) const override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual QStringList     mimeTypes() const override;
	// ~QAbstractItemModel

private:

	void        ConnectSignals();
	QModelIndex IndexFromItem(CAsset const* pAsset) const;

	CLibrary* const m_pLibrary;
	i32 const       m_nameColumn;
};
} //endns ACE
