// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_Script.cpp
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptBind_Script.h>
#include <drx3D/Script/ScriptTimerMgr.h>
#include <drx3D/Script/ScriptSystem.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Entity/IEntitySystem.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CScriptBind_Script::CScriptBind_Script(IScriptSystem* pScriptSystem, ISystem* pSystem)
{
	CScriptableBase::Init(pScriptSystem, pSystem);
	SetGlobalName("Script");

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Script::

	SCRIPT_REG_FUNC(ReloadScripts);
	SCRIPT_REG_FUNC(ReloadScript);
	SCRIPT_REG_TEMPLFUNC(ReloadEntityScript, "className");
	SCRIPT_REG_FUNC(LoadScript);
	SCRIPT_REG_FUNC(UnloadScript);
	SCRIPT_REG_FUNC(DumpLoadedScripts);
	SCRIPT_REG_TEMPLFUNC(SetTimer, "nMilliseconds,Function");
	SCRIPT_REG_TEMPLFUNC(SetTimerForFunction, "nMilliseconds,Function");
	SCRIPT_REG_TEMPLFUNC(KillTimer, "nTimerId");
}

CScriptBind_Script::~CScriptBind_Script()
{
}

/*!reload all previosly loaded scripts
 */
i32 CScriptBind_Script::ReloadScripts(IFunctionHandler* pH)
{
	SCRIPT_CHECK_PARAMETERS(0);
	m_pSS->ReloadScripts();
	return pH->EndFunction();
}

/*!reload a specified script. If the script wasn't loaded at least once before the function will fail
   @param sFileName path of the script that has to be reloaded
 */
i32 CScriptBind_Script::ReloadScript(IFunctionHandler* pH)
{
	tukk sFileName;
	if (!pH->GetParams(sFileName))
		return pH->EndFunction();

	m_pSS->ExecuteFile(sFileName, true, gEnv->IsEditor()); // Force reload if in editor mode

	return pH->EndFunction();
}

i32 CScriptBind_Script::ReloadEntityScript(IFunctionHandler* pH, tukk className)
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	IEntityClass* pClass = pEntitySystem->GetClassRegistry()->FindClass(className);

	if (pClass)
	{
		pClass->LoadScript(false);
	}

	return pH->EndFunction();
}

/*!load a specified script
   @param sFileName path of the script that has to be loaded
 */
i32 CScriptBind_Script::LoadScript(IFunctionHandler* pH)
{
	bool bReload = false;
	bool bRaiseError = true;

	if (pH->GetParamCount() >= 3)
	{
		pH->GetParam(3, bRaiseError);
	}
	if (pH->GetParamCount() >= 2)
	{
		pH->GetParam(2, bReload);
	}

	tukk sScriptFile;
	pH->GetParam(1, sScriptFile);

	if (m_pSS->ExecuteFile(sScriptFile, bRaiseError, bReload))
		return pH->EndFunction(1);
	else
		return pH->EndFunction();
}

/*!unload script from the "loaded scripts map" so if this script is loaded again
   the Script system will reloadit. this function doesn't
   involve the LUA VM so the resources allocated by the script will not be released
   unloading the script
   @param sFileName path of the script that has to be loaded
 */
i32 CScriptBind_Script::UnloadScript(IFunctionHandler* pH)
{
	tukk sScriptFile;
	if (!pH->GetParams(sScriptFile))
		return pH->EndFunction();

	m_pSS->UnloadScript(sScriptFile);
	return pH->EndFunction();
}

/*!Dump all loaded scripts path calling IScriptSystemSink::OnLoadedScriptDump
   @see IScriptSystemSink
 */
i32 CScriptBind_Script::DumpLoadedScripts(IFunctionHandler* pH)
{
	m_pSS->DumpLoadedScripts();
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Script::SetTimer(IFunctionHandler* pH, i32 nMilliseconds, HSCRIPTFUNCTION hFunc)
{
	SmartScriptTable pUserData;
	bool bUpdateDuringPause = false;
	if (pH->GetParamCount() > 2)
	{
		pH->GetParam(3, pUserData);
	}
	if (pH->GetParamCount() > 3)
	{
		pH->GetParam(4, bUpdateDuringPause);
	}
	ScriptTimer timer;
	timer.bUpdateDuringPause = bUpdateDuringPause;
	timer.sFuncName[0] = 0;
	timer.pScriptFunction = hFunc;
	timer.pUserData = pUserData;
	timer.nMillis = nMilliseconds;

	i32 nTimerId = ((CScriptSystem*)m_pSS)->GetScriptTimerMgr()->AddTimer(timer);
	ScriptHandle timerHandle;
	timerHandle.n = nTimerId;

	return pH->EndFunction(timerHandle);
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Script::SetTimerForFunction(IFunctionHandler* pH, i32 nMilliseconds, tukk sFunctionName)
{
	SmartScriptTable pUserData;
	bool bUpdateDuringPause = false;
	if (pH->GetParamCount() > 2)
	{
		pH->GetParam(3, pUserData);
	}
	if (pH->GetParamCount() > 3)
	{
		pH->GetParam(4, bUpdateDuringPause);
	}
	ScriptTimer timer;
	timer.bUpdateDuringPause = bUpdateDuringPause;
	if (strlen(sFunctionName) > (sizeof(timer.sFuncName) - 1))
	{
		gEnv->pLog->LogError("SetTimerForFunction: Function name too long - %s", sFunctionName);
	}
	else
	{
		drx_strcpy(timer.sFuncName, sFunctionName);
	}
	timer.pScriptFunction = 0;
	timer.pUserData = pUserData;
	timer.nMillis = nMilliseconds;

	i32 nTimerId = ((CScriptSystem*)m_pSS)->GetScriptTimerMgr()->AddTimer(timer);
	ScriptHandle timerHandle;
	timerHandle.n = nTimerId;

	return pH->EndFunction(timerHandle);
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Script::KillTimer(IFunctionHandler* pH, ScriptHandle nTimerId)
{
	i32 nid = (i32)nTimerId.n;
	((CScriptSystem*)m_pSS)->GetScriptTimerMgr()->RemoveTimer(nid);
	return pH->EndFunction();
}
