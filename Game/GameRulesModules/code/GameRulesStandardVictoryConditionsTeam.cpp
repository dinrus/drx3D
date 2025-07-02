// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Standard game rules module to handle victory conditions
	-------------------------------------------------------------------------
	История:
	- 08:09:2009  : Created by Ben Johnson

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesStandardVictoryConditionsTeam.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/IGameRulesStateModule.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/Audio/Announcer.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesScoringModule.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/CoreX/TypeInfo_impl.h>

#define OTHER_TEAM(x) 3 - x
#define GAMERULES_VICTORY_TEAM_MSG_DELAY_TIMER_LENGTH	3.f

static AUTOENUM_BUILDNAMEARRAY(s_ESVC_DrawResolutionListStrs, ESVC_DrawResolutionList);
static AUTOENUM_BUILDNAMEARRAY(s_ESVC_SoundsListStrs, ESVC_SoundsList);

//-------------------------------------------------------------------------
CGameRulesStandardVictoryConditionsTeam::CGameRulesStandardVictoryConditionsTeam()
{
	m_pGameRules = NULL;
	m_currentSound = eSVC_StartGame;

	m_VictoryConditionFlags = 0; // will be set during Init() from XML

	m_currentSoundTime[eSVC_StartGame] = 599.5f;
	m_currentSoundTime[eSVC_3secsIn] = 597.0f;
	m_currentSoundTime[eSVC_Halfway] = 181.0f;
	m_currentSoundTime[eSVC_1min] = 61.0f;
	m_currentSoundTime[eSVC_30secs] = 31.0f;
	m_currentSoundTime[eSVC_15secs] = 16.0f;
	m_currentSoundTime[eSVC_5secs] = 6.0f;
	m_currentSoundTime[eSVC_3secs] = 4.0f;
	m_currentSoundTime[eSVC_end] = 0.25f;
	m_fNextScoreLimitCheck = 0.0f;
	m_numUpdates=0;
  m_winningVictim = 0;
  m_winningAttacker = 0;
}

//-------------------------------------------------------------------------
CGameRulesStandardVictoryConditionsTeam::~CGameRulesStandardVictoryConditionsTeam()
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (pGameRules)
	{
		if (gEnv->bServer && AreAnyFlagsSet(eSVCF_WinOnPrimaryTeamDead | eSVCF_WinOnSecondaryTeamDead) )
		{
			pGameRules->UnRegisterSurvivorCountListener(this);
		}
		pGameRules->UnRegisterTeamChangedListener(this);
	}
}

ESVC_DrawResolution CGameRulesStandardVictoryConditionsTeam::GetDrawResolutionFromName(tukk inName)
{
	ESVC_DrawResolution result=ESVC_DrawResolution_invalid;

	for (i32 i=0; i<ESVC_DrawResolution_num; i++)
	{
		if (!stricmp(inName, s_ESVC_DrawResolutionListStrs[i]))
		{
			result=static_cast<ESVC_DrawResolution>(i);
			break;
		}
	}

	return result;
}

void CGameRulesStandardVictoryConditionsTeam::InitDrawResolutions(XmlNodeRef xml)
{
	i32 numChildren = xml->getChildCount();
	for (i32 i = 0; i < numChildren; ++ i)
	{
		XmlNodeRef xmlChild = xml->getChild(i);

		if (!stricmp(xmlChild->getTag(), "level"))
		{
			tukk drawResolutionLevelName=xmlChild->getAttr("name");

			ESVC_DrawResolution resolution=GetDrawResolutionFromName(drawResolutionLevelName);
			if (resolution != ESVC_DrawResolution_invalid)
			{
				SDrawResolutionData &resolutionData = m_drawResolutionData[resolution];

				if (!resolutionData.m_active)
				{
					tukk pString = 0;
					if (xmlChild->getAttr("winMessage", &pString))
					{
						resolutionData.m_winningMessage.Format("@%s", pString);
					}

					if (xmlChild->getAttr("type", &pString))
					{
						if (!stricmp(pString, "i32"))
						{
							resolutionData.m_dataType = SDrawResolutionData::eDataType_int;
						}
						else if (!stricmp(pString, "float"))
						{
							resolutionData.m_dataType = SDrawResolutionData::eDataType_float;
						}
					}

					if (xmlChild->getAttr("test", &pString))
					{
						if (!stricmp(pString, "greater"))
						{
							resolutionData.m_dataTest = SDrawResolutionData::eWinningData_greater_than;
						}
						else if (!stricmp(pString, "lessthan"))
						{
							resolutionData.m_dataTest = SDrawResolutionData::eWinningData_less_than;
						}
					}

					resolutionData.m_active=true;
				}
				else
				{
					DrxLogAlways("CGameRulesStandardVictoryConditionsTeam::InitDrawResolutions() trying to setup a draw resolution level that's already been setup");
				}
			}
			else
			{
				DrxLogAlways("CGameRulesStandardVictoryCondtionsTeam::InitDrawResolutions() failed to find a draw resolution from name=%s", drawResolutionLevelName);
			}
		}
		else
		{
			DrxLogAlways("CGameRulesStandardVictoryConditionsTeam::InitDrawResolutions() unexpected xml child found %s", xmlChild->getTag());
		}
	}
}

static ILINE void SetFlagFromXML(XmlNodeRef xml, tukk  pAttrib, TVictoryFlags flagToSet, bool bDefaultValue, TVictoryFlags& flags)
{
	bool temp;
	if (!xml->getAttr(pAttrib, temp))
		temp = bDefaultValue;
	if(temp)
		flags |= flagToSet;
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::Init( XmlNodeRef xml )
{
	m_pGameRules = g_pGame->GetGameRules();

	TVictoryFlags tempFlags = eSVCF_None;

	SetFlagFromXML(xml, "checkScore", eSVCF_CheckScore, true, tempFlags);

	SetFlagFromXML(xml, "checkTime", eSVCF_CheckTime, true, tempFlags);
	
	SetFlagFromXML(xml, "checkScoreAsTime", eSVCF_CheckScoreAsTime, false, tempFlags);

	SetFlagFromXML(xml, "winOnPrimaryTeamDead", eSVCF_WinOnPrimaryTeamDead, false, tempFlags);

	SetFlagFromXML(xml, "winOnSecondaryTeamDead", eSVCF_WinOnSecondaryTeamDead, false, tempFlags);

	SetFlagFromXML(xml, "decideRoundWinnerUsingJustPointsScoredInRound", eSVCF_DecideRoundWinnerUsingRoundPoints, false, tempFlags);

	SetFlagFromXML(xml, "treatScoreRoundVictoryAsGameVictory", eSVCF_TreatScoreRoundVictoryAsGameVictory, false, tempFlags);

	SetFlagFromXML(xml, "decideGameWinnerWithTeamScore", eSVCF_DecideGameWinnerWithTeamScore, false, tempFlags);

	SetFlagFromXML(xml, "playHalfwaySoundAfterUpdatingTimeLimit", eSVCF_PlayHalfwaySoundAfterUpdatingTimeLimit, true, tempFlags);

	SetFlagFromXML(xml, "playHalfwaySound", eSVCF_PlayHalfwaySound, true, tempFlags);

	SetFlagFromXML(xml, "playerSwitchTeams", eSVCF_PlayerSwitchTeams, false, tempFlags);

	SetFlagFromXML(xml, "playTimeAnnouncements", eSVCF_PlayTimeAnnouncements, true, tempFlags);
	
	m_VictoryConditionFlags = tempFlags;


	if (gEnv->bServer && AreAnyFlagsSet( eSVCF_WinOnPrimaryTeamDead | eSVCF_WinOnSecondaryTeamDead ) )
	{
		m_pGameRules->RegisterSurvivorCountListener(this);
	}

	// ensure the array of draw resolutions is always allocated, even if its unused for this gamemode
	for (i32 i=ESVC_DrawResolution_level_1; i<ESVC_DrawResolution_num; i++)
	{
		SDrawResolutionData emptyData;
		m_drawResolutionData.push_back(emptyData);
	}

	i32 numChildren = xml->getChildCount();
	for (i32 i = 0; i < numChildren; ++ i)
	{
		XmlNodeRef xmlChild = xml->getChild(i);

		if (!stricmp(xmlChild->getTag(), "DrawResolution"))
		{
			InitDrawResolutions(xmlChild);
		}
	}
}

void CGameRulesStandardVictoryConditionsTeam::SetFloatsInDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data)
{
	if (inResolutionLevel >= ESVC_DrawResolution_level_1 && inResolutionLevel < ESVC_DrawResolution_num)
	{
		SDrawResolutionData &resolutionData = m_drawResolutionData[inResolutionLevel];

		DRX_ASSERT_MESSAGE(resolutionData.m_active, "SetFloatsInDrawResolutionData() trying to add data to an inactive resolution level");
		if (resolutionData.m_active)
		{
			DRX_ASSERT_MESSAGE(resolutionData.m_dataType == SDrawResolutionData::eDataType_float, "SetFloatsInDrawResolutionData() trying to add floats to a level that's not configured for floats");
			if (resolutionData.m_dataType == SDrawResolutionData::eDataType_float)
			{
				resolutionData.m_floatDataForTeams[0] = inTeam1Data;
				resolutionData.m_floatDataForTeams[1] = inTeam2Data;
			}
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, "SetFloatsInDrawResolutionData() unhandled inResolutionLevel");
	}
}

void CGameRulesStandardVictoryConditionsTeam::MaxFloatsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data)
{
	if (inResolutionLevel >= ESVC_DrawResolution_level_1 && inResolutionLevel < ESVC_DrawResolution_num)
	{
		SDrawResolutionData &resolutionData = m_drawResolutionData[inResolutionLevel];

		DRX_ASSERT_MESSAGE(resolutionData.m_active, "MaxFloatsToDrawResolutionData() trying to add data to an inactive resolution level");
		if (resolutionData.m_active)
		{
			DRX_ASSERT_MESSAGE(resolutionData.m_dataType == SDrawResolutionData::eDataType_float, "MaxFloatsToDrawResolutionData() trying to add floats to a level that's not configured for floats");
			if (resolutionData.m_dataType == SDrawResolutionData::eDataType_float)
			{
				resolutionData.m_floatDataForTeams[0] = max(inTeam1Data, resolutionData.m_floatDataForTeams[0]);
				resolutionData.m_floatDataForTeams[1] = max(inTeam2Data, resolutionData.m_floatDataForTeams[1]);
			}
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, "MaxFloatsToDrawResolutionData() unhandled inResolutionLevel");
	}
}

void CGameRulesStandardVictoryConditionsTeam::AddFloatsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, const float inTeam1Data, const float inTeam2Data)
{
	if (inResolutionLevel >= ESVC_DrawResolution_level_1 && inResolutionLevel < ESVC_DrawResolution_num)
	{
		SDrawResolutionData &resolutionData = m_drawResolutionData[inResolutionLevel];

		DRX_ASSERT_MESSAGE(resolutionData.m_active, "AddFloatsToDrawResolutionData() trying to add data to an inactive resolution level");
		if (resolutionData.m_active)
		{
			DRX_ASSERT_MESSAGE(resolutionData.m_dataType == SDrawResolutionData::eDataType_float, "AddFloatsToDrawResolutionData() trying to add floats to a level that's not configured for floats");
			if (resolutionData.m_dataType == SDrawResolutionData::eDataType_float)
			{
				resolutionData.m_floatDataForTeams[0] += inTeam1Data;
				resolutionData.m_floatDataForTeams[1] += inTeam2Data;
			}
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, "AddToDrawResolutionData() unhandled inResolutionLevel");
	}
}

void CGameRulesStandardVictoryConditionsTeam::AddIntsToDrawResolutionData(ESVC_DrawResolution inResolutionLevel, i32k inTeam1Data, i32k inTeam2Data)
{
	if (inResolutionLevel >= ESVC_DrawResolution_level_1 && inResolutionLevel < ESVC_DrawResolution_num)
	{
		SDrawResolutionData &resolutionData = m_drawResolutionData[inResolutionLevel];

		DRX_ASSERT_MESSAGE(resolutionData.m_active, "AddToDrawResolutionData() trying to add data to an inactive resolution level");
		if (resolutionData.m_active)
		{
			DRX_ASSERT_MESSAGE(resolutionData.m_dataType == SDrawResolutionData::eDataType_int, "AddIntsToDrawResolutionData() trying to add ints to a level that's not configured for ints");
			if (resolutionData.m_dataType == SDrawResolutionData::eDataType_int)
			{
				resolutionData.m_intDataForTeams[0] += inTeam1Data;
				resolutionData.m_intDataForTeams[1] += inTeam2Data;
			}
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(0, "AddToDrawResolutionData() unhandled inResolutionLevel");
	}
}

template <class T>
bool CGameRulesStandardVictoryConditionsTeam::IsTeam1Winning(const SDrawResolutionData::EWinningDataTest inDataTest, T team1Data, T team2Data)
{
	bool team1Winning=false;

	switch(inDataTest)
	{
		case SDrawResolutionData::eWinningData_greater_than:
			if (team1Data > team2Data)
			{
				team1Winning=true;
			}
			break;
		case SDrawResolutionData::eWinningData_less_than:
			if (team1Data < team2Data)
			{
				team1Winning=true;
			}
			break;
	}

	return team1Winning;
}

i32 CGameRulesStandardVictoryConditionsTeam::TryToResolveDraw(ESVC_DrawResolution *outWinningResolution)
{
	i32 winningTeam=0; // defaults to a draw
	*outWinningResolution=ESVC_DrawResolution_invalid;

	for (i32 i=ESVC_DrawResolution_level_1; i<ESVC_DrawResolution_num; i++)
	{
		SDrawResolutionData &resolutionData = m_drawResolutionData[i];

		if (resolutionData.m_active)
		{
			switch (resolutionData.m_dataType)
			{
				case SDrawResolutionData::eDataType_float:
					
					if (IsTeam1Winning(resolutionData.m_dataTest, resolutionData.m_floatDataForTeams[0], resolutionData.m_floatDataForTeams[1]))
					{
						winningTeam=1;
					}
					else
					{
						winningTeam=2;
					}
					break;
				case SDrawResolutionData::eDataType_int:
					if (resolutionData.m_intDataForTeams[0] == resolutionData.m_intDataForTeams[1])
					{
						// still a draw
						if(m_pGameRules->GetGameMode() == eGM_Assault)
						{
							//If we're playing assault we don't want to take into account the second tier resolution data unless
							//	the attacking team has won both rounds
							IGameRulesScoringModule * pScoring = m_pGameRules->GetScoringModule();
							if(pScoring)
							{
								if(!pScoring->AttackingTeamWonAllRounds())
								{
									winningTeam = -1;
								}								
							}
						}
					}
					else if (IsTeam1Winning(resolutionData.m_dataTest, resolutionData.m_intDataForTeams[0], resolutionData.m_intDataForTeams[1]))
					{
						winningTeam=1;
					}
					else
					{
						winningTeam=2;
					}
					break;
				default:
					DRX_ASSERT_MESSAGE(0, "TryToResolveDraw() unhandled data type");
					break;
			}

			if (winningTeam != 0)
			{
				if(winningTeam > 0)
					*outWinningResolution=static_cast<ESVC_DrawResolution>(i);
				else
				{
					winningTeam = 0;
					*outWinningResolution = ESVC_DrawResolution_level_1;
				}

				break;
			}
		}
	}

	return winningTeam;
}

void CGameRulesStandardVictoryConditionsTeam::DebugDrawResolutions()
{
#ifndef _RELEASE
	if (g_pGameCVars->g_victoryConditionsDebugDrawResolution)
	{
		for (i32 i=ESVC_DrawResolution_level_1; i<ESVC_DrawResolution_num; i++)
		{
			SDrawResolutionData &resolutionData = m_drawResolutionData[i];

			if (resolutionData.m_active)
			{
				switch(resolutionData.m_dataType)
				{
					case SDrawResolutionData::eDataType_int:
						DrxWatch("draw res level %d; team1=%d; team2=%d; msg=%s", i, resolutionData.m_intDataForTeams[0], resolutionData.m_intDataForTeams[1], CHUDUtils::LocalizeString(resolutionData.m_winningMessage.c_str()));
						break;
					case SDrawResolutionData::eDataType_float:
						DrxWatch("draw res level %d; team1=%.2f; team2=%.2f; msg=%s", i, resolutionData.m_floatDataForTeams[0], resolutionData.m_floatDataForTeams[1], CHUDUtils::LocalizeString(resolutionData.m_winningMessage.c_str()));
						break;
				}
			}
			else
			{
				DrxWatch("draw res level %d; INACTIVE", i);
			}
		}
	}
#endif
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::Update( float frameTime )
{
	IGameRulesStateModule *pStateModule = m_pGameRules->GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() != IGameRulesStateModule::EGRS_InGame)
		return;

	IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule();
	if (pRoundsModule != NULL && !pRoundsModule->IsInProgress())
	{
		return;
	}

	UpdateScoreLimitSounds();
	UpdateTimeLimitSounds();

	m_numUpdates++;

	if (!gEnv->bServer)
	{
		return;
	}
	
	DebugDrawResolutions();

	SvCheckOpponents();
	CheckTimeLimit();
	CheckScoreLimit();

	CheckScoreAsTimeLimit();
	DRX_TODO(09, 09, 2009, "Check Score limit could be done when the score changes as well as here.")
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::GetMaxTeamScore(STeamScoreResult &result)
{
	i32 numTeams = m_pGameRules->GetTeamCount();
	for (i32 i=1; i <= numTeams; ++i)
	{
		i32 score = ( AreAnyFlagsSet(eSVCF_DecideRoundWinnerUsingRoundPoints) ? m_pGameRules->SvGetTeamsScoreScoredThisRound(i) : m_pGameRules->GetTeamsScore(i));
		if (score >= result.score)
		{
			if (result.score==score)
			{
				result.scoreTeamId = 0;
			}
			else
			{
				result.scoreTeamId = i;
				result.score = score;
			}
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::GetMinTeamScore(STeamScoreResult &result)
{
	result.score = -1;

	i32 numTeams = m_pGameRules->GetTeamCount();
	for (i32 i=1; i <= numTeams; ++i)
	{
		i32 score = ( AreAnyFlagsSet(eSVCF_DecideRoundWinnerUsingRoundPoints) ? m_pGameRules->SvGetTeamsScoreScoredThisRound(i) : m_pGameRules->GetTeamsScore(i));
		if ((result.score==-1) || (score <= result.score))
		{
			if (result.score==score)
			{
				result.scoreTeamId = 0;
			}
			else
			{
				result.scoreTeamId = i;
				result.score = score;
			}
		}
	}
}

//-------------------------------------------------------------------------
bool CGameRulesStandardVictoryConditionsTeam::SvGameStillHasOpponentPlayers()
{
	DRX_ASSERT(gEnv->bServer);

	const bool  team1HasPlayers = (m_pGameRules->GetTeamPlayerCount(1, false) > 0);
	const bool  team2HasPlayers = (m_pGameRules->GetTeamPlayerCount(2, false) > 0);

	return (team1HasPlayers && team2HasPlayers);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::SvOpponentsCheckFailed()
{
	DRX_ASSERT(gEnv->bServer);
	CCCPOINT(VictoryConditionsTeam_SvOpponentsCheckFailed);

	i32  winnerTeam = 1;
	if (gEnv->IsClient())
	{
		winnerTeam = m_pGameRules->GetTeam(gEnv->pGame->GetIGameFramework()->GetClientActorId());
	}
	else
	{
		if (m_pGameRules->GetNumChannels() >= 1)
		{
			i32k  winnerChan = m_pGameRules->GetChannelIds()->front();
			const IActor*  winnerAct = m_pGameRules->GetActorByChannelId(winnerChan);
			if (winnerAct)
			{
				winnerTeam = m_pGameRules->GetTeam(winnerAct->GetEntityId());
			}
		}
	}

	if (winnerTeam > 0)
	{
		i32k  winnerTeamSz = m_pGameRules->GetTeamPlayerCount(winnerTeam);
		if (winnerTeamSz <= 1)
		{
			i32k  loserTeam = (3 - winnerTeam);
			const EDisconnectionCause  lastLoserDiscoCause = m_pGameRules->SvGetLastTeamDiscoCause(loserTeam);
			if (lastLoserDiscoCause != eDC_SessionDeleted)
			{
				DrxLog("CGameRulesStandardVictoryConditionsTeam::SvOpponentsCheckFailed: treating the game as a draw because there's no more than 1 player on the remaining team (there's %d) and the last disconnection cause on the losing team was not eDC_SessionDeleted (it was %d)", winnerTeamSz, lastLoserDiscoCause);
				winnerTeam = 0;  // if the final opponent quit via the ingame menus (SessionDeleted) or if there's more than one player on the remaining team then it's a victory, otherwise (here) treat it as a draw in case someone was trying to use a cable pull exploit
			}
		}
	}
			
	if (IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule())
	{
		pRoundsModule->SetTreatCurrentRoundAsFinalRound(true);
	}

	DrxLog("CGameRulesStandardVictoryConditionsTeam::SvOpponentsCheckFailed() no opponents left, declaring team %d as the winner", winnerTeam);

	OnEndGame(winnerTeam, EGOR_OpponentsDisconnected);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::SvForceEndGame(i32 winnerTeam, EGameOverReason eGameOverReason)
{
	OnEndGame(winnerTeam, eGameOverReason);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::SvCheckOpponents()
{
	DRX_ASSERT(gEnv->bServer);

	DRX_ASSERT_MESSAGE((g_pGame->GetHostMigrationState() == CGame::eHMS_NotMigrating), "Victory Conditions are being checked during a host migration - tell TomH about this!");

	const bool  hasOpponents = SvGameStillHasOpponentPlayers();
	if(hasOpponents)
	{
		SetFlags(eSVCF_HasEverHadOpponents);
	}
	
	if(!AreAnyFlagsSet(eSVCF_PlayerSwitchTeams))
	{
#ifndef _RELEASE
		if (g_pGameCVars->g_disable_OpponentsDisconnectedGameEnd > 0)
		{
			return;
		}
#endif

		if (!hasOpponents && AreAnyFlagsSet(eSVCF_HasEverHadOpponents))
		{
			SvOpponentsCheckFailed();
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::CheckTimeLimit()
{
#ifndef _RELEASE
	IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule();
	if (pRoundsModule)
	{
		DRX_ASSERT_MESSAGE(pRoundsModule->IsInProgress(), "CheckTimeLimit() is being called when a round isn't in progress. This shouldn't happen");
	}
#endif

	if (AreAnyFlagsSet(eSVCF_CheckTime) && m_pGameRules->IsTimeLimited())
	{
		float timeRemaining = m_pGameRules->GetRemainingGameTime();

		if(timeRemaining <= 0.f)
		{
			TimeLimitExpired();
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::TimeLimitExpired()
{
	DRX_ASSERT(gEnv->bServer);

	if (IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule())
	{
		if (pRoundsModule->IsInProgress())
		{
			m_pGameRules->SvOnTimeLimitExpired_NotifyListeners();  // nb. must come before the call to GetMaxTeamScore(), because listeners of this notification might want to award scores which then need to be taken into account by GetMaxTeamScore()
		}
	}

	STeamScoreResult result;
	GetMaxTeamScore(result);

	CCCPOINT(VictoryConditionsTeam_SvTimeLimitExpired);
	OnEndGame(result.scoreTeamId, EGOR_TimeLimitReached);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::ClUpdatedTimeLimit()
{
	const bool timeLimited = m_pGameRules->IsTimeLimited();
	const float timeLimit = (timeLimited ? m_pGameRules->GetTimeLimit() * 60.0f : 600.0f);
	float timeRemaining = m_pGameRules->GetRemainingGameTime();

	//DrxLog("CGameRulesStandardVictoryConditionsTeam::ClUpdatedTimeLimit() isTimeLimited=%d; timeLimit=%f", timeLimited, timeLimit);

	m_currentSoundTime[eSVC_StartGame] = (timeLimit - 0.5f);	//0.5 to account for network compression
	m_currentSoundTime[eSVC_3secsIn] = (timeLimit - 3.0f);
	m_currentSoundTime[eSVC_Halfway] = (timeRemaining * 0.5f);	// timeRemaining is what we measure against, in modes where we adjust timelimits a lot (predator) the old timeLimit check here lead to odd results

	m_currentSound = eSVC_StartGame;

	// depending on the length of game these soundTime thresholds can sometimes get whacked out of chronological order
	// e.g in a 2 minute game the "1min" threshold will come before the "halfway" threshold (Should be only valid case)
	// In this case we will entirely skip the halfway signal
	COMPILE_TIME_ASSERT(eSVC_size == DRX_ARRAY_COUNT(m_currentSoundTime));
	for(i32 i = 0; i < (eSVC_size - 1); i++)
	{
		if(m_currentSoundTime[i] <= m_currentSoundTime[i + 1])
		{
			m_currentSoundTime[i] = FLT_MAX;	//Will be skipped
		}
	}

	if (m_numUpdates >= k_num_updates_actually_in_play)
	{
		SetFlags(eSVCF_HaveUpdatedTimeLimit);
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::UpdateScoreLimitSounds()
{
	if(gEnv->IsClient())
	{
		float gameTime = m_pGameRules->GetCurrentGameTime();
		if(gameTime > m_fNextScoreLimitCheck)
		{
			const float timeBetweenChecks = 10.0f;
			m_fNextScoreLimitCheck = gameTime += timeBetweenChecks;

			EGameMode mode = m_pGameRules->GetGameMode();
			if((mode == eGM_CrashSite) ||(mode == eGM_TeamInstantAction) || (mode == eGM_InstantAction))
			{
				i32k scoreLimit = m_pGameRules->GetScoreLimit();
				if(scoreLimit && !AreAnyFlagsSet(eSVCF_PlayedNearScoreLimit))
				{
					i32 highScore = 0;
					EGameOverType clientWinning = IsClientWinning(highScore);

					const float nearCompletePercentage = 0.90f;
					i32 nearScoreLimit = (i32) (scoreLimit * nearCompletePercentage);
					if(highScore >= nearScoreLimit)
					{
						SetFlags(eSVCF_PlayedNearScoreLimit);
						if(clientWinning == EGOT_Win)
						{
							CAnnouncer::GetInstance()->Announce("NearScoreLimitWinning", CAnnouncer::eAC_inGame);
						}
						else if(clientWinning == EGOT_Draw)
						{
							CAnnouncer::GetInstance()->Announce("NearScoreLimitDrawing", CAnnouncer::eAC_inGame);
						}
						else if(clientWinning == EGOT_Lose)
						{
							CAnnouncer::GetInstance()->Announce("NearScoreLimitLosing", CAnnouncer::eAC_inGame);
						}
					}				
				}
			}
		}
	}
}

//-------------------------------------------------------------------------
// TODO - this needs to be fixed for !timelimited modes (possible in a variant) which wouldn't play the announcement
void CGameRulesStandardVictoryConditionsTeam::UpdateTimeLimitSounds()
{
	if(gEnv->IsClient() && m_pGameRules->IsTimeLimited() && m_currentSound < eSVC_size)
	{
		const IGameRulesRoundsModule* const pRoundsModule = m_pGameRules->GetRoundsModule();
		const bool isInSuddenDeath = pRoundsModule ? pRoundsModule->IsInSuddenDeath() : false;
		float timeRemaining = m_pGameRules->GetRemainingGameTime();
		i32 csi = m_currentSound;

		// Skip ones that should have already happened (for mid-game joins)
		COMPILE_TIME_ASSERT(eSVC_size == DRX_ARRAY_COUNT(m_currentSoundTime));
		while ((csi < eSVC_size) && (timeRemaining <= m_currentSoundTime[csi] - 1.0f))
		{
			DrxLog("skipping over csi=%d (%s) as timeRemaining is less than m_currentSoundTime=%f minus 1.0f", csi, s_ESVC_SoundsListStrs[csi], m_currentSoundTime[csi]);
			csi++;
		}
		
		DRX_ASSERT(csi >= 0 && csi < eSVC_size);

		//DrxWatch(">>>> cur sound index: %d (%s) (tim: %f, rem: %f)", csi, s_ESVC_SoundsListStrs[csi], m_currentSoundTime[csi], timeRemaining);
		if(timeRemaining <= m_currentSoundTime[csi])
		{
			i32 teamId = 0;	//Gets it from your client actors team when zero

			if(m_pGameRules->GetGameMode() == eGM_Assault)
			{
				const EntityId  localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
				if (i32k localTeam=m_pGameRules->GetTeam(localClientId))
				{
					//in assault the attackers have commander 2 and defenders have commander 1 (Recorded voice lines are specific to attackers and defenders)
					teamId = (pRoundsModule && localTeam == pRoundsModule->GetPrimaryTeam()) ? 2 : 1;
				}
			}

			DrxLog("CGameRulesStandardVictoryConditionsTeam::UpdateTimeLimitSounds() about to play timelimit sound %d (%s)", csi, s_ESVC_SoundsListStrs[csi]);

			switch(csi)
			{
			case eSVC_StartGame:
				{
					csi++;
					DRX_ASSERT(csi == eSVC_3secsIn);
				}
				break;
			case eSVC_3secsIn:
				{
					//Supporting announcement from commander (after gamemode announcement)

					if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
					{
						CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "StartGame", CAnnouncer::eAC_inGame);

						//Some team-based, rounds-based modes use these "StartGamePrimaryTeam" and "StartGameDefendingTeam" announcements instead.
						//(We have to trigger them as well as StartGame above, but as long as only one or the other is set in the announcments xml for a particular gamemode then only one of them will play.)

						if (pRoundsModule && !isInSuddenDeath)
						{
							const EntityId  localClientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
							if (i32k localTeam=m_pGameRules->GetTeam(localClientId))
							{
								tukk  announcement;
								if (localTeam == pRoundsModule->GetPrimaryTeam())
								{
									announcement = "StartGamePrimaryTeam";
								}
								else
								{
									announcement = "StartGameDefendingTeam";
								}
								CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, announcement, CAnnouncer::eAC_inGame);
							}
						}
					}

					csi++;
					DRX_ASSERT(csi == eSVC_Halfway);
				}
				break;
			case eSVC_Halfway:
				{
					csi++;
					DRX_ASSERT(csi == eSVC_1min);
				}
				break;
			case eSVC_1min:
				{
					if (!isInSuddenDeath)
					{
						if(!AreAnyFlagsSet(eSVCF_PlayedNearScoreLimit))
						{
							SetFlags(eSVCF_PlayedNearScoreLimit);

							if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
							{
								i32 highScore = 0;
								EGameOverType clientWinning = IsClientWinning(highScore);
								if(clientWinning == EGOT_Win)
								{
									CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "1MinuteLeftWinning", CAnnouncer::eAC_inGame);
								}
								else if(clientWinning == EGOT_Draw)
								{
									CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "1MinuteLeftDrawing", CAnnouncer::eAC_inGame);
								}
								else if(clientWinning == EGOT_Lose)
								{
									CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "1MinuteLeftLosing", CAnnouncer::eAC_inGame);
								}
							}
						}
					}
					
					csi++;
					DRX_ASSERT(csi == eSVC_30secs);
				}
				break;
			case eSVC_30secs:
				{
					if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
					{
						i32 highScore = 0;
						EGameOverType clientWinning = IsClientWinning(highScore);
						if(clientWinning == EGOT_Win)
						{
							CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "30SecondsLeftWinning", CAnnouncer::eAC_inGame);
						}
						else if(clientWinning == EGOT_Draw)
						{
							CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "30SecondsLeftDrawing", CAnnouncer::eAC_inGame);
						}
						else if(clientWinning == EGOT_Lose)
						{
							CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "30SecondsLeftLosing", CAnnouncer::eAC_inGame);
						}
					}

					csi++;
					DRX_ASSERT(csi == eSVC_15secs);
				}
				break;
			case eSVC_15secs:
				{
					if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
					{
						CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "15SecondsLeft", CAnnouncer::eAC_inGame);
					}
					csi++;
					DRX_ASSERT(csi == eSVC_5secs);
				}
				break;
			case eSVC_5secs:
				{
					if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
					{
						CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "5SecondsLeft", CAnnouncer::eAC_inGame);
					}
					csi++;
					DRX_ASSERT(csi == eSVC_3secs);
				}
				break;
			case eSVC_3secs:
				{
					if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
					{
						CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "3SecondsLeft", CAnnouncer::eAC_inGame);
					}
					csi++;
					DRX_ASSERT(csi == eSVC_end);
				}
				break;
			case eSVC_end:	
				if (AreAnyFlagsSet(eSVCF_PlayTimeAnnouncements))
				{
					CAnnouncer::GetInstance()->AnnounceAsTeam(teamId, "TimeUp", CAnnouncer::eAC_inGame);
				}
				csi++;
				DRX_ASSERT(csi == eSVC_size);

				break;
			}
		}

		DRX_ASSERT(csi >= 0 && csi <= eSVC_size);
		m_currentSound = (ESVC_Sounds) csi;
	}
}

//-------------------------------------------------------------------------
bool CGameRulesStandardVictoryConditionsTeam::ScoreLimitReached()
{
	i32 scoreLimit = (AreAnyFlagsSet(eSVCF_CheckScore) ? m_pGameRules->GetScoreLimit() : 0);
	if (scoreLimit > 0)
	{
		STeamScoreResult result;
		GetMaxTeamScore(result);

		if (result.score >= scoreLimit)
		{
			return true;
		}
	}
	return false;
}


//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::OnHostMigrationPromoteToServer()
{
	SetFlags(eSVCF_HasEverHadOpponents); // this is based on the inference that if a player has managed to migrate (and in this case become the server) then they must be in a game that before the migration had opponents in it	
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::CheckScoreLimit()
{
#ifndef _RELEASE
	IGameRulesRoundsModule* pDebugRoundsModule=m_pGameRules->GetRoundsModule();
	if (pDebugRoundsModule)
	{
		DRX_ASSERT_MESSAGE(pDebugRoundsModule->IsInProgress(), "CheckScoreLimit() is being called when a round isn't in progress. This shouldn't happen");
	}
#endif

	i32 scoreLimit = (AreAnyFlagsSet(eSVCF_CheckScore) ? m_pGameRules->GetScoreLimit() : 0);
	if (scoreLimit > 0)
	{
		STeamScoreResult result;
		GetMaxTeamScore(result);

		if (result.score >= scoreLimit)
		{
			if (AreAnyFlagsSet(eSVCF_TreatScoreRoundVictoryAsGameVictory))
			{
				if (IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule())
				{
					pRoundsModule->SetTreatCurrentRoundAsFinalRound(true);
				}
			}

			CCCPOINT(VictoryConditionsTeam_SvScoreLimitReached);
			OnEndGame(result.scoreTeamId, EGOR_ScoreLimitReached, ESVC_DrawResolution_invalid, m_winningVictim, m_winningAttacker);
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::CheckScoreAsTimeLimit()
{
	if ( AreAnyFlagsSet(eSVCF_CheckScoreAsTime) )
	{
		STeamScoreResult result;
		GetMinTeamScore(result);

		i32 gameTime = (i32)ceil(m_pGameRules->GetCurrentGameTime());

		if (result.score - gameTime  <= 0)
		{
			CCCPOINT(VictoryConditionsTeam_SvTimeLimitReached);
			OnEndGame(result.scoreTeamId ? OTHER_TEAM(result.scoreTeamId) : 0, EGOR_TimeLimitReached); // TODO: Another win type - OtherTeamEliminated?
		}
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::ClVictoryTeam(i32 teamId, EGameOverReason reason, ESVC_DrawResolution winningResolution,const SDrawResolutionData& level1, const SDrawResolutionData& level2, const EntityId killedEntity, const EntityId shooterEntity)
{
	if (gEnv->IsClient())
	{
		m_drawResolutionData[ESVC_DrawResolution_level_1] = level1;
		m_drawResolutionData[ESVC_DrawResolution_level_2] = level2;
		OnEndGame(teamId, reason, winningResolution, killedEntity, shooterEntity);

		m_victoryParams.winningTeamId = teamId;
		m_pGameRules->RegisterTeamChangedListener(this);
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::OnRestart()
{
	ResetFlags(eSVCF_HasEverHadOpponents | eSVCF_HaveUpdatedTimeLimit | eSVCF_PlayedNearScoreLimit);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::SvOnEndGame()
{
	DRX_ASSERT(gEnv->bServer);
	//parameters are calculated in function on server
	OnEndGame(0, EGOR_Unknown, ESVC_DrawResolution_invalid);
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::OnEndGame(i32 teamId, EGameOverReason reason, ESVC_DrawResolution winningResolution, EntityId killedEntity, EntityId shooterEntity)
{
	EGameOverType gameOverType = EGOT_Unknown;

	if (gEnv->bServer)
	{
		IGameRulesRoundsModule *pRoundsModule = m_pGameRules->GetRoundsModule();
		if (pRoundsModule)
		{
			pRoundsModule->OnEndGame(teamId, 0, reason);
			if (!pRoundsModule->IsGameOver())
			{
				if(pRoundsModule->GetPrimaryTeam() == teamId)
				{
					CAnnouncer::GetInstance()->AnnounceFromTeamId(teamId, "RoundWinPrimary", CAnnouncer::eAC_always);
				}
				else
				{
					CAnnouncer::GetInstance()->AnnounceFromTeamId(teamId, "RoundLostPrimary", CAnnouncer::eAC_always);
				}
				
				// Game isn't actually over yet!
				return;
			}
			else
			{
				i32 team1Score=0;
				i32 team2Score=0;

				if (AreAnyFlagsSet(eSVCF_DecideGameWinnerWithTeamScore))
				{
					team1Score = m_pGameRules->GetTeamsScore(1);
					team2Score = m_pGameRules->GetTeamsScore(2);
				}
				else
				{
					team1Score = m_pGameRules->GetTeamRoundScore(1);
					team2Score = m_pGameRules->GetTeamRoundScore(2);
				}

				if (team1Score>team2Score)
					teamId = 1;
				else if (team2Score>team1Score)
					teamId = 2;
				else
				{
					teamId = TryToResolveDraw(&winningResolution);

					if (teamId)
					{
						DrxLog("CGameRulesStandardVictoryConditionsTeam::OnEndGame() successfully resolved a draw into a win for team %d using resolution %d", teamId, winningResolution);
						reason = EGOR_DrawRoundsResolvedWithMessage;
					}
				}
			}  
		}
		else
		{
			// TODO - add parameter in xml to control whether we try to resolve draws
			if (teamId == 0)
			{
				teamId = TryToResolveDraw(&winningResolution);

				if (teamId)
				{
					DrxLog("CGameRulesStandardVictoryConditionsTeam::OnEndGame() successfully resolved a draw into a win for team %d using resolution %d", teamId, winningResolution);
					reason = EGOR_DrawRoundsResolvedWithMessage;
				}
			}
		}
    
    EntityId localClient = gEnv->pGame->GetIGameFramework()->GetClientActorId();
    if ( shooterEntity == localClient )
    {
      g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_WinningKill);
    }
    if ( killedEntity == localClient )
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

		i32 team1Score = m_pGameRules->GetTeamsScore(1);
		i32 team2Score = m_pGameRules->GetTeamsScore(2);
		m_victoryParams = CGameRules::VictoryTeamParams(teamId, reason, team1Score, team2Score, winningResolution, m_drawResolutionData[ESVC_DrawResolution_level_1], m_drawResolutionData[ESVC_DrawResolution_level_2], m_winningVictim, m_winningAttacker);
    m_winningVictim = 0;
    m_winningAttacker = 0;
		m_pGameRules->GetGameObject()->InvokeRMI(CGameRules::ClVictoryTeam(), m_victoryParams, eRMI_ToRemoteClients);
		m_pGameRules->RegisterTeamChangedListener(this);
	}
	
	m_pGameRules->OnEndGame();

	if (gEnv->IsClient())
	{
		EntityId localClient = gEnv->pGame->GetIGameFramework()->GetClientActorId();
		i32 clientTeam = m_pGameRules->GetTeam(localClient);
		if (teamId)
		{
			if(clientTeam == teamId)
			{
				CAudioSignalPlayer::JustPlay("MatchWon");
        if ( shooterEntity == localClient && !gEnv->bServer) // server has already incremented stats above
        {
          g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_WinningKill);
        }
			}
			else
			{
				CAudioSignalPlayer::JustPlay("MatchLost");
        if ( killedEntity == localClient && !gEnv->bServer) // server has already incremented stats above
        {
          g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_VictimOnFinalKillcam);
        }
			}
		}
		else
		{
			CAudioSignalPlayer::JustPlay("MatchDraw");
		}

		stack_string message_cached("");
		if (reason == EGOR_DrawRoundsResolvedWithMessage)
		{
			if (winningResolution >= ESVC_DrawResolution_level_1 && winningResolution < ESVC_DrawResolution_num)
			{
				message_cached = CHUDUtils::LocalizeString(m_drawResolutionData[winningResolution].m_winningMessage);
				DrxLog("CGameRulesStandardVictoryConditionsTeam::OnEndGame() successfully resolved a draw into a win for team %d using resolution %d. Got winning message=%s", teamId, winningResolution, message_cached.c_str());
			}
			else
			{
				DRX_ASSERT_MESSAGE(0, "OnEndGame() invalid winning resolution set for a game that was resolved from a draw to a win");
			}
		}

		if(teamId == 0)
		{
			gameOverType = EGOT_Draw;
		}
		else
		{
			gameOverType = (clientTeam==teamId) ? EGOT_Win : EGOT_Lose;
		}
		DRX_ASSERT(gameOverType != EGOT_Unknown);	

		const bool  clientScoreIsTop = (gameOverType != EGOT_Lose);

		i32k clientScore = m_pGameRules->GetTeamsScore(clientTeam);
		i32k opoonentScore = m_pGameRules->GetTeamsScore(3 - clientTeam);

		EAnnouncementID announcementId = INVALID_ANNOUNCEMENT_ID;

		GetGameOverAnnouncement(gameOverType, clientScore, opoonentScore, announcementId);
	
		if (!m_pGameRules->GetRoundsModule())
		{
			// rounds will play the one shot themselves
			CAudioSignalPlayer::JustPlay("RoundEndOneShot"); 
		}

		SHUDEventWrapper::OnGameEnd(teamId, clientScoreIsTop, reason, message_cached.c_str(), winningResolution, announcementId);
	}

	m_pGameRules->GameOver( gameOverType );
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::GetGameOverAnnouncement(const EGameOverType gameOverType, i32k clientScore, i32k opponentScore, EAnnouncementID& announcement)
{
	bool closeGame = false;
	bool standardGame = true;

	i32 scoreLimit = m_pGameRules->GetScoreLimit();
	if(scoreLimit > 0)
	{
		i32 scoreDiff = abs(clientScore - opponentScore);
		closeGame = scoreDiff < (scoreLimit * 0.15f);													//close if within 15% of score limit
		standardGame = !closeGame && scoreDiff < (scoreLimit * 0.6f);					// standard if not close and within 60% of score limit
	}

	CGameAudio *pGameAudio = g_pGame->GetGameAudio();
	CAnnouncer *pAnnouncer = CAnnouncer::GetInstance();

	if(gameOverType == EGOT_Win)
	{
		if(closeGame)
		{
			announcement = pAnnouncer->NameToID("WinClose");
		}
		else if (standardGame)
		{
			announcement = pAnnouncer->NameToID("WinStandard");
		}
		else
		{
			announcement = pAnnouncer->NameToID("WinBig");
		}
	}
	else if(gameOverType == EGOT_Lose)
	{
		if(closeGame)
		{
			announcement = pAnnouncer->NameToID("LoseClose");
		}
		else if (standardGame)
		{
			announcement = pAnnouncer->NameToID("LoseStandard");
		}
		else
		{
			announcement = pAnnouncer->NameToID("LoseBig");
		}
	}
	else if(gameOverType == EGOT_Draw)
	{
		announcement = pAnnouncer->NameToID("Draw");
	}
}

//-------------------------------------------------------------------------
void CGameRulesStandardVictoryConditionsTeam::SvSurvivorCountRefresh( i32 count, const EntityId survivors[], i32 numKills )
{
	// Only care about lives if the round is in progress
	IGameRulesRoundsModule *pRoundsModule = g_pGame->GetGameRules()->GetRoundsModule();
	if (pRoundsModule != NULL && !pRoundsModule->IsInProgress())
	{
		return;
	}

	// Don't end the round due to people dying if there is no-one on the teams!
	i32 team1Members = m_pGameRules->GetTeamPlayerCountWhoHaveSpawned(1);
	i32 team2Members = m_pGameRules->GetTeamPlayerCountWhoHaveSpawned(2);

	if (team1Members && team2Members)
	{
		i32 primaryTeamId = (m_pGameRules->GetRoundsModule()) ? m_pGameRules->GetRoundsModule()->GetPrimaryTeam() : 1;

		i32 primaryCount = 0;
		i32 secondaryCount = 0;

		for (i32 i = 0; i < count; ++ i)
		{
			if (m_pGameRules->GetTeam(survivors[i]) == primaryTeamId)
			{
				++ primaryCount;
			}
			else
			{
				++ secondaryCount;
			}
		}

		if (AreAllFlagsSet(eSVCF_WinOnPrimaryTeamDead | eSVCF_WinOnSecondaryTeamDead) && !primaryCount && !secondaryCount)
		{
			// Draw
			OnEndGame(0, EGOR_NoLivesRemaining);
		}
		else if ( AreAnyFlagsSet(eSVCF_WinOnPrimaryTeamDead) && !primaryCount)
		{
			// Primary team is dead, secondary team wins
			OnEndGame(3 - primaryTeamId, EGOR_NoLivesRemaining);
		}
		else if ( AreAnyFlagsSet(eSVCF_WinOnSecondaryTeamDead) && !secondaryCount)
		{
			// Secondary team is dead, primary team wins
			OnEndGame(primaryTeamId, EGOR_NoLivesRemaining);
		}
	}
}

i32k CGameRulesStandardVictoryConditionsTeam::GetClientTeam(const EntityId clientId) const
{
	return m_pGameRules->GetTeam(clientId);
}

const EGameOverType CGameRulesStandardVictoryConditionsTeam::IsClientWinning(i32& highScore) const
{
	const EntityId clientId = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	i32k clientTeam = GetClientTeam(clientId);
	if(clientTeam)
	{
		 return IsClientWinning(clientTeam, highScore);
	}
	else
	{
		IGameRulesPlayerStatsModule* pPlayStatsMo = m_pGameRules->GetPlayerStatsModule();
		if (pPlayStatsMo)
		{
			i32 clientScore = 0;
			if (const SGameRulesPlayerStat* s = pPlayStatsMo->GetPlayerStats(clientId))
			{
				clientScore = s->kills;
				highScore = s->kills;
			}

			EGameOverType clientWinning = EGOT_Win;

			i32k numStats = pPlayStatsMo->GetNumPlayerStats();
			for (i32 i=0; i<numStats; i++)
			{
				const SGameRulesPlayerStat*  s = pPlayStatsMo->GetNthPlayerStats(i);
				if (s->flags & SGameRulesPlayerStat::PLYSTATFL_HASSPAWNEDTHISROUND)
				{
					if(s->kills > highScore)
					{
						highScore = s->kills;
						clientWinning = EGOT_Lose;
					}
					else if(s->kills == clientScore && clientWinning == EGOT_Win)
					{
						clientWinning = EGOT_Draw;
					}
				}
			}

			return clientWinning;
		}
	}

	return EGOT_Unknown;
}

const EGameOverType CGameRulesStandardVictoryConditionsTeam::IsClientWinning(i32k clientTeam, i32& highScore) const
{
	DRX_ASSERT(clientTeam != 0);
	i32k clientTeamScore = m_pGameRules->GetTeamsScore(clientTeam);
	i32k nonClientTeamScore = m_pGameRules->GetTeamsScore(OTHER_TEAM(clientTeam));
	highScore = max(clientTeamScore, nonClientTeamScore);
	if(clientTeamScore == nonClientTeamScore)
	{
		return EGOT_Draw;
	}
	else
	{
		return (clientTeamScore >= nonClientTeamScore) ? EGOT_Win : EGOT_Lose;
	}
}

void CGameRulesStandardVictoryConditionsTeam::SendVictoryMessage( i32 channelId )
{
	m_pGameRules->GetGameObject()->InvokeRMI(CGameRules::ClVictoryTeam(), m_victoryParams, eRMI_ToClientChannel, channelId);
}

void CGameRulesStandardVictoryConditionsTeam::OnChangedTeam( EntityId entityId, i32 oldTeamId, i32 newTeamId )
{
}

#undef OTHER_TEAM
