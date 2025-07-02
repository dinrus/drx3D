// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   ConsoleWindow.h
//  Version:     v1.00
//  Created:     03/03/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <drx3D/CoreX/Platform/platform.h>
#include <DrxSystem/ISystem.h>
#include <drx3D/CoreX/BoostHelpers.h> // to make sure we get own throw_exceptions
#include <boost/circular_buffer.hpp>
#include <algorithm>
#include "Messages.h"
#include "AutoCompletePopup.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

#include "TabLineEdit.h"
#include <QtViewPane.h>

#include "Util/EditorUtils.h"

#include <QTextEdit>

#include <DrxSystem/ILog.h>

#include "EditorFramework/Editor.h"
#include <QMenu>
#include <QMenuBar>

#include <QSearchBox.h>

//the limit to number of console messages to cache
//this ensures an upper bound in memory usage and processing time in case of huge log-spam
#define CONSOLE_MAX_HISTORY (size_t)2000

//console window implementation
//see also ConsoleWindow.ui and ConsoleWindow.cpp
class CConsoleWindow : public CDockableEditor, public ILogCallback
{
	Q_OBJECT;
	virtual tukk GetEditorName() const override { return "Console"; }

	//handle incoming update message
	void HandleMessage(const Messages::SHistoryLine& line);

	//handle incoming auto-complete reply
	void HandleAutoComplete(const Messages::SAutoCompleteReply& reply);

	//handle changed text
	void HandleInput(const QString& current);

	//handle tab pressed
	void HandleTab();

	//handle enter pressed
	void HandleEnter();

	//handle auto-complete selection change
	void HandleAutoCompleteSelection(const QString& selection, CAutoCompletePopup::ESelectReason reason);

	//handle CVar button pressed
	void HandleCVarButtonPressed();

	//move the pop-up window
	void MovePopup(bool force = false);

	//when one of the following is called, also move the pop-up window
	void keyPressEvent(QKeyEvent* e);

	void OnWriteToFile(tukk sText, bool bNewLine) {};
	void OnWriteToConsole(tukk sText, bool bNewLine);

	//setup this window
	void         SetupUI();

	virtual bool OnSave() override;
	virtual bool OnFind() override;
	virtual bool OnFindNext() override;
	virtual bool OnFindPrevious() override;

	bool         ClearConsole();
	void         SearchBox(const QString& text);
	void         ClearSearchBox();
	void         HighlightSelection(const QString& text);
	void         SearchBoxEnterPressed();
	bool         OnCloseSearch();
	bool m_searchBackwards;

public:
	//constructor
	CConsoleWindow(QWidget* pParent = nullptr);

	//destructor
	~CConsoleWindow();

	//static QTextEdit& GetEditLog(){ return m_pHistory;}

	//////////////////////////////////////////////////////////
	// CDockableWindow implementation
	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_BOTTOM; }
	//////////////////////////////////////////////////////////

protected:
	virtual bool eventFilter(QObject* o, QEvent* ev) Q_DECL_OVERRIDE;

private:
	bool event(QEvent* pEvent);
	void InitMenu();

private:
	//the address for this instance
	const string m_address;

	//the last execute request
	string m_lastExecuteRequest;

	//the last auto-complete request
	QString m_lastAutoCompleteRequest;

	//pop-up dialog for auto-completion
	CAutoCompletePopup m_popup;

	//timer for keeping popup in position
	//currently, move events don't work properly from MFC docking panels
	QTimer*       m_pTimer;

	QTextEdit*    m_pHistory;
	QTabLineEdit* m_pInput;

	i32           m_lastConsoleLineNumber;

	//if set, keep the history scrolled down all the way at all times
	i32               m_sticky;

	QSearchBox        m_searchBox;
	QWidget*          m_searchWidget;
	QString           m_lastSearchBoxString;
};

