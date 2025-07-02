// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <StdAfx.h>
#include "CustomCommand.h"

#include "CommandManager.h"
#include "CommandModuleDescription.h"
#include <drx3D/Sandbox/Editor/EditorCommon/ICommandManager.h>

CCustomCommand::CCustomCommand(const string& name, const string& command)
	: CCommand0("custom", name, CCommandDescription(""), Functor0())
	, m_command(command)
{
}

void CCustomCommand::Register()
{
	GetIEditorImpl()->GetCommandManager()->AddCustomCommand(this);
}

void CCustomCommand::Unregister()
{
	GetIEditorImpl()->GetCommandManager()->RemoveCustomCommand(this);
}

void CCustomCommand::SetName(tukk name)
{
	m_name = name;
}

void CCustomCommand::SetCommandString(tukk commandStr)
{
	m_command = commandStr;
}

