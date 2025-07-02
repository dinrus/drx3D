// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Network/ISimpleHttpServer.h>

#include <drx3D/Act/SimpleHttpServerWebsocketEchoListener.h>

#if defined(HTTP_WEBSOCKETS)

CSimpleHttpServerWebsocketEchoListener CSimpleHttpServerWebsocketEchoListener::s_singleton;

CSimpleHttpServerWebsocketEchoListener& CSimpleHttpServerWebsocketEchoListener::GetSingleton()
{
	return s_singleton;
}

void CSimpleHttpServerWebsocketEchoListener::OnUpgrade(i32 connectionID)
{
	DrxLogAlways("CSimpleHttpServerWebsocketEchoListener::OnUpgrade: connection %d upgraded to websocket!", connectionID);

	SMessageData data;
	data.m_eType = eMT_Text;
	data.m_pBuffer = "Upgraded connection to Websocket.\n";
	data.m_bufferSize = 35;

	gEnv->pNetwork->GetSimpleHttpServerSingleton()->SendWebsocketData(connectionID, data, false);
};

void CSimpleHttpServerWebsocketEchoListener::OnReceive(i32 connectionID, SMessageData& data)
{
	// BEWARE! This is for test purposes only!
	// Technically we shouldn't display data.m_pBuffer directly as a string. There is no guarantee that the buffer will contain a null terminator,
	// even if the data.type is set to eMT_Text.
	// For safety (for exactly this reason), our message handler always appends a null, but does not include in the length of the message.
	// (data.m_pBuffer is actually data.m_bufferSize + 1 in length, with the last char set to null).
	DrxLogAlways("CSimpleHttpServerWebsocketEchoListener::OnReceive: connection %d, MessageType %d: \"%s\"", connectionID, data.m_eType, data.m_pBuffer);

	// ECHO
	// Send the incoming data back to the client
	gEnv->pNetwork->GetSimpleHttpServerSingleton()->SendWebsocketData(connectionID, data, false);
}

void CSimpleHttpServerWebsocketEchoListener::OnClosed(i32 connectionID, bool bGraceful)
{
	DrxLogAlways("CSimpleHttpServerWebsocketEchoListener::OnClosed: connection %d, graceful = %d", connectionID, bGraceful);
}

#endif // HTTP_WEBSOCKETS
