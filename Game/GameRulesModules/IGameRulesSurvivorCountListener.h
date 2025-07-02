// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a class that receives events about the number of players
		remaining in lives-limited game modes
	-------------------------------------------------------------------------
	История:
	- 06:11:2009  : Created by Thomas Houghton

*************************************************************************/

#ifndef _IGAME_RULES_SURVIVOR_COUNT_LISTENER_H_
#define _IGAME_RULES_SURVIVOR_COUNT_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

class IGameRulesSurvivorCountListener
{
public:
	virtual ~IGameRulesSurvivorCountListener() {}

	virtual void SvSurvivorCountRefresh(i32 count, const EntityId survivors[], i32 numKills) = 0;
};

#endif // _IGAME_RULES_SURVIVOR_COUNT_LISTENER_H_
