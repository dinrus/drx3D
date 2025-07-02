// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ServiceUpr.h>
#include  <drx3D/Network/Network.h>
#include  <drx3D/Network/Config.h>

#include  <drx3D/Network/NetProfileTokens.h>

static const float POLL_TIME = 10.0f;

CServiceUpr::CServiceUpr()
{
	SCOPED_GLOBAL_LOCK;
	m_timer = TIMER.ADDTIMER(g_time + POLL_TIME, TimerCallback, this, "CServiceUpr::CServiceUpr() m_timer");
}

CServiceUpr::~CServiceUpr()
{
	SCOPED_GLOBAL_LOCK;
	TIMER.CancelTimer(m_timer);
	for (TServices::iterator it = m_services.begin(), eit = m_services.end(); it != eit; ++it)
	{
		it->second->Close();
	}
}

INetworkServicePtr CServiceUpr::GetService(const string& name)
{
	TServices::iterator iter = m_services.find(name);
	if (iter == m_services.end() || iter->second->GetState() == eNSS_Closed)
	{
		INetworkServicePtr pService = CreateService(name);
		if (pService)
			m_services[name] = pService;
		return pService;
	}
	return iter->second;
}

INetworkServicePtr CServiceUpr::CreateService(const string& name)
{
	if (name == "NetProfileTokens")
		return new CNetProfileTokens();
	return NULL;
}

void CServiceUpr::CreateExtension(bool server, IContextViewExtensionAdder* adder)
{
	for (TServices::iterator iter = m_services.begin(); iter != m_services.end(); ++iter)
	{
		iter->second->CreateContextViewExtensions(server, adder);
	}
}

void CServiceUpr::TimerCallback(NetTimerId id, uk pUser, CTimeValue tm)
{
	CServiceUpr* pThis = static_cast<CServiceUpr*>(pUser);

	for (TServices::iterator iter = pThis->m_services.begin(); iter != pThis->m_services.end(); )
	{
		TServices::iterator next = iter;
		++next;
		if (iter->second->GetState() == eNSS_Closed)
			pThis->m_services.erase(iter);
		iter = next;
	}

	SCOPED_GLOBAL_LOCK;
	pThis->m_timer = TIMER.ADDTIMER(g_time + POLL_TIME, TimerCallback, pThis, "CServiceUpr::TimerCallback() m_timer");
}
