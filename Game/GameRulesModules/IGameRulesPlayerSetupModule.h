// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 07:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesPlayerSetupModule_h_
#define _GameRulesPlayerSetupModule_h_

#if _MSC_VER > 1000
# pragma once
#endif


class IGameRulesPlayerSetupModule
{
public:
	virtual ~IGameRulesPlayerSetupModule() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void PostInit() = 0;

	virtual void OnClientConnect(i32 channelId, bool isReset, tukk playerName, bool isSpectator) = 0;
	virtual void OnPlayerRevived(EntityId playerId) = 0;
	virtual void OnActorJoinedFromSpectate(IActor* pActor, i32 channelId) = 0;

	virtual void SvOnStartNewRound(bool isReset) = 0;
};

#endif // _GameRulesPlayerSetupModule_h_