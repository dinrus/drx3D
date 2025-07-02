// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __REMOTECONTROL_H__
#define __REMOTECONTROL_H__

#pragma once

#include <drx3D/Network/IRemoteControl.h>

#include <drx3D/Network/Protocol.h>

class CRemoteControlSystem : public IRemoteControlSystem
{
public:
	static CRemoteControlSystem& GetSingleton();

	IRemoteControlServer*        GetServerSingleton();
	IRemoteControlClient*        GetClientSingleton();

private:
	CRemoteControlSystem();
	~CRemoteControlSystem();

	static CRemoteControlSystem s_singleton;
};

class CRemoteControlServer;

class CRemoteControlServerInternal : public IStreamListener
{
	friend class CRemoteControlServer;

public:
	void Start(u16 serverPort, string password);
	void Stop();
	void SendResult(u32 commandId, string result);

	void OnConnectionAccepted(IStreamSocketPtr pStreamSocket, uk pUserData);
	void OnConnectCompleted(bool succeeded, uk pUserData) { NET_ASSERT(0); }
	void OnConnectionClosed(bool graceful, uk pUserData);
	void OnIncomingData(u8k* pData, size_t nSize, uk pUserData);

	void AddRef() const  {}
	void Release() const {}
	bool IsDead() const  { return false; }

private:
	CRemoteControlServerInternal(CRemoteControlServer* pServer);
	~CRemoteControlServerInternal();

	static void AuthenticationTimeoutTimer(NetTimerId, uk , CTimeValue);
	void GetBindAddress(TNetAddress& addr, u16 port);

	void Reset();

	template<typename T>
	void GC_AuthenticationError()
	{
		if (m_pSocketSession)
		{
			T msg;
			m_pSocketSession->Send((u8*)&msg, sizeof(msg));
			m_pSocketSession->Close();
			m_pSocketSession = NULL;
		}

		Reset();
	}

	CRemoteControlServer*   m_pServer;

	IStreamSocketPtr        m_pSocketListen;
	IStreamSocketPtr        m_pSocketSession;

	TNetAddress             m_remoteAddr;

	string                  m_password;
	SRCONServerMsgChallenge m_challenge;

	enum ESessionState {eSS_Unsessioned, eSS_ChallengeSent, eSS_Authorized};
	ESessionState m_sessionState;

	enum EReceivingState {eRS_ReceivingHead, eRS_ReceivingBody};
	EReceivingState    m_receivingState;

	u8              m_receivingHead[sizeof(SRCONMessageHdr)];
	std::vector<u8> m_receivingBody;

	size_t             m_bufferIndicator; // buffer indicator
	size_t             m_amountRemaining; // in bytes

	NetTimerId         m_authenticationTimeoutTimer;
};

class CRemoteControlServer : public IRemoteControlServer
{
	friend class CRemoteControlServerInternal;

public:
	static CRemoteControlServer& GetSingleton();

	void                         Start(u16 serverPort, const string& password, IRemoteControlServerListener* pListener);
	void                         Stop();
	void                         SendResult(u32 commandId, const string& result);

private:
	CRemoteControlServer();
	~CRemoteControlServer();

	CRemoteControlServerInternal  m_internal;

	IRemoteControlServerListener* m_pListener;

	static CRemoteControlServer   s_singleton;
};

class CRemoteControlClient;

class CRemoteControlClientInternal : public IStreamListener
{
	friend class CRemoteControlClient;

public:
	void Connect(string serverAddr, u16 serverPort, string password);
	void Disconnect();
	void SendCommand(u32 commandId, string command);

	void OnConnectionAccepted(IStreamSocketPtr pStreamSocket, uk pUserData) { NET_ASSERT(0); }
	void OnConnectCompleted(bool succeeded, uk pUserData);
	void OnConnectionClosed(bool graceful, uk pUserData);
	void OnIncomingData(u8k* pData, size_t nSize, uk pUserData);

	void AddRef() const  {}
	void Release() const {}
	bool IsDead() const  { return false; }

private:
	CRemoteControlClientInternal(CRemoteControlClient* pClient);
	~CRemoteControlClientInternal();

	void Reset();

	CRemoteControlClient* m_pClient;

	IStreamSocketPtr      m_pSocketSession;

	string                m_password;

	enum EConnectionState {eCS_NotConnected, eCS_InProgress, eCS_Established};
	EConnectionState m_connectionState;

	enum ESessionState {eSS_Unsessioned, eSS_ChallengeWait, eSS_DigestSent, eSS_Authorized};
	ESessionState m_sessionState;

	enum EReceivingState {eRS_ReceivingHead, eRS_ReceivingBody};
	EReceivingState          m_receivingState;

	u8                    m_receivingHead[sizeof(SRCONMessageHdr)];
	std::vector<u8>       m_receivingBody;

	size_t                   m_bufferIndicator; // buffer indicator
	size_t                   m_amountRemaining; // in bytes

	std::map<u32, string> m_pendingCommands; // <commandID, command>
	u32                   m_resultCommandId;
};

class CRemoteControlClient : public IRemoteControlClient
{
	friend class CRemoteControlClientInternal;

public:
	static CRemoteControlClient& GetSingleton();

	void                         Connect(const string& serverAddr, u16 serverPort, const string& password, IRemoteControlClientListener* pListener);
	void                         Disconnect();
	u32                       SendCommand(const string& command);

private:
	CRemoteControlClient();
	~CRemoteControlClient();

	CRemoteControlClientInternal  m_internal;

	IRemoteControlClientListener* m_pListener;

	static CRemoteControlClient   s_singleton;
};

#endif
