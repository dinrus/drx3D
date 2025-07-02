// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>
#include <QTreeView>

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "IObjectEnumerator.h"
#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include "QAdvancedTreeView.h"

class QStandardItemModel;
class QStandardItem;
class QHBoxLayout;
class QSplitter;
class QDeepFilterProxyModel;

class EDITOR_COMMON_API QObjectTreeView : public QAdvancedTreeView
{
public:
	enum Roles
	{
		Id = Qt::UserRole + 1,
		NodeType,
		Sort
	};

	QObjectTreeView(QWidget* pParent = nullptr);
	void SetIndexFilter(std::function<void(QModelIndexList&)> indexFilter) { m_indexFilterFn = indexFilter; }

protected:
	virtual void startDrag(Qt::DropActions supportedActions) override;

public:
	std::function<void(QModelIndexList&)> m_indexFilterFn;
};

class EDITOR_COMMON_API QObjectTreeWidget : public QWidget, public IObjectEnumerator
{
	Q_OBJECT
public:
	enum NodeType
	{
		Group,
		Entry
	};

public:
	QObjectTreeWidget(QWidget* pParent = nullptr, tukk szRegExp = "[/\\\\.]");

	QStringList    GetSelectedIds() const;
	const QRegExp& GetRegExp() const                { return m_regExp; }
	void           SetRegExp(const QRegExp& regExp) { m_regExp = regExp; }

	virtual bool   PathExists(tukk path);

	virtual void   AddEntry(tukk path, tukk id, tukk sortStr = "") override;
	virtual void   RemoveEntry(tukk path, tukk id) override;
	virtual void   ChangeEntry(tukk path, tukk id, tukk sortStr = "") override;

	void           Clear();

	void           ExpandAll();
	void           CollapseAll();

protected:
	QStandardItem* Find(const QString& itemName, QStandardItem* pParent = nullptr);
	QStandardItem* FindOrCreate(const QString& itemName, QStandardItem* pParent = nullptr);
	QStandardItem* TakeItemForId(const QString& id, const QModelIndex& parentIdx = QModelIndex());
	virtual void   dragEnterEvent(QDragEnterEvent* pEvent) override;
	virtual void   paintEvent(QPaintEvent* pEvent) override;

public:
	CDrxSignal<void(tukk)>              signalOnClickFile;
	CDrxSignal<void(tukk)>              signalOnDoubleClickFile;
	CDrxSignal<void(QDragEnterEvent*, QDrag*)> signalOnDragStarted;
	CDrxSignal<void()>                         singalOnDragEnded;

protected:
	QObjectTreeView*       m_pTreeView;
	QStandardItemModel*    m_pModel;
	QHBoxLayout*           m_pToolLayout;
	QSplitter*             m_pSplitter;
	QDeepFilterProxyModel* m_pProxy;
	QRegExp                m_regExp;
	bool                   m_bIsDragTracked;
};

Q_DECLARE_METATYPE(QObjectTreeWidget::NodeType)

