// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EditorFramework/Editor.h"
#include "CSharpEditorPlugin.h"
#include "CSharpOutputTextEdit.h"

//! Window designed to show compile messages from the CompiledMonoLibrary.
class CCSharpOutputWindow 
	: public CDockableEditor
	, public ICSharpMessageListener
	, public ITextEventListener
{
public:
	CCSharpOutputWindow();
	~CCSharpOutputWindow();

	virtual tukk GetEditorName() const override { return "C# Output"; };

	// ICSharpMessageListener
	virtual void OnMessagesUpdated(string messages) override;
	// ~ICSharpMessageListener

	// ITextEventListener
	virtual void OnMessageDoubleClicked(QMouseEvent* event) override;
	virtual bool OnMouseMoved(QMouseEvent* event) override;
	// ~ITextEventListener

private:
	CCSharpOutputTextEdit* m_pCompileTextWidget;
	string m_plainText;

	string GetLineFromRange(const string& text, i32 lineStart, i32 lineEnd)
	{
		string line = text.substr(lineStart, lineEnd - lineStart);
		line.Trim();
		return line;
	}

	void GetLineRange(const string& text, i32 index, i32& lineStart, i32& lineEnd);
	bool TryParseStackTraceLine(const string& line, string& path, i32& lineNumber);
};
