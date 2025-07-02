// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Game rules module to handle player scores and stats
	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _GAME_RULES_PLAYER_STATS_H_
#define _GAME_RULES_PLAYER_STATS_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/IGameRulesRevivedListener.h>
#include <drx3D/Game/IGameRulesRoundsListener.h>

class CGameRules;

class CGameRulesStandardPlayerStats :	public IGameRulesPlayerStatsModule,
																			public IGameRulesRevivedListener,
																			public IGameRulesRoundsListener
{
public:
	CGameRulesStandardPlayerStats();
	virtual ~CGameRulesStandardPlayerStats();

	// IGameRulesPlayerStatsModule
	virtual void	Init(XmlNodeRef xml);
	virtual void	Reset(); // TODO : call
	virtual void	Update(float frameTime);

	virtual void	OnStartNewRound();

	virtual bool	NetSerialize( EntityId playerId, TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual void	CreatePlayerStats(EntityId playerId, u32 channelId);
	virtual void	RemovePlayerStats(EntityId playerId);
	virtual void	ClearAllPlayerStats();

	virtual void	OnPlayerKilled(const HitInfo &info);
	virtual void	IncreasePoints(EntityId playerId, i32 amount);
	virtual void	IncreaseGamemodePoints(EntityId playerId, i32 amount);
	virtual void	ProcessSuccessfulExplosion(EntityId playerId, float damageDealt = 0.0f, u16 projectileClassId = 0);
	virtual void	IncrementAssistKills(EntityId playerId);
#if ENABLE_PLAYER_KILL_RECORDING
	virtual void	IncreaseKillCount( const EntityId playerId, const EntityId victimId );
#endif // ENABLE_PLAYER_KILL_RECORDING
	
	virtual i32		GetNumPlayerStats() const;
	virtual const SGameRulesPlayerStat*  GetNthPlayerStats(i32 n);
	virtual const SGameRulesPlayerStat* GetPlayerStats(EntityId playerId);

	virtual void SetEndOfGameStats(const CGameRules::SPlayerEndGameStatsParams &inStats);

	virtual void CalculateSkillRanking();

	virtual void RecordSurvivalTime(EntityId playerId, float timeSurvived);
	// ~IGameRulesPlayerStatsModule
	
	// IGameRulesRevivedListener
	virtual void EntityRevived(EntityId entityId);
	// ~IGameRulesRevivedListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart() {}
	virtual void OnRoundEnd();
	virtual void OnSuddenDeath() {}
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState) {}
	virtual void OnRoundAboutToStart() {}
	// ~IGameRulesRoundsListener

protected:
	static const float GR_PLAYER_STATS_PING_UPDATE_INTERVAL;

	SGameRulesPlayerStat * GetPlayerStatsInternal(EntityId playerId);
	void SendHUDExplosionEvent();

	void IncrementWeaponHitsStat(tukk pWeaponName);

	//typedef std::map<EntityId, SGameRulesPlayerStat> TPlayerStats;
	typedef std::vector<SGameRulesPlayerStat> TPlayerStats;

	TPlayerStats m_playerStats;

	float m_lastUpdatedPings;
	i32 m_dbgWatchLvl;

  u16 m_classidC4;
  u16 m_classidFlash;
  u16 m_classidFrag;
	u16 m_classidLTAG;
	u16 m_classidJawRocket;

	bool m_bRecordTimeSurvived;
};

#endif // _GAME_RULES_PLAYER_STATS_H_
