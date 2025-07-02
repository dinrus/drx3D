// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
	
	-------------------------------------------------------------------------
	История:
	- 19/05/2010  : Created by Tom Houghton

*************************************************************************/

#ifndef _GAME_RULES_ACTOR_ACTION_LISTENER_H_
#define _GAME_RULES_ACTOR_ACTION_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>

class IGameRulesActorActionListener
{
public:
	virtual ~IGameRulesActorActionListener() {}

	virtual void OnAction(const ActionId& actionId, i32 activationMode, float value) = 0;
};


#endif // _GAME_RULES_ACTOR_ACTION_LISTENER_H_