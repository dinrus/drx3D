// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Standard game rules module to handle victory conditions for all vs 
		all games
	-------------------------------------------------------------------------
	История:
	- 19:10:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_STANDARD_VICTORY_CONDITIONS_PLAYER_H_
#define _GAME_RULES_STANDARD_VICTORY_CONDITIONS_PLAYER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesStandardVictoryConditionsTeam.h>

class CGameRules;

class CGameRulesStandardVictoryConditionsPlayer : public CGameRulesStandardVictoryConditionsTeam
{
public:
	typedef CGameRulesStandardVictoryConditionsTeam  inherited;

public:
	CGameRulesStandardVictoryConditionsPlayer();
	~CGameRulesStandardVictoryConditionsPlayer();

	virtual void  Init(XmlNodeRef xml);
	virtual void	Update(float frameTime);

	// IGameRulesVictoryConditionsModule
	virtual void ClVictoryTeam(i32 teamId, EGameOverReason reason, ESVC_DrawResolution drawWinningResolution, const SDrawResolutionData& level1, const SDrawResolutionData& level2, const EntityId killedEntity, const EntityId shooterEntity) {};
	virtual void ClVictoryPlayer(i32 playerId, EGameOverReason reason, const EntityId killedEntity, const EntityId shooterEntity);
	virtual void TeamCompletedAnObjective(i32 teamId) {};
	virtual bool ScoreLimitReached();
	virtual void SvOnEndGame();
	virtual void SendVictoryMessage(i32 channelId);
	virtual void OnNewPlayerJoined(i32 channelId);
	// ~IGameRulesVictoryConditionsModule

protected:
	struct SPlayerScoreResult
	{
		SPlayerScoreResult() : m_maxScore(0), m_maxScorePlayerId(0) { };

		i32 m_maxScore;
		EntityId m_maxScorePlayerId;
	};

	typedef DrxFixedStringT<32> TFixedString;

	void	GetMaxPlayerScore(SPlayerScoreResult &result);

	// CGameRulesStandardVictoryConditionsTeam
	virtual bool SvGameStillHasOpponentPlayers();
	virtual void SvOpponentsCheckFailed();
	virtual void CheckScoreLimit();
	virtual void CheckTimeLimit();
	virtual void TimeLimitExpired();
	// ~CGameRulesStandardVictoryConditionsTeam

	void	OnEndGamePlayer(EntityId playerId, EGameOverReason reason, EntityId killedEntity=0, EntityId shooterEntity=0, bool roundsGameActuallyEnded=false);

	void	CallLuaFunc(TFixedString* funcName);

	TFixedString  m_luaFuncStartSuddenDeathSv;
	TFixedString  m_luaFuncEndSuddenDeathSv;

	TFixedString  m_tmpSuddenDeathMsg;

	bool	m_bKillsAsScore;	// Use kills instead of score when checking victory conditions.

	CGameRules::VictoryPlayerParams m_playerVictoryParams;
};

#endif // _GAME_RULES_STANDARD_VICTORY_CONDITIONS_PLAYER_H_
