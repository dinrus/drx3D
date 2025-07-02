// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HandlerBase.h"
#include "SyncLock.h"

struct ServerHandler : public HandlerBase
{
	ServerHandler(tukk bucket, i32 affinity, i32 serverTimeout);

	void DoScan();
	bool Sync();

private:
	i32 m_serverTimeout;
	std::vector<std::unique_ptr<SSyncLock>> m_clientLocks;
	std::vector<std::unique_ptr<SSyncLock>> m_srvLocks;
	CTimeValue                              m_lastScan;
};
