// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a class that receives events when clients connect or
		disconnect
	-------------------------------------------------------------------------
	История:
	- 25:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _IGAME_RULES_CLIENT_CONNECTION_LISTENER_H_
#define _IGAME_RULES_CLIENT_CONNECTION_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

class IGameRulesClientConnectionListener
{
public:
	virtual ~IGameRulesClientConnectionListener() {}

	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) = 0;
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId) = 0;
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) = 0;
	virtual void OnOwnClientEnteredGame() = 0;
};

#endif // _IGAME_RULES_CLIENT_CONNECTION_LISTENER_H_
