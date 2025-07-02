// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <SharedData.h>

namespace ACE
{
class CControl;

class CConnectionsModel final : public QAbstractItemModel
{
	Q_OBJECT

public:

	enum class EColumns
	{
		Notification,
		Name,
		Path,
		Count,
	};

	explicit CConnectionsModel(QObject* const pParent);
	virtual ~CConnectionsModel() override;

	CConnectionsModel() = delete;

	void Init(CControl* const pControl);
	void DisconnectSignals();

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

signals:

	void SignalConnectionAdded(ControlId const id);

private:

	void ConnectSignals();
	void ResetCache();
	void ResetModelAndCache();

	CControl*                  m_pControl;
	std::vector<ConnectionPtr> m_connectionsCache;
};
} //endns ACE

