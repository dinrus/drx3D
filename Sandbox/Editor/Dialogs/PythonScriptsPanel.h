// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <EditorFramework/Editor.h>

class QAdvancedTreeView;

class CPythonScriptsPanel : public CDockableEditor
{
	Q_OBJECT

public:
	CPythonScriptsPanel();
	~CPythonScriptsPanel();

	//////////////////////////////////////////////////////////
	// CDockableEditor implementation
	virtual tukk GetEditorName() const override { return "Python Scripts"; }
	virtual QRect       GetPaneRect() override        { return QRect(0, 0, 200, 200); }
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// QWidget
	virtual QSize sizeHint() const override { return QSize(240, 600); }
	//////////////////////////////////////////////////////////////////////////

private slots:
	void ExecuteScripts() const;

private:
	QAdvancedTreeView* m_pTree;
};

