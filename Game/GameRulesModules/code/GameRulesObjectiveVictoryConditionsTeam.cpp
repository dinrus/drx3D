// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Game rules module to handle victory conditions for team games that
		use are won by completing an objective
	-------------------------------------------------------------------------
	История:
	- 24:09:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesObjectiveVictoryConditionsTeam.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/IGameRulesStateModule.h>
#include <drx3D/Game/IGameRulesObjectivesModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>

//-------------------------------------------------------------------------
void CGameRulesObjectiveVictoryConditionsTeam::Update( float frameTime )
{
	IGameRulesStateModule *pStateModule = m_pGameRules->GetStateModule();
	if (pStateModule != NULL && pStateModule->GetGameState() != IGameRulesStateModule::EGRS_InGame)
	{
		return;
	}

	IGameRulesRoundsModule* pRoundsModule=m_pGameRules->GetRoundsModule();
	if (pRoundsModule != NULL && !pRoundsModule->IsInProgress())
	{
		return;
	}

	CGameRulesStandardVictoryConditionsTeam::UpdateTimeLimitSounds();

	if (!gEnv->bServer)
	{
		return;
	}
	
	DebugDrawResolutions();

	SvCheckOpponents();

	if (!CheckObjectives())		// Check objectives first, if the game isn't over then check the time limit
	{
		CheckTimeLimit();
	}
}

//-------------------------------------------------------------------------
bool CGameRulesObjectiveVictoryConditionsTeam::CheckObjectives()
{
	IGameRulesObjectivesModule *pObjectivesModule = m_pGameRules->GetObjectivesModule();
	if (pObjectivesModule)
	{
		bool team1HasCompleted = pObjectivesModule->HasCompleted(1);
		bool team2HasCompleted = pObjectivesModule->HasCompleted(2);

		if (team1HasCompleted && team2HasCompleted)
		{
			OnEndGame(0, EGOR_ObjectivesCompleted);
			return true;
		}
		else if (team1HasCompleted || team2HasCompleted)
		{
			const float  fTimeTaken = m_pGameRules->GetCurrentGameTime();

			if (team1HasCompleted)
			{
				AddFloatsToDrawResolutionData(ESVC_DrawResolution_level_2, fTimeTaken, 0.f);

				OnEndGame(1, EGOR_ObjectivesCompleted);
				return true;
			}
			else
			{
				DRX_ASSERT(team2HasCompleted);

				AddFloatsToDrawResolutionData(ESVC_DrawResolution_level_2, 0.f, fTimeTaken);

				OnEndGame(2, EGOR_ObjectivesCompleted);
				return true;
			}
		}
	}
	return false;
}
