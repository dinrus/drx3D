// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ISIMPLEHTTPSERVER_H__
#define __ISIMPLEHTTPSERVER_H__

#pragma once

#define HTTP_WEBSOCKETS

#ifdef HTTP_WEBSOCKETS
struct IHttpWebsocketProtocol
{
	virtual ~IHttpWebsocketProtocol(){}

	enum EMessageType {eMT_Text, eMT_Binary, eMT_Unknown};

	struct SMessageData
	{
		EMessageType m_eType;
		tuk        m_pBuffer;
		u32       m_bufferSize;
	};

	virtual void OnUpgrade(i32 connectionID) = 0;
	virtual void OnReceive(i32 connectionID, SMessageData& data) = 0;
	virtual void OnClosed(i32 connectionID, bool bGraceful) = 0;

	//! Required by work queueing.
	virtual void AddRef() const  {}
	virtual void Release() const {}
	virtual bool IsDead() const  { return false; }
};
#endif

struct IHttpServerListener
{
	enum EResultDesc {eRD_Okay, eRD_Failed, eRD_AlreadyStarted};
	// <interfuscator:shuffle>
	virtual ~IHttpServerListener(){}
	virtual void OnStartResult(bool started, EResultDesc desc) = 0;

	virtual void OnClientConnected(i32 connectionID, string client) = 0;
	virtual void OnClientDisconnected(i32 connectionID) = 0;

	virtual void OnGetRequest(i32 connectionID, string url) = 0;
	virtual void OnRpcRequest(i32 connectionID, string xml) = 0;

	//! Required by work queueing.
	virtual void AddRef() const  {}
	virtual void Release() const {}
	virtual bool IsDead() const  { return false; }
	// </interfuscator:shuffle>
};

struct ISimpleHttpServer
{
	enum EStatusCode {eSC_SwitchingProtocols, eSC_Okay, eSC_BadRequest, eSC_NotFound, eSC_RequestTimeout, eSC_NotImplemented, eSC_ServiceUnavailable, eSC_UnsupportedVersion, eSC_InvalidStatus};
	enum EContentType {eCT_HTML, eCT_XML, eCT_TXT, eCT_MAX};
	enum { NoConnectionID = -1 };

	// <interfuscator:shuffle>
	virtual ~ISimpleHttpServer(){}

	//! Starts an HTTP server with a password using Digest Access Authentication method.
	virtual void Start(u16 port, const string& password, IHttpServerListener* pListener) = 0;

	//! Stops the HTTP server.
	virtual void Stop() = 0;
	virtual void SendResponse(i32 connectionID, EStatusCode statusCode, EContentType contentType, const string& content, bool closeConnection = false) = 0;

	virtual void SendWebpage(i32 connectionID, const string& webpage) = 0;
	// </interfuscator:shuffle>

#ifdef HTTP_WEBSOCKETS
	virtual void AddWebsocketProtocol(const string& protocolName, IHttpWebsocketProtocol* pProtocol) = 0;

	//! Sends some data to an established websocket connection. Data is copied and client is free to dispose of it immediately after this call.
	virtual void SendWebsocketData(i32 connectionID, IHttpWebsocketProtocol::SMessageData& data, bool closeConnection) = 0;
#endif
};

#endif
