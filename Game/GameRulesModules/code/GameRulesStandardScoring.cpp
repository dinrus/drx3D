// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

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

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesStandardScoring.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/Utility/DesignerWarning.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/StatsRecordingMgr.h>

AUTOENUM_BUILDNAMEARRAY(CGameRulesStandardScoring::s_gamerulesScoreType, EGRSTList);

#define STANDARD_SCORING_STATE_ASPECT		eEA_GameServerA

//-------------------------------------------------------------------------
CGameRulesStandardScoring::CGameRulesStandardScoring()
{
	m_maxTeamScore = 0;
	m_startTeamScore = 0;
	m_useScoreAsTime = false;
	m_bAttackingTeamWonAllRounds = true;
	m_deathScoringModifier = 0;

	for(i32 i = 0; i < EGRST_Num; i++)
	{
		m_playerScorePoints[i] = 0;
		m_playerScoreXP[i] = 0;
		m_teamScorePoints[i] = 0;
	}
}

//-------------------------------------------------------------------------
CGameRulesStandardScoring::~CGameRulesStandardScoring()
{
}

//-------------------------------------------------------------------------
void CGameRulesStandardScoring::Init( XmlNodeRef xml )
{
	i32 numScoreCategories = xml->getChildCount();
	for (i32 i = 0; i < numScoreCategories; ++ i)
	{
		XmlNodeRef categoryXml = xml->getChild(i);
		tukk categoryTag = categoryXml->getTag();

		if (!stricmp(categoryTag, "Player"))
		{
			InitScoreData(categoryXml, &m_playerScorePoints[0], m_playerScoreXP);
		}
		else if (!stricmp(categoryTag, "Team"))
		{
			InitScoreData(categoryXml, &m_teamScorePoints[0], NULL);
			categoryXml->getAttr("maxScore", m_maxTeamScore);
			categoryXml->getAttr("startTeamScore", m_startTeamScore);
			categoryXml->getAttr("useScoreAsTime", m_useScoreAsTime);
		}
	}
}

bool CGameRulesStandardScoring::NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags )
{
	if (aspect == STANDARD_SCORING_STATE_ASPECT)
	{
		i32 deathScoringModifierWas=m_deathScoringModifier;
		ser.Value("deathScoreModifier", m_deathScoringModifier, 'ui10'); // max 1023
	}

	return true;
}

void CGameRulesStandardScoring::InitScoreData(XmlNodeRef categoryXml, TGameRulesScoreInt *scoringData, TGameRulesScoreInt *xpData)
{
	DRX_ASSERT(scoringData);

	i32 numScoreValues = categoryXml->getChildCount();
	for (i32 j = 0; j < numScoreValues; ++ j)
	{
		XmlNodeRef childXml = categoryXml->getChild(j);
		tukk scoringTag = childXml->getTag();

		if (!stricmp(scoringTag, "Event"))
		{
			i32 points = 0;
			if (childXml->getAttr("points", points))
			{
				i32 type = EGRST_Unknown;
				tukk  pChar = NULL;
				if (childXml->getAttr("type", &pChar))
				{
					bool  typeOk = AutoEnum_GetEnumValFromString(pChar, s_gamerulesScoreType, EGRST_Num, &type);
					if (typeOk)
					{
						DesignerWarning( points < SGameRulesScoreInfo::SCORE_MAX && points > SGameRulesScoreInfo::SCORE_MIN, "Adding score for player which is out of net-serialize bounds (%d is not within [%d .. %d])", points, SGameRulesScoreInfo::SCORE_MIN, SGameRulesScoreInfo::SCORE_MAX );
						scoringData[type] = static_cast<TGameRulesScoreInt>(points);

						if (xpData)
						{
							i32 xp = points;		// Default XP to be the same as points if not specified
							childXml->getAttr("xp", xp);

							xpData[type] = static_cast<TGameRulesScoreInt>(xp);
						}
					}
					else
					{
						DrxLogAlways("GameRulesStandardScoring::Init() : Scoring Event type not recognised: %s.", pChar);
					}
				}
				else
				{
					DrxLogAlways("GameRulesStandardScoring::Init() : Scoring Event has no type declared.");
				}
			}
			else
			{
				DrxLogAlways("GameRulesStandardScoring::Init() : Scoring Event has no points declared.");
			}
		}
	}
}

//-------------------------------------------------------------------------
TGameRulesScoreInt CGameRulesStandardScoring::GetPlayerPointsByType(EGRST pointsType) const
{
	return GetPointsByType(&m_playerScorePoints[0], pointsType);
}

//-------------------------------------------------------------------------
TGameRulesScoreInt CGameRulesStandardScoring::GetPlayerXPByType(EGRST pointsType) const
{
	return GetPointsByType(m_playerScoreXP, pointsType);
}

//-------------------------------------------------------------------------
TGameRulesScoreInt CGameRulesStandardScoring::GetTeamPointsByType(EGRST pointsType) const
{
	return GetPointsByType(&m_teamScorePoints[0], pointsType);
}

//-------------------------------------------------------------------------
TGameRulesScoreInt CGameRulesStandardScoring::GetPointsByType(const TGameRulesScoreInt *scoringData, EGRST pointsType) const
{
	DRX_ASSERT_MESSAGE(pointsType > EGRST_Unknown && pointsType < EGRST_Num, "Out of range parameter passed into CGameRulesStandardScoring::GetPointsByType");
	const TGameRulesScoreInt &scoreData = scoringData[pointsType];

	if(scoreData == 0)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "Scoring not setup for %s in gamemode %s", s_gamerulesScoreType[pointsType], gEnv->pConsole->GetCVar("sv_gamerules")->GetString());
	}

	return scoreData;
}

void CGameRulesStandardScoring::DoScoringForDeath(IActor *pTargetActor, EntityId shooterId, i32 damage, i32 material, i32 hit_type)
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	IActor *pShooterActor =  g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(shooterId);

	if (pGameRules != NULL && pTargetActor != NULL && pShooterActor != NULL)
	{
		if (pGameRules->HasGameActuallyStarted() == false)
		{
			return;
		}

		// No scoring at game end
		IGameRulesStateModule *pStateModule = pGameRules->GetStateModule();
		if (pStateModule != NULL && pStateModule->GetGameState() == IGameRulesStateModule::EGRS_PostGame)
		{
			return;
		}

		EntityId targetId = pTargetActor->GetEntityId();
		bool bTeamGame = (pGameRules->GetTeamCount() > 1);
		i32 targetTeam = pGameRules->GetTeam(targetId);
		i32 shooterTeam = pGameRules->GetTeam(shooterId);
		
		i32 playerPoints = 0, playerXP = 0;
		EGRST scoreType;

		bool useModifier=true;
		if (pTargetActor == pShooterActor)
		{
			scoreType = EGRST_Suicide;
			useModifier = false;
		}
		else if (bTeamGame && (targetTeam == shooterTeam))
		{
			scoreType = EGRST_PlayerTeamKill;
		}
		else
		{
			scoreType = EGRST_PlayerKill;
		}

		// map reason from one enum to other
		EXPReason			reason=EXPReasonFromEGRST(scoreType);

		playerPoints = GetPlayerPointsByType(scoreType);
		playerXP = GetPlayerXPByType(scoreType);
		
		if (useModifier)
		{
			playerPoints += m_deathScoringModifier;
			playerXP += m_deathScoringModifier;
		}

		if(pShooterActor->IsPlayer())
		{
			playerXP = static_cast<CPlayer*>(pShooterActor)->GetXPBonusModifiedXP(playerXP);
		}

		SGameRulesScoreInfo scoreInfo((EGameRulesScoreType) scoreType, playerPoints, playerXP, reason);
		scoreInfo.AttachVictim(targetId);
		DrxLog("About to call pGameRules->IncreasePoints, pGameRules=%p", pGameRules);
		INDENT_LOG_DURING_SCOPE();

		OnTeamScoringEvent(shooterTeam, scoreType);
		pGameRules->IncreasePoints(shooterId, scoreInfo);
	}
}

bool CGameRulesStandardScoring::ShouldScore(CGameRules *pGameRules) const
{
	if (!pGameRules)
		return false;

	if (pGameRules->HasGameActuallyStarted() == false)
	{
		return false;
	}

	// No scoring at game end
	IGameRulesStateModule *pStateModule = pGameRules->GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() == IGameRulesStateModule::EGRS_PostGame)
	{
		return false;
	}

	return true;
}

void CGameRulesStandardScoring::OnPlayerScoringEvent( EntityId playerId, EGRST pointsType)
{
	DRX_ASSERT(pointsType != EGRST_Unknown);

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(ShouldScore(pGameRules))
	{
		i32 playerPoints = GetPlayerPointsByType(pointsType);
		i32 playerXP = GetPlayerXPByType(pointsType);
		
		IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId);
		if(pActor && pActor->IsPlayer())
		{
			playerXP = static_cast<CPlayer*>(pActor)->GetXPBonusModifiedXP(playerXP);
		}

		if(playerPoints != 0)
		{
			EXPReason		reason=EXPReasonFromEGRST(pointsType);
			SGameRulesScoreInfo  scoreInfo(pointsType, playerPoints, playerXP, reason);
			pGameRules->IncreasePoints(playerId, scoreInfo);
		}
	}
}

void CGameRulesStandardScoring::OnPlayerScoringEventWithInfo(EntityId playerId, SGameRulesScoreInfo* scoreInfo)
{
	DRX_ASSERT(scoreInfo);
	DRX_ASSERT(scoreInfo->type != EGRST_Unknown);

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(ShouldScore(pGameRules))
	{
		if(scoreInfo->score != 0)
		{
			SGameRulesScoreInfo newScoreInfo(*scoreInfo);
			pGameRules->IncreasePoints(playerId, newScoreInfo);
		}
	}
}

void CGameRulesStandardScoring::OnPlayerScoringEventToAllTeamWithInfo(i32k teamId, SGameRulesScoreInfo* scoreInfo)
{
	DRX_ASSERT(gEnv->bServer);

	CGameRules::TPlayers  teamPlayers;
	g_pGame->GetGameRules()->GetTeamPlayers(teamId, teamPlayers);
	
	CGameRules::TPlayers::const_iterator  it = teamPlayers.begin();
	CGameRules::TPlayers::const_iterator  end = teamPlayers.end();
	for (; it!=end; ++it)
	{
		CPlayer  *loopPlr = static_cast< CPlayer* >( gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(*it) );
		OnPlayerScoringEventWithInfo(loopPlr->GetEntityId(), scoreInfo);
	}
}

void CGameRulesStandardScoring::OnTeamScoringEvent( i32 teamId, EGRST pointsType)
{
	DRX_ASSERT(pointsType != EGRST_Unknown);

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(ShouldScore(pGameRules))
	{
		bool bTeamGame = (pGameRules->GetTeamCount() > 1);
		if (bTeamGame)
		{
			i32 teamPoints = GetTeamPointsByType(pointsType);
			if (teamPoints)
			{
				i32 teamScore = pGameRules->GetTeamsScore(teamId) + teamPoints;

				if (m_maxTeamScore)
				{
					if (m_useScoreAsTime)
					{
						float gameTime = pGameRules->GetCurrentGameTime();
						i32 maxActualScore = (i32)floor(gameTime + m_maxTeamScore);
						if (teamScore > maxActualScore)
						{
							teamScore = maxActualScore;
						}
					}
					else
					{
						if (teamScore > m_maxTeamScore)
						{
							teamScore = m_maxTeamScore;
						}
					}
				}
				pGameRules->SetTeamsScore(teamId, teamScore);

				CStatsRecordingMgr* pRecordingMgr = g_pGame->GetStatsRecorder();
				if (pRecordingMgr)
				{
					pRecordingMgr->OnTeamScore(teamId, teamPoints, pointsType);
				}
			}
		}
	}
}

void CGameRulesStandardScoring::SvResetTeamScore(i32 teamId)
{
	DRX_ASSERT(gEnv->bServer);

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if(ShouldScore(pGameRules))
	{
		bool bTeamGame = (pGameRules->GetTeamCount() > 1);
		DRX_ASSERT_MESSAGE(bTeamGame, "we can't reset team score in a non-team game");
		if (bTeamGame)
		{
			pGameRules->SetTeamsScore(teamId, 0);
		}
	}
}

// IGameRulesScoringModule
void CGameRulesStandardScoring::SvSetDeathScoringModifier(TGameRulesScoreInt inModifier)
{
	DRX_ASSERT(gEnv->bServer);

	if (inModifier != m_deathScoringModifier)
	{
		m_deathScoringModifier = inModifier; 
		CGameRules *pGameRules = g_pGame->GetGameRules();
		CHANGED_NETWORK_STATE(pGameRules, STANDARD_SCORING_STATE_ASPECT);
	}
}
