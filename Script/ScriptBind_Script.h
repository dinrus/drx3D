// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_Script.h
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptBind_Script_h__
#define __ScriptBind_Script_h__

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/Script/IScriptSystem.h>

/*!
 *	<description>This class implements script-functions for exposing the scripting system functionalities.</description>
 *	<remarks>After initialization of the script-object it will be globally accessable through scripts using the namespace "Script".</remarks>
 */
class CScriptBind_Script : public CScriptableBase
{
public:
	CScriptBind_Script(IScriptSystem* pScriptSystem, ISystem* pSystem);
	virtual ~CScriptBind_Script();
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>Script.LoadScript(scriptName)</code>
	//! <description>Loads the script.</description>
	//!		<param name="scriptName">name of the script to load</param>
	i32 LoadScript(IFunctionHandler* pH);

	//! <code>Script.ReloadScripts()</code>
	//! <description>Reloads all the scripts.</description>
	i32 ReloadScripts(IFunctionHandler* pH);

	//! <code>Script.ReloadScript()</code>
	//! <description>Reload the script.</description>
	i32 ReloadScript(IFunctionHandler* pH);

	//! <code>Script.ReloadEntityScript( className )</code>
	//!		<param name="className">Name of the entity script.</param>
	//! <description>Reloads the specified entity script.</description>
	i32 ReloadEntityScript(IFunctionHandler* pH, tukk className);

	//! <code>Script.UnloadScript()</code>
	//! <description>Unloads the script.</description>
	i32 UnloadScript(IFunctionHandler* pH);

	//! <code>Script.DumpLoadedScripts()</code>
	//! <description>Dumps all the loaded scripts.</description>
	i32 DumpLoadedScripts(IFunctionHandler* pH);

	//! <code>Script.SetTimer( nMilliseconds, luaFunction [, userData [, bUpdateDuringPause]] )</code>
	//! <description>
	//!    Set a general script timer, when timer expires will call back a specified lua function.
	//!    Lua function will accept 1 or 2 parameters,
	//!    if userData is specified luaFunction must be:
	//!      <pre>LuaCallback = function( userData,nTimerId )
	//!			-- function body
	//!      end;</pre>
	//!    if userData is not specified luaFunction must be:
	//!      <pre>LuaCallback = function( nTimerId )
	//!			-- function body
	//!      end;</pre>
	//! </description>
	//!		<param name="nMilliseconds">Delay of trigger in milliseconds.</param>
	//!		<param name="luaFunction">.</param>
	//!		<param name="userData">(optional) Any user defined table. If specified will be passed as a first argument of the callback function.</param>
	//!		<param name="bUpdateDuringPause">(optional) will be updated and trigger even if in pause mode.</param>
	//! <returns>ID assigned to this timer or nil if not specified.</returns>
	i32 SetTimer(IFunctionHandler* pH, i32 nMilliseconds, HSCRIPTFUNCTION hFunc);

	//! <code>Script.SetTimerForFunction( nMilliseconds, luaFunction [, userData [, bUpdateDuringPause]] )</code>
	//! <description>
	//!    Set a general script timer, when timer expires will call back a specified lua function.
	//!    Lua function will accept 1 or 2 parameters,
	//!    if userData is specified luaFunction must be:
	//!      <pre>LuaCallback = function( userData,nTimerId )
	//!			-- function body
	//!      end;</pre>
	//!    if userData is not specified luaFunction must be:
	//!      <pre>LuaCallback = function( nTimerId )
	//!			-- function body
	//!      end;</pre>
	//!.</description>
	//!		<param name="nMilliseconds">Delay of trigger in milliseconds.</param>
	//!		<param name="luaFunction">.</param>
	//!		<param name="userData"> (optional) Any user defined table. If specified it will be passed as a first argument of the callback function.</param>
	//!		<param name="bUpdateDuringPause"> (optional) will be updated and trigger even if in pause mode.</param>
	//! <returns>ID assigned to this timer or nil if not specified.</returns>
	i32 SetTimerForFunction(IFunctionHandler* pH, i32 nMilliseconds, tukk sFunctionName);

	//! <code>Script.KillTimer( nTimerId )</code>
	//! <description>Stops a timer set by the Script.SetTimer function.</description>
	//!		<param name="nTimerId">ID of the timer returned by the Script.SetTimer function.</param>
	i32 KillTimer(IFunctionHandler* pH, ScriptHandle nTimerId);
};

#endif // __ScriptBind_Script_h__
