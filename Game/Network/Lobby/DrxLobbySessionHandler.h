// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: DrxLobby session handler implementation.

-------------------------------------------------------------------------
История:
- 08:12:2009 : Created By Ben Johnson

*************************************************************************/
#ifndef __DRXLOBBY_SESSION_HANDLER_H__
#define __DRXLOBBY_SESSION_HANDLER_H__

#include <IGameSessionHandler.h>
#include <drx3D/CoreX/Lobby/IDrxMatchMaking.h>

class CDrxLobbySessionHandler : public IGameSessionHandler
{
public:
	CDrxLobbySessionHandler();
	virtual ~CDrxLobbySessionHandler();

	// IGameSessionHandler
	virtual bool ShouldCallMapCommand(tukk pLevelName, tukk pGameRules);
	virtual void JoinSessionFromConsole(DrxSessionID session);
	virtual void LeaveSession();
	
	virtual i32 StartSession();
	virtual i32 EndSession();

	virtual void OnUserQuit();
	virtual void OnGameShutdown();

	virtual DrxSessionHandle GetGameSessionHandle() const;
	virtual bool ShouldMigrateNub() const;

	virtual bool IsMultiplayer() const;
	virtual i32 GetNumberOfExpectedClients();

	virtual bool IsGameSessionMigrating() const;
	virtual bool IsMidGameLeaving() const;
	virtual bool IsGameSessionMigratable() const;
	// ~IGameSessionHandler

protected:
	bool m_userQuit;
};

#endif //__DRXLOBBY_SESSION_HANDLER_H__
