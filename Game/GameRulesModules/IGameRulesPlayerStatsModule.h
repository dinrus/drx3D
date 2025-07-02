// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for the game rule module to handle player scores and stats
	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Ben Johnson

*************************************************************************/

#ifndef _GAME_RULES_PLAYER_STATS_MODULE_H_
#define _GAME_RULES_PLAYER_STATS_MODULE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Game/GameRules.h>

#define ENABLE_PLAYER_KILL_RECORDING 0

struct SGameRulesPlayerStat
{
	enum EFlags
	{
		PLYSTATFL_NULL											= (0),
		PLYSTATFL_HASSPAWNEDTHISROUND				= (1 << 0),
		PLYSTATFL_DIEDINEXTRATIMETHISROUND	= (1 << 1),
		PLYSTATFL_HASHADROUNDRESTART				= (1 << 2),
		PLYSTATFL_CANTSPAWNTHISROUND				= (1 << 3),
		PLYSTATFL_HASSPAWNEDTHISGAME				= (1 << 4),
		PLYSTATFL_USEINITIALSPAWNS					= (1 << 5),
	};

	SGameRulesPlayerStat(EntityId _playerId)
	{
		playerId = _playerId;
		Clear();
	}

	void Clear()
	{
		points = kills = assists = deaths = deathsThisRound = headshots = gamemodePoints = teamKills = ping = successfulFlashbangs = successfulFrags = successfulC4 = successfulLTAG = successfulJAW = flags = 0;
		damageDealt = timeSurvived = 0.0f;
		skillPoints = 0;
		bUseTimeSurvived = false;
	}

	void NetSerialize(TSerialize ser)
	{
		ser.Value("points", points, 'i32');
		ser.Value("kills", kills, 'u16');
		ser.Value("assists", assists, 'u16');
		ser.Value("deaths", deaths, 'u16');
		ser.Value("deathsThisRound", deathsThisRound, 'u16');
		ser.Value("teamKills", teamKills, 'u16');
		ser.Value("headshots", headshots, 'u16');
		ser.Value("gamemodePoints", gamemodePoints, 'i16');
		ser.Value("ping", ping, 'u16');
		ser.Value("frag", successfulFrags, 'u8');
    ser.Value("flash", successfulFlashbangs, 'u8');
    ser.Value("c4", successfulC4, 'u8');
		ser.Value("ltag", successfulLTAG, 'u8');
		ser.Value("jaw", successfulJAW, 'u8');
		ser.Value("damage", damageDealt, 'xdmg');
		ser.Value("flags", flags, 'u8');
#if !defined(_RELEASE) && !PROFILE_PERFORMANCE_NET_COMPATIBLE
		bool bBranch = bUseTimeSurvived;
		ser.Value("bBranch", bBranch, 'bool');
		if (bBranch != bUseTimeSurvived)
		{
			DrxFatalError("Invalid branch inside netserialise, please tell Colin (bBranch=%s)", bBranch ? "true" : "false");
		}
#endif
		// This branch is ok even inside a netserialise because the value of bUseTimeSurvived never changes for a given game
		if (bUseTimeSurvived)
		{
			ser.Value("timeSurvived", timeSurvived, 'ssfl');
		}
	}

#if ENABLE_PLAYER_KILL_RECORDING
	void IncrementTimesPlayerKilled( const EntityId otherPlayerId )
	{
		TKilledByCounter::iterator res = killedByCounts.find( otherPlayerId );
		if( res == killedByCounts.end() )
		{
			killedByCounts.insert(TKilledByCounter::value_type(otherPlayerId, 1));
			return;
		}

		res->second ++;
	}

	u8 GetTimesKilledPlayer( const EntityId otherPlayerId ) const
	{
		TKilledByCounter::const_iterator res = killedByCounts.find( otherPlayerId );
		if( res == killedByCounts.end() )
		{
			return 0;
		}

		return res->second;
	}

	// TKilledByCounter will be as big as the number of entities killed.
	//   The registration for a player is not cleaned when a player exits.
	//   This may be desired behaviour.
	DRX_TODO( 09, 03, 2010, "Replace TKilledByCounter (a std::map) with something faster.")
	typedef std::map<EntityId, u8> TKilledByCounter;
	TKilledByCounter killedByCounts;
#endif // ENABLE_PLAYER_KILL_RECORDING

	EntityId playerId;
	i32 points;
	u16 kills;
	u16 assists;
	u16 deaths;
	u16 deathsThisRound;
	u16 teamKills;
	u16 headshots;
	i16 gamemodePoints;
	u16 ping;
	u16 skillPoints;			// Not serialized with the other stats, this one is only sent when the game finishes
	u8 successfulFrags;
  u8 successfulFlashbangs;
  u8 successfulC4;
	u8 successfulLTAG;
	u8 successfulJAW;
	float damageDealt;
	float timeSurvived;			// Only used in hunter gamemode - time survived as a marine this round (0.f if started as a hunter)
	bool bUseTimeSurvived;
	u8 flags;  // see EFlags above
};

class IGameRulesPlayerStatsModule
{
public:
	virtual ~IGameRulesPlayerStatsModule() {}

	virtual void	Init(XmlNodeRef xml) = 0;
	virtual void	Reset() = 0;
	virtual void	Update(float frameTime) = 0;

	virtual void	OnStartNewRound() = 0;

	virtual bool	NetSerialize( EntityId playerId, TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) = 0;

	virtual void	CreatePlayerStats(EntityId playerId, u32 channelId) = 0;
	virtual void	RemovePlayerStats(EntityId playerId) = 0;

	virtual void	ClearAllPlayerStats() = 0;

	virtual void	OnPlayerKilled(const HitInfo &info) = 0;
	virtual void	IncreasePoints(EntityId playerId, i32 amount) = 0;
	virtual void	IncreaseGamemodePoints(EntityId playerId, i32 amount) = 0;
	virtual void	ProcessSuccessfulExplosion(EntityId playerId, float damageDealt = 0.0f, u16 projectileClassId = 0) = 0;
	virtual void	IncrementAssistKills(EntityId playerId) = 0;
#if ENABLE_PLAYER_KILL_RECORDING
	virtual void	IncreaseKillCount( const EntityId playerId, const EntityId victimId ) = 0;
#endif // ENABLE_PLAYER_KILL_RECORDING

	virtual i32		GetNumPlayerStats() const = 0;
	virtual const SGameRulesPlayerStat*  GetNthPlayerStats(i32 n) = 0;
	virtual const SGameRulesPlayerStat*  GetPlayerStats(EntityId playerId) = 0;

	virtual void SetEndOfGameStats(const CGameRules::SPlayerEndGameStatsParams &inStats) = 0;

	virtual void CalculateSkillRanking() = 0;
	virtual void RecordSurvivalTime(EntityId playerId, float timeSurvived) = 0;
};

#endif // _GAME_RULES_PLAYER_STATS_MODULE_H_
