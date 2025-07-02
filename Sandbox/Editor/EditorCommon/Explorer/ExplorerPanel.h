// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <memory>
#include <QWidget>
#include <QItemSelection>
#include <QAdvancedTreeView.h>
#include <QMetaObject>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/NameGeneration.h>
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Explorer.h"

class QToolButton;
class QLineEdit;
class QPushButton;
class QString;
class QItemSelection;
class QModelIndex;
class QAbstractItemModel;
class QMenu;
class QDockWidget;
class QMainWindow;
class QSearchBox;

namespace Explorer
{

class EDITOR_COMMON_API ExplorerTreeView : public QAdvancedTreeView
{
	Q_OBJECT
public:
	ExplorerTreeView();

	void setModel(QAbstractItemModel* model);
	void mousePressEvent(QMouseEvent* ev) override;

signals:
	void SignalClickOnSelectedItem(const QModelIndex& index);
	void SignalStartDrag(const QModelIndex& index);

protected:
	void mouseMoveEvent(QMouseEvent* event) override;
	void startDrag(Qt::DropActions supportedActions) override;

private:
	QMetaObject::Connection m_selectionHandler;
	bool                    m_dragging;
	bool                    m_treeSelectionChanged;
};

class ExplorerData;
class ExplorerModel;
class ExplorerFilterProxyModel;
struct FilterOptions;

class EDITOR_COMMON_API ExplorerPanel : public QWidget
{
	Q_OBJECT

public:
	ExplorerPanel(QWidget* parent, ExplorerData* explorerData);
	~ExplorerPanel();
	ExplorerData*  GetExplorerData() const { return m_explorerData; }
	void           SetDockWidget(QDockWidget* dockWidget);
	void           SetRootIndex(i32 rootIndex);
	i32            RootIndex() const { return m_explorerRootIndex; }
	void           GetSelectedEntries(ExplorerEntries* entries, enum BMerge merge = BMerge(0)) const;
	void           SetSelectedEntries(const ExplorerEntries& entries);
	bool           IsEntrySelected(ExplorerEntry* entry) const;
	ExplorerEntry* GetEntryByPath(tukk szPath) const;
	ExplorerEntry* GetCurrentEntry() const;
	void           SetCurrentEntry(ExplorerEntry* entry);

	void           Serialize(Serialization::IArchive& ar);

public slots:
	void           OnFilterTextChanged(const QString& str);
	void           OnTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void           OnClickedSelectedItem(const QModelIndex& index);
	void           OnStartDrag(const QModelIndex& index);
	void           OnHeaderContextMenu(const QPoint& pos);
	void           OnHeaderColumnToggle();
	void           OnContextMenu(const QPoint& pos);
	void           OnMenuCopyName();
	void           OnMenuCopyPath();
	void           OnMenuPasteSelection();
	void           OnMenuExpandAll();
	void           OnMenuCollapseAll();

	void           OnActivated(const QModelIndex& index);
	void           OnExplorerAction(const ExplorerAction& action);
	void           OnEntryImported(ExplorerEntry* entry, ExplorerEntry* oldEntry);
	void           OnEntryLoaded(ExplorerEntry* entry);
	void           OnRootButtonPressed();
	void           OnRootSelected(bool);
	void           OnExplorerEndReset();
	void           OnExplorerBeginBatchChange(i32 subtree);
	void           OnExplorerEndBatchChange(i32 subtree);
	void           OnExplorerEntryModified(ExplorerEntryModifyEvent& ev);
	void           OnAssetLoaded();
	void           OnRefreshFilter();
	void           OnFilterButtonToggled(bool filterMode);
	void           OnFilterOptionsChanged();
signals:
	void           SignalSelectionChanged();
	void           SignalActivated(const ExplorerEntry* entry);
	void           SignalStartDrag(const ExplorerEntry* entry);
	void		   SignalFocusIn();
protected:
	bool           eventFilter(QObject* sender, QEvent* ev) override;
private:
	void           SetTreeViewModel(QAbstractItemModel* model);
	void           UpdateRootMenu();
	void           SetExpanded(const QModelIndex& index, bool bExpand, uint depth = 0);
	QModelIndex    FindIndexByEntry(ExplorerEntry* entry) const;
	ExplorerEntry* GetEntryByIndex(const QModelIndex& index) const;

	unique_ptr<FilterOptions> m_filterOptions;
	QDockWidget*              m_dockWidget;
	ExplorerTreeView*         m_treeView;
	ExplorerModel*            m_model;
	ExplorerData*             m_explorerData;
	ExplorerFilterProxyModel* m_filterModel;
	QSearchBox*               m_searchBox;
	QPushButton*              m_rootButton;
	QToolButton*              m_filterButton;
	QMenu*                    m_rootMenu;

	vector<QAction*>          m_rootMenuActions;
	i32                       m_explorerRootIndex;
	bool                      m_filterMode;
	uint                      m_batchChangesRunning;
	QPropertyTree*            m_filterOptionsTree;
};

}

