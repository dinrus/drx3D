// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HandlerBase.h"
#include "SyncLock.h"

struct ClientHandler : public HandlerBase
{
	ClientHandler(tukk bucket, i32 affinity, i32 clientTimeout);

	void Reset();
	bool ServerIsValid();
	bool Sync();

private:
	i32                        m_clientTimeout;
	std::unique_ptr<SSyncLock> m_clientLock;
	std::unique_ptr<SSyncLock> m_srvLock;
};
