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

#ifndef _GAME_RULES_MP_ACTOR_ACTION_H_
#define _GAME_RULES_MP_ACTOR_ACTION_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesActorActionModule.h>

class CGameRules;

class CGameRulesMPActorAction : public IGameRulesActorActionModule
{
public:
	CGameRulesMPActorAction();
	virtual ~CGameRulesMPActorAction();

	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();

	virtual void OnActorAction(IActor *pActor, const ActionId& actionId, i32 activationMode, float value);
};

#endif // _GAME_RULES_MP_ACTOR_ACTION_H_
