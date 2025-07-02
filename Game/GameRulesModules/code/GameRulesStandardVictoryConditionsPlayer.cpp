// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Standard game rules module to handle victory conditions for all vs 
		all modes
	-------------------------------------------------------------------------
	История:
	- 19:10:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesStandardVictoryConditionsPlayer.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/IGameRulesStateModule.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/Audio/Announcer.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/PersistantStats.h>

#if DRX_WATCH_ENABLED
#define WATCH_SURVONE_LV1(...)  { if (1 <= g_pGameCVars->g_SurvivorOneVictoryConditions_watchLvl) DrxWatch(__VA_ARGS__); }
#define WATCH_SURVONE_LV2(...)  { if (2 <= g_pGameCVars->g_SurvivorOneVictoryConditions_watchLvl) DrxWatch(__VA_ARGS__); }
#else
#define WATCH_SURVONE_LV1(...)
#define WATCH_SURVONE_LV2(...)
#endif

#define GAMERULES_VICTORY_PLAYER_MSG_DELAY_TIMER_LENGTH	3.f

//-------------------------------------------------------------------------
CGameRulesStandardVictoryConditionsPlayer::CGameRulesStandardVictoryConditionsPlayer()
{
	m_bKillsAsScore = false;
}

//-------------------------------------------------------------------------
CGameRulesStandardVictoryConditionsPlayer::~CGameRulesStandardVictoryConditionsPlayer()
{
	if (gEnv->bServer)
	{
		CallLuaFunc(&m_luaFuncEndSuddenDeathSv);  // calling here on destruction for sanity's sake
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::Init( XmlNodeRef xml )
{
	inherited::Init(xml);

	m_luaFuncStartSuddenDeathSv.Format("%s", xml->getAttr("luaFuncStartSuddenDeathSv"));
	m_luaFuncEndSuddenDeathSv.Format("%s", xml->getAttr("luaFuncEndSuddenDeathSv"));

	m_tmpSuddenDeathMsg.Format("%s", xml->getAttr("tmpSuddenDeathMsg"));

	i32 tempKillsAsScore = 0;
	if (xml->getAttr("killsAsScore", tempKillsAsScore))
	{
		m_bKillsAsScore = (tempKillsAsScore!=0);
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::Update( float frameTime )
{
	inherited::Update(frameTime);

	DRX_ASSERT_MESSAGE(g_pGameCVars->g_timelimitextratime == 0.f, "We always expect timelimitextratime to be 0.f. This cvar is deprecated now");

	IGameRulesStateModule *pStateModule = m_pGameRules->GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() != IGameRulesStateModule::EGRS_InGame)
		return;

	IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule();
	if (pRoundsModule != NULL && !pRoundsModule->IsInProgress())
	{
		return;
	}

	if (AreAnyFlagsSet(eSVCF_CheckTime) && m_pGameRules->IsTimeLimited())
	{
		if (g_pGameCVars->g_timelimitextratime > 0.f)
		{
			float  timeRemaining = m_pGameRules->GetRemainingGameTimeNotZeroCapped();
			if (timeRemaining >= 0.f)
			{
				WATCH_SURVONE_LV1("TIME: %.2f", timeRemaining);
			}
			else
			{
				if ((frameTime > 0.f) && (-timeRemaining <= frameTime))
				{
					if (gEnv->bServer)
					{
						CCCPOINT(VictoryConditionsPlayer_SvStartSuddenDeath);
						CallLuaFunc(&m_luaFuncStartSuddenDeathSv);
					}
					if (gEnv->IsClient())
					{
						if (!m_tmpSuddenDeathMsg.empty())
						{
							tukk msg = m_tmpSuddenDeathMsg.c_str();

							SHUDEventWrapper::RoundMessageNotify(msg);
						}
					}
				}
				float  rem = MAX(0.f, ((g_pGameCVars->g_timelimitextratime * 60.f) + timeRemaining));  // remember, timeRemaining will be negative here
				WATCH_SURVONE_LV1("TIME: %.2f *** SUDDEN DEATH!! ***", rem);
			}
		}
		else
		{
			float  timeRemaining = m_pGameRules->GetRemainingGameTime();
			WATCH_SURVONE_LV1("TIME: %.2f", timeRemaining);
		}
		WATCH_SURVONE_LV1(" ");
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::ClVictoryPlayer( i32 playerId, EGameOverReason reason, const EntityId killedEntity, const EntityId shooterEntity )
{
	if (gEnv->IsClient())
	{
		OnEndGamePlayer(playerId, reason, killedEntity, shooterEntity);

		m_playerVictoryParams.playerId = playerId;
	}
}

//-------------------------------------------------------------------------
bool CGameRulesStandardVictoryConditionsPlayer::SvGameStillHasOpponentPlayers()
{
	DRX_ASSERT(gEnv->bServer);
	return (m_pGameRules->GetNumChannels() > 1);  // this should work fine for both dedicated and normal server (a dedicated server with one player connected reports 1 channel)
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::SvOpponentsCheckFailed()
{
	DRX_ASSERT(gEnv->bServer);
	CCCPOINT(VictoryConditionsPlayer_SvOpponentsCheckFailed);

	const EDisconnectionCause  lastDiscoCause = m_pGameRules->SvGetLastDiscoCause();

	EntityId  winnerEid;
	if (lastDiscoCause == eDC_SessionDeleted)
	{
		if (gEnv->IsClient())
		{
			winnerEid = gEnv->pGame->GetIGameFramework()->GetClientActorId();
		}
		else
		{
			DRX_ASSERT(m_pGameRules->GetNumChannels() == 1);
			i32k  winnerChan = m_pGameRules->GetChannelIds()->front();
			const IActor*  winnerAct = m_pGameRules->GetActorByChannelId(winnerChan);
			DRX_ASSERT(winnerAct);
			winnerEid = winnerAct->GetEntityId();
		}
	}
	else
	{
		DrxLog("CGameRulesStandardVictoryConditionsPlayer::SvOpponentsCheckFailed: treating the game as a draw because the last disconnection cause in the game was not eDC_SessionDeleted (it was %d)", lastDiscoCause);
		winnerEid = 0;  // if the final opponent quit via the ingame menus (SessionDeleted) then it's a victory, otherwise (here) treat it as a draw in case someone was trying to use a cable pull exploit
	}

	if (IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule())
	{
		pRoundsModule->SetTreatCurrentRoundAsFinalRound(true);
	}

	OnEndGamePlayer(winnerEid, EGOR_OpponentsDisconnected);
}


//-------------------------------------------------------------------------
bool CGameRulesStandardVictoryConditionsPlayer::ScoreLimitReached()
{
	i32 scoreLimit = (AreAnyFlagsSet(eSVCF_CheckScore) ? m_pGameRules->GetScoreLimit() : 0);
	if (scoreLimit)
	{
		SPlayerScoreResult result;
		GetMaxPlayerScore(result);

		if (result.m_maxScore >= scoreLimit)
		{
			return true;
		}
	}
	return false;
}


//-------------------------------------------------------------------------
// Stop this call falling through to team based victory conditions
void CGameRulesStandardVictoryConditionsPlayer::SvOnEndGame()
{
	SPlayerScoreResult result;
	GetMaxPlayerScore(result);

	if (result.m_maxScorePlayerId)
	{
		OnEndGamePlayer(result.m_maxScorePlayerId, EGOR_ObjectivesCompleted, 0, 0, true);
	}
	else
	{
		OnEndGamePlayer(0, EGOR_ObjectivesCompleted, 0, 0, true);
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::CheckScoreLimit()
{
	i32 scoreLimit = (AreAnyFlagsSet(eSVCF_CheckScore) ? m_pGameRules->GetScoreLimit() : 0);
	if (scoreLimit)
	{
		SPlayerScoreResult result;
		GetMaxPlayerScore(result);

		if (result.m_maxScore >= scoreLimit)
		{
			if (result.m_maxScorePlayerId)
			{
				OnEndGamePlayer(result.m_maxScorePlayerId, EGOR_ScoreLimitReached, m_winningVictim, m_winningAttacker);
			}
			else
			{
				OnEndGamePlayer(0, EGOR_ScoreLimitReached, m_winningVictim, m_winningAttacker);
			}
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::CheckTimeLimit()
{
	assert(gEnv->bServer);
	if (g_pGameCVars->g_timelimitextratime > 0.f)
	{
		if (AreAnyFlagsSet(eSVCF_CheckTime) && m_pGameRules->IsTimeLimited())
		{
			float  timeRemaining = m_pGameRules->GetRemainingGameTimeNotZeroCapped();
			if (timeRemaining <= -(g_pGameCVars->g_timelimitextratime * 60.f))
			{
				CCCPOINT(VictoryConditionsPlayer_SvSuddenDeathTimeExpired);
				TimeLimitExpired();
			}
		}
	}
	else
	{
		inherited::CheckTimeLimit();
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::TimeLimitExpired()
{
	SPlayerScoreResult result;
	GetMaxPlayerScore(result);

	if (result.m_maxScorePlayerId)
	{
		OnEndGamePlayer(result.m_maxScorePlayerId, EGOR_TimeLimitReached);
	}
	else
	{
		OnEndGamePlayer(0, EGOR_TimeLimitReached);
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::GetMaxPlayerScore(SPlayerScoreResult &result)
{
	IGameRulesPlayerStatsModule *pStatsModule = m_pGameRules->GetPlayerStatsModule();
	if (pStatsModule)
	{
		CGameRules::TPlayers players;
		m_pGameRules->GetPlayers(players);

		i32 numPlayers = players.size();
		for (i32 i = 0; i < numPlayers; ++ i)
		{
			EntityId playerId = players[i];

			const SGameRulesPlayerStat *pPlayerStats = pStatsModule->GetPlayerStats(playerId);
			if (pPlayerStats)
			{
				if (m_bKillsAsScore)
				{
					// Using Kills
					if (pPlayerStats->kills > result.m_maxScore)
					{
						result.m_maxScorePlayerId = playerId;
						result.m_maxScore = pPlayerStats->kills;
					}
					else if (pPlayerStats->kills == result.m_maxScore)
					{
						result.m_maxScorePlayerId = 0;
					}
				}
				else
				{
					// Using Points
					if (pPlayerStats->points > result.m_maxScore)
					{
						result.m_maxScorePlayerId = playerId;
						result.m_maxScore = pPlayerStats->points;
					}
					else if (pPlayerStats->points == result.m_maxScore)
					{
						result.m_maxScorePlayerId = 0;
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsPlayer::OnEndGamePlayer( EntityId playerId, EGameOverReason reason, EntityId killedEntity, EntityId shooterEntity, bool roundsGameActuallyEnded )
{
	EGameOverType gameOverType = EGOT_Unknown;

	if (gEnv->bServer)
	{
		IGameRulesRoundsModule *pRoundsModule = m_pGameRules->GetRoundsModule();
		if (pRoundsModule)
		{
			float  timeRemaining = m_pGameRules->GetRemainingGameTimeNotZeroCapped();
			if (timeRemaining < 0.f)
			{
				CCCPOINT(VictoryConditionsPlayer_SvEndSuddenDeath);
				CallLuaFunc(&m_luaFuncEndSuddenDeathSv);
			}

			pRoundsModule->OnEndGame(0, playerId, reason);

			// This function will be called twice when the last round in a game ends
			// the first call we want to early exit here so that the round result HUD
			// gets chance to display. The next call will show the actual game ended screen
			if (pRoundsModule->GetRoundsRemaining() != 0 || !roundsGameActuallyEnded)
			{
				// Game isn't actually over yet!
				return;
			}
			else
			{
				DRX_TODO(20, 05, 2010, "Need to figure out the true winner of the match, and not the last round");
			}
		}


    EntityId localClient = gEnv->pGame->GetIGameFramework()->GetClientActorId();
    if ( shooterEntity == localClient && localClient != 0)
    {
      g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_WinningKill);
    }
    if ( killedEntity == localClient && localClient != 0)
    {
      g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_VictimOnFinalKillcam);
    }

		IGameRulesStateModule *pStateModule = m_pGameRules->GetStateModule();
		if (pStateModule)
			pStateModule->OnGameEnd();


		IGameRulesPlayerStatsModule *pStatsModule = m_pGameRules->GetPlayerStatsModule();
		if (pStatsModule)
		{
			pStatsModule->CalculateSkillRanking();
		}

		m_playerVictoryParams = CGameRules::VictoryPlayerParams(playerId, killedEntity, shooterEntity, reason);
		m_pGameRules->GetGameObject()->InvokeRMI(CGameRules::ClVictoryPlayer(), m_playerVictoryParams, eRMI_ToRemoteClients);
		
		// have finished processing these winning stats now
		m_winningAttacker=0;
		m_winningVictim=0;
	}
	
	m_pGameRules->OnEndGame();

	if (gEnv->IsClient())
	{
		const EntityId clientId = g_pGame->GetIGameFramework()->GetClientActorId();
		const bool localPlayerWon = (playerId == clientId);
		tukk localizedText = "";

		if (playerId)
		{
			if (localPlayerWon)
			{
				CAudioSignalPlayer::JustPlay("MatchWon");
			}
			else
			{
				CAudioSignalPlayer::JustPlay("MatchLost");
			}
		}
		else
		{
			CAudioSignalPlayer::JustPlay("MatchDraw");
		}

		// the server has already increased these stats in the server block above
		if (!gEnv->bServer)
		{
	    EntityId localClient = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	    if ( shooterEntity == localClient && localClient != 0)
	    {
	      g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_WinningKill);
	    }
	    if ( killedEntity == localClient && localClient != 0)
	    {
	      g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_VictimOnFinalKillcam);
	    }
		}

		i32 clientScore = 0;
		i32 highestOpponentScore = 0;
		bool clientScoreIsTop = localPlayerWon; 

		IGameRulesPlayerStatsModule* pPlayStatsMo = m_pGameRules->GetPlayerStatsModule();
		if (pPlayStatsMo)
		{
			if (const SGameRulesPlayerStat* s = pPlayStatsMo->GetPlayerStats(clientId))
			{
				clientScore = s->points;
			}

			i32k numStats = pPlayStatsMo->GetNumPlayerStats();
			for (i32 i=0; i<numStats; i++)
			{
				const SGameRulesPlayerStat*  s = pPlayStatsMo->GetNthPlayerStats(i);
				if (s->flags & SGameRulesPlayerStat::PLYSTATFL_HASSPAWNEDTHISROUND)
				{
					if (s->points >= highestOpponentScore && s->playerId != clientId)
					{
						highestOpponentScore = s->points;
					}
				}
			}

			clientScoreIsTop = (clientScoreIsTop || (clientScore == highestOpponentScore));
		}

		if(playerId == 0)  // overall the game was a Draw (at the top of the scoreboard)...
		{
			gameOverType = (clientScoreIsTop ? EGOT_Draw : EGOT_Lose);  // ...but whether or not the local client considers the game a Draw or a Loss depends on whether they are one of the players who are drawn at the top
		}
		else
		{
			gameOverType = (localPlayerWon ? EGOT_Win : EGOT_Lose);
		}
		DRX_ASSERT(gameOverType != EGOT_Unknown);

		EAnnouncementID announcementId = INVALID_ANNOUNCEMENT_ID;

		GetGameOverAnnouncement(gameOverType, clientScore, highestOpponentScore, announcementId);

		if (!m_pGameRules->GetRoundsModule())
		{
			// rounds will play the one shot themselves
			CAudioSignalPlayer::JustPlay("RoundEndOneShot");
		}
		SHUDEventWrapper::OnGameEnd((i32) playerId, clientScoreIsTop, reason, NULL, ESVC_DrawResolution_invalid, announcementId);	
	}

	m_pGameRules->GameOver( gameOverType );
}

void CGameRulesStandardVictoryConditionsPlayer::CallLuaFunc(TFixedString* funcName)
{
	if (funcName && !funcName->empty())
	{
		IScriptTable*  pTable = m_pGameRules->GetEntity()->GetScriptTable();
		HSCRIPTFUNCTION  func;
		if (pTable != NULL && pTable->GetValue(funcName->c_str(), func))
		{
			IScriptSystem*  pScriptSystem = gEnv->pScriptSystem;
			Script::Call(pScriptSystem, func, pTable);
		}
	}
}

void CGameRulesStandardVictoryConditionsPlayer::SendVictoryMessage( i32 channelId )
{
	m_pGameRules->GetGameObject()->InvokeRMI(CGameRules::ClVictoryPlayer(), m_playerVictoryParams, eRMI_ToClientChannel, channelId);
}

void CGameRulesStandardVictoryConditionsPlayer::OnNewPlayerJoined( i32 channelId )
{
}


#undef WATCH_SURVONE_LV1
#undef WATCH_SURVONE_LV2
