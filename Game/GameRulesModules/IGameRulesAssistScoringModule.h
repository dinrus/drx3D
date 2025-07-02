// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for the game rule module to handle scoring points
	-------------------------------------------------------------------------
	История:
	- 14:09:2009  : Created by James Bamford

*************************************************************************/

#ifndef _GAME_RULES_ASSIST_SCORING_MODULE_H_
#define _GAME_RULES_ASSIST_SCORING_MODULE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>

class IGameRulesAssistScoringModule
{
public:
	virtual ~IGameRulesAssistScoringModule() {}

	virtual void	Init(XmlNodeRef xml) = 0;

	virtual void	RegisterAssistTarget(EntityId targetId) = 0;
	virtual void	UnregisterAssistTarget(EntityId targetId) = 0;
	virtual void  OnEntityHit(const HitInfo &info, const float tagDuration=-1.f) = 0;
//virtual void  ClAwardAssistKillPoints(EntityId victimId) = 0;
//virtual void	SvDoScoringForDeath(IActor *pActor, EntityId shooterId, tukk weaponClassName, i32 damage, i32 material, i32 hit_type) = 0;
	virtual EntityId	SvGetMostRecentAttacker(EntityId targetId) = 0;
};

#endif // _GAME_RULES_ASSIST_SCORING_MODULE_H_
