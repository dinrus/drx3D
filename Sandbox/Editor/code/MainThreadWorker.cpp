// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "MainThreadWorker.h"

namespace Private_MainThreadWorker
{

inline bool IsMainThread()
{
	return gEnv->mMainThreadId == DrxGetCurrentThreadId();
}

} //endns Private_MainThreadWorker

CMainThreadWorker& CMainThreadWorker::GetInstance()
{
	DRX_ASSERT(s_pTheInstance);
	return *s_pTheInstance;
}

CMainThreadWorker::CMainThreadWorker()
{
	DRX_ASSERT(!s_pTheInstance);
	s_pTheInstance = this;
}

void CMainThreadWorker::AddTask(std::function<void()> task)
{
	using namespace Private_MainThreadWorker;

	if (IsMainThread())
	{
		task();
	}
	else
	{
		m_tasks.push(task);
	}
}

bool CMainThreadWorker::TryExecuteNextTask()
{
	std::function<void()> task;
	if (m_tasks.try_pop(task))
	{
		task();
		return true;
	}
	return false;
}

CMainThreadWorker* CMainThreadWorker::s_pTheInstance = nullptr;

