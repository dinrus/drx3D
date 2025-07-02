// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SIMPLEHTTPSERVERLISTENER_H__
#define __SIMPLEHTTPSERVERLISTENER_H__

#pragma once

class CSimpleHttpServerListener : public IHttpServerListener, public IOutputPrintSink
{
public:
	static CSimpleHttpServerListener& GetSingleton(ISimpleHttpServer* http_server);
	static CSimpleHttpServerListener& GetSingleton();

	void                              Update();

private:
	CSimpleHttpServerListener();
	~CSimpleHttpServerListener();

	void OnStartResult(bool started, EResultDesc desc);

	void OnClientConnected(i32 connectionID, string client);
	void OnClientDisconnected(i32 connectionID);

	void OnGetRequest(i32 connectionID, string url);
	void OnRpcRequest(i32 connectionID, string xml);

	string m_output;
	void Print(tukk inszText);

	typedef std::deque<string> TCommandsVec;
	TCommandsVec m_commands;

	enum EAuthorizationState {eAS_Disconnected, eAS_WaitChallengeRequest, eAS_WaitAuthenticationRequest, eAS_Authorized};
	EAuthorizationState              m_state;
	i32                              m_connectionID;

	string                           m_client; // current session client
	string                           m_challenge;

	static CSimpleHttpServerListener s_singleton;
	static ISimpleHttpServer*        s_http_server;
};

#endif
