// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Описание:  Remote command system implementation
   -------------------------------------------------------------------------
   История:
   - 10/04/2013   : Tomasz Jonarski, Created

*************************************************************************/

#ifndef __REMOTECOMMAND_H__
#define __REMOTECOMMAND_H__

//-----------------------------------------------------------------------------

#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Network/IRemoteCommand.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>
#include <drx3D/Network/DrxSocks.h>

class CRemoteCommandUpr;

// Remote command client implementation
class CRemoteCommandClient : public IRemoteCommandClient, public IThread
{
protected:
	//-------------------------------------------------------------

	class Command
	{
	public:
		ILINE IServiceNetworkMessage* GetMessage() const
		{
			return m_pMessage;
		}

		ILINE u32 GetCommandId() const
		{
			return m_id;
		}

	public:
		// Create command data from serializing a remote command object
		static Command* Compile(const IRemoteCommand& cmd, u32k commandId, u32k classId);

		void            AddRef();
		void            Release();

	private:
		Command();
		~Command();

		 i32            m_refCount;
		u32                  m_id;
		tukk             m_szClassName; // debug only
		IServiceNetworkMessage* m_pMessage;
	};

	//-------------------------------------------------------------

	// Local connection reference to command
	// NOTE: pCommand is reference counted from the calling code
	struct CommandRef
	{
		Command* m_pCommand;
		uint64   m_lastSentTime;

		ILINE CommandRef()
			: m_pCommand(NULL)
			, m_lastSentTime(0)
		{}

		ILINE CommandRef(Command* pCommand)
			: m_pCommand(pCommand)
			, m_lastSentTime(0)
		{}

		// Order function for set container (we want to keep the commands sorted by ID)
		static ILINE bool CompareCommandRefs(CommandRef* const& a, CommandRef* const& b)
		{
			return a->m_pCommand->GetCommandId() < b->m_pCommand->GetCommandId();
		}
	};

	//-------------------------------------------------------------

	// Remote server connection wrapper
	class Connection : public IRemoteCommandConnection
	{
		// How many commands we can send upfront before waiting for an ACK
		static u32k kCommandSendLead = 50;

		// How much command data can be merged into a single packet (KB)
		static u32k kCommandMaxMergePacketSize = 1024;

		// Time after which we start resending commands (ms)
		static u32k kCommandResendTime = 2000;

	protected:
		CRemoteCommandUpr* m_pUpr;
		 i32           m_refCount;

		// Connection (from service network layer)
		IServiceNetworkConnection* m_pConnection;

		// Cached address of the remote endpoint
		ServiceNetworkAddress m_remoteAddress;

		// Pending commands, they are kept ed here until they are ACKed as executed by server
		typedef std::vector<CommandRef*> TCommands;
		TCommands m_pCommands;
		DrxMutex  m_commandAccessMutex;

		// A queue of raw messages
		typedef DrxMT::CLocklessPointerQueue<IServiceNetworkMessage> TRawMessageQueue;
		TRawMessageQueue m_pRawMessages;
		DrxMutex         m_rawMessagesMutex;

		// Last command that was ACKed as received by server
		// This is used to synchronize the both ends of the pipeline
		u32 m_lastReceivedCommand;

		// Last command that was ACKed as executed by server
		// This is used to synchronize the both ends of the pipeline
		u32 m_lastExecutedCommand;

	public:
		ILINE CRemoteCommandUpr* GetUpr() const
		{
			return m_pUpr;
		}

	public:
		Connection(CRemoteCommandUpr* pUpr, IServiceNetworkConnection* pConnection, u32 currentCommandId);

		// Add command to sending queue in this connection
		void AddToSendQueue(Command* pCommand);

		// Process the communication, returns false if connection should be deleted
		bool Update();

		// Send the "disconnect" message to the remote side therefore gracefully closing the connection.
		void SendDisconnectMessage();

	public:
		// IRemoteCommandConnection interface implementation
		virtual bool                         IsAlive() const;
		virtual const ServiceNetworkAddress& GetRemoteAddress() const;
		virtual bool                         SendRawMessage(IServiceNetworkMessage* pMessage);
		virtual IServiceNetworkMessage*      ReceiveRawMessage();
		virtual void                         Close(bool bFlushQueueBeforeClosing = false);
		virtual void                         AddRef();
		virtual void                         Release();

	private:
		~Connection();
	};

protected:
	CRemoteCommandUpr* m_pUpr;

	typedef std::vector<Connection*> TConnections;
	TConnections m_pConnections;
	TConnections m_pConnectionsToDelete;
	DrxMutex     m_accessMutex;

	// Local command ID counter, incremented atomically using DrxInterlockedIncrement
	 u32 m_commandId;

	DrxEvent        m_threadEvent;
	bool            m_bCloseThread;

public:
	ILINE CRemoteCommandUpr* GetUpr() const
	{
		return m_pUpr;
	}

public:
	CRemoteCommandClient(CRemoteCommandUpr* pUpr);
	virtual ~CRemoteCommandClient();

	// IRemoteCommandClient interface
	virtual void                      Delete();
	virtual bool                      Schedule(const IRemoteCommand& command);
	virtual IRemoteCommandConnection* ConnectToServer(const class ServiceNetworkAddress& serverAddress);

	// IThread interface implementation
	virtual void ThreadEntry();
	void         SignalStopWork();
};

//-----------------------------------------------------------------------------

// Remote command server implementation
class CRemoteCommandServer : public IRemoteCommandServer, public IThread
{
protected:
	// Wrapped commands
	class WrappedCommand
	{
	private:
		IRemoteCommand* m_pCommand;
		 i32    m_refCount;
		u32          m_commandID;

	public:
		ILINE u32k GetId() const
		{
			return m_commandID;
		}

		ILINE IRemoteCommand* GetCommand() const
		{
			return m_pCommand;
		}

	public:
		WrappedCommand(IRemoteCommand* pCommand, u32k commandId);
		void AddRef();
		void Release();

	private:
		~WrappedCommand();
	};

	// Local endpoint
	class Endpoint
	{
	private:
		IServiceNetworkConnection*  m_pConnection;
		class CRemoteCommandServer* m_pServer;
		CRemoteCommandUpr*      m_pUpr;

		// ACK counters for synchronization
		u32   m_lastReceivedCommand;
		u32   m_lastExecutedCommand;
		u32   m_lastReceivedCommandACKed;
		u32   m_lastExecutedCommandACKed;
		DrxMutex m_accessLock;

		// We have received class list (it's a valid RC connection)
		bool m_bHasReceivedClassList;

		// Locally mapped class id (because IDs on remote side can be different than here)
		typedef std::vector<IRemoteCommandClass*> TLocalClassFactoryList;
		TLocalClassFactoryList m_pLocalClassFactories;

		// Commands that were received and should be executed
		typedef DrxMT::CLocklessPointerQueue<WrappedCommand> TCommandQueue;
		TCommandQueue m_pCommandsToExecute;
		DrxMutex      m_commandListLock;

	public:
		ILINE CRemoteCommandUpr* GetUpr() const
		{
			return m_pUpr;
		}

		// Get the endpoint connection
		ILINE IServiceNetworkConnection* GetConnection() const
		{
			return m_pConnection;
		}

		// Have we received a class list from the client
		ILINE bool HasReceivedClassList() const
		{
			return m_bHasReceivedClassList;
		}

	public:
		Endpoint(CRemoteCommandUpr* pUpr, class CRemoteCommandServer* pServer, IServiceNetworkConnection* pConnection);
		~Endpoint();

		// Execute pending commands (called from main thread)
		void Execute();

		// Update (send/receive, etc) Returns false if endpoint died.
		bool Update();

		// Get the class name as translated by this endpoint (by ID)
		tukk GetClassName(u32k classId) const;

		// Create command object by class ID
		IRemoteCommand* CreateObject(u32k classId) const;
	};

	// Received raw message
	// Beware to use always via pointer to this type since propper reference counting is not implemented for copy and assigment
	struct RawMessage
	{
		// We keep a reference to connection so we know where to send the response
		IServiceNetworkConnection* m_pConnection;
		IServiceNetworkMessage*    m_pMessage;

		ILINE RawMessage(IServiceNetworkConnection* pConnection, IServiceNetworkMessage* pMessage)
			: m_pConnection(pConnection)
			, m_pMessage(pMessage)
		{
			m_pMessage->AddRef();
			m_pConnection->AddRef();
		}

		ILINE ~RawMessage()
		{
			m_pMessage->Release();
			m_pConnection->Release();
		}

	private:
		ILINE RawMessage(const RawMessage& other) {};
		ILINE RawMessage& operator==(const RawMessage& other) { return *this; }
	};

protected:
	CRemoteCommandUpr* m_pUpr;

	// Network listening socket
	IServiceNetworkListener* m_pListener;

	// Live endpoints
	typedef std::vector<Endpoint*> TEndpoints;
	TEndpoints m_pEndpoints;
	TEndpoints m_pUpdateEndpoints;
	DrxMutex   m_accessLock;

	// Endpoints that were discarded and should be deleted
	// We can delete endpoints only from the update thread
	TEndpoints m_pEndpointToDelete;

	// Received raw messages
	typedef DrxMT::CLocklessPointerQueue<RawMessage> TRawMessagesQueue;
	TRawMessagesQueue m_pRawMessages;
	DrxMutex          m_rawMessagesLock;

	// Listeners for raw messages that require synchronous processing
	typedef std::vector<IRemoteCommandListenerSync*> TRawMessageListenersSync;
	TRawMessageListenersSync m_pRawListenersSync;

	// Listeners for raw messages that can be processed asynchronously (faster path)
	typedef std::vector<IRemoteCommandListenerAsync*> TRawMessageListenersAsync;
	TRawMessageListenersAsync m_pRawListenersAsync;

	// Suppression counter (execution of commands is suppressed when>0)
	// This is updated using DrxInterlocked* functions
	 i32 m_suppressionCounter;
	bool         m_bIsSuppressed;

	// Request to close the network thread
	bool m_bCloseThread;

public:
	ILINE CRemoteCommandUpr* GetUpr() const
	{
		return m_pUpr;
	}

public:
	CRemoteCommandServer(CRemoteCommandUpr* pUpr, IServiceNetworkListener* pListener);
	virtual ~CRemoteCommandServer();

	// IRemoteCommandServer interface implementation
	virtual void Delete();
	virtual void FlushCommandQueue();
	virtual void SuppressCommands();
	virtual void ResumeCommands();
	virtual void RegisterSyncMessageListener(IRemoteCommandListenerSync* pListener);
	virtual void UnregisterSyncMessageListener(IRemoteCommandListenerSync* pListener);
	virtual void RegisterAsyncMessageListener(IRemoteCommandListenerAsync* pListener);
	virtual void UnregisterAsyncMessageListener(IRemoteCommandListenerAsync* pListener);
	virtual void Broadcast(IServiceNetworkMessage* pMessage);
	virtual bool HasConnectedClients() const;

	// IThread
	virtual void ThreadEntry();
	void         SignalStopWorker();

protected:
	void ProcessRawMessageAsync(IServiceNetworkMessage* pMessage, IServiceNetworkConnection* pConnection);
	void ProcessRawMessagesSync();
};

//-----------------------------------------------------------------------------

// Remote command manager implementation
class CRemoteCommandUpr : public IRemoteCommandUpr
{
public:
	CRemoteCommandUpr();
	virtual ~CRemoteCommandUpr();

	// IRemoteCommandUpr interface implementation
	virtual void                  SetVerbosityLevel(u32k level);
	virtual IRemoteCommandServer* CreateServer(u16 localPort);
	virtual IRemoteCommandClient* CreateClient();
	virtual void                  RegisterCommandClass(IRemoteCommandClass& commandClass);

	// Debug print
#ifdef RELEASE
	void Log(tukk txt, ...) const        {};
	bool CheckVerbose(u32k level) const { return false; }
#else
	void Log(tukk txt, ...) const;
	bool CheckVerbose(u32k level) const;
#endif

	// Build ID->Class Factory mapping given the class name list, will report errors to the log.
	void BuildClassMapping(const std::vector<string>& classNames, std::vector<IRemoteCommandClass*>& outClasses);

	// Get list of class names (in order of their IDs)
	void GetClassList(std::vector<string>& outClassNames) const;

	// Find class ID for given class, returns false if not found
	bool FindClassId(IRemoteCommandClass* commandClass, u32& outClassId) const;

public:
	ILINE CRemoteCommandUpr* GetUpr()
	{
		return this;
	}

private:
	// Class name mapping
	typedef std::map<string, IRemoteCommandClass*> TClassMap;
	TClassMap m_pClasses;

	// Class ID lookup
	typedef std::vector<IRemoteCommandClass*> TClassIDList;
	TClassIDList m_pClassesByID;

	// Class ID mapping
	typedef std::map<string, i32> TClassIDMap;
	TClassIDMap m_pClassesMap;

	// Verbose level
	ICVar* m_pVerboseLevel;
};

#endif
