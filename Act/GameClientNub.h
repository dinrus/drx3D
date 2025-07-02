// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/INetwork.h>

class CGameClientChannel;
class CGameContext;
struct IGameFramework;

class CGameClientNub final :
	public IGameClientNub
{
public:
	CGameClientNub(IGameFramework* pFramework) : m_pGameContext(0), m_pClientChannel(0), m_pFramework(pFramework), m_fInactivityTimeout(300.0f) {};
	virtual ~CGameClientNub();

	// IGameNub
	virtual void                 Release();
	virtual SCreateChannelResult CreateChannel(INetChannel* pChannel, tukk pRequest);
	virtual void                 FailedActiveConnect(EDisconnectionCause cause, tukk description);
	// ~IGameNub

	// IGameClientNub
	virtual INetChannel* GetNetChannel();
	// ~IGameClientNub

	void                Disconnect(EDisconnectionCause cause, tukk msg);
	void                SetGameContext(CGameContext* pGameContext) { m_pGameContext = pGameContext; };
	CGameClientChannel* GetGameClientChannel() const               { return m_pClientChannel; }

	void                ClientChannelClosed();

	void                GetMemoryUsage(IDrxSizer* s) const;

private:
	CGameContext*       m_pGameContext;
	CGameClientChannel* m_pClientChannel;
	IGameFramework*     m_pFramework;
	float               m_fInactivityTimeout;
};