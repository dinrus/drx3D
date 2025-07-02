// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Interface to handle sessions.

   -------------------------------------------------------------------------
   История:
   - 08:12:2009 : Created By Ben Johnson

*************************************************************************/

#ifndef __I_GAME_SESSION_HANDLER_H__
#define __I_GAME_SESSION_HANDLER_H__

#include <drx3D/CoreX/Lobby/CommonIDrxLobby.h>

struct SGameStartParams;

struct IGameSessionHandler
{
public:
	virtual ~IGameSessionHandler() {}

	virtual bool             ShouldCallMapCommand(tukk pLevelName, tukk pGameRules) = 0;
	virtual void             JoinSessionFromConsole(DrxSessionID session) = 0;
	virtual i32              EndSession() = 0;

	virtual i32              StartSession() = 0;
	virtual void             LeaveSession() = 0;

	virtual void             OnUserQuit() = 0;
	virtual void             OnGameShutdown() = 0;

	virtual DrxSessionHandle GetGameSessionHandle() const = 0;
	virtual bool             ShouldMigrateNub() const = 0;

	virtual bool             IsMultiplayer() const = 0;

	virtual i32              GetNumberOfExpectedClients() = 0;

	virtual bool             IsGameSessionMigrating() const = 0;
	virtual bool             IsMidGameLeaving() const = 0;
};

#endif //__I_GAME_SESSION_HANDLER_H__
