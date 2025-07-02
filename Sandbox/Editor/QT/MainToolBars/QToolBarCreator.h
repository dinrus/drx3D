// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include "Controls/EditorDialog.h"
#include "QMainToolBarManager.h"

class QToolBar;
class QAdvancedTreeView;
class QDeepFilterProxyModel;
class QHBoxLayout;
class QListView;
class QEditableComboBox;
class QLineEdit;
class QToolButton;

class QToolBarCreator : public CEditorDialog
{
	Q_OBJECT

private:
	class QDropContainer;
	class QCustomToolBar;
	class DropCommandModel;

public:
	QToolBarCreator();

	QString GetCurrentToolBarText();
	void    IconSelected(tukk szIconPath);
	void    CVarsSelected(const QStringList& selectedCVars);
	void    SetCVarName(tukk cvarName);
	void    SetCVarValue(tukk cvarValue);
	void    SetCommandName(tukk commandName);
	void    SetIconPath(tukk szIconPath);
	void    OnSelectedItemChanged(std::shared_ptr<QMainToolBarManager::QItemDesc> selectedItem);

public Q_SLOTS:
	void CurrentItemChanged(i32 index);
	void OnAddToolBar();
	void OnRemoveToolBar();
	void OnContextMenu(const QPoint& position) const;

protected:
	virtual void  dragEnterEvent(QDragEnterEvent* pEvent) override;
	virtual void  dropEvent(QDropEvent* pEvent) override;
	virtual QSize sizeHint() const override { return QSize(640, 480); }

	void          RenameToolBar(const QString& before, const QString& after);
	void          ShowCommandWidgets();
	void          ShowCVarWidgets();
	void          SetCVarValueRegExp();

private:
	std::shared_ptr<QMainToolBarManager::QItemDesc> m_pSelectedItem;
	QDropContainer* m_pDropContainer;
	QEditableComboBox*                              m_pToolbarSelect;
	QAdvancedTreeView*                              m_pTreeView;
	DropCommandModel*                               m_pItemModel;
	QDeepFilterProxyModel*                          m_pProxyModel;
	QListView*        m_pPreview;
	QLineEdit*        m_pNameInput;
	QLineEdit*        m_pCommandInput;
	QLineEdit*        m_pIconInput;
	QLineEdit*        m_pCVarInput;
	QLineEdit*        m_pCVarValueInput;
	QToolButton*      m_pIconBrowserButton;
	QToolButton*      m_pCVarBrowserButton;

	QVector<QWidget*> m_commandWidgets;
	QVector<QWidget*> m_cvarWidgets;
};

//////////////////////////////////////////////////////////////////////////

class QToolBarCreator::QDropContainer : public QWidget
{
	Q_OBJECT
public:
	QDropContainer(QToolBarCreator* pParent);
	~QDropContainer();

	void                                               AddItem(const QVariant& itemVariant, i32 idx = -1);
	void                                               AddCommand(const CCommand* pCommand, i32 idx = -1);
	void                                               AddSeparator(i32 sourceIdx = -1, i32 targetIdx = -1);
	void                                               AddCVar(const QString& cvarName, i32 idx = -1);

	void                                               RemoveCommand(const CCommand* pCommand);
	void                                               RemoveItem(std::shared_ptr<QMainToolBarManager::QItemDesc> pItem);
	void                                               RemoveItemAt(i32 idx);
	void                                               BuildPreview();

	QCustomToolBar*                                    CreateToolBar(const QString& title, const std::shared_ptr<QMainToolBarManager::QToolBarDesc> toolBarDesc);
	std::shared_ptr<QMainToolBarManager::QToolBarDesc> GetCurrentToolBarDesc() { return m_pCurrentToolBarDesc; }
	void                                               SetSelectedActionIcon(tukk szPath);

	virtual bool                                       eventFilter(QObject* pObject, QEvent* pEvent) override;
	virtual void                                       mouseMoveEvent(QMouseEvent* pEvent) override;

	virtual void                                       dragEnterEvent(QDragEnterEvent* pEvent) override;
	virtual void                                       dragMoveEvent(QDragMoveEvent* pEvent) override;
	virtual void                                       dropEvent(QDropEvent* pEvent) override;
	virtual void                                       dragLeaveEvent(QDragLeaveEvent* pEvent) override;

public Q_SLOTS:
	void ShowToolBarContextMenu(const QPoint& position);

protected:
	i32      GetIndexFromMouseCoord(const QPoint& globalPos);
	QAction* ActionAt(const QPoint& globalPos);

public:
	CDrxSignal<void(std::shared_ptr<QMainToolBarManager::QItemDesc> )> selectedItemChanged;
private:
	std::shared_ptr<QMainToolBarManager::QItemDesc>                    m_pSelectedItem;
	QPoint           m_DragStartPosition;
	QToolBarCreator* m_pToolBarCreator;
	QHBoxLayout*     m_pPreviewLayout;
	QCustomToolBar*  m_pCurrentToolBar;
	std::shared_ptr<QMainToolBarManager::QToolBarDesc> m_pCurrentToolBarDesc;

	bool m_bDragStarted;
};

