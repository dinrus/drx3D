// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#if ENABLE_NET_DEBUG_INFO

	#include  <drx3D/Network/NetDebugServerInfo.h>
	#include  <drx3D/Network/NetCVars.h>
	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/CoreX/Game/IGameFramework.h>

CNetDebugServerInfo::CNetDebugServerInfo()
	: CNetDebugInfoSection(CNetDebugInfo::eP_ServerInfo, 50.0F, 100.0F)
{
}

CNetDebugServerInfo::~CNetDebugServerInfo()
{
}

void CNetDebugServerInfo::Tick(const SNetworkProfilingStats* const pProfilingStats)
{
}

void CNetDebugServerInfo::Draw(const SNetworkProfilingStats* const pProfilingStats)
{
	IGameFramework* pGameFramework = gEnv->pGameFramework;

	if (pGameFramework)
	{
		m_line = 0;

		CNetNub* pServerNub = static_cast<CNetNub*>(pGameFramework->GetServerNetNub());

		if (pServerNub)
		{
			pServerNub->NetDump(eNDT_ServerConnectionState, *this);
		}

		CNetNub* pClientNub = static_cast<CNetNub*>(pGameFramework->GetClientNetNub());

		if (pClientNub)
		{
			pClientNub->NetDump(eNDT_ClientConnectionState, *this);
		}
	}
}

void CNetDebugServerInfo::Log(tukk pFmt, ...)
{
	va_list args;

	va_start(args, pFmt);
	VDrawLine(m_line, s_white, pFmt, args);
	va_end(args);
	++m_line;
}

#endif
