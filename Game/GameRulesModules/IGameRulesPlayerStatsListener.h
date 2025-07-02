// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a class that receives events when playerstats change
	-------------------------------------------------------------------------
	История:
	- 18:11:2009  : Created by Thomas Houghton

*************************************************************************/

#ifndef _IGAME_RULES_PLAYERSTATS_LISTENER_H_
#define _IGAME_RULES_PLAYERSTATS_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

struct SGameRulesPlayerStat;

class IGameRulesPlayerStatsListener
{
public:
	virtual ~IGameRulesPlayerStatsListener() {}

	virtual void ClPlayerStatsNetSerializeReadDeath(const SGameRulesPlayerStat* s, u16 prevDeathsThisRound, u8 prevFlags) = 0;

};

#endif // _IGAME_RULES_PLAYERSTATS_LISTENER_H_
