// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "ScriptEnvironment.h"

#include "IEditorImpl.h"
#include "Objects/EntityObject.h"

EditorScriptEnvironment::EditorScriptEnvironment()
{
	GetIEditorImpl()->RegisterNotifyListener(this);
}

EditorScriptEnvironment::~EditorScriptEnvironment()
{
	GetIEditorImpl()->UnregisterNotifyListener(this);
}

void EditorScriptEnvironment::OnEditorNotifyEvent(EEditorNotifyEvent event)
{
	if (event == eNotify_OnInit)
	{
		RegisterWithScriptSystem();
	}

	if (event == eNotify_OnSelectionChange)
	{
		SelectionChanged();
	}
}

void EditorScriptEnvironment::RegisterWithScriptSystem()
{
	Init(gEnv->pScriptSystem, gEnv->pSystem);
	SetGlobalName("Editor");

	m_selection = SmartScriptTable(gEnv->pScriptSystem, false);
	m_pMethodsTable->SetValue("Selection", m_selection);

	// Editor.* functions go here
	RegisterTemplateFunction("Command", "commandName", *this, &EditorScriptEnvironment::Command);
}

void EditorScriptEnvironment::SelectionChanged()
{
	if (!m_selection)
		return;

	m_selection->Clear();

	const CSelectionGroup* selectionGroup = GetIEditorImpl()->GetSelection();
	i32 selectionCount = selectionGroup->GetCount();

	for (i32 i = 0, k = 0; i < selectionCount; ++i)
	{
		CBaseObject* object = selectionGroup->GetObject(i);
		if (object->GetType() == OBJTYPE_ENTITY)
		{
			CEntityObject* entity = (CEntityObject*)object;

			if (IEntity* iEntity = entity->GetIEntity())
				m_selection->SetAt(++k, ScriptHandle(iEntity->GetId()));
		}
	}
}

i32 EditorScriptEnvironment::Command(IFunctionHandler* pH, tukk commandName)
{
	GetIEditorImpl()->GetCommandManager()->Execute(commandName);

	return pH->EndFunction();
}

