// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface that allows gamerules modules to receive and send RMIs
	-------------------------------------------------------------------------
	История:
	- 23:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _IGAME_RULES_MODULE_RMI_LISTENER_H_
#define _IGAME_RULES_MODULE_RMI_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRules.h>

class IGameRulesModuleRMIListener
{
public:
	virtual ~IGameRulesModuleRMIListener() {}

	virtual void OnSingleEntityRMI(CGameRules::SModuleRMIEntityParams params) = 0;
	virtual void OnDoubleEntityRMI(CGameRules::SModuleRMITwoEntityParams params) = 0;
	virtual void OnEntityWithTimeRMI(CGameRules::SModuleRMIEntityTimeParams params) = 0;

	virtual void OnSvClientActionRMI(CGameRules::SModuleRMISvClientActionParams params, EntityId fromEid) = 0;
};

#endif // _IGAME_RULES_MODULE_RMI_LISTENER_H_
