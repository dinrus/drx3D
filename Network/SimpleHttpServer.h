// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SIMPLEHTTPSERVER_H__
#define __SIMPLEHTTPSERVER_H__

#pragma once

#include <drx3D/Network/ISimpleHttpServer.h>
#include <vector>

#define WEBSOCKET_EXTRA_HEADER_SPACE 10

static const string USERNAME = "anonymous";
static i32k MAX_HTTP_CONNECTIONS = 16;                  // arbitrary figure to prevent bugs causing things to grow out of control
static i32k WEBSOCKET_MAX_MESSAGE_SIZE = (1000 * 1024); // arbitrary maximum message size for all websocket payloads in a sequence

class CSimpleHttpServer;

class CSimpleHttpServerInternal : public IStreamListener
{
	friend class CSimpleHttpServer;

	static i32k HTTP_CONNECTIONS_RESERVE = 4;

public:
	void Start(u16 port, string password);
	void Stop();
	void SendResponse(i32 connectionID, ISimpleHttpServer::EStatusCode statusCode, ISimpleHttpServer::EContentType contentType, string content, bool closeConnection);
	void SendWebpage(i32 connectionID, string webpage);

#ifdef HTTP_WEBSOCKETS
	void AddWebsocketProtocol(string protocolName, IHttpWebsocketProtocol* pProtocol);
	void SendToWebsocket(i32 connectionID, IHttpWebsocketProtocol::EMessageType type, tuk pBuffer, u32 bufferSize, bool closeConnection);
	void OnWebsocketUpgrade(i32 connectionID, IHttpWebsocketProtocol* pProtocol);
	void OnWebsocketReceive(i32 connectionID, IHttpWebsocketProtocol* pProtocol, IHttpWebsocketProtocol::SMessageData data);
	void OnWebsocketClose(i32 connectionID, IHttpWebsocketProtocol* pProtocol, bool bGraceful);
#endif

	void OnConnectionAccepted(IStreamSocketPtr pStreamSocket, uk pUserData);
	void OnConnectCompleted(bool succeeded, uk pUserData) { NET_ASSERT(0); }
	void OnConnectionClosed(bool graceful, uk pUserData);
	void OnIncomingData(u8k* pData, size_t nSize, uk pUserData);

	void AddRef() const  {}
	void Release() const {}
	bool IsDead() const  { return false; }

private:
	enum ESessionState {eSS_Unsessioned, eSS_WaitFirstRequest, eSS_ChallengeSent, eSS_Authorized, eSS_IgnorePending};
	enum EReceivingState {eRS_ReceivingHeaders, eRS_ReceivingContent, eRS_Ignoring, eRS_Websocket};
	enum ERequestType {eRT_None, eRT_Get, eRT_Rpc, eRT_WebsocketUpgrade};

#ifdef HTTP_WEBSOCKETS
	std::map<string, IHttpWebsocketProtocol*> m_pProtocolHandlers;

	enum EWsOpCode { eWSOC_Continuation = 0x0, eWSOC_Text = 0x1, eWSOC_Binary = 0x2, eWSOC_ConnectionClose = 0x8, eWSOC_Ping = 0x9, eWSOC_Pong = 0xa, eWSOC_Max = 0x10 };
	static const string s_WsOpCodes[eWSOC_Max];

	enum EWsConnectState { eWSCS_RecvHeader, eWSCS_RecvExtendedSize, eWSCS_RecvMask, eWSCS_RecvPayload, eWSCS_SendGracefulClose, eWSCS_Invalid };
	static const string s_WsConnectStates[eWSCS_Invalid];
	static const string s_WsGUID;

	#pragma pack(push, 1)
	#if !defined(NEED_ENDIAN_SWAP)
	struct WsDataHeader
	{
		u8 opcode : 4;
		u8 rsv3   : 1;
		u8 rsv2   : 1;
		u8 rsv1   : 1;
		u8 fin    : 1;
		u8 len    : 7;
		u8 mask   : 1;
	};
	#else
	struct WsDataHeader
	{
		u8 fin    : 1;
		u8 rsv1   : 1;
		u8 rsv2   : 1;
		u8 rsv3   : 1;
		u8 opcode : 4;
		u8 mask   : 1;
		u8 len    : 7;
	};
	#endif
	#pragma pack(pop)

	void ProcessIncomingWebSocketData(i32 connectionID, u8k* pData, size_t nSize);
	bool WebsocketUpgradeResponse(i32 connectionID);

#endif

	struct HttpConnection
	{
		HttpConnection(i32 ID);

		i32              m_ID;

		IStreamSocketPtr m_pSocketSession;

		TNetAddress      m_remoteAddr;

		ESessionState    m_sessionState;
		EReceivingState  m_receivingState;
		ERequestType     m_requestType;

		size_t           m_remainingContentBytes;

		float            m_authenticationTimeoutStart;
		float            m_lineReadingTimeoutStart;

		string           m_line, m_content, m_uri;

#if defined(HTTP_WEBSOCKETS)
		IHttpWebsocketProtocol* m_websocketProtocol;
		string                  m_websocketKey;

		struct WsReadChunk
		{
			WsReadChunk(u8k* pData, size_t nSize);

			u8* m_pBuffer;
			uint64 m_bufferSize;
			uint64 m_readOffset;
		};

		typedef std::vector<WsReadChunk> TWsReadChunkList;
		TWsReadChunkList m_WsReadChunks;

		EWsConnectState  m_WsConnectState;

		WsDataHeader     m_WsRecvCurrentHeader;
		uint64           m_WsRecvCurrentPayloadSize;
		char             m_WsRecvCurrentMask[4];

		typedef std::vector<char> TWsRecvMessageBuffer;
		TWsRecvMessageBuffer                 m_WsRecvCombinedMessageBuffer;
		IHttpWebsocketProtocol::EMessageType m_WsRecvCombinedMessageType;

		void ClearAllWebsocketData();
		void ClearWebsocketProcessingState();

		bool AddWsReadChunk(u8k* pBuffer, size_t nSize);
		bool CollateWsReadChunks(tuk pBuffer, uint64 bufferSize);
#endif
	};

	CSimpleHttpServerInternal(CSimpleHttpServer* pServer);
	~CSimpleHttpServerInternal();

	void            GetBindAddress(TNetAddress& addr, u16 port) const;
	i32             GetNewConnectionID() { return m_connectionID++; }
	HttpConnection* GetConnection(i32 connectionID);

	void            UpdateClosedConnections();
	void            CloseHttpConnection(i32 connectionID, bool bGraceful);

	void            BuildNormalResponse(i32 connectionID, string& response, ISimpleHttpServer::EStatusCode statusCode, ISimpleHttpServer::EContentType contentType, const string& content);
	void            UnauthorizedResponse(i32 connectionID, string& response);
	void            BadRequestResponse(i32 connectionID); // helper
	void            NotImplementedResponse(i32 connectionID);

	string          NextToken(string& line) const;
	bool            ParseHeaderLine(i32 connectionID, string& line);
	void            IgnorePending(i32 connectionID);

	bool            ParseDigestAuthorization(i32 connectionID, string& line);

	CSimpleHttpServer*          m_pServer;

	IStreamSocketPtr            m_pSocketListen;

	string                      m_password;
	string                      m_hostname;

	string                      m_realm;
	string                      m_nonce;
	string                      m_opaque;

	DrxMutex                    m_mutex;

	std::vector<IStreamSocketPtr> m_closedConnections;
	std::vector<HttpConnection> m_connections;
	i32                         m_connectionID;

	static const string         s_statusCodes[];
	static const string         s_contentTypes[];
};

class CSimpleHttpServer : public ISimpleHttpServer
{
	friend class CSimpleHttpServerInternal;

public:
	static CSimpleHttpServer& GetSingleton();

#ifdef HTTP_WEBSOCKETS
	void  AddWebsocketProtocol(const string& protocolName, IHttpWebsocketProtocol* pProtocol);
	void  SendWebsocketData(i32 connectionID, IHttpWebsocketProtocol::SMessageData& data, bool closeConnection);

	uk AllocateHeapBuffer(u32 bufferSize);
	void  FreeHeapBuffer(uk pBuffer);
#endif

	void Start(u16 port, const string& password, IHttpServerListener* pListener);
	void Stop();
	void Quit();
	void Tick();
	void SendResponse(i32 connectionID, EStatusCode statusCode, EContentType contentType, const string& content, bool closeConnection);
	void SendWebpage(i32 connectionID, const string& webpage);

private:
	CSimpleHttpServer();
	~CSimpleHttpServer();

	CSimpleHttpServerInternal m_internal;

	IHttpServerListener*      m_pListener;

#if defined(HTTP_WEBSOCKETS)
	static i32k    HEAP_RESERVE_SIZE = 2048;
	static i32k    HEAP_MAX_SIZE = 4 * 1024 * 1024;
	IGeneralMemoryHeap* m_pWsAllocHeap;
#endif

	static CSimpleHttpServer s_singleton;
};

#endif
