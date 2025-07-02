// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IBackgroundTaskManager.h"
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

#include <DrxThreading/IThreadManager.h>

namespace BackgroundTaskManager
{

typedef i32 TTaskID;

struct STaskHandle
{
	ETaskPriority    priority;
	ETaskThreadMask  threadMask;
	TTaskID          id;
	IBackgroundTask* pTask;

	bool operator<(const STaskHandle& rhs) const
	{
		if (priority < rhs.priority)
			return true;
		if (priority > rhs.priority)
			return false;

		// The older tasks have a higher priority (FIFO).
		return id > rhs.id;
	}
};

struct SCompletedTask
{
	ETaskResult      state;
	TTaskID          id;
	ETaskThreadMask  threadMask;
	IBackgroundTask* pTask;
};

struct SScheduledTask
{
	u32 time;
	STaskHandle  handle;
};

class CTaskManager : public IBackgroundTaskManager, public IEditorNotifyListener
{
public:
	CTaskManager();
	~CTaskManager();

	// IBackgroundTaskManager interface implementation
	virtual void AddTask(IBackgroundTask* pTask, ETaskPriority priority, ETaskThreadMask threadMask) override;
	virtual void ScheduleTask(IBackgroundTask* pTask, ETaskPriority priority, i32 delayMilliseconds, ETaskThreadMask threadMask) override;
	void         AddListener(IBackgroundTaskManagerListener* pListener, tukk name) override;
	void         RemoveListener(IBackgroundTaskManagerListener* pListener) override;

private:
	// IEditorNotifyListener interface implementation
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent ev) override;

	void         Start(u32k threadCount = kDefaultThreadCount);
	void         Stop();
	void         StartScheduledTasks();
	void         AddTask(const STaskHandle& outTask);
	void         AddCompletedTask(const STaskHandle& outTask, ETaskResult resultState);
	void         Update();

	inline bool  IsStopped() const
	{
		return m_bStop;
	}

private:
	// Internal queue (per thread mask)
	class CQueue
	{
	public:
		CQueue();

		// Add task to list
		void AddTask(const STaskHandle& taskHandle);

		// Pop task from list
		void PopTask(STaskHandle& outTaskHandle);

		// Release thread semaphore without adding a task
		void ReleaseSemaphore();

		// Remove all pending tasks
		void Clear();

	private:
		DrxSemaphore             m_semaphore;
		std::vector<STaskHandle> m_pendingTasks;
		DrxMutex                 m_lock;
	};

	// Worker thread class implementation
	class CThread : public IThread
	{
	public:
		CThread(CTaskManager* pManager, CQueue* pQueue);
		~CThread();

		virtual void ThreadEntry();
		void         SignalStopWork();

	private:
		CTaskManager* m_pManager;
		CQueue*       m_pQueue;
		volatile bool m_bQuit;
	};

private:
	static u32k kMaxThreadCloseWaitTime = 10000; // ms
	static u32k kDefaultThreadCount = 4;         // good enough for LiveCreate (main user right now), do not set to less than 2

	CQueue              m_pendingTasks[eTaskThreadMask_COUNT];

	// Task scheduled for execution in the future
	std::vector<SScheduledTask> m_scheduledTasks;

	// Completed tasks (waiting for the "finalize" call)
	std::vector<SCompletedTask> m_completedTasks;

	volatile TTaskID            m_nextTaskID;

	typedef std::vector<CThread*> TWorkerThreads;
	TWorkerThreads m_pThreads;

	DrxMutex       m_tasksLock;
	bool           m_bStop;

	typedef CListenerSet<IBackgroundTaskManagerListener*> TListeners;
	TListeners m_listeners;
};

//////////////////////////////////////////////////////////////////////////
// Implements listeners for all background tasks and sums them for display
//////////////////////////////////////////////////////////////////////////
class CBackgroundTasksListener : public IAsyncTextureCompileListener, public IBackgroundTaskManagerListener
{
public:
	enum EListeners
	{
		eL_AsyncTextureCompile,
		eL_BackgroundTaskmanager,
		eL_Count
	};
	enum EStatus
	{
		eStatusOffline,
		eStatusOnline,
		eStatusDisabled,
		eStatusPending,
	};

public:
	CBackgroundTasksListener()
	{
		m_nTasksPending[eL_AsyncTextureCompile] = 0;
		m_nTasksPending[eL_BackgroundTaskmanager] = 0;
	}

public:
	i32 TotalTasksPending() const { return m_nTasksPending[eL_AsyncTextureCompile] + m_nTasksPending[eL_BackgroundTaskmanager]; }

	// IAsyncTextureCompileListener
	virtual void OnCompilationStarted(tukk source, tukk target, i32 nPending) override
	{
		AUTO_LOCK(m_csState);
		m_nTasksPending[eL_AsyncTextureCompile] = nPending;
	}

	virtual void OnCompilationFinished(tukk source, tukk target, ERcExitCode eReturnCode) override
	{
		AUTO_LOCK(m_csState);
		if (eReturnCode != eRcExitCode_Success)
		{
			m_sFailures.insert(target);
		}
		else
		{
			m_sFailures.erase(target);
		}
	}

	virtual void OnCompilationQueueTriggered(i32 nPending) override
	{
		AUTO_LOCK(m_csState);
		m_nTasksPending[eL_AsyncTextureCompile] = nPending;
	}

	virtual void OnCompilationQueueDepleted() override
	{
		AUTO_LOCK(m_csState);
		m_nTasksPending[eL_AsyncTextureCompile] = 0;
	}
	// ~IAsyncTextureCompileListener

	// IBackgroundTaskManagerListener
	virtual void OnBackgroundTaskAdded(tukk description) override
	{
		AUTO_LOCK(m_csState);
		++m_nTasksPending[eL_BackgroundTaskmanager];
	}

	virtual void OnBackgroundTaskCompleted(ETaskResult taskResult, tukk description) override
	{
		AUTO_LOCK(m_csState);

		--m_nTasksPending[eL_BackgroundTaskmanager];

		assert(taskResult != eTaskResult_Resume);

		if (taskResult == eTaskResult_Failed)
		{
			m_sFailures.insert(description);
		}
		else if (taskResult == eTaskResult_Completed)
		{
			m_sFailures.erase(description);
		}
	}
	// ~IBackgroundTaskManagerListener

	void GetStatus(bool& hasFailures, bool& isIdle, string& statusText, string& extendedStatusText)
	{
		AUTO_LOCK(m_csState);

		string bginfo;
		string tooltip;

		i32k totalTasksPending = TotalTasksPending();
		isIdle = (0 == totalTasksPending);
		if (isIdle)
		{
			bginfo = "Idle";
			tooltip = "No background tasks running";
		}
		else
		{
			bginfo.Format("Running: %d", totalTasksPending);
			tooltip.Format("%d background tasks running", totalTasksPending);
		}

		i32k failureCount = m_sFailures.size();
		hasFailures = (0 != failureCount);
		if (hasFailures)
		{
			string failbgInfo;
			failbgInfo.Format("(Failed: %d)", failureCount);
			bginfo += failbgInfo;

			string failTooltip;
			failTooltip.Format("\n%d failed tasks:\n", failureCount);
			tooltip += failTooltip;

			std::set<string>::const_iterator walk;
			for (walk = m_sFailures.begin(); walk != m_sFailures.end(); walk++)
			{
				tooltip += *walk + "\n";
			}
		}

		statusText = bginfo, tooltip;
		extendedStatusText = tooltip;
	}

public:
	DrxCriticalSection m_csState;
	i32                m_nTasksPending[eL_Count];
	std::set<string>   m_sFailures;
};

}

//-----------------------------------------------------------------------------

