// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/StdAfx.h>
#include <drx3D/Network/IServiceNetwork.h>
#include <drx3D/Network/IRemoteCommand.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>

#include <drx3D/LiveCreate/LiveCreateUpr.h>
#include <drx3D/LiveCreate/LiveCreateCommands.h>
#include <drx3D/LiveCreate/LiveCreate_System.h>
#include <drx3D/LiveCreate/LiveCreate_Objects.h>

#include <drx3D/LiveCreate/PlatformHandler_GamePC.h>
#include <drx3D/LiveCreate/PlatformHandler_Any.h>

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

//-----------------------------------------------------------------------------

typedef IPlatformHandlerFactory* (* TCreatePlatformHandlerFactory)();

//-----------------------------------------------------------------------------

CHostInfo::CHostInfo(
	class CUpr* pUpr,
		IPlatformHandler* pPlatform,
		tukk szValidAddress)
	: m_pUpr(pUpr)
	, m_pPlatform(pPlatform)
	, m_address(szValidAddress)
	, m_pConnection(NULL)
	, m_lastReadyInfoRequestSentTime(0)
	, m_bIsConnectionConfirmed(false)
	, m_bIsRemoteSideReady(false)
	, m_bIsSuspended(false)
	, m_refCount(1)
{
	// hold reference to platform handler
	m_pPlatform->AddRef();
}

CHostInfo::~CHostInfo()
{
	// Always disconnect
	if (NULL != m_pConnection)
	{
		// Close connection
		m_pConnection->Release();
		m_pConnection->Close();
		m_pConnection = NULL;
	}

	// Release the platform handler
	if (NULL != m_pPlatform)
	{
		m_pPlatform->Release();
		m_pPlatform = NULL;
	}
}

void CHostInfo::AddRef()
{
	DrxInterlockedIncrement(&m_refCount);
}

void CHostInfo::Release()
{
	if (0 == DrxInterlockedDecrement(&m_refCount))
	{
		delete this;
	}
}

tukk CHostInfo::GetTargetName() const
{
	return m_pPlatform->GetTargetName();
}

tukk CHostInfo::GetAddress() const
{
	return m_address.c_str();
}

tukk CHostInfo::GetPlatformName() const
{
	return m_pPlatform->GetPlatformName();
}

bool CHostInfo::IsConnected() const
{
	return (NULL != m_pConnection) && m_pConnection->IsAlive();
}

bool CHostInfo::IsReady() const
{
	return IsConnected() && m_bIsConnectionConfirmed && m_bIsRemoteSideReady;
}

bool CHostInfo::Connect()
{
	m_pUpr->LogMessagef(eLogType_Normal, "Connecting to target '%s' on platform '%s'",
	                        GetTargetName(), GetPlatformName());

	// Dead connection, remove to allow reconnection
	if (NULL != m_pConnection && !m_pConnection->IsAlive())
	{
		m_pUpr->LogMessagef(eLogType_Warning, "There's an existing dead connection. Closing.");

		const bool bGracefully = true;
		m_pConnection->Release();
		m_pConnection->Close(bGracefully);
		m_pConnection = NULL;
	}

	// Already connected with a non-dead connection
	if (NULL != m_pConnection)
	{
		m_pUpr->LogMessagef(eLogType_Warning, "There's an existing connection.");
		return true;
	}

	// Try to connect
	const ServiceNetworkAddress remoteAddress = gEnv->pServiceNetwork->GetHostAddress(m_address, LiveCreate::kDefaultHostListenPort);
	IRemoteCommandConnection* pConnection = m_pUpr->GetCommandDispatcher()->ConnectToServer(remoteAddress);
	if (NULL == pConnection)
	{
		m_pUpr->LogMessagef(eLogType_Warning, "Unable to connect to address '%s'", remoteAddress.ToString().c_str());
		return false;
	}

	// Valid connection created
	m_pConnection = pConnection;
	m_pUpr->LogMessagef(eLogType_Normal, "Connected to host '%s' on '%s' = '%s'",
	                        GetTargetName(), GetPlatformName(), remoteAddress.ToString().c_str());

	// Request a ready flag (until we don't get it the connection is in unconfirmed state)
	m_lastReadyInfoRequestSentTime = gEnv->pTimer->GetAsyncTime().GetMilliSecondsAsInt64();
	SendRequestMessage("GetReadyFlag");

	// Cleanup the readiness flag
	m_bIsRemoteSideReady = false;

	// Inform the caller that technically, we have a connection
	return true;
}

void CHostInfo::SendRequestMessage(tukk szInfoType)
{
	if (NULL != m_pConnection)
	{
		IDataWriteStream* pWriter = gEnv->pServiceNetwork->CreateMessageWriter();
		if (NULL != pWriter)
		{
			// write the name of the
			pWriter->WriteString(szInfoType);

			// send message
			IServiceNetworkMessage* pMessage = pWriter->BuildMessage();
			if (NULL != pMessage)
			{
				m_pConnection->SendRawMessage(pMessage);
				pMessage->Release();
			}

			// delete writer
			pWriter->Delete();
		}
	}
}

void CHostInfo::Disconnect()
{
	// Close connection
	if (NULL != m_pConnection)
	{
		// Close signaling connection but only after messages are sent
		const bool bSendDisconnect = true;
		m_pConnection->Close(bSendDisconnect);
		m_pConnection = NULL;
	}

	// Reset connection confirmed flag
	m_bIsConnectionConfirmed = false;
	m_bIsRemoteSideReady = false;
}

IPlatformHandler* CHostInfo::GetPlatformInterface()
{
	return m_pPlatform;
}

bool CHostInfo::CanSend() const
{
	if (NULL != m_pConnection && m_pConnection->IsAlive())
	{
		if (m_bIsConnectionConfirmed && !m_bIsSuspended)
		{
			return true;
		}
	}

	// not able to send
	return false;
}

bool CHostInfo::SendRawMessage(IServiceNetworkMessage* pMessage)
{
	if (NULL != m_pConnection && m_bIsConnectionConfirmed)
	{
		if (!m_bIsSuspended)
		{
			return m_pConnection->SendRawMessage(pMessage);
		}
	}

	// not able to send
	return false;
}

void CHostInfo::Update()
{
	// Process incoming messages
	if (NULL != m_pConnection)
	{
		// Connection is dead, mark as disconnected
		if (!m_pConnection->IsAlive())
		{
			m_pUpr->LogMessagef(eLogType_Warning, "Connection to '%s' is dead. Closing.",
			                        m_pConnection->GetRemoteAddress().ToString().c_str());

			Disconnect();
			return;
		}

		// Get raw messages from command connection (signaling messages)
		IServiceNetworkMessage* pMessage = m_pConnection->ReceiveRawMessage();
		while (NULL != pMessage)
		{
			TAutoDelete<IDataReadStream> reader(pMessage->CreateReader());
			if (reader)
			{
				// Read message type and process the message
				const string msgType = reader->ReadString();
				ParseMessage(msgType, reader);
			}

			// Get next message
			pMessage->Release();
			pMessage = m_pConnection->ReceiveRawMessage();
		}

		// Ask for the ready-busy flag, if we do not have a confirmed connection yet as a little bit more often
		{
			const int64 kResendTime = m_bIsConnectionConfirmed ? 5000 : 500; // ms
			const int64 currentTime = gEnv->pTimer->GetAsyncTime().GetMilliSecondsAsInt64();
			if ((currentTime - m_lastReadyInfoRequestSentTime) > kResendTime)
			{
				m_pUpr->LogMessagef(eLogType_Normal, "Requesting ready flag '%s'",
				                        m_pConnection->GetRemoteAddress().ToString().c_str());

				m_lastReadyInfoRequestSentTime = currentTime;
				SendRequestMessage("GetReadyFlag");
			}
		}
	}
}

bool CHostInfo::ParseMessage(const string& cmd, IDataReadStream& message)
{
	// Getting a message confirms the connection
	if (!m_bIsConnectionConfirmed)
	{
		m_pUpr->LogMessagef(eLogType_Normal, "Connection to '%s' confirmed",
		                        m_pConnection->GetRemoteAddress().ToString().c_str());

		m_bIsConnectionConfirmed = true;
	}

	// Got "ready" flag message
	if (cmd == "Ready")
	{
		m_pUpr->LogMessagef(eLogType_Normal, "Target '%s' is READY",
		                        m_pConnection->GetRemoteAddress().ToString().c_str());

		m_bIsRemoteSideReady = true;
		return true;
	}

	// Got "busy" flag message
	if (cmd == "Busy")
	{
		m_pUpr->LogMessagef(eLogType_Normal, "Target '%s' is BUSY",
		                        m_pConnection->GetRemoteAddress().ToString().c_str());

		m_bIsRemoteSideReady = false;
		return true;
	}

	// Not processed
	return false;
}

//-----------------------------------------------------------------------------

CUpr::CUpr()
	: m_pCommandDispatcher(NULL)
	, m_bIsEnabled(false)
	, m_bCanSend(false)
{
	// Create the remote command dispatched interface
	m_pCommandDispatcher = gEnv->pRemoteCommandUpr->CreateClient();
	DRX_ASSERT(NULL != m_pCommandDispatcher);

	// Logging CVar
	m_pCVarLogEnabled = gEnv->pConsole->RegisterInt("lc_logEnabled", 0, VF_DEV_ONLY);

	// Load the additional handlers for platforms
	LoadPlatformHandlerFactoryDLLs();
}

CUpr::~CUpr()
{
	// Unload the platforms
	for (TPlatformList::iterator it = m_platformFactories.begin();
	     it != m_platformFactories.end(); ++it)
	{
		PlatformDLL& dll = *it;

		// Release the factory
		if (NULL != dll.pFactory)
		{
			//m_platformFactories[i].pFactory->Release();
			dll.pFactory = NULL;
		}

		// Unload the DLL
		if (NULL != dll.hLibrary)
		{
			DrxFreeLibrary(dll.hLibrary);
			dll.hLibrary = NULL;
		}
	}
	m_platformFactories.clear();

	// Delete hosts
	for (THostList::iterator it = m_pHosts.begin();
	     it != m_pHosts.end(); ++it)
	{
		(*it)->Release();
	}
	m_pHosts.clear();

	// Release the command dispatched interface
	if (NULL != m_pCommandDispatcher)
	{
		m_pCommandDispatcher->Delete();
		m_pCommandDispatcher = NULL;
	}

	SAFE_RELEASE(m_pCVarLogEnabled);
}

bool CUpr::IsNullImplementation() const
{
	return false;
}

bool CUpr::CanSend() const
{
	return m_bCanSend;
}

bool CUpr::IsEnabled() const
{
	return m_bIsEnabled;
}

void CUpr::SetEnabled(bool bIsEnabled)
{
	if (m_bIsEnabled != bIsEnabled)
	{
		m_bIsEnabled = bIsEnabled;
		EvaluateSendingStatus();
	}
}

bool CUpr::IsLogEnabled() const
{
	return m_pCVarLogEnabled->GetIVal() != 0;
}

void CUpr::SetLogEnabled(bool bIsEnabled)
{
	m_pCVarLogEnabled->Set(bIsEnabled ? 1 : 0);
}

bool CUpr::RegisterListener(IUprListenerEx* pListener)
{
	// add only valid listeners and make sure they are registered only once
	if (NULL != pListener)
	{
		if (m_pListeners.end() == std::find(m_pListeners.begin(), m_pListeners.end(), pListener))
		{
			DrxAutoLock<DrxMutex> lock(m_listenersLock);
			m_pListeners.push_back(pListener);
			return true;
		}
	}

	// not added
	return false;
}

bool CUpr::UnregisterListener(IUprListenerEx* pListener)
{
	// remove listener from the list
	TListeners::iterator it = std::find(m_pListeners.begin(), m_pListeners.end(), pListener);
	if (m_pListeners.end() != it)
	{
		DrxAutoLock<DrxMutex> lock(m_listenersLock);
		m_pListeners.erase(it);
		return true;
	}

	// not removed (was not there)
	return false;
}

IHostInfo* CUpr::CreateHost(IPlatformHandler* pPlatform, tukk szValidAddress)
{
	DrxAutoLock<DrxMutex> lock(m_hostsLock);

	// Invalid platform
	if (NULL == pPlatform)
	{
		return NULL;
	}

	// Don't allow a duplicated entry to be created
	for (THostList::iterator it = m_pHosts.begin();
	     it != m_pHosts.end(); ++it)
	{
		CHostInfo* pHost = *it;

		if ((0 == stricmp(pHost->GetPlatformName(), pPlatform->GetPlatformName())) &&
		    (0 == stricmp(pHost->GetTargetName(), pPlatform->GetTargetName())))
		{
			LogMessagef(eLogType_Error, "Host entry for '%s' alredy exists", pPlatform->GetTargetName());

			// A duplicate, do not register nor return existing one
			return NULL;
		}
	}

	// Create new one
	CHostInfo* pHost = new CHostInfo(this, pPlatform, szValidAddress);

	LogMessagef(eLogType_Normal, "Created host entry for '%s' on '%s'. Last known address = '%s'",
	            pPlatform->GetTargetName(), pPlatform->GetPlatformName(), szValidAddress);

	// When in our list of hosts keep an extra reference
	pHost->AddRef();
	m_pHosts.push_back(pHost);

	return pHost;
}

bool CUpr::RemoveHost(IHostInfo* pHost)
{
	DrxAutoLock<DrxMutex> lock(m_hostsLock);

	for (THostList::iterator it = m_pHosts.begin();
	     it != m_pHosts.end(); ++it)
	{
		if (pHost == *it)
		{
			LogMessagef(eLogType_Normal, "Removed host entry for '%s' on '%s'",
			            pHost->GetTargetName(), pHost->GetPlatformName());

			m_pHosts.erase(it);
			pHost->Release();
			return true;
		}
	}

	// not found
	return false;
}

uint CUpr::GetNumHosts() const
{
	DrxAutoLock<DrxMutex> lock(m_hostsLock);
	return m_pHosts.size();
}

IHostInfo* CUpr::GetHost(const uint index) const
{
	DrxAutoLock<DrxMutex> lock(m_hostsLock);
	if (index < m_pHosts.size())
	{
		return m_pHosts[index];
	}
	else
	{
		// host index out of bounds
		return NULL;
	}
}

uint CUpr::GetNumPlatforms() const
{
	return m_platformFactories.size();
}

IPlatformHandlerFactory* CUpr::GetPlatformFactory(const uint index) const
{
	DRX_ASSERT(index < m_platformFactories.size());
	return m_platformFactories[index].pFactory;
}

bool CUpr::LoadPlatformHandlerDLL(tukk pFilename, PlatformDLL& dllInfo)
{
	HMODULE hLibrary = DrxLoadLibrary(pFilename);
	if (NULL != hLibrary)
	{
		TCreatePlatformHandlerFactory pFunc = (TCreatePlatformHandlerFactory)DrxGetProcAddress(hLibrary, "CreatePlatformHandlerFactory");
		if (NULL != pFunc)
		{
			IPlatformHandlerFactory* pFactory = pFunc();
			if (NULL != pFactory)
			{
				dllInfo.hLibrary = hLibrary;
				dllInfo.pFactory = pFactory;
				return true;
			}
		}

		DrxFreeLibrary(hLibrary);
	}

	// Not loaded
	return false;
}

void CUpr::LoadPlatformHandlerFactoryDLLs()
{
	//TODO: use a search algo, for file masks ? like "Tools\\LiveCreatePlatform*.dll" ?
	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
		#define LC_BIN_POSTFIX "_64"
	#else
		#define LC_BIN_POSTFIX ""
	#endif

	// PC factory
	#if DRX_PLATFORM_WINDOWS
	{
		PlatformDLL pcFactory;
		pcFactory.hLibrary = NULL;
		pcFactory.pFactory = new PlatformHandlerFactory_GamePC();
		m_platformFactories.push_back(pcFactory);
	}

	{
		PlatformDLL pcFactory;
		pcFactory.hLibrary = NULL;
		pcFactory.pFactory = new PlatformHandlerFactory_Any();
		m_platformFactories.push_back(pcFactory);
	}
	#endif

	PlatformDLL dllInfo;

	if (LoadPlatformHandlerDLL(".\\Tools\\LiveCreatePlatformPS4" LC_BIN_POSTFIX ".dll", dllInfo))
	{
		m_platformFactories.push_back(dllInfo);
	}

	if (LoadPlatformHandlerDLL(".\\Tools\\LiveCreatePlatformXBOXONE" LC_BIN_POSTFIX ".dll", dllInfo))
	{
		m_platformFactories.push_back(dllInfo);
	}

}

void CUpr::Update()
{
	DrxAutoLock<DrxMutex> lock(m_hostsLock);

	// process connections
	for (THostList::iterator it = m_pHosts.begin();
	     it != m_pHosts.end(); ++it)
	{
		CHostInfo* pHostInfo = (*it);

		// update the host, keep track of connection status
		pHostInfo->Update();
	}

	// recheck if we are allowed to send data
	EvaluateSendingStatus();

	// send the connected/disconnected/ready events
	for (THostList::iterator it = m_pHosts.begin();
	     it != m_pHosts.end(); ++it)
	{
		CHostInfo* pHostInfo = (*it);

		// get cached state from previous update
		bool bWasReady = false;
		bool bWasConnected = false;
		TLastHostStateCache::const_iterator jt = m_lastHostStates.find(pHostInfo);
		if (jt != m_lastHostStates.end())
		{
			bWasReady = (*jt).second.m_bIsReady;
			bWasConnected = (*jt).second.m_bIsConnected;
		}

		// state changed
		const bool bIsReady = pHostInfo->IsReady();
		const bool bIsConnected = pHostInfo->IsConnected();
		if (bIsReady != bWasReady)
		{
			if (bIsReady)
			{
				for (TListeners::const_iterator it = m_pListeners.begin();
				     it != m_pListeners.end(); ++it)
				{
					(*it)->OnHostReady(pHostInfo);
				}
			}
			else
			{
				for (TListeners::const_iterator it = m_pListeners.begin();
				     it != m_pListeners.end(); ++it)
				{
					(*it)->OnHostBusy(pHostInfo);
				}
			}
		}
		else if (bIsConnected != bWasConnected)
		{
			if (bIsConnected)
			{
				// propagate event to listeners
				for (TListeners::const_iterator it = m_pListeners.begin();
				     it != m_pListeners.end(); ++it)
				{
					(*it)->OnHostConnected(pHostInfo);
				}
			}
			else
			{
				// propagate event to listeners
				for (TListeners::const_iterator it = m_pListeners.begin();
				     it != m_pListeners.end(); ++it)
				{
					(*it)->OnHostDisconnected(pHostInfo);
				}
			}
		}

		// update state
		LastHostState hostState;
		hostState.m_bIsConnected = bIsConnected;
		hostState.m_bIsReady = bIsReady;
		m_lastHostStates[pHostInfo] = hostState;
	}
}

void CUpr::EvaluateSendingStatus()
{
	bool bCanSend = false;
	if (m_bIsEnabled)
	{
		DrxAutoLock<DrxMutex> lock(m_hostsLock);

		// we need at least one enabled and ready host
		for (THostList::const_iterator it = m_pHosts.begin();
		     it != m_pHosts.end(); ++it)
		{
			if ((*it)->IsReady())
			{
				bCanSend = true;
				break;
			}
		}
	}

	if (bCanSend != m_bCanSend)
	{
		DrxAutoLock<DrxMutex> lock(m_listenersLock);

		m_bCanSend = bCanSend;

		// Signal listeners
		for (TListeners::const_iterator it = m_pListeners.begin();
		     it != m_pListeners.end(); ++it)
		{
			(*it)->OnSendingStatusChanged(bCanSend);
		}

		if (m_bCanSend)
		{
			LogMessagef(eLogType_Normal, "Sending ENABLED");
		}
		else
		{
			LogMessagef(eLogType_Normal, "Sending DISABLED");
		}
	}
}

void CUpr::LogMessage(ELogMessageType aType, tukk pMessage)
{
	if (m_pCVarLogEnabled->GetIVal())
	{
		// output to normal log
		{
			// format the log message with a prefix (until we get a logging manager that supports channels)

			char szBuffer[1024];
			drx_strcpy(szBuffer, "<LC> ");
			drx_strcat(szBuffer, pMessage);

			switch (aType)
			{
			case eLogType_Normal:
				gEnv->pLog->Log(szBuffer);
				break;
			case eLogType_Warning:
				gEnv->pLog->LogWarning(szBuffer);
				break;
			case eLogType_Error:
				gEnv->pLog->LogError(szBuffer);
				break;
			}
		}

		for (TListeners::const_iterator it = m_pListeners.begin();
		     it != m_pListeners.end(); ++it)
		{
			(*it)->OnLogMessage(aType, pMessage);
		}
	}
}

void CUpr::LogMessagef(ELogMessageType aType, tukk pMessage, ...)
{
	if (!m_pListeners.empty() && m_pCVarLogEnabled->GetIVal())
	{
		char szBuffer[2048];
		va_list args;
		va_start(args, pMessage);
		drx_vsprintf(szBuffer, pMessage, args);
		va_end(args);

		LogMessage(aType, szBuffer);
	}
}

bool CUpr::SendCommand(const IRemoteCommand& command)
{
	if (CanSend())
	{
		return m_pCommandDispatcher->Schedule(command);
	}
	else
	{
		return false;
	}
}

}

#endif
