// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D, 2014.
// -------------------------------------------------------------------------
//  File name: QtMain.h
//  Created:   26/09/2014 by timur
//  Description: QT MainFrame window
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMainWindow>

#include <QMenu>

#include "EditorFramework/EventLoopHandler.h"
#include "LevelEditor/LevelEditor.h"

class QToolWindowManager;
class QMainToolBarManager;
class QLoading;
class CWaitProgress;

class CEditorMainFrame : public QMainWindow, public IAutoEditorNotifyListener //TODO : class name doesn't match filename
{
	Q_OBJECT
public:
	CEditorMainFrame(QWidget* parent = 0);
	virtual ~CEditorMainFrame();
	void PostLoad();

	//////////////////////////////////////////////////////////////////////////

	static CEditorMainFrame* GetInstance();

	//////////////////////////////////////////////////////////////////////////

	QToolWindowManager*  GetToolManager();
	QMainToolBarManager* GetToolBarManager();

	//Temporary functions to handle CWaitProgress more elegantly
	void          AddWaitProgress(CWaitProgress* task);
	void          RemoveWaitProgress(CWaitProgress* task);

	bool          IsClosing() const;

	CLevelEditor* GetLevelEditor() { return m_levelEditor.get(); }

private:
	void OnIdleCallback();
	bool OnNativeEvent(uk message, long *result);
	void OnBackgroundUpdateTimer();

	void OnAutoBackupTimeChanged();
	void OnAutoSaveTimer();
	void OnAutoRemindTimer();
	void OnEditorNotifyEvent(EEditorNotifyEvent event);
	void UpdateWindowTitle(const QString& levelPath = "");

	bool focusNextPrevChild(bool next) override;
	void contextMenuEvent(QContextMenuEvent* pEvent);

	void SetDefaultLayout();
	void CreateToolsMenu();
	void BindSnapMenu();
	void BindAIMenu();
	void AboutToShowEditMenu(QMenu* editMenu);
	void CreateLayoutMenu(QMenu* layoutMenu);
	void InitActions();
	void InitLayout();
	void InitMenus();
	void InitMenuBar();
	bool BeforeClose();
	void closeEvent(QCloseEvent*);
	void SaveConfig();
	bool event(QEvent *event) override;

	void OnAxisConstrainChanged(i32 axis);

public:
	static CEditorMainFrame*      m_pInstance;

	QToolWindowManager*           m_toolManager;
	class CAboutDialog*           m_pAboutDlg;

	CEventLoopHandler             m_loopHandler;
	std::unique_ptr<CLevelEditor> m_levelEditor;

private:
	//Should not be accessible
	QStatusBar* statusBar() const { return QMainWindow::statusBar(); }
	QMainToolBarManager*        m_pMainToolBarManager;
	QTimer*                     m_pAutoBackupTimer;
	std::vector<CWaitProgress*> m_waitTasks;
	QMetaObject::Connection     m_layoutChangedConnection;
	bool                        m_bClosing;

	CTimeValue                  m_lastFrameDuration;
	CTimeValue                  m_lastUserInputTime;
	bool                        m_bUserEventPriorityMode; // emergency mode will disregard all updates to the engine while
};

