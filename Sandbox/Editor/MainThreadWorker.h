// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxThreading/MultiThread_Containers.h>

#include <functional>

//! CMainThreadWorker stores a list of tasks for execution on the main thread.
class CMainThreadWorker
{
public:
	static CMainThreadWorker& GetInstance();

	CMainThreadWorker();

	void AddTask(std::function<void()> task);
	bool TryExecuteNextTask();

private:
	DrxMT::queue<std::function<void()>> m_tasks;

	static CMainThreadWorker* s_pTheInstance;
};

