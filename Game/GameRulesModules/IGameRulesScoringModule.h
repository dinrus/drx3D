// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for the game rule module to handle scoring points
	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _GAME_RULES_SCORING_MODULE_H_
#define _GAME_RULES_SCORING_MODULE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Game/GameRulesTypes.h>

class IGameRulesScoringModule
{
public:
	virtual ~IGameRulesScoringModule() {}

	virtual void	Init(XmlNodeRef xml) = 0;

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) = 0;

	virtual TGameRulesScoreInt GetPlayerPointsByType(EGRST pointsType) const = 0;
	virtual TGameRulesScoreInt GetPlayerXPByType(EGRST pointsType) const = 0;
	virtual TGameRulesScoreInt GetTeamPointsByType(EGRST pointsType) const = 0;
	virtual void	DoScoringForDeath(IActor *pActor, EntityId shooterId, i32 damage, i32 material, i32 hit_type) = 0;
	virtual void	OnPlayerScoringEvent(EntityId playerId, EGRST type) = 0;
	virtual void  OnPlayerScoringEventWithInfo(EntityId playerId, SGameRulesScoreInfo* scoreInfo) = 0;
	virtual void  OnPlayerScoringEventToAllTeamWithInfo(i32k teamId, SGameRulesScoreInfo* scoreInfo) = 0;
	virtual void	OnTeamScoringEvent(i32 teamId, EGRST pointsType) = 0;
	virtual void	SvResetTeamScore(i32 teamId) = 0;
	virtual i32		GetStartTeamScore() = 0;
	virtual i32		GetMaxTeamScore() = 0;
	virtual void  SetAttackingTeamLost() = 0;
	virtual bool	AttackingTeamWonAllRounds() = 0;
	virtual TGameRulesScoreInt GetDeathScoringModifier() = 0;
	virtual void SvSetDeathScoringModifier(TGameRulesScoreInt inModifier) = 0;
};

#endif // _GAME_RULES_SCORING_MODULE_H_
