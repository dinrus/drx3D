// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "aibehavior.h"
#include "Util\FileUtil.h"
#include <DrxScriptSystem/IScriptSystem.h>

//////////////////////////////////////////////////////////////////////////
void CAIBehavior::ReloadScript()
{
	// Execute script file in script system.
	if (m_script.IsEmpty())
		return;

	if (CFileUtil::CompileLuaFile(GetScript()))
	{
		IScriptSystem* scriptSystem = GetIEditorImpl()->GetSystem()->GetIScriptSystem();
		// Script compiled succesfully.
		scriptSystem->ReloadScript(m_script);
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIBehavior::Edit()
{
	CFileUtil::EditTextFile(GetScript());
}

//////////////////////////////////////////////////////////////////////////
void CAICharacter::ReloadScript()
{
	// Execute script file in script system.
	if (m_script.IsEmpty())
		return;

	if (CFileUtil::CompileLuaFile(GetScript()))
	{
		IScriptSystem* scriptSystem = GetIEditorImpl()->GetSystem()->GetIScriptSystem();
		// Script compiled succesfully.
		scriptSystem->ReloadScript(m_script);
	}
}

//////////////////////////////////////////////////////////////////////////
void CAICharacter::Edit()
{
	CFileUtil::EditTextFile(GetScript());
}

