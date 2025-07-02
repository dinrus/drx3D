// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#define SURVIVOR_ONE_ENABLED 0

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Game rules module to handle victory upon being the last player left
	-------------------------------------------------------------------------
	История:
	- 06:11:2009  : Created by Thomas Houghton

*************************************************************************/
#if SURVIVOR_ONE_ENABLED

#ifndef _GAME_RULES_SURVIVOR_ONE_CONDITIONS_PLAYER_H_
#define _GAME_RULES_SURVIVOR_ONE_CONDITIONS_PLAYER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesStandardVictoryConditionsPlayer.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>

class CGameRules;

class CGameRulesSurvivorOneVictoryConditions :	
									public CGameRulesStandardVictoryConditionsPlayer,
									public IGameRulesRoundsListener
{
private:
	typedef CGameRulesStandardVictoryConditionsPlayer  inherited;

protected:
	i32  m_svLatestSurvCount;
	EntityId  m_svLatestSurvList[MAX_PLAYER_LIMIT];

	typedef struct SRadarPing
	{
		float timeLimit;
		float pingTime; // pingTime > 0.f triggers pings with that frequency

		SRadarPing()
		{
			timeLimit=0.f;
			pingTime=0.f;
		}
	};

	typedef DrxFixedArray<SRadarPing, 3> TRadarPingArray;
	TRadarPingArray m_radarPingArray;
	u32 m_radarPingStage;
	float m_radarPingStageTimer;
	float m_radarPingTimer;
	i32 m_scoreIncreasePerElimination;

public:

public:
	CGameRulesSurvivorOneVictoryConditions();
	~CGameRulesSurvivorOneVictoryConditions();

	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	// IGameRulesVictoryConditionsModule
	virtual void	ClVictoryTeam(i32 teamId, EGameOverReason reason, ESVC_DrawResolution winningResolution, const SDrawResolutionData& level1, const SDrawResolutionData& level2, const EntityId killedEntity, const EntityId shooterEntity) {};
	virtual void	ClVictoryPlayer(i32 playerId, EGameOverReason reason, const EntityId killedEntity, const EntityId shooterEntity);
	// ~IGameRulesVictoryConditionsModule

	// IGameRulesSurvivorCountListener
	virtual void SvSurvivorCountRefresh(i32 count, const EntityId survivors[], i32 numKills);
	// ~IGameRulesSurvivorCountListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
	virtual void OnSuddenDeath();
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {}
	virtual void OnRoundAboutToStart() {}
	// ~IGameRulesRoundsListener

protected:
	void UpdateRadarPings(float frameTime);
	virtual void OnEndGamePlayer(EntityId playerId, EGameOverReason reason);

	// CGameRulesStandardVictoryConditionsPlayer
	virtual void TimeLimitExpired();
	// ~CGameRulesStandardVictoryConditionsPlayer

};

#endif // _GAME_RULES_SURVIVOR_ONE_CONDITIONS_PLAYER_H_

#endif // SURVIVOR_ONE_ENABLED