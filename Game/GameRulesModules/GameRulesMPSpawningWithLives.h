// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 

-------------------------------------------------------------------------
История:
- 04:11:2009 : Created by Thomas Houghton

*************************************************************************/

#ifndef __GAMERULES_MP_SPAWNING_WITH_LIVES_H__
#define __GAMERULES_MP_SPAWNING_WITH_LIVES_H__

#include <drx3D/Game/GameRulesMPSimpleSpawning.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>

class CGameRulesMPSpawningWithLives :	public CGameRulesRSSpawning,
																			public IGameRulesPlayerStatsListener
{
private:

	typedef CGameRulesRSSpawning  inherited;

protected:

	typedef DrxFixedStringT<32> TFixedString;

	struct SElimMarker
	{
		EntityId  markerEid;
		EntityId  playerEid;
		float  time;
	};

	static i32k  m_kMaxElimMarkers = MAX_PLAYER_LIMIT;

protected:

	SElimMarker  m_elimMarkers[m_kMaxElimMarkers];

	TFixedString  m_tmpPlayerEliminatedMsg;
	TFixedString  m_tmpPlayerEliminatedMarkerTxt;

	SElimMarker*  m_freeElimMarker;

	i32  m_numLives;
	i32  m_dbgWatchLvl;
	float  m_elimMarkerDuration;
	bool m_bLivesDirty;

public:

	// (public data)

public:

	CGameRulesMPSpawningWithLives();
	virtual ~CGameRulesMPSpawningWithLives();

	// IGameRulesSpawningModule
	virtual void ReviveAllPlayers(bool isReset, bool bOnlyIfDead);
	virtual i32  GetRemainingLives(EntityId playerId);
	virtual i32  GetNumLives()  { return m_numLives; }
	// ~IGameRulesSpawningModule

	// IGameRulesKillListener - already inherited from in CGameRulesMPSpawningBase
	virtual void OnEntityKilled(const HitInfo &hitInfo);
	// ~IGameRulesKillListener

	// IGameRulesRevivedListener
	virtual void EntityRevived(EntityId entityId);
	// ~IGameRulesRevivedListener

	// IGameRulesPlayerStatsListener
	virtual void ClPlayerStatsNetSerializeReadDeath(const SGameRulesPlayerStat* s, u16 prevDeathsThisRound, u8 prevFlags);
	// ~IGameRulesPlayerStatsListener

	// IGameRulesRoundsListener
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState);
	// ~IGameRulesRoundsListener

protected:

	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void PerformRevive(EntityId playerId, i32 teamId, EntityId preferredSpawnId);
	virtual void PlayerJoined(EntityId playerId);
	virtual void PlayerLeft(EntityId playerId);

	void OnPlayerElimination(EntityId playerId);
	void SvNotifySurvivorCount();

	void CreateElimMarker(EntityId playerId);
	void UpdateElimMarkers(float frameTime);
	void RemoveAllElimMarkers();

	bool IsInSuddenDeath();
};


#endif // __GAMERULES_MP_SPAWNING_WITH_LIVES_H__
