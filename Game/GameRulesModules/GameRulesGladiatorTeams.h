// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 09:05:2011  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesGladiatorTeams_h_
#define _GameRulesGladiatorTeams_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesStandardTwoTeams.h>
#include <drx3D/CoreX/String/DrxFixedString.h>

class CGameRulesGladiatorTeams : public CGameRulesStandardTwoTeams
{
public:
	virtual i32		GetAutoAssignTeamId(EntityId playerId) { return 0; }		// Defer team selection for objectives module
	virtual void	RequestChangeTeam(EntityId playerId, i32 teamId, bool onlyIfUnassigned) {};
};

#endif // _GameRulesGladiatorTeams_h_
