// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:  Service network implementation
   -------------------------------------------------------------------------
   История:
   - 10/04/2013   : Tomasz Jonarski, Created

*************************************************************************/
#ifndef __CSERVICENETWORK_H__
#define __CSERVICENETWORK_H__

//-----------------------------------------------------------------------------

#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Network/DrxSocks.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CServiceNetwork;

//-----------------------------------------------------------------------------

// General message buffer
class CServiceNetworkMessage : public IServiceNetworkMessage
{
private:
	uk        m_pData;
	u32       m_id;
	u32       m_size;
	i32  m_refCount;

public:
	CServiceNetworkMessage(u32k id, u32k size);
	virtual ~CServiceNetworkMessage();

	// IServiceNetworMessage interface
	virtual u32                  GetId() const;
	virtual u32                  GetSize() const;
	virtual uk                   GetPointer();
	virtual ukk             GetPointer() const;
	virtual struct IDataReadStream* CreateReader() const;
	virtual void                    AddRef();
	virtual void                    Release();
};

//-----------------------------------------------------------------------------

// General network TCP/IP connection
class CServiceNetworkConnection : public IServiceNetworkConnection
{
public:
	friend class CServiceNetworkListener;

	// maximum size of a single message (0.5MB by default)
	static u32k kMaximumMessageSize = 5 << 19;

	// initialization message send period (ms)
	static const uint64 kInitializationPerior = 1000;

	// keep alive period (ms), by default every 2s
	static const uint64 kKeepAlivePeriod = 2000;

	// reconnection retries period (ms)
	static const uint64 kReconnectTryPerior = 1000;

	// timeout for assuming server side connection dead (reconnection timeout)
	static const uint64 hReconnectTimeOut = 30 * 1000;

	// communication time out (ms)
	static const uint64 kTimeout = 5000;

	// Type of endpoint
	enum EEndpoint
	{
		// This is the server side of the connection (on the side of the listening socket)
		eEndpoint_Server,

		// This is the client side of the connection (we connected to the listening socket)
		eEndpoint_Client,
	};

	// Internal state machine
	enum EState
	{
		// Connection is initializing
		eState_Initializing,

		// Connection is valid
		eState_Valid,

		// Operation on the socket failed (we may need to reconnect)
		eState_Lost,

		// Connection is closed
		eState_Closed,
	};

	// Command IDs, do not change the numerical values
	enum ECommand
	{
		// Data block command
		eCommand_Data = 1,

		// Keep alive command
		eCommand_KeepAlive = 2,

		// Initialize communication channel (sent only once)
		eCommand_Initialize = 3,
	};

#pragma pack(push)
#pragma pack(1)

	struct Header
	{
		u8  m_cmd;
		u32 m_size;

		void   Swap();
	};

	struct InitHeader
	{
		u8  m_cmd;
		u8  m_pad0;
		u8  m_pad1;
		u8  m_pad2;
		u32 m_tryCount;
		uint64 m_guid0;
		uint64 m_guid1;

		void   Swap();
	};

#pragma pack(pop)

private:
	CServiceNetwork* m_pUpr;

	// Type of endpoint (client/server)
	EEndpoint m_endpointType;

	// Connection state (internal)
	EState m_state;

	// Reference count (updated using DrxInterlocked* functions)
	i32  m_refCount;

	// Internal socket data
	DRXSOCKET m_socket;

	// Local address
	ServiceNetworkAddress m_localAddress;

	// Remote connection address
	ServiceNetworkAddress m_remoteAddress;

	// Internal connection ID (unique)
	DrxGUID m_connectionID;

	// Internal time counters
	uint64 m_lastReconnectTime;
	uint64 m_lastKeepAliveSendTime;
	uint64 m_lastMessageReceivedTime;
	uint64 m_lastInitializationSendTime;
	u32 m_reconnectTryCount;

	// Statistics (updated from threads using DrxIntelocked* functions)
	 u32 m_statsNumPacketsSend;
	 u32 m_statsNumPacketsReceived;
	 u32 m_statsNumDataSend;
	 u32 m_statsNumDataReceived;

	// Queue of messages to send (thread access possible)
	typedef DrxMT::CLocklessPointerQueue<CServiceNetworkMessage> TSendQueue;
	CServiceNetworkMessage* m_pSendedMessages;
	TSendQueue              m_pSendQueue;
	u32                  m_messageDataSentSoFar;
	 i32            m_sendQueueDataSize;

	// Queue of received message
	typedef DrxMT::CLocklessPointerQueue<CServiceNetworkMessage> TReceiveQueue;
	TReceiveQueue m_pReceiveQueue;
	u32        m_receiveQueueDataSize;
	u32        m_messageDataReceivedSoFar;
	u32        m_messageReceiveLength;

	// Message being received "right now"
	CServiceNetworkMessage* m_pCurrentReceiveMessage;
	u32                  m_messageDummyReadLength;

	// External request to close this connection was issued
	bool m_bCloseRequested;

	// Do not accept any new data for sending or receiving
	bool m_bDisableCommunication;

public:
	ILINE bool IsInitialized() const
	{
		return m_state != eState_Initializing;
	}

	ILINE bool IsSendingQueueEmpty() const
	{
		return m_pSendQueue.empty();
	}

	ILINE CServiceNetwork* GetUpr() const
	{
		return m_pUpr;
	}

public:
	CServiceNetworkConnection(
		class CServiceNetwork* manager,
			EEndpoint endpointType,
			DRXSOCKET socket,
			const DrxGUID& connectionID,
			const ServiceNetworkAddress& localAddress,
			const ServiceNetworkAddress& remoteAddress);

	virtual ~CServiceNetworkConnection();

	// IServiceNetworkConnection interface implementation
	virtual const ServiceNetworkAddress& GetRemoteAddress() const;
	virtual const ServiceNetworkAddress& GetLocalAddress() const;
	virtual const DrxGUID&               GetGUID() const;
	virtual bool                         IsAlive() const;
	virtual u32                       GetMessageSendCount() const;
	virtual u32                       GetMessageReceivedCount() const;
	virtual uint64                       GetMessageSendDataSize() const;
	virtual uint64                       GetMessageReceivedDataSize() const;
	virtual bool                         SendMsg(IServiceNetworkMessage* message);
	virtual IServiceNetworkMessage*      ReceiveMsg();
	virtual void                         FlushAndClose(u32k timeout);
	virtual void                         FlushAndWait();
	virtual void                         Close();
	virtual void                         AddRef();
	virtual void                         Release();

	// All remote connections are updated on the client side
	// This is called from service network update thread, try not to call by hand :)
	void Update();

private:
	void ProcessSendingQueue();
	void ProcessReceivingQueue();

	// Keep alive message handling
	void ProcessKeepAlive();
	void SendKeepAlive(const uint64 currentNetworkTime);
	bool HandleTimeout(const uint64 currentNetworkTime);

	// Handle the reconnection request
	bool HandleReconnect(DRXSOCKET socket, u32k tryCount);

	// General send/receive functions with error handling.
	// If socket error occurs the connection will be put in the lost state.
	u32 TrySend(ukk dataBuffer, u32 dataSize, bool autoHandleErrors);

	// Internal receive function with error handling
	u32 TryReceive(uk dataBuffer, u32 dataSize, bool autoHandleErrors);

	// Try to reconnect to the remote address
	bool TryReconnect();

	// Try to send the initialization header
	bool TryInitialize();

	// Low-level socket shutdown (hash way)
	void Shutdown();

	// Reset the connection (put in the lost state and reconnect)
	void Reset();
};

//-----------------------------------------------------------------------------

// TCP/IP listener
class CServiceNetworkListener : public IServiceNetworkListener
{
	typedef CServiceNetworkConnection::InitHeader TInitHeader;

	struct PendingConnection
	{
		// Connection socket
		DRXSOCKET m_socket;

		// Initialization of initialization header received so far
		u32 m_dataReceivedSoFar;

		// Initialization header
		TInitHeader m_initHeader;

		// Remote address (as returned from accept)
		ServiceNetworkAddress m_remoteAddress;
	};

protected:
	// Owner (the manager)
	CServiceNetwork* m_pUpr;

	// Reference count, updated using DrxInterlocked* functions
	i32  m_refCount;

	// Listening socket
	DRXSOCKET m_socket;

	// Local address (usually has the IP in 127.0.0.1:port form)
	ServiceNetworkAddress m_localAddress;

	// Request to close this listener was received
	bool m_closeRequestReceived;

	// Pending connections (but not yet initialized)
	typedef std::vector<PendingConnection*> TPendingConnectionList;
	TPendingConnectionList m_pPendingConnections;

	// All active connections spawned from this listener
	typedef std::vector<CServiceNetworkConnection*> TConnectionList;
	TConnectionList m_pLocalConnections;

	// Access lock for the class members (thread safe)
	DrxMutex m_accessLock;

public:
	ILINE CServiceNetwork* GetUpr() const
	{
		return m_pUpr;
	}

public:
	CServiceNetworkListener(CServiceNetwork* pUpr, DRXSOCKET socket, const ServiceNetworkAddress& address);
	virtual ~CServiceNetworkListener();

	void Update();

	// IServiceNetworkListener interface implementation
	virtual const ServiceNetworkAddress& GetLocalAddress() const;
	virtual u32                       GetConnectionCount() const;
	virtual IServiceNetworkConnection*   Accept();
	virtual bool                         IsAlive() const;
	virtual void                         AddRef();
	virtual void                         Release();
	virtual void                         Close();

private:
	void ProcessIncomingConnections();
	void ProcessPendingConnections();
};

//-----------------------------------------------------------------------------

// TCP/IP manager for service connection channels
class CServiceNetwork : public IServiceNetwork, public IThread
{
	static u32k kThreadAffinityDurango = 3;

protected:
	struct ConnectionToClose
	{
		CServiceNetworkConnection* pConnection;

		// timeout for forded close
		uint64 maxWaitTime;
	};

protected:
	// Local listeners
	typedef std::vector<CServiceNetworkListener*> TListenerArray;
	TListenerArray m_pListeners;

	// Local connections
	typedef std::vector<CServiceNetworkConnection*> TConnectionArray;
	TConnectionArray m_pConnections;

	// Connections that are waiting for all of their data to be sent before closing
	typedef std::vector<ConnectionToClose> TConnectionsToCloseArray;
	TConnectionsToCloseArray m_connectionsToClose;

	// We are running on threads, needed to sync the access to arrays
	DrxMutex m_accessMutex;

	// Current network time (ms)
	uint64 m_networkTime;

	// Exit was requested
	bool m_bExitRequested;

	// Message verbose level
	ICVar* m_pVerboseLevel;

	// Buffer ID allocator (unique, incremented atomically using DrxInterlockedIncrement)
	 i32 m_bufferID;

	// Random number generator for GUID creation
	CRndGen m_guidGenerator;

	// Send/Receive queue size limit
	ICVar* m_pReceiveDataQueueLimit;
	ICVar* m_pSendDataQueueLimit;

public:
	ILINE const uint64 GetNetworkTime() const
	{
		return m_networkTime;
	}

	ILINE const CServiceNetwork* GetUpr() const
	{
		return this;
	}

	ILINE u32k GetReceivedDataQueueLimit() const
	{
		return m_pReceiveDataQueueLimit->GetIVal();
	}

	ILINE u32k GetSendDataQueueLimit() const
	{
		return m_pSendDataQueueLimit->GetIVal();
	}

public:
	CServiceNetwork();
	virtual ~CServiceNetwork();

	// IServiceNetwork interface implementation
	virtual void                       SetVerbosityLevel(u32k level);
	virtual IServiceNetworkMessage*    AllocMessageBuffer(u32k size);
	virtual struct IDataWriteStream*   CreateMessageWriter();
	virtual struct IDataReadStream*    CreateMessageReader(ukk pData, u32k dataSize);
	virtual ServiceNetworkAddress      GetHostAddress(const string& addressString, u16 optionalPort = 0) const;
	virtual IServiceNetworkListener*   CreateListener(u16 localPort);
	virtual IServiceNetworkConnection* Connect(const ServiceNetworkAddress& remoteAddress);

	// IThread
	virtual void ThreadEntry();
	void         SignalStopWork();

	// Register connection in the connection list (thread safe)
	void RegisterConnection(CServiceNetworkConnection& con);

	// Register connection for closing one all of the outgoing messages are sent
	void RegisterForDeferredClose(CServiceNetworkConnection& con, u32k timeout);

	// Debug print
#ifdef RELEASE
	void Log(tukk txt, ...) const        {};
	bool CheckVerbose(u32k level) const { return false; }
#else
	void Log(tukk txt, ...) const;
	bool CheckVerbose(u32k level) const;
#endif
};

//-----------------------------------------------------------------------------

#endif
