// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
	
	-------------------------------------------------------------------------
	История:
	- 09:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_ACTOR_ACTION_MODULE_H_
#define _GAME_RULES_ACTOR_ACTION_MODULE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>

struct IActor;

class IGameRulesActorActionModule
{
public:
	virtual ~IGameRulesActorActionModule() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void PostInit() = 0;

	virtual void OnActorAction(IActor *pActor, const ActionId& actionId, i32 activationMode, float value) = 0;
};

#endif // _GAME_RULES_ACTOR_ACTION_MODULE_H_