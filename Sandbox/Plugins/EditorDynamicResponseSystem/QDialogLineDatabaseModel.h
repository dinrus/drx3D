// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QPushButton>

namespace DRS
{
struct IDialogLineDatabase;
}
struct SLineItem;

enum class EItemType
{
	INVALID = 0,
	DIALOG_LINE_SET,
	DIALOG_LINE
};

class QDialogLineDatabaseModel : public QAbstractItemModel
{

public:
	QDialogLineDatabaseModel(QObject* pParent);
	~QDialogLineDatabaseModel();

	EItemType ItemType(const QModelIndex& index) const;
	bool      CanEdit(const QModelIndex& index) const;

	//////////////////////////////////////////////////////////
	// QAbstractTableModel implementation
	virtual i32           rowCount(const QModelIndex& parent) const override;
	virtual i32           columnCount(const QModelIndex& parent) const override;
	virtual QVariant      data(const QModelIndex& index, i32 role) const override;
	virtual bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	virtual QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual bool          insertRows(i32 row, i32 count, const QModelIndex& parent = QModelIndex()) override;
	virtual bool          removeRows(i32 row, i32 count, const QModelIndex& parent = QModelIndex()) override;
	virtual QModelIndex   index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex   parent(const QModelIndex& index) const override;
	virtual bool          hasChildren(const QModelIndex& parent) const override;
	//////////////////////////////////////////////////////////

	bool ExecuteScript(i32 row, i32 count);
	void ForceDataReload();  //revert

private:
	SLineItem*  ItemFromIndex(const QModelIndex& index) const;
	QModelIndex IndexFromItem(const SLineItem* item) const;

	DRS::IDialogLineDatabase* m_pDatabase;
	SLineItem*                m_pRoot;
};

class QDialogLineDelegate : public QStyledItemDelegate
{
public:
	QDialogLineDelegate(QWidget* pParent = 0);

	//////////////////////////////////////////////////////////
	// QStyledItemDelegate implementation
	virtual bool     editorEvent(QEvent* pEvent, QAbstractItemModel* pModel, const QStyleOptionViewItem& option, const QModelIndex& index) override;
	virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	virtual void     paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	//////////////////////////////////////////////////////////

private:
	QWidget* m_pParent;
};

