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

#ifndef _GameRulesStandardTwoTeams_h_
#define _GameRulesStandardTwoTeams_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameRulesTeamsModule.h>
#include <drx3D/CoreX/String/DrxFixedString.h>

class CGameRules;

class CGameRulesStandardTwoTeams : public IGameRulesTeamsModule
{
public:
	CGameRulesStandardTwoTeams();
	virtual ~CGameRulesStandardTwoTeams();

	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();

	virtual void RequestChangeTeam(EntityId playerId, i32 teamId, bool onlyIfUnassigned);

	virtual i32 GetAutoAssignTeamId(EntityId playerId);

	virtual bool	CanTeamModifyWeapons(i32 teamId);

protected:
	void DoTeamChange(EntityId playerId, i32 teamId);

	CGameRules *m_pGameRules;
	bool m_bCanTeamModifyWeapons[2];
};

#endif // _GameRulesStandardTwoTeams_h_