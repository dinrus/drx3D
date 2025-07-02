// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	CVars for the FlowSystem

   -------------------------------------------------------------------------
   История:
   - 02:03:2006  12:00 : Created by AlexL

*************************************************************************/

#ifndef __FLOWSYSTEM_CVARS_H__
#define __FLOWSYSTEM_CVARS_H__

#pragma once

struct ICVar;

struct CFlowSystemCVars
{
	i32                             m_enableUpdates;    // Enable/Disable the whole FlowSystem
	i32                             m_profile;          // Profile the FlowSystem
	i32                             m_abortOnLoadError; // Abort execution when an error in FlowGraph loading is encountered
	i32                             m_inspectorLog;     // Log inspector to console
	i32                             m_noDebugText;      // Don't show debug text from HUD nodes (default 0)

	i32                             m_enableFlowgraphNodeDebugging;
	i32                             m_debugNextStep;

	// Disable HUD debug text
	i32                             fg_DisableHUDText;

	i32                             g_disableSequencePlayback;

	i32                             g_disableInputKeyFlowNodeInDevMode;


	static inline CFlowSystemCVars& Get()
	{
		assert(s_pThis != 0);
		return *s_pThis;
	}

private:
	friend class CFlowSystem; // Our only creator

	CFlowSystemCVars(); // singleton stuff
	~CFlowSystemCVars();
	CFlowSystemCVars(const CFlowSystemCVars&);
	CFlowSystemCVars& operator=(const CFlowSystemCVars&);

	static CFlowSystemCVars* s_pThis;
};

#endif // __FLOWSYSTEM_CVARS_H__
