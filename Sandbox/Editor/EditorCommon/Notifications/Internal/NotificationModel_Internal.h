// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace Internal
{
	class CNotification;
}

class CNotificationFilterModel : public QSortFilterProxyModel
{
public:
	CNotificationFilterModel(QObject* pParent = nullptr);
	void SetFilterTypeState(unsigned type, bool bEnable);

protected:
	virtual bool filterAcceptsRow(i32 sourceRow, const QModelIndex& sourceParent) const override;

protected:
	unsigned m_typeMask;
};

//////////////////////////////////////////////////////////////////////////

class CNotificationModel : public QAbstractItemModel
{
public:
	enum class Roles : i32
	{
		Type = Qt::UserRole,
		SortRole,
	};

	CNotificationModel(QObject* pParent = nullptr);

	void OnNotificationAdded(i32 id);

	void CopyHistory() const;
	void CopyHistory(QModelIndexList indices) const;
	void ClearAll();
	void ShowCompleteHistory();

	void AddFilterType(i32 type);
	void RemoveFilterType(i32 type);

	//QAbstractItemModel implementation begin
	virtual i32           columnCount(const QModelIndex& parent = QModelIndex()) const override { return s_ColumnCount; }
	virtual QVariant      data(const QModelIndex& index, i32 role /* = Qt::DisplayRole */) const override;
	virtual bool          hasChildren(const QModelIndex& parent /* = QModelIndex() */) const override;
	virtual QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role /* = Qt::DisplayRole */) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual QModelIndex   index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex   parent(const QModelIndex& index) const override;
	virtual bool          setData(const QModelIndex& index, const QVariant& value, i32 role /*= Qt::EditRole*/) override;
	virtual i32           rowCount(const QModelIndex& parent /* = QModelIndex() */) const override;
	//QAbstractItemModel implementation end

	Internal::CNotification* NotificationFromIndex(const QModelIndex& index) const;

protected:
	// Determines the offset into the notifications array. Since we can clear the list we want to only show new notifications
	i32                m_startIdx;
	i32                m_startSize;
	i32                m_currIdx;
	static i32k   s_ColumnCount = 4;
	static tukk s_ColumnNames[s_ColumnCount];
};

