// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_LIVECREATEMANAGER_H_
#define _H_LIVECREATEMANAGER_H_

#include <drx3D/LiveCreate/ILiveCreateUpr.h>
#include <drx3D/Network/IRemoteCommand.h>

struct IPlatformHandler;
struct IPlatformHandlerFactory;
struct IServiceNetworkConnection;
struct IRemoteCommandClient;
struct IRemoteCommandConnection;

//-----------------------------------------------------------------------------

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

//-----------------------------------------------------------------------------

// Host information interface implementation
class CHostInfo : public IHostInfo
{
protected:
	class CUpr* m_pUpr;

	 i32    m_refCount;

	// Platform handler interface
	// In case of this class it's mainly needed for low-level file transfer functionality (FileSync).
	IPlatformHandler* m_pPlatform;

	// Connection to command dispatched, this can also handle raw network traffic.
	// This can exist or not, depends on whether we are connected.
	IRemoteCommandConnection* m_pConnection;

	// Last known valid address of the remote host, supplied when the host info was created.
	// This should not be used to identify the hosts since (in theory) this address can change.
	string m_address;

	// Was the connection confirmed by receiving BuildInfo packet ?
	// We need that kind of ACK message to be sure that the remote side is responding.
	bool m_bIsConnectionConfirmed;

	// Is the remote side ready to accept commands ?
	bool m_bIsRemoteSideReady;

	// Is the remote side suspended for any reason - for example level loading?
	bool m_bIsSuspended;

private:
	int64 m_lastReadyInfoRequestSentTime;

public:
	CHostInfo(
		class CUpr* pUpr,
			IPlatformHandler* pPlatform,
			tukk szValidAddress);

	// Update and maintain connection
	void Update();

	// Can we send to this host ?
	// This checks if we have a valid and confirmed connection and that the remote side is not suspended.
	bool CanSend() const;

	// Send message using the raw communication channel
	// If we don't have a valid connection or the remote side is suspended a message can be rejected.
	// Functions returns true only if the low-level communication layer accepted the message for sending.
	bool SendRawMessage(IServiceNetworkMessage* pMessage);

protected:
	// Process raw message (as received from LiveCreate host)
	bool ParseMessage(const string& msg, IDataReadStream& data);

	// Send a info request messages to remote connection
	void SendRequestMessage(tukk szInfoType);

public:
	// IHost interface implementation
	virtual tukk       GetTargetName() const;
	virtual tukk       GetAddress() const;
	virtual tukk       GetPlatformName() const;
	virtual bool              IsConnected() const;
	virtual bool              IsReady() const;
	virtual IPlatformHandler* GetPlatformInterface();
	virtual bool              Connect();
	virtual void              Disconnect();
	virtual void              AddRef();
	virtual void              Release();

private:
	virtual ~CHostInfo();
};

//-----------------------------------------------------------------------------

class CUpr : public IUpr
{
protected:
	struct PlatformDLL
	{
		PlatformDLL(IPlatformHandlerFactory* factory = NULL)
			: hLibrary(0)
			, pFactory(factory)
		{}

		uk                    hLibrary;
		IPlatformHandlerFactory* pFactory;
	};

	struct LastHostState
	{
		bool m_bIsReady;
		bool m_bIsConnected;
	};

protected:
	// Remote command dispatched
	IRemoteCommandClient* m_pCommandDispatcher;

	// Logging
	ICVar* m_pCVarLogEnabled;

	// Upr wide flags
	bool m_bIsEnabled;
	bool m_bCanSend;

	// Platform factories
	typedef std::vector<PlatformDLL> TPlatformList;
	TPlatformList m_platformFactories;

	// Registered hosts (usually never deleted)
	typedef std::vector<CHostInfo*> THostList;
	THostList m_pHosts;
	DrxMutex  m_hostsLock;

	// Upr listeners
	typedef std::vector<IUprListenerEx*> TListeners;
	TListeners m_pListeners;
	DrxMutex   m_listenersLock;

	// Cached host states
	typedef std::map<CHostInfo*, LastHostState> TLastHostStateCache;
	TLastHostStateCache m_lastHostStates;

public:
	ILINE IRemoteCommandClient* GetCommandDispatcher() const
	{
		return m_pCommandDispatcher;
	}

public:
	CUpr();
	virtual ~CUpr();

	// IUprEx interface implementation
	virtual bool                     IsNullImplementation() const;
	virtual bool                     CanSend() const;
	virtual bool                     IsEnabled() const;
	virtual void                     SetEnabled(bool bIsEnabled);
	virtual bool                     IsLogEnabled() const;
	virtual void                     SetLogEnabled(bool bIsEnabled);
	virtual bool                     RegisterListener(IUprListenerEx* pListener);
	virtual bool                     UnregisterListener(IUprListenerEx* pListener);
	virtual uint                     GetNumHosts() const;
	virtual IHostInfo*               GetHost(const uint index) const;
	virtual uint                     GetNumPlatforms() const;
	virtual IPlatformHandlerFactory* GetPlatformFactory(const uint index) const;
	virtual IHostInfo*               CreateHost(IPlatformHandler* pPlatform, tukk szValidIpAddress);
	virtual bool                     RemoveHost(IHostInfo* pHost);
	virtual void                     Update();
	virtual bool                     SendCommand(const IRemoteCommand& command);
	virtual void                     LogMessage(ELogMessageType aType, tukk pMessage);

	// Formated logging
	void LogMessagef(ELogMessageType aType, tukk pMessage, ...);

private:
	// Load platform factory form a DLL file
	bool LoadPlatformHandlerDLL(tukk pFilename, PlatformDLL& dllInfo);

	// Load all of the platform factory DLLs
	void LoadPlatformHandlerFactoryDLLs();

	// Reevaluate the sending conditions
	void EvaluateSendingStatus();
};

//-----------------------------------------------------------------------------

}

#endif

#endif
