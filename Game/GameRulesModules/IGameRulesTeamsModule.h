// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 02:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesTeamsModule_h_
#define _GameRulesTeamsModule_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Network/SerializeFwd.h>
#include <drx3D/Game/IGameObject.h>

class IGameRulesTeamsModule
{
public:
	virtual ~IGameRulesTeamsModule() {}

	virtual void	Init(XmlNodeRef xml) = 0;
	virtual void	PostInit() = 0;

	virtual void	RequestChangeTeam(EntityId playerId, i32 teamId, bool onlyIfUnassigned) = 0;

	virtual i32		GetAutoAssignTeamId(EntityId playerId)	= 0;

	virtual bool	CanTeamModifyWeapons(i32 teamId)				= 0;
};

#endif // _GameRulesTeamsModule_h_