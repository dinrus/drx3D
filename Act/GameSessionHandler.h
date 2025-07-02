// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Default session handler implementation.

   -------------------------------------------------------------------------
   История:
   - 08:12:2009 : Created By Ben Johnson

*************************************************************************/
#ifndef __GAME_SESSION_HANDLER_H__
#define __GAME_SESSION_HANDLER_H__

#include <drx3D/Act/IGameSessionHandler.h>

class CGameSessionHandler : public IGameSessionHandler
{
public:
	CGameSessionHandler();
	virtual ~CGameSessionHandler();

	// IGameSessionHandler
	virtual bool             ShouldCallMapCommand(tukk pLevelName, tukk pGameRules);
	virtual void             JoinSessionFromConsole(DrxSessionID sessionID);
	virtual i32              EndSession();

	virtual i32              StartSession();
	virtual void             LeaveSession();

	virtual void             OnUserQuit();
	virtual void             OnGameShutdown();

	virtual DrxSessionHandle GetGameSessionHandle() const;
	virtual bool             ShouldMigrateNub() const;

	virtual bool             IsMultiplayer() const;
	virtual i32              GetNumberOfExpectedClients();

	virtual bool             IsGameSessionMigrating() const;
	virtual bool             IsMidGameLeaving() const;
	// ~IGameSessionHandler
};

#endif //__GAME_SESSION_HANDLER_H__
