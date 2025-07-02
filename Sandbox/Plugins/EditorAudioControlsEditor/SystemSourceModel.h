// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>

class CItemModelAttribute;

namespace ACE
{
class CAsset;
class CLibrary;

class CSystemSourceModel : public QAbstractItemModel
{
public:

	enum class EColumns
	{
		Notification,
		Type,
		Placeholder,
		NoConnection,
		NoControl,
		PakStatus,
		InPak,
		OnDisk,
		Scope,
		Name,
		Count,
	};

	CSystemSourceModel(QObject* const pParent);
	virtual ~CSystemSourceModel() override;

	void                        DisconnectSignals();

	static CItemModelAttribute* GetAttributeForColumn(EColumns const column);
	static QVariant             GetHeaderData(i32 const section, Qt::Orientation const orientation, i32 const role);

	static CAsset*              GetAssetFromIndex(QModelIndex const& index, i32 const column);
	static QMimeData*           GetDragDropData(QModelIndexList const& list);

	static bool                 CanDropData(QMimeData const* const pData, CAsset const& parent);
	static bool                 DropData(QMimeData const* const pData, CAsset* const pParent);

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
	virtual Qt::DropActions supportedDropActions() const override;
	virtual QStringList     mimeTypes() const override;
	// ~QAbstractItemModel

private:

	void ConnectSignals();

	bool      m_ignoreLibraryUpdates;
	i32 const m_nameColumn;
};
} //endns ACE
