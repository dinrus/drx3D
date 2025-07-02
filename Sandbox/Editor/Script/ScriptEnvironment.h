// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ScriptEnvironment_h__
#define __ScriptEnvironment_h__

#pragma once

#include <DrxScriptSystem/ScriptHelpers.h>

class EditorScriptEnvironment
	: public IEditorNotifyListener
	  , public CScriptableBase
{
public:
	EditorScriptEnvironment();
	~EditorScriptEnvironment();
	// IEditorNotifyListener
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event);
	// ~IEditorNotifyListener

private:
	void RegisterWithScriptSystem();
	void SelectionChanged();

	SmartScriptTable m_selection;
private:
	i32 Command(IFunctionHandler* pH, tukk commandName);
};

#endif

