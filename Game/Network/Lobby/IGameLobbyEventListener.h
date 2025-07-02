// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for Game Lobby events

	-------------------------------------------------------------------------
	История:
	- 06:09:2012  : Created by Jim Bamford

*************************************************************************/

#ifndef _IGAMELOBBYEVENTLISTENER_H_
#define _IGAMELOBBYEVENTLISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/CoreX/Lobby/IDrxLobbyUI.h>

class IGameLobbyEventListener
{
public:
	virtual ~IGameLobbyEventListener() {}

	virtual void InsertedUser(DrxUserID userId, tukk userName) = 0;
	virtual void SessionChanged(const DrxSessionHandle inOldSession, const DrxSessionHandle inNewSession) = 0;
};

#endif // _IGAMELOBBYEVENTLISTENER_H_