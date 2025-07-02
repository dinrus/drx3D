// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a class that receives events when entities change
		teams
	-------------------------------------------------------------------------
	История:
	- 30:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _IGAME_RULES_TEAM_CHANGED_LISTENER_H_
#define _IGAME_RULES_TEAM_CHANGED_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

class IGameRulesTeamChangedListener
{
public:
	virtual ~IGameRulesTeamChangedListener() {}

	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId) = 0;
};

#endif // _IGAME_RULES_TEAM_CHANGED_LISTENER_H_
