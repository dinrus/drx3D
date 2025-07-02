// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 

-------------------------------------------------------------------------
История:
- 27:07:2010   Extracted from BodyDamage.h/.cpp by Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef BODY_DAMAGE_MANAGER_CVARS_H
#define BODY_DAMAGE_MANAGER_CVARS_H

class CBodyUprCVars
{
	friend class CBodyDamageProfile;
public:
	static void RegisterCommands();
	static void RegisterVariables();
	static void UnregisterCommands(IConsole* pConsole);
	static void UnregisterVariables(IConsole* pConsole);

	static i32 IsBodyDamageLogEnabled() { return g_bodyDamage_log; }
	static i32 IsBodyDestructionDebugEnabled() { return g_bodyDestruction_debug; }
	static bool IsBodyDestructionDebugFilterFor(tukk entityName)
	{
		DRX_ASSERT(entityName);
		DRX_ASSERT(g_bodyDestruction_debugFilter);

		tukk filter = g_bodyDestruction_debugFilter->GetString();
		
		return (strcmp(filter, entityName) == 0);
	}

	static bool IsBodyDestructionDebugFilterEnabled()
	{
		tukk filter = g_bodyDestruction_debugFilter->GetString();

		return ((strlen(filter) > 0) && (strcmp(filter, "0") != 0));
	}

private:
	static void Reload(IActor* pIActor);
	static void Reload(IEntity* pEntity);
	static void ReloadBodyDamage(IConsoleCmdArgs* pArgs);
	static void ReloadBodyDestruction(IConsoleCmdArgs* pArgs);

	static i32 g_bodyDamage_log;
	static i32 g_bodyDestruction_debug;

	static ICVar* g_bodyDestruction_debugFilter;
};

#endif