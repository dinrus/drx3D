// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
-------------------------------------------------------------------------
Имя файла:   GameCodeCoverageGUI.cpp
$Id$
Описание: 

-------------------------------------------------------------------------
История:
- ?
- 2 Mar 2009	: Evgeny Adamenkov: Removed parameter of type IRenderer from DebugDraw

*********************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageGUI.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageUpr.h>
#include <drx3D/Game/Utility/DrxWatch.h>

#if ENABLE_GAME_CODE_COVERAGE

CGameCodeCoverageGUI * CGameCodeCoverageGUI::s_instance = NULL;

CGameCodeCoverageGUI::CGameCodeCoverageGUI(void) : REGISTER_GAME_MECHANISM(CGameCodeCoverageGUI)
{
	assert (s_instance == NULL);
	s_instance = this;

	m_showListWhenNumUnhitCheckpointsIs = 10;

	REGISTER_CVAR2("GCC_showListWhenNumUnhitCheckpointsIs", & m_showListWhenNumUnhitCheckpointsIs, m_showListWhenNumUnhitCheckpointsIs, VF_NULL, "Show checkpoint names on-screen when the number of unhit checkpoints gets down to this number");
}

CGameCodeCoverageGUI::~CGameCodeCoverageGUI(void)
{
	assert (s_instance == this);
	s_instance = NULL;

	//GetISystem()->GetIConsole()->UnregisterVariable("GCC_showListWhenNumUnhitCheckpointsIs");
}

void CGameCodeCoverageGUI::Draw()
{
	CGameCodeCoverageUpr * mgr = CGameCodeCoverageUpr::GetInstance();

	DrxWatch ("======================================================================================================");

	if (mgr->IsContextValid())
	{
		i32 iTotal = mgr->GetTotalCheckpointsReadFromFileAndValid();

		if (iTotal > 0)
		{
			i32 iTotalReg = mgr->GetTotalValidCheckpointsHit();
			i32 nPointsLeft = iTotal - iTotalReg;
			float percentageDone = ( iTotal ? iTotalReg / (float)iTotal : -0.0f);

			DrxWatch ("Code coverage %d%% complete! (Hit %d out of %d, %d left)", (i32)(percentageDone * 100.f), iTotalReg, iTotal, nPointsLeft);

			if ((nPointsLeft <= m_showListWhenNumUnhitCheckpointsIs) && (nPointsLeft > 0))
			{
				std::vector<tukk > vecRemaining;
				mgr->GetRemainingCheckpointLabels(vecRemaining);

				for (std::vector<tukk >::iterator it = vecRemaining.begin(); it != vecRemaining.end(); ++it)
				{
					tukk  txt = *it;
					DrxWatch ("  Still need to hit: $8%s", txt);
				}
			}
			else
			{
				for (i32 i = 0; i < mgr->GetNumAutoNamedGroups(); ++ i)
				{
					const CNamedCheckpointGroup * group = mgr->GetAutoNamedCheckpointGroup(i);
					i32 numHit = group->GetNumHit();
					i32 numValid = group->GetNumValidForCurrentGameState();
					if (numHit < numValid)
					{
						const SLabelInfoFromFile * finalCheckpoint = (numHit == numValid - 1) ? group->GetFirstUnhitValidCheckpoint() : NULL;
						DrxWatch ("  Have hit %s%d/%d checkpoints in $7%s$o group%s%s", numHit ? "$3" : "$4", numHit, numValid, group->GetName(), finalCheckpoint ? " - $8" : "", finalCheckpoint ? finalCheckpoint->m_labelName : "");
					}
				}
			}
		}
		else
		{
			DrxWatch ("Not expecting to hit any code coverage checkpoints while the game is in this state!");
		}
	}
	else
	{
		DrxWatch ("Code coverage context is invalid! Check game log file for details...");
	}

	const CGameCodeCoverageUpr::SRecentlyHitList & recentlyHitList = mgr->GetRecentlyHitList();
	for (i32 i = 0; i < recentlyHitList.m_count; ++i)
	{
		DrxWatch ("  Recently hit: $5%s%s", recentlyHitList.m_array[i].pStr, recentlyHitList.m_array[i].bExpected ? "" : " $4UNEXPECTED!");
	}

	DrxWatch ("======================================================================================================");
}

#endif