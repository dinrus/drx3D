// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/NotificationNetwork.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Network/DrxSocks.h>
#include <drx3D/CoreX/String/Path.h>

#if DRX_PLATFORM_WINAPI
	#undef EWOULDBLOCK
	#undef EALREADY
	#undef ECONNRESET
	#undef EINPROGRESS

	#define EWOULDBLOCK   WSAEWOULDBLOCK
	#define EISCONNECTED  WSAEISCONN
	#define EALREADY      WSAEALREADY
	#define ECONNRESET    WSAECONNRESET
	#define EINPROGRESS   WSAEINPROGRESS
	#define ENOTCONNECTED WSAENOTCONN
typedef DWORD TErrorType;
#endif

#if DRX_PLATFORM_POSIX
//#define EWOULDBLOCK   EWOULDBLOCK
	#define EISCONNECTED EISCONN
//#define EALREADY      EALREADY
//#define ECONNRESET    ENETRESET
	#define ENOTCONNECTED ENOTCONN
typedef i32 TErrorType;
#endif

#undef LockDebug
//#define LockDebug(str1,str2)	{string strMessage;strMessage.Format(str1,str2);if (m_clients.size())	OutputDebugString(strMessage.c_str());}
#define LockDebug(str1, str2)

using namespace NotificationNetwork;

class CQueryNotification :
	public INotificationNetworkListener
{
	// INotificationNetworkListener
public:
	virtual void OnNotificationNetworkReceive(ukk pBuffer, size_t length)
	{
		INotificationNetwork* pNotificationNetwork =
		  gEnv->pSystem->GetINotificationNetwork();
		if (!pNotificationNetwork)
			return;

		tukk path = gEnv->pDrxPak->GetGameFolder();

		if (!path)
		{
			if (ICVar* pVar = gEnv->pConsole->GetCVar("sys_game_folder"))
				path = pVar->GetString();
		}
		if (!path)
			return;

		pNotificationNetwork->Send("SystemInfo", path, ::strlen(path) + 1);
	}
} g_queryNotification;

DRXSOCKET CConnectionBase::CreateSocket()
{
	// Note: Because of platform differences we can't do a compare to satisfy CppCheck here
	// cppcheck-ignore resourceLeak
	DRXSOCKET s = DrxSock::socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0 || s == DRX_INVALID_SOCKET)
	{
		DrxLog("CNotificationNetworkClient::Create: Failed to create socket.");
		return DRX_INVALID_SOCKET;
	}

	i32k yes = 1;
	if (DrxSock::setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	                        (tukk)&yes, sizeof(i32)) < 0)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Create: Failed to set SO_REUSEADDR option.");
		return DRX_INVALID_SOCKET;
	}

#if DRX_PLATFORM_WINDOWS
	u_long nAsyncSockectOperation(1);
	if (ioctlsocket(s, FIONBIO, &nAsyncSockectOperation) == DRX_SOCKET_ERROR)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Connect: Failed to set socket to asynchronous operation.");
		return DRX_INVALID_SOCKET;
	}

#endif

	// TCP_NODELAY required for win32 because of high latency connection otherwise
#if DRX_PLATFORM_WINDOWS
	if (DrxSock::setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
	                        (tukk)&yes, sizeof(i32)) < 0)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Create: Failed to set TCP_NODELAY option.");
		return DRX_INVALID_SOCKET;
	}
#endif

	return s;
}

bool CConnectionBase::Connect(tukk address, u16 port)
{
	DRXSOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = DrxSock::GetAddrForHostOrIP(address, 0);
	if (DrxSock::connect(m_socket, (DRXSOCKADDR*)&addr, sizeof(addr)) < 0)
	{
		TErrorType nError(GetLastError());
		if (nError == EWOULDBLOCK)
		{
			return true;
		}

		if (nError == EISCONNECTED)
		{
			if (!m_boIsConnected)
			{
				m_boIsConnected = true;
				m_boIsFailedToConnect = false;
				OnConnect(true);
			}
			return true;
		}

		if (nError == EALREADY)
		{
			// It will happen, in case of DNS problems, or if the console is not
			// reachable or turned off.
			//DrxLog("CNotificationNetworkClient::Connect: Failed to connect. Reason: already conencted.");
			return true;
		}

		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Connect: Failed to connect. Reason: %u ", nError);
		return false;
	}

	return true;
}

/*

   CChannel

 */

bool CChannel::IsNameValid(tukk name)
{
	if (!name)
		return false;
	if (!*name)
		return false;

	if (::strlen(name) > NN_CHANNEL_NAME_LENGTH_MAX)
		return false;

	return true;
}

//

CChannel::CChannel()
{
}

CChannel::CChannel(tukk name)
{
	if (!name)
		return;

	if (!*name)
		return;

	size_t length = std::min(::strlen(name), (size_t)NN_CHANNEL_NAME_LENGTH_MAX);
	::memcpy(m_name, name, length);
	::memset(m_name + length, 0, NN_CHANNEL_NAME_LENGTH_MAX - length);
}

CChannel::~CChannel()
{
}

//

void CChannel::WriteToPacketHeader(uk pPacket) const
{
	::memcpy((u8*)pPacket + NN_PACKET_HEADER_OFFSET_CHANNEL,
	         m_name, NN_CHANNEL_NAME_LENGTH_MAX);
}

void CChannel::ReadFromPacketHeader(uk pPacket)
{
	::memcpy(m_name, (u8*)pPacket + NN_PACKET_HEADER_OFFSET_CHANNEL,
	         NN_CHANNEL_NAME_LENGTH_MAX);
}

//

bool CChannel::operator==(const CChannel& channel) const
{
	return ::strncmp(m_name, channel.m_name, NN_CHANNEL_NAME_LENGTH_MAX) == 0;
}

bool CChannel::operator!=(const CChannel& channel) const
{
	return ::strncmp(m_name, channel.m_name, NN_CHANNEL_NAME_LENGTH_MAX) != 0;
}

/*

   CListeners

 */

CListeners::CListeners()
{
	m_pNotificationWrite = &m_notifications[0];
	m_pNotificationRead = &m_notifications[1];
}

CListeners::~CListeners()
{
	while (!m_pNotificationRead->empty())
	{
		SBuffer buffer = m_pNotificationRead->front();
		m_pNotificationRead->pop();
		delete[] buffer.pData;

	}

	while (!m_pNotificationWrite->empty())
	{
		SBuffer buffer = m_pNotificationWrite->front();
		m_pNotificationWrite->pop();
		delete[] buffer.pData;
	}
}

//

size_t CListeners::Count(const CChannel& channel)
{
	size_t count = 0;
	for (size_t i = 0; i < m_listeners.size(); ++i)
	{
		if (m_listeners[i].second != channel)
			continue;

		++count;
	}

	return count;
}

CChannel* CListeners::Channel(INotificationNetworkListener* pListener)
{
	for (size_t i = 0; i < m_listeners.size(); ++i)
	{
		if (m_listeners[i].first != pListener)
			continue;

		return &m_listeners[i].second;
	}

	return NULL;
}

bool CListeners::Bind(const CChannel& channel, INotificationNetworkListener* pListener)
{
	for (size_t i = 0; i < m_listeners.size(); ++i)
	{
		if (m_listeners[i].first == pListener)
		{
			m_listeners[i].second = channel;
			return true;
		}
	}

	m_listeners.push_back(std::pair<INotificationNetworkListener*, CChannel>());
	m_listeners.back().first = pListener;
	m_listeners.back().second = channel;
	return true;
}

bool CListeners::Remove(INotificationNetworkListener* pListener)
{
	for (size_t i = 0; i < m_listeners.size(); ++i)
	{
		if (m_listeners[i].first != pListener)
			continue;

		m_listeners[i] = m_listeners.back();
		m_listeners.pop_back();
		return true;
	}

	return false;
}

void CListeners::NotificationPush(const SBuffer& buffer)
{
	DrxAutoCriticalSection lock(m_notificationCriticalSection);
	m_pNotificationWrite->push(buffer);
}

void CListeners::NotificationsProcess()
{
	m_notificationCriticalSection.Lock();
	std::swap(m_pNotificationWrite, m_pNotificationRead);
	m_notificationCriticalSection.Unlock();

	while (!m_pNotificationRead->empty())
	{
		SBuffer buffer = m_pNotificationRead->front();
		m_pNotificationRead->pop();

		for (size_t i = 0; i < m_listeners.size(); ++i)
		{
			if (m_listeners[i].second != buffer.channel)
				continue;

			m_listeners[i].first->OnNotificationNetworkReceive(
			  buffer.pData, buffer.length);
		}

		delete[] buffer.pData;
	}
}

/*

   CConnectionBase

 */

CConnectionBase::CConnectionBase(CNotificationNetwork* pNotificationNetwork)
{
	m_pNotificationNetwork = pNotificationNetwork;

	m_port = 0;

	m_socket = DRX_INVALID_SOCKET;

	m_buffer.pData = NULL;
	m_buffer.length = 0;
	m_dataLeft = 0;

	m_boIsConnected = false;
	m_boIsFailedToConnect = false;

	::memset(m_address, 0, sizeof(m_address));
}

CConnectionBase::~CConnectionBase()
{
	if (m_buffer.pData)
		delete[] m_buffer.pData;

	if (m_socket != DRX_INVALID_SOCKET)
		DrxSock::closesocket(m_socket);
}

//

void CConnectionBase::SetAddress(tukk address, u16 port)
{
	size_t length = std::min(::strlen(address), (size_t)15);
	::memset(m_address, 0, sizeof(m_address));
	::memcpy(m_address, address, length);
	m_port = port;
}

bool CConnectionBase::Validate()
{
	if (m_socket != DRX_INVALID_SOCKET)
	{
		if (!m_port)
		{
			DRXSOCKADDR addr;
			DRXSOCKLEN_T length = sizeof(addr);
			if (DrxSock::getsockname(m_socket, &addr, &length) < 0)
				return false;
		}

		fd_set stExceptions;
		fd_set stWriteSockets;

		FD_ZERO(&stExceptions);
		FD_SET(m_socket, &stExceptions);

		FD_ZERO(&stWriteSockets);
		FD_SET(m_socket, &stWriteSockets);

		timeval timeOut = { 0, 0 };
		i32 r = (i32)::select(i32(m_socket + 1), NULL, &stWriteSockets, &stExceptions, &timeOut); // Ian: DRXSOCKET possible truncation on x64
		if (r < 0)
		{
			TErrorType nErrorType(GetLastError());
			DrxLog("CNotificationNetworkClient::Validate: Failed to select socket. Reason: %u ", nErrorType);
			DrxSock::closesocket(m_socket);
			m_socket = DRX_INVALID_SOCKET;
			if (m_boIsConnected)
			{
				OnDisconnect();
			}
			m_boIsFailedToConnect = true;
			m_boIsConnected = false;
			return false;
		}

		if (r == 0)
		{
			if (m_boIsConnected)
			{
				return true;
			}

			//char szCharacters[4096]="";
			//drx_sprintf(szCharacters,"Assuming the connection will succeed:\n - Address: %s\n - Port: %d\n",m_address,m_port);
			//OutputDebugString(szCharacters);
			return false;
		}

		if (FD_ISSET(m_socket, &stExceptions))
		{
			DrxSock::closesocket(m_socket);
			m_socket = DRX_INVALID_SOCKET;
			m_boIsConnected = false;
			m_boIsFailedToConnect = true;
			OnConnect(m_boIsConnected); // Handles failed attempt to connect.
			return false;
		}
		else if (FD_ISSET(m_socket, &stWriteSockets)) // In Windows a socket can be in both lists.
		{
			if (!m_boIsConnected)
			{
				m_boIsConnected = true;
				m_boIsFailedToConnect = false;
				OnConnect(m_boIsConnected); // Handles successful attempt to connect.
			}
			return true;
		}

		return false;
	}

	if (!m_port) // If port is not set we don't want to try to reconnect.
		return false;

	m_socket = CreateSocket();
	// If the create sockect will fail, it is likely that we will never be able to connect,
	// we might want to signal that.

	Connect(m_address, m_port);

	return false;
}

bool CConnectionBase::Send(ukk pBuffer, size_t length)
{
	if (!Validate())
		return false;

	size_t sent = 0;
	while (sent < length)
	{
		i32 r = DrxSock::send(m_socket, (tukk)pBuffer + sent, length - sent, 0);
		if (r < 0)
		{
			TErrorType nCurrentError(GetLastError());
			if (nCurrentError == ENOTCONNECTED)
			{
				r = 0;
				break;
			}
			else if (nCurrentError == EWOULDBLOCK)
			{
				r = 0;
			}
			else
			{
				DrxLog("CNotificationNetworkClient::Send: Failed to send package. Reason: %u ", nCurrentError);
				DrxSock::closesocket(m_socket);
				m_socket = DRX_INVALID_SOCKET;
				if (m_boIsConnected)
				{
					OnDisconnect();
				}
				m_boIsConnected = false;
				return false;
			}
		}

		sent += r;
	}

	return true;
}

bool CConnectionBase::SendMessage(EMessage eMessage, const CChannel& channel, u32 data)
{
	char header[NN_PACKET_HEADER_LENGTH];
	::memset(header, 0, NN_PACKET_HEADER_LENGTH);
	*(u32*)&header[NN_PACKET_HEADER_OFFSET_MESSAGE] = htonl(eMessage);
	*(u32*)&header[NN_PACKET_HEADER_OFFSET_DATA_LENGTH] = htonl(data);
	channel.WriteToPacketHeader(header);

	if (!Send(header, NN_PACKET_HEADER_LENGTH))
		return false;

	return true;
}

bool CConnectionBase::SendNotification(const CChannel& channel, ukk pBuffer, size_t length)
{
	if (!SendMessage(eMessage_DataTransfer, channel, length))
		return false;
	if (!length)
		return true;

	if (!Send(pBuffer, length))
		return false;

	return true;
}

bool CConnectionBase::ReceiveMessage(CListeners& listeners)
{
	if (!Validate())
		return false;

	if (!m_dataLeft)
		m_dataLeft = NN_PACKET_HEADER_LENGTH;
	i32 r = DrxSock::recv(m_socket, (tuk)&m_bufferHeader[NN_PACKET_HEADER_LENGTH - m_dataLeft], m_dataLeft, 0);
	if (!r)
	{
		// Connection terminated.
		m_dataLeft = 0;

		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		if (m_boIsConnected)
		{
			OnDisconnect();
		}
		m_boIsConnected = false;
		return false;
	}
	if (r < 0)
	{
		m_dataLeft = 0;

		TErrorType nError(GetLastError());

		if (nError == ECONNRESET)
		{
			DrxLog("CNotificationNetworkClient::ReceiveMessage: Failed to receive package. Reason: Connection Reset.");
			DrxSock::closesocket(m_socket);
			m_socket = DRX_INVALID_SOCKET;
			if (m_boIsConnected)
			{
				OnDisconnect();
			}
			m_boIsConnected = false;
			return false;
		}
		else
		{
			DrxLog("CNotificationNetworkClient::ReceiveMessage: Failed to receive package. Reason: %u ", nError);
			DrxSock::closesocket(m_socket);
			m_socket = DRX_INVALID_SOCKET;
			if (m_boIsConnected)
			{
				OnDisconnect();
			}
			m_boIsConnected = false;
			return false;
		}
	}

	if (m_dataLeft -= r)
		return true;

	// The whole message was received, process it...

	EMessage eMessage = (EMessage)ntohl(
	  *(u32*)&m_bufferHeader[NN_PACKET_HEADER_OFFSET_MESSAGE]);
	const CChannel& channel = *(CChannel*)&m_bufferHeader[NN_PACKET_HEADER_OFFSET_CHANNEL];

	if (eMessage == eMessage_DataTransfer)
	{
		m_dataLeft = ntohl(*(u32*)&m_bufferHeader[NN_PACKET_HEADER_OFFSET_DATA_LENGTH]);
		if (!m_dataLeft)
		{
			SBuffer buffer;
			buffer.channel = channel;
			buffer.pData = NULL;
			buffer.length = 0;
			listeners.NotificationPush(buffer);
			return true;
		}

		m_buffer.pData = new u8[m_buffer.length = m_dataLeft];
		if (!m_buffer.pData)
		{
			DrxLog("CNotificationNetwork::CConnection::Receive: Failed to allocate buffer.\n");
			m_dataLeft = 0;

			DrxSock::closesocket(m_socket);
			m_socket = DRX_INVALID_SOCKET;
			if (m_boIsConnected)
			{
				OnDisconnect();
			}
			m_boIsConnected = false;
			return false;
		}

		m_buffer.channel.ReadFromPacketHeader(m_bufferHeader);
		return +1;
	}

	if (!OnMessage(eMessage, channel))
	{
		DrxLog("NotificationNetwork::CConnectionBase::ReceiveMessage: "
		       "Unknown message received, terminating Connection...\n");
		m_dataLeft = 0;

		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		if (m_boIsConnected)
		{
			OnDisconnect();
		}
		m_boIsConnected = false;
		return false;
	}

	return true;
}

bool CConnectionBase::ReceiveNotification(CListeners& listeners)
{
	i32 r = DrxSock::recv(m_socket, (tuk)&m_buffer.pData[m_buffer.length - m_dataLeft], m_dataLeft, 0);
	if (!r)
	{
		DrxLog("CNotificationNetworkClient::ReceiveNotification: Failed to receive package. Reason: Connection terminated.");
		// Connection terminated.
		m_dataLeft = 0;

		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		if (m_boIsConnected)
		{
			OnDisconnect();
		}
		m_boIsConnected = false;
		return false;
	}
	if (r < 0)
	{
		m_dataLeft = 0;

		TErrorType nCurrentError(GetLastError());
		DrxLog("CNotificationNetworkClient::ReceiveNotification: Failed to receive package. Reason: %u ", nCurrentError);
		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		if (m_boIsConnected)
		{
			OnDisconnect();
		}
		m_boIsConnected = false;
		return false;
	}

	if (m_dataLeft -= r)
		return true;

	listeners.NotificationPush(m_buffer);
	m_buffer.pData = NULL;
	m_buffer.length = 0;
	m_dataLeft = 0;
	return true;
}

bool CConnectionBase::Receive(CListeners& listeners)
{
	if (m_buffer.pData)
		return ReceiveNotification(listeners);

	return ReceiveMessage(listeners);
}

bool CConnectionBase::GetIsConnectedFlag()
{
	fd_set stExceptions;
	fd_set stWriteSockets;

	FD_ZERO(&stExceptions);
	FD_SET(m_socket, &stExceptions);

	FD_ZERO(&stWriteSockets);
	FD_SET(m_socket, &stWriteSockets);

	timeval timeOut = { 0, 0 };

	if (m_socket == DRX_INVALID_SOCKET)
	{
		return false;
	}

	i32 r = (i32)::select(i32(m_socket + 1), NULL, &stWriteSockets, &stExceptions, &timeOut); // Ian: DRXSOCKET possible truncation on x64
	if (r < 0)
	{
		TErrorType nErrorType(GetLastError());
		DrxLog("CNotificationNetworkClient::GetIsConnectedFlag: Failed to select socket. Reason: %u ", nErrorType);
		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		if (m_boIsConnected)
		{
			m_boIsConnected = false;
			m_boIsFailedToConnect = true;
			OnDisconnect();
		}
		return false;
	}

	if (r == 0)
	{
		if (m_boIsConnected)
		{
			return true;
		}

		//char szCharacters[4096]="";
		//drx_sprintf(szCharacters,"Assuming the connection will succeed:\n - Address: %s\n - Port: %d\n",m_address,m_port);
		//OutputDebugString(szCharacters);
		return false;
	}

	if (FD_ISSET(m_socket, &stExceptions))
	{
		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
		m_boIsConnected = false;
		m_boIsFailedToConnect = true;
		OnConnect(m_boIsConnected); // Handles failed attempt to connect.
		return false;
	}
	else if (FD_ISSET(m_socket, &stWriteSockets)) // In Windows a socket can be in both lists.
	{
		if (!m_boIsConnected)
		{
			m_boIsConnected = true;
			m_boIsFailedToConnect = false;
			OnConnect(m_boIsConnected); // Handles successful attempt to connect.
		}
		return true;
	}

	return m_boIsConnected;
}

bool CConnectionBase::GetIsFailedToConnectFlag() const
{
	return m_boIsFailedToConnect;
}

/*

   CClient

 */

CClient* CClient::Create(CNotificationNetwork* pNotificationNetwork, tukk address, u16 port)
{
	CClient* pClient = new CClient(pNotificationNetwork);
	DRXSOCKET s = pClient->CreateSocket();
	// In the current implementation, this is REALLY UNLIKELY to happen.
	if (s == DRX_INVALID_SOCKET)
	{
		delete pClient;
		return NULL;
	}

	//
	pClient->SetSocket(s);
	pClient->Connect(address, port);

	pClient->SetAddress(address, port);
	pClient->SetSocket(s);
	return pClient;
}

CClient* CClient::Create(CNotificationNetwork* pNotificationNetwork)
{
	CClient* pClient = new CClient(pNotificationNetwork);
	return pClient;
}

//

CClient::CClient(CNotificationNetwork* pNotificationNetwork) :
	CConnectionBase(pNotificationNetwork)
{
}

CClient::~CClient()
{
	GetNotificationNetwork()->ReleaseClients(this);
}

//

void CClient::Update()
{
	m_listeners.NotificationsProcess();
}

// CConnectionBase

bool CClient::OnConnect(bool boConnected)
{
	if (boConnected)
	{
		for (size_t i = 0; i < m_listeners.Count(); ++i)
		{
			if (!SendMessage(eMessage_ChannelRegister, m_listeners.Channel(i), 0))
				return false;
		}
	}

	DrxAutoLock<DrxCriticalSection> lock(m_stConnectionCallbacksLock);
	for (size_t nCount = 0; nCount < m_cNotificationNetworkConnectionCallbacks.size(); ++nCount)
	{
		m_cNotificationNetworkConnectionCallbacks[nCount]->OnConnect(this, boConnected);
	}

	return boConnected;
}

bool CClient::OnDisconnect()
{
	DrxAutoLock<DrxCriticalSection> lock(m_stConnectionCallbacksLock);
	for (size_t nCount = 0; nCount < m_cNotificationNetworkConnectionCallbacks.size(); ++nCount)
	{
		m_cNotificationNetworkConnectionCallbacks[nCount]->OnDisconnected(this);
	}

	return true;
}

bool CClient::OnMessage(EMessage eMessage, const CChannel& channel)
{
	return false;
}

// INotificationNetworkClient

bool CClient::Connect(tukk address, u16 port)
{
	bool bReturnValue(false);

	if (m_socket == DRX_INVALID_SOCKET)
	{
		m_socket = CreateSocket();
	}

	bReturnValue = CConnectionBase::Connect(address, port);
	if (bReturnValue)
	{
		SetAddress(address, port);
	}

	return bReturnValue;
}

bool CClient::ListenerBind(tukk channelName, INotificationNetworkListener* pListener)
{
	if (!CChannel::IsNameValid(channelName))
		return false;

	if (!m_listeners.Bind(CChannel(channelName), pListener))
		return false;

	if (!SendMessage(eMessage_ChannelRegister, CChannel(channelName), 0))
		return false;

	return true;
}

bool CClient::ListenerRemove(INotificationNetworkListener* pListener)
{
	CChannel* pChannel = m_listeners.Channel(pListener);
	if (!pChannel)
		return false;

	if (!m_listeners.Remove(pListener))
		return false;

	if (!SendMessage(eMessage_ChannelUnregister, *pChannel, 0))
		return false;

	return true;
}

bool CClient::Send(tukk channelName, ukk pBuffer, size_t length)
{
	DRX_ASSERT(CChannel::IsNameValid(channelName));
	//	DRX_ASSERT_MESSAGE(channelLength <= NN_CHANNEL_NAME_LENGTH_MAX,
	//		"Channel name \"%s\" was passed to a Notification Network method, the name cannot be longer than %d chars.",
	//		channel, NN_CHANNEL_NAME_LENGTH_MAX);

	if (!CChannel::IsNameValid(channelName))
		return false;
	if (!SendNotification(CChannel(channelName), pBuffer, length))
		return false;

	return true;
}

bool CClient::RegisterCallbackListener(INotificationNetworkConnectionCallback* pConnectionCallback)
{
	DrxAutoLock<DrxCriticalSection> lock(m_stConnectionCallbacksLock);
	return stl::push_back_unique(m_cNotificationNetworkConnectionCallbacks, pConnectionCallback);
}

bool CClient::UnregisterCallbackListener(INotificationNetworkConnectionCallback* pConnectionCallback)
{
	DrxAutoLock<DrxCriticalSection> lock(m_stConnectionCallbacksLock);
	return stl::find_and_erase(m_cNotificationNetworkConnectionCallbacks, pConnectionCallback);
}

/*

   CNotificationNetwork::CConnection

 */

CNotificationNetwork::CConnection::CConnection(CNotificationNetwork* pNotificationNetwork, DRXSOCKET s) :
	CConnectionBase(pNotificationNetwork)
{
	SetSocket(s);
	m_listeningChannels.reserve(8);
}

CNotificationNetwork::CConnection::~CConnection()
{
}

//

bool CNotificationNetwork::CConnection::IsListening(const CChannel& channel)
{
	for (size_t i = 0; i < m_listeningChannels.size(); ++i)
	{
		if (m_listeningChannels[i] == channel)
			return true;
	}

	return false;
}

// CConnectionBase

bool CNotificationNetwork::CConnection::OnMessage(EMessage eMessage, const CChannel& channel)
{
	switch (eMessage)
	{
	case eMessage_ChannelRegister:
		for (size_t i = 0; i < m_listeningChannels.size(); ++i)
		{
			if (m_listeningChannels[i] == channel)
				return true;
		}
		m_listeningChannels.push_back(channel);
		return true;

	case eMessage_ChannelUnregister:
		for (size_t i = 0; i < m_listeningChannels.size(); ++i)
		{
			if (m_listeningChannels[i] != channel)
				continue;

			m_listeningChannels[i] = m_listeningChannels.back();
			m_listeningChannels.pop_back();
			return true;
		}
		return true;
	}

	return false;
}

/*

   CNotificationNetwork::CThread

 */

CNotificationNetwork::CThread::CThread()
{
	m_pNotificationNetwork = NULL;
	m_bRun = true;
}

CNotificationNetwork::CThread::~CThread()
{
}

//

bool CNotificationNetwork::CThread::Begin(CNotificationNetwork* pNotificationNetwork)
{
	m_pNotificationNetwork = pNotificationNetwork;

	if (!gEnv->pThreadUpr->SpawnThread(this, NN_THREAD_NAME))
	{
		DrxFatalError("Error spawning \"%s\" thread.", NN_THREAD_NAME);
	}

	return true;
}

void CNotificationNetwork::CThread::SignalStopWork()
{
	m_bRun = false;
	//	WaitForThread();

	// TODO: Should properly close!
}

// DrxRunnable

void CNotificationNetwork::CThread::ThreadEntry()
{
	while (m_bRun)
	{
		m_pNotificationNetwork->ProcessSockets();
	}
}

/*

   CNotificationNetwork

 */

CNotificationNetwork* CNotificationNetwork::Create()
{
#if DRX_PLATFORM_WINDOWS
	WSADATA wsaData;
	::WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif

	// Note: Because of platform differences we can't do a compare to satisfy CppCheck here
	// cppcheck-ignore resourceLeak
	DRXSOCKET s = DrxSock::socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0 || s == DRX_INVALID_SOCKET)
	{
		DrxLog("CNotificationNetwork::Create: Failed to create socket.\n");
		return NULL;
	}

	// Disable nagling of small blocks to fight high latency connection
#if DRX_PLATFORM_ORBIS || DRX_PLATFORM_WINDOWS
	i32k yes = 1;
	if (DrxSock::setsockopt(s, SOL_SOCKET, TCP_NODELAY,
	                        (tukk)&yes, sizeof(i32)) < 0)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Create: Failed to set TCP_NODELAY option.");
		return NULL;
	}
#endif

#if DRX_PLATFORM_WINDOWS
	u_long nAsyncSockectOperation(1);
	if (ioctlsocket(s, FIONBIO, &nAsyncSockectOperation) == DRX_SOCKET_ERROR)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Connect: Failed to set socket to asynchronous operation.");
		return NULL;
	}
#elif DRX_PLATFORM_ORBIS
	if (DrxSock::setsockopt(s, SOL_SOCKET, SO_NBIO, (tukk)&yes, sizeof(i32)) < 0)
	{
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		DrxLog("CNotificationNetworkClient::Connect: Failed to set socket to asynchronous operation.");
		return NULL;
	}
#endif

	DRXSOCKADDR_IN addr;
	::memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	if (gEnv->IsEditor())
	{
		// Editor uses a different port to avoid conflicts when running both editor and game on same PC
		// But allows the lua remote debugger to connect to the editor
		addr.sin_port = htons(9433);
	}
	else
	{
		addr.sin_port = htons(9432);
	}
	if (DrxSock::bind(s, (const DRXSOCKADDR*)&addr, sizeof(addr)) < 0)
	{
		DrxLog("CNotificationNetwork::Create: Failed to bind socket.\n");
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		return NULL;
	}

	if (DrxSock::listen(s, 8) < 0)
	{
		DrxLog("CNotificationNetwork::Create: Failed to listen.\n");
		DrxSock::closesocket(s);
		s = DRX_INVALID_SOCKET;
		return NULL;
	}

	CNotificationNetwork* pNotificationNetwork = new CNotificationNetwork();
	pNotificationNetwork->m_socket = s;

	pNotificationNetwork->m_thread.Begin(pNotificationNetwork);

	return pNotificationNetwork;
}

//

CNotificationNetwork::CNotificationNetwork()
{
	m_socket = DRX_INVALID_SOCKET;

	m_connections.reserve(4);

	m_listeners.Bind("Query", &g_queryNotification);
}

CNotificationNetwork::~CNotificationNetwork()
{
	m_thread.SignalStopWork();
	gEnv->pThreadUpr->JoinThread(&m_thread, eJM_Join);
	while (!m_connections.empty())
	{
		delete m_connections.back();
		m_connections.pop_back();
	}

	if (m_socket != DRX_INVALID_SOCKET)
	{
		DrxSock::closesocket(m_socket);
		m_socket = DRX_INVALID_SOCKET;
	}

#if DRX_PLATFORM_WINDOWS
	::WSACleanup();
#endif
}

//

void CNotificationNetwork::ReleaseClients(CClient* pClient)
{
	// TODO: Use DrxAutoLock
	LockDebug("Lock %s\n", "CNotificationNetwork::ReleaseClients()");
	m_clientsCriticalSection.Lock();
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		if (m_clients[i] != pClient)
			continue;

		m_clients[i] = m_clients.back();
		m_clients.pop_back();
		break;
	}
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock %s\n", "CNotificationNetwork::ReleaseClients()");
}

void CNotificationNetwork::ProcessSockets()
{
	fd_set read;
	FD_ZERO(&read);
	DRXSOCKET socketMax = 0;
	if (m_socket != DRX_INVALID_SOCKET)
	{
		FD_SET(m_socket, &read);
		socketMax = m_socket;
	}
	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		if (m_connections[i]->Validate())
		{
			DRXSOCKET s = m_connections[i]->GetSocket();
			FD_SET(s, &read);

			if (socketMax < s)
				socketMax = s;

			continue;
		}

		// The Connection is invalid, remove it.
		CConnection* pConnection = m_connections[i];
		m_connections[i] = m_connections.back();
		m_connections.pop_back();
		delete pConnection;

		// Invalidate the loop increment since we just removed a Connection and
		// in the process potentially replaced its slot with an unprocessed one.
		--i;

		DrxLog("Notification Network Connection terminated, current total: %d\n",
		       (i32)m_connections.size());
	}

	LockDebug("Lock %s\n", "CNotificationNetwork::ProcessSockets()");
	m_clientsCriticalSection.Lock();
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		if (!m_clients[i]->Validate())
			continue;

		DRXSOCKET s = m_clients[i]->GetSocket();
		FD_SET(s, &read);

		if (socketMax < s)
			socketMax = s;
	}
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock %s\n", "CNotificationNetwork::ProcessSockets()");

	timeval timeOut = { 1, 0 };
	i32 r = (i32)DrxSock::select(i32(socketMax + 1), &read, NULL, NULL, &timeOut);
	if (r == 0)
		return;

	// When we have no sockets, the select statement will fail and not
	// block for even 1 second, as it should...
	if (r < 0)
	{
		// So we force the sleep here for now.
		DrxSleep(1000);
		return;
	}

	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		if (!FD_ISSET(m_connections[i]->GetSocket(), &read))
			continue;

		m_connections[i]->Receive(m_listeners);
	}

	LockDebug("Lock 2 %s\n", "CNotificationNetwork::ProcessSockets()");
	m_clientsCriticalSection.Lock();
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		if (!FD_ISSET(m_clients[i]->GetSocket(), &read))
			continue;

		m_clients[i]->Receive();
	}
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock 2 %s\n", "CNotificationNetwork::ProcessSockets()");

	if (m_socket == DRX_INVALID_SOCKET)
		return;
	if (!FD_ISSET(m_socket, &read))
		return;

	DRXSOCKADDR_IN addr;
	DRXSOCKLEN_T addrLength = sizeof(addr);
	DRXSOCKET s = DrxSock::accept(m_socket, (DRXSOCKADDR*)&addr, &addrLength);
	if (s < 0 || s == DRX_INVALID_SOCKET)
	{
		return;
	}

	m_connections.push_back(new CConnection(this, s));

	DrxLog("Notification Network accepted new Connection, current total: %d\n",
	       (i32)m_connections.size());
}

// INotificationNetwork

INotificationNetworkClient* CNotificationNetwork::CreateClient()
{
	CClient* pClient = CClient::Create(this);

	LockDebug("Lock %s\n", "CNotificationNetwork::CreateClient()");
	m_clientsCriticalSection.Lock();
	m_clients.push_back(pClient);
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock %s\n", "CNotificationNetwork::CreateClient()");

	return pClient;
}

INotificationNetworkClient* CNotificationNetwork::Connect(tukk address, u16 port)
{
	CClient* pClient = CClient::Create(this, address, port);
	if (!pClient)
		return NULL;

	LockDebug("Lock %s\n", "CNotificationNetwork::Connect()");
	m_clientsCriticalSection.Lock();
	m_clients.push_back(pClient);
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock %s\n", "CNotificationNetwork::Connect()");

	return pClient;
}

size_t CNotificationNetwork::GetConnectionCount(tukk channelName)
{
	if (!channelName)
		return m_connections.size();

	if (!CChannel::IsNameValid(channelName))
		return 0;

	CChannel channel(channelName);
	size_t count = 0;
	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		if (!m_connections[i]->IsListening(channel))
			continue;

		++count;
	}
	return count;
}

bool CNotificationNetwork::ListenerBind(tukk channelName, INotificationNetworkListener* pListener)
{
	if (!CChannel::IsNameValid(channelName))
		return false;

	return m_listeners.Bind(CChannel(channelName), pListener);
}

bool CNotificationNetwork::ListenerRemove(INotificationNetworkListener* pListener)
{
	return m_listeners.Remove(pListener);
}

void CNotificationNetwork::Update()
{
	m_listeners.NotificationsProcess();

	LockDebug("Lock %s\n", "CNotificationNetwork::Update()");
	m_clientsCriticalSection.Lock();
	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		m_clients[i]->Update();
	}
	m_clientsCriticalSection.Unlock();
	LockDebug("Unlock %s\n", "CNotificationNetwork::Update()");
}

u32 CNotificationNetwork::Send(tukk channelName, ukk pBuffer, size_t length)
{
	if (!CChannel::IsNameValid(channelName))
		return 0;

	CChannel channel(channelName);

	// TODO: There should be a mutex lock here to ensure thread safety.

	u32 count = 0;
	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		if (!m_connections[i]->IsListening(channel))
			continue;

		if (m_connections[i]->SendNotification(channel, pBuffer, length))
			++count;
	}

	return count;
}
