// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Game rules module to handle victory conditions for extraction game mode
	-------------------------------------------------------------------------
	История:
	- 14/05/2009  : Created by Jim Bamford

*************************************************************************/

#ifndef _GAME_RULES_EXTRACTION_VICTORY_CONDITIONS_H_
#define _GAME_RULES_EXTRACTION_VICTORY_CONDITIONS_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesStandardVictoryConditionsTeam.h>

class CGameRules;

class CGameRulesExtractionVictoryConditions : public CGameRulesStandardVictoryConditionsTeam
{
public:

	// IGameRulesVictoryConditionsModule
	virtual void	Update(float frameTime);
	// ~IGameRulesVictoryConditionsModule

	virtual void  TeamCompletedAnObjective(i32 teamId);

protected:

	void					UpdateResolutionData(i32 primaryTeam);
	void					CheckObjectives();
	virtual void	TimeLimitExpired();
};

#endif // _GAME_RULES_EXTRACTION_VICTORY_CONDITIONS_H_