// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EditorActionButton.h"
#include <Util/BoostPythonHelpers.h>

namespace Serialization
{
struct CommandActionButton : public StdFunctionActionButton
{
	explicit CommandActionButton(string command, tukk icon_ = "") : StdFunctionActionButton(std::bind(&CommandActionButton::ExecuteCommand, command), icon_) {}
	static void ExecuteCommand(string command)
	{
		GetIEditor()->GetICommandManager()->Execute(command.c_str());
	}
};
inline bool Serialize(Serialization::IArchive& ar, Serialization::CommandActionButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(button), name, label);
	else
		return false;
}

struct PythonActionButton : public StdFunctionActionButton
{
	explicit PythonActionButton(string pythonCmd, tukk icon_ = "") : StdFunctionActionButton(std::bind(&PythonActionButton::ExecutePython, pythonCmd), icon_) {}
	static void ExecutePython(string cmd)
	{
		PyScript::Execute(cmd.c_str());
	}
};
inline bool Serialize(Serialization::IArchive& ar, Serialization::PythonActionButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(button), name, label);
	else
		return false;
}
}

