// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 02:09:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesStandardTwoTeams.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/IGameRulesRoundsModule.h>

#define STANDARDTWOTEAMS_AUTO_BALANCE_TIME				10.f
#define STANDARDTWOTEAMS_AUTO_BALANCE_WARNING_TIME		5.f

#if NUM_ASPECTS > 8
	#define STANDARD_TEAMS_ASPECT			eEA_GameServerB
#else
	#define STANDARD_TEAMS_ASPECT			eEA_GameServerStatic
#endif

//-------------------------------------------------------------------------
CGameRulesStandardTwoTeams::CGameRulesStandardTwoTeams()
{
	DrxLog("CGameRulesStandardTwoTeams::CGameRulesStandardTwoTeams()");

	m_pGameRules = NULL;

	m_bCanTeamModifyWeapons[0] = true;
	m_bCanTeamModifyWeapons[1] = true;
}

//-------------------------------------------------------------------------
CGameRulesStandardTwoTeams::~CGameRulesStandardTwoTeams()
{
	DrxLog("CGameRulesStandardTwoTeams::~CGameRulesStandardTwoTeams()");
}

//-------------------------------------------------------------------------
void CGameRulesStandardTwoTeams::Init( XmlNodeRef xml )
{
	DrxLog("CGameRulesStandardTwoTeams::Init()");
	m_pGameRules = g_pGame->GetGameRules();

	m_pGameRules->CreateTeam("tan");
	m_pGameRules->CreateTeam("black");

	m_pGameRules->CreateTeamAlias("marines", 1);
	m_pGameRules->CreateTeamAlias("cell", 2);
	
	m_pGameRules->CreateTeamAlias("1", 1);
	m_pGameRules->CreateTeamAlias("2", 2);

	xml->getAttr("canTeam1ModifyWeapons", m_bCanTeamModifyWeapons[0]);
	xml->getAttr("canTeam2ModifyWeapons", m_bCanTeamModifyWeapons[1]);
}

//-------------------------------------------------------------------------
void CGameRulesStandardTwoTeams::PostInit()
{
	DrxLog("CGameRulesStandardTwoTeams::PostInit()");
}

//-------------------------------------------------------------------------
void CGameRulesStandardTwoTeams::RequestChangeTeam(EntityId playerId, i32 teamId, bool onlyIfUnassigned)
{
	DrxLog("CGameRulesStandardTwoTeams::RequestChangeTeam()");

	if (teamId == 0) // auto pick
	{
		teamId = GetAutoAssignTeamId(playerId);
	}

	i32 currentTeam = m_pGameRules->GetTeam(playerId);

	if (onlyIfUnassigned && currentTeam!=0)
	{
		DrxLog("Player %d already on team %d, onlyIfUnassigned is set", (i32)playerId, teamId);
		return;
	}

	if (currentTeam == teamId)
	{
		DrxLog("Player %d already on team %d, not changing", (i32)playerId, teamId);
		return;
	}

	i32 maxTeamPlayers = 6;
	if (ICVar* pMaxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers"))
	{
		i32 maxPlayers = pMaxPlayers->GetIVal();
		maxTeamPlayers = (maxPlayers>0) ? maxPlayers / 2 : 1;
	}

	i32 newTeamPlayerCount = m_pGameRules->GetTeamPlayerCount(teamId) + 1;			// Include player switching team
	if ((g_pGameCVars->g_autoAssignTeams == 1) && (currentTeam != 0))
	{
		// If we're switching teams due to them being unbalanced then we need to make sure the game doesn't become further unbalanced
		maxTeamPlayers = m_pGameRules->GetTeamPlayerCount(currentTeam, false);
	}

	if (newTeamPlayerCount > maxTeamPlayers)
	{
		DrxLog("Team %i is full, cannot switch player id %d", teamId, (i32)playerId);
		m_pGameRules->GetGameObject()->InvokeRMI(CGameRules::ClTeamFull(), CGameRules::UInt8Param(teamId), eRMI_ToClientChannel, m_pGameRules->GetChannelId(playerId));

		return;
	}

	DoTeamChange(playerId, teamId);
}

//-------------------------------------------------------------------------
void CGameRulesStandardTwoTeams::DoTeamChange(EntityId playerId, i32 teamId)
{
	// Change team
	IActor *pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId);
	if (pActor != NULL && pActor->IsPlayer())
	{
		CPlayer *pPlayer = static_cast<CPlayer *>(pActor);
		if (pPlayer->IsDead()==false)
		{
			HitInfo hitInfo;
			hitInfo.shooterId = playerId;
			hitInfo.targetId = playerId;
			hitInfo.weaponId = 0;
			hitInfo.damage = 1000.0f;
			hitInfo.partId = -1;
			hitInfo.type = CGameRules::EHitType::Normal;
			m_pGameRules->KillPlayer(pActor, true, true, hitInfo);
			m_pGameRules->PostHitKillCleanup(pPlayer);
		}
	}
	m_pGameRules->SetTeam(teamId, playerId);
}

//-------------------------------------------------------------------------
i32 CGameRulesStandardTwoTeams::GetAutoAssignTeamId(EntityId playerId)
{
	i32 team1Players = m_pGameRules->GetTeamPlayerCount(1);
	i32 team2Players = m_pGameRules->GetTeamPlayerCount(2);

	// If already on a team, don't count towards
	i32 currentTeam = m_pGameRules->GetTeam(playerId);
	if (currentTeam==1)
	{
		team1Players -= 1;
	}
	else if (currentTeam==2)
	{
		team2Players -= 1;
	}

	if (currentTeam && team1Players==team2Players)
	{
		return currentTeam;
	}
	if (team1Players > team2Players)
	{
		return 2;
	}
	return 1;
}

bool CGameRulesStandardTwoTeams::CanTeamModifyWeapons( i32 teamId )
{
	if(IGameRulesRoundsModule* pRounds = m_pGameRules->GetRoundsModule())
	{
		teamId = teamId==pRounds->GetPrimaryTeam() ? 0 : 1;
	}
	else
	{
		teamId = max(min(teamId-1,1),0);
	}
	return m_bCanTeamModifyWeapons[teamId];
}
