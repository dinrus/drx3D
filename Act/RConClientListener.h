// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: IRemoteControlClientListener implementation
   -------------------------------------------------------------------------
   История:
   - Created by Lin Luo, November 09, 2006
*************************************************************************/

#ifndef __RCONCLIENTLISTENER_H__
#define __RCONCLIENTLISTENER_H__

#include <drx3D/Network/IRemoteControl.h>

#pragma once

class CRConClientListener : public IRemoteControlClientListener
{
public:
	static CRConClientListener& GetSingleton(IRemoteControlClient* rcon_client);

	void                        OnConnectResult(bool okay, EResultDesc desc);

	void                        OnSessionStatus(bool connected, EStatusDesc desc);

	void                        OnCommandResult(u32 commandId, string command, string result);

	bool                        IsSessionAuthorized() const;

private:
	CRConClientListener();
	~CRConClientListener();

	bool                         m_sessionAuthorized;

	static CRConClientListener   s_singleton;

	static IRemoteControlClient* s_rcon_client;
};

#endif
