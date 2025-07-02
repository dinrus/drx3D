// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: IRemoteControlServerListener implementation
   -------------------------------------------------------------------------
   История:
   - Created by Lin Luo, November 08, 2006
   - Refactored by Lin Luo, November 22, 2006
*************************************************************************/

#ifndef __RCONSERVERLISTENER_H__
#define __RCONSERVERLISTENER_H__

#pragma once

class CRConServerListener : public IRemoteControlServerListener, public IOutputPrintSink
{
public:
	static CRConServerListener& GetSingleton(IRemoteControlServer* rcon_server);
	static CRConServerListener& GetSingleton();

	void                        Update();

private:
	CRConServerListener();
	~CRConServerListener();

	void OnStartResult(bool started, EResultDesc desc);

	void OnClientAuthorized(string clientAddr);

	void OnAuthorizedClientLeft(string clientAddr);

	void OnClientCommand(u32 commandId, string command);

	string m_output;
	void Print(tukk inszText);

	typedef std::map<u32, string> TCommandsMap;
	TCommandsMap                 m_commands;

	static CRConServerListener   s_singleton;
	static IRemoteControlServer* s_rcon_server;
};

#endif
