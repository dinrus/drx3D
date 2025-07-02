// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 
Interface for a class that received events when clients score
-------------------------------------------------------------------------
История:
- 12:02:2010  : Created by Ben Parbury

*************************************************************************/

#ifndef _IGAME_RULES_CLIENT_SCORE_LISTENER_H_
#define _IGAME_RULES_CLIENT_SCORE_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesTypes.h>

class IGameRulesClientScoreListener
{
public:
	virtual ~IGameRulesClientScoreListener() {}

	virtual void ClientScoreEvent(EGameRulesScoreType type, i32 points, EXPReason inReason, i32 currentTeamScore) = 0;
};

#endif // _IGAME_RULES_CLIENT_SCORE_LISTENER_H_
