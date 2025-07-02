// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SIMPLEHTTPSERVERWEBSOCKETECHOLISTENER_H__
#define __SIMPLEHTTPSERVERWEBSOCKETECHOLISTENER_H__

#pragma once

#if defined(HTTP_WEBSOCKETS)

class CSimpleHttpServerWebsocketEchoListener : public IHttpWebsocketProtocol
{
public:

	static CSimpleHttpServerWebsocketEchoListener& GetSingleton();

private:

	CSimpleHttpServerWebsocketEchoListener() {};
	~CSimpleHttpServerWebsocketEchoListener() {};

	void OnUpgrade(i32 connectionID);
	void OnReceive(i32 connectionID, SMessageData& data);
	void OnClosed(i32 connectionID, bool bGraceful);

	static CSimpleHttpServerWebsocketEchoListener s_singleton;
};

#endif // HTTP_WEBSOCKETS

#endif // __SIMPLEHTTPSERVERWEBSOCKETECHOLISTENER_H__
