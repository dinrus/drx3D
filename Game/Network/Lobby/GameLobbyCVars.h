// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMELOBBYCVARS_H__
#define __GAMELOBBYCVARS_H__

#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Game/Network/Lobby/GameBrowser.h>	// for USE_SESSION_SEARCH_SIMULATOR

class CGameLobbyCVars
{
	public:

		CGameLobbyCVars();
		~CGameLobbyCVars();

		static CGameLobbyCVars* Get()			{ return m_pThis; }

	public:

		float gl_initialTime;
		float gl_findGameTimeoutBase;
		float gl_findGameTimeoutPerPlayer;
		float gl_findGameTimeoutRandomRange;
		float gl_leaveGameTimeout;

		i32 gl_ignoreBadServers;
		i32 gl_allowLobbyMerging;
		i32 gl_allowEnsureBestHostCalls;

		float gl_timeTillEndOfGameForNoMatchMaking;
		float gl_timeBeforeStartOfGameForNoMatchMaking;
		float gl_skillChangeUpdateDelayTime;
		float gl_gameTimeRemainingRestrictLoad;
		float gl_startTimerMinTimeAfterPlayerJoined;

		i32 gl_startTimerMaxPlayerJoinResets;
		i32 gl_findGameNumJoinRetries;

		float gl_findGamePingMultiplier;
		float gl_findGamePlayerMultiplier;
		float gl_findGameLobbyMultiplier;
		float gl_findGameSkillMultiplier;
		float gl_findGameLanguageMultiplier;
		float gl_findGameRandomMultiplier;
		float gl_findGameStandardVariantMultiplier;
		float gl_findGamePingScale;
		float gl_findGameIdealPlayerCount;

#if GAMELOBBY_USE_COUNTRY_FILTERING
		float gl_findGameExpandSearchTime;
#endif

		float gl_hostMigrationEnsureBestHostDelayTime;
		float gl_hostMigrationEnsureBestHostOnStartCountdownDelayTime;
		float gl_hostMigrationEnsureBestHostOnReturnToLobbyDelayTime;
		float gl_hostMigrationEnsureBestHostGameStartMinimumTime;

		i32 gl_precachePaks;

		float gl_slotReservationTimeout;

#ifndef _RELEASE
		i32 gl_debug;
		i32 gl_voteDebug;
		i32 gl_voip_debug;
		i32 gl_skipPreSearch;
		i32 gl_dummyUserlist;
		i32 gl_dummyUserlistNumTeams;
		i32 gl_debugBadServersList;
		i32 gl_debugBadServersTestPerc;
		i32 gl_debugForceLobbyMigrations;
		i32 gl_debugLobbyRejoin;

		float gl_debugForceLobbyMigrationsTimer;
		float gl_debugLobbyRejoinTimer;
		float gl_debugLobbyRejoinRandomTimer;

		i32 gl_lobbyForceShowTeams;
		i32 gl_debugLobbyBreaksGeneral;
		i32 gl_debugLobbyHMAttempts;
		i32 gl_debugLobbyHMTerminations;
		i32 gl_debugLobbyBreaksHMShard;
		i32 gl_debugLobbyBreaksHMHints;
		i32 gl_debugLobbyBreaksHMTasks;
		i32 gl_resetWrongVersionProfiles;
		i32 gl_enableOfflinePlaylistVoting;
		i32 gl_hostMigrationUseAutoLobbyMigrateInPrivateGames;
	  i32 gl_allowDevLevels;
#endif

#ifdef USE_SESSION_SEARCH_SIMULATOR
		i32 gl_searchSimulatorEnabled;
#endif

		i32		gl_skip;
		i32		gl_experimentalPlaylistRotationAdvance;
		i32		gl_enablePlaylistVoting;
		i32		gl_minPlaylistSizeToEnableVoting;
		i32		gl_precacheLogging;

		float gl_votingCloseTimeBeforeStart;
		float gl_time;
		float gl_checkDLCBeforeStartTime;
		float gl_maxSessionNameTimeWithoutConnection;
		
		i32 gl_getServerFromDedicatedServerArbitrator;
		i32 gl_serverIsAllocatedFromDedicatedServerArbitrator;

#if defined(DEDICATED_SERVER)
		i32   g_pingLimit;
		float g_pingLimitTimer;
#endif


	private:

		static CGameLobbyCVars *m_pThis;
};

#endif // __GAMELOBBYCVARS_H__
