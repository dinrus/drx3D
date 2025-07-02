// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Game rules module to handle scoring points values
	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _GAME_RULES_STANDARD_SCORING_H_
#define _GAME_RULES_STANDARD_SCORING_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesScoringModule.h>

class CGameRules;

class CGameRulesStandardScoring : public IGameRulesScoringModule
{
public:
	CGameRulesStandardScoring();
	virtual ~CGameRulesStandardScoring();

	// IGameRulesScoringModule
	virtual void	Init(XmlNodeRef xml);

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual TGameRulesScoreInt GetPlayerPointsByType(EGRST pointsType) const;
	virtual TGameRulesScoreInt GetPlayerXPByType(EGRST pointsType) const;
	virtual TGameRulesScoreInt GetTeamPointsByType(EGRST pointsType) const;
	virtual void	DoScoringForDeath(IActor *pActor, EntityId shooterId, i32 damage, i32 material, i32 hit_type);
	virtual void	OnPlayerScoringEvent(EntityId playerId, EGRST type);
	virtual void  OnPlayerScoringEventWithInfo(EntityId playerId, SGameRulesScoreInfo* scoreInfo);
	virtual void  OnPlayerScoringEventToAllTeamWithInfo(i32k teamId, SGameRulesScoreInfo* scoreInfo);
	virtual void	OnTeamScoringEvent(i32 teamId, EGRST pointsType);
	virtual void	SvResetTeamScore(i32 teamId);
	virtual i32		GetStartTeamScore() { return m_startTeamScore; }
	virtual i32		GetMaxTeamScore() { return m_maxTeamScore; }
	virtual void	SetAttackingTeamLost()			{ m_bAttackingTeamWonAllRounds = false; }
	virtual bool	AttackingTeamWonAllRounds() { return m_bAttackingTeamWonAllRounds; }
	virtual TGameRulesScoreInt GetDeathScoringModifier() { return m_deathScoringModifier; }
	virtual void SvSetDeathScoringModifier(TGameRulesScoreInt inModifier); 
	// ~IGameRulesScoringModule

	

	static tukk  s_gamerulesScoreType[];

protected:
	void	InitScoreData(XmlNodeRef xml, TGameRulesScoreInt *scoringData, TGameRulesScoreInt *xpData);
	TGameRulesScoreInt GetPointsByType(const TGameRulesScoreInt *scoringData, EGRST type) const;
	bool	ShouldScore(CGameRules* pGameRules) const;

	TGameRulesScoreInt m_playerScorePoints[EGRST_Num];
	TGameRulesScoreInt m_playerScoreXP[EGRST_Num];
	TGameRulesScoreInt m_teamScorePoints[EGRST_Num];

	i32 m_maxTeamScore;
	i32 m_startTeamScore;
	bool m_useScoreAsTime; // Calculate score using time.
	bool m_bAttackingTeamWonAllRounds;

	TGameRulesScoreInt m_deathScoringModifier;
};

#endif // _GAME_RULES_STANDARD_SCORING_H_
