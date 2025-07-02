// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 17:11:2006   15:38 : Created by Stas Spivakov

*************************************************************************/
#ifndef __GAMESTATS_H__
#define __GAMESTATS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IGameplayRecorder.h"
#include "IActorSystem.h"
#include <drx3D/Act/ILevelSystem.h>

struct IStatsTrack;
struct IServerReport;

class CGameStats :
	public IGameplayListener,
	public ILevelSystemListener,
	public IGameFrameworkListener,
	public IHostMigrationEventListener
{
private:

	struct  SPlayerStats;
	struct  SRoundStats;

	struct  SPlayerInfo
	{
		//these are used to report score/status to server list
		string                name;
		i32                   team;
		i32                   rank;
		bool                  spectator;
		std::map<string, i32> scores;
		//these are 'real' statistics
		i32                   id;

		typedef std::vector<_smart_ptr<SPlayerStats>> TStatsVct;
		TStatsVct stats;

		void      GetMemoryUsage(IDrxSizer* pSizer) const;

	};
	typedef std::map<i32, SPlayerInfo> PlayerStatsMap;

	struct STeamStats
	{
		i32    id;
		string name;
		i32    score;
	};
	typedef std::map<i32, STeamStats> TeamStatsMap;

	typedef std::map<string, i32>     TStatsKeyMap;

	struct Listener;
	struct SStatsTrackHelper;

public:
	CGameStats(CDrxAction* pGameFramework);
	virtual ~CGameStats();

	i32 ILINE GetChannelId(IEntity* pEntity)
	{
		IActor* pActor = m_pActorSystem->GetActor(pEntity->GetId());
		if (pActor)
			return pActor->GetChannelId();
		return 0;
	}

	//IGamePlayListener
	virtual void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event);
	//

	//ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName);
	virtual void OnLoadingStart(ILevelInfo* pLevel);
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel);
	virtual void OnLoadingComplete(ILevelInfo* pLevel);
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error);
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount);
	virtual void OnUnloadComplete(ILevelInfo* pLevel);
	//

	//IGameFrameworkListener
	virtual void OnActionEvent(const SActionEvent& event);
	virtual void OnPostUpdate(float fDeltaTime)    {}
	virtual void OnSaveGame(ISaveGame* pSaveGame)  {}
	virtual void OnLoadGame(ILoadGame* pLoadGame)  {}
	virtual void OnLevelEnd(tukk nextLevel) {}
	//

	// IHostMigrationEventListener
	virtual EHostMigrationReturn OnInitiate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnDisconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnDemoteToClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnPromoteToServer(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnReconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnFinalise(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual void                 OnComplete(SHostMigrationInfo& hostMigrationInfo) {}
	virtual EHostMigrationReturn OnTerminate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnReset(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	// ~IHostMigrationEventListener

	void StartSession();
	void EndSession();
	void OnMapSwitch();
	void StartGame(bool server);
	void EndGame(bool server);
	void SuddenDeath(bool server);
	void EndRound(bool server, i32 winner);
	void NewPlayer(i32 plr, i32 team, bool spectator, bool restored);
	void RemovePlayer(i32 plr, bool keep);
	void CreateNewLifeStats(i32 plr);
	void ResetScore(i32 plr);
	void StartLevel();
	void OnKill(i32 plr, EntityId* extra);
	void OnDeath(i32 plr, i32 shooterId);

	void GameReset();
	void Connected();

	void SetName(i32 plr, tukk name);
	void SetScore(i32 plr, tukk score, i32 value);
	void SetTeam(i32 plr, i32 value);
	void SetRank(i32 plr, i32 value);
	void SetSpectator(i32 plr, i32 value);
	void Update();

	void Dump();
	void SaveStats();
	void ResetStats();

	void GetMemoryStatistics(IDrxSizer* s);

	void PromoteToServer();

private:
	void Init();
	//set session-wide parameters
	void ReportSession();
	//set game(map)-wide parameters
	void ReportGame();
	//set dynamic parameters
	void Report();
	//submit stats for player to StatsTrack service
	void SubmitPlayerStats(const SPlayerInfo& plr, bool server, bool client);
	//submit stats for server
	void SubmitServerStats();

	//player stats
	void ProcessPlayerStat(IEntity* pEntity, const GameplayEvent& event);

	bool GetLevelName(string& mapname);

	CDrxAction*    m_pGameFramework;
	IActorSystem*  m_pActorSystem;

	bool           m_playing;
	bool           m_stateChanged;
	bool           m_startReportNeeded;
	bool           m_reportStarted;

	PlayerStatsMap m_playerMap;
	TeamStatsMap   m_teamMap;

	IStatsTrack*   m_statsTrack;
	IServerReport* m_serverReport;
	Listener*      m_pListener;
	CTimeValue     m_lastUpdate;
	CTimeValue     m_lastReport; //sync data to network engine

	//some data about game started
	string                       m_gameMode;
	string                       m_mapName;

	CGameStatsConfig*            m_config;

	CTimeValue                   m_lastPosUpdate;
	CTimeValue                   m_roundStart;

	std::unique_ptr<SRoundStats> m_roundStats;
};

#endif /*__GAMESTATS_H__*/
