// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Lobby task queue
							- Ensures only 1 task is in progress at once

-------------------------------------------------------------------------
История:
- 21:06:2010 : Created by Colin Gulliver

*************************************************************************/

#ifndef __LOBBY_TASK_QUEUE_H__
#define __LOBBY_TASK_QUEUE_H__
#pragma once

//---------------------------------------------------------------------------
class CLobbyTaskQueue
{
public:
	enum ESessionTask
	{
		eST_None,
		eST_Create,
		eST_Migrate,
		eST_Join,
		eST_Delete,
		eST_SetLocalUserData,
		eST_SessionStart,
		eST_SessionEnd,
		eST_Query,
		eST_Update,
		eST_EnsureBestHost,
		eST_FindGame,
		eST_TerminateHostHinting,
		eST_SessionUpdateSlotType,
		eST_SessionSetLocalFlags,
		eST_SessionRequestDetailedInfo,
		eST_Unload,
		eST_SetupDedicatedServer,
		eST_ReleaseDedicatedServer,
	};

	typedef void (*TaskStartedCallback)(ESessionTask task, uk pArg);

	CLobbyTaskQueue();

	void Reset();
	void Init(TaskStartedCallback cb, uk pArg);

	void ClearNotStartedTasks();

	void AddTask(ESessionTask task, bool bUnique);
	void RestartTask();
	void TaskFinished();
	void Update();
	void CancelTask(ESessionTask task);

	bool HasTaskInProgress();
	i32 GetCurrentQueueSize();
	ESessionTask GetCurrentTask();

	void ClearNonVitalTasks();
	void ClearInternalSessionTasks();

private:
	void CancelTaskByIndex(i32 index);

	static i32k MAX_TASKS = 8;

	ESessionTask m_taskQueue[MAX_TASKS];
	i32 m_numTasks;
	bool m_taskInProgress;

	TaskStartedCallback m_taskStartedCB;
	uk m_pCBArg;
};


//---------------------------------------------------------------------------
inline CLobbyTaskQueue::CLobbyTaskQueue()
{
	m_taskStartedCB = NULL;
	m_pCBArg = NULL;

	Reset();
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::Reset()
{
	for (i32 i = 0; i < MAX_TASKS; ++ i)
	{
		m_taskQueue[i] = eST_None;
	}
	m_numTasks = 0;
	m_taskInProgress = false;
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::Init(TaskStartedCallback cb, uk pArg)
{
	DRX_ASSERT(m_taskStartedCB == NULL);
	m_taskStartedCB = cb;
	m_pCBArg = pArg;
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::ClearNotStartedTasks()
{
	if (m_taskQueue[0] != eST_None)
	{
		i32k startPos = (m_taskInProgress ? 1 : 0);
		for (i32 i = startPos; i < MAX_TASKS; ++ i)
		{
			m_taskQueue[i] = eST_None;
		}
		m_numTasks = startPos;
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::AddTask(ESessionTask task, bool bUnique)
{
	DRX_ASSERT(m_numTasks < MAX_TASKS);
	if (m_numTasks < MAX_TASKS)
	{
		if (bUnique)
		{
			// If we're adding unique, make sure the task isn't in the queue.
			// Note: If the same task is currently in progress then add it again since the data
			// will probably have changed
			i32k startPoint = (m_taskInProgress ? 1 : 0);
			for (i32 i = startPoint; i < MAX_TASKS; ++ i)
			{
				if (m_taskQueue[i] == task)
				{
					return;
				}
			}
		}

		m_taskQueue[m_numTasks] = task;
		++ m_numTasks;
	}
#ifndef _RELEASE
	else
	{
		DrxLog("CLobbyTaskQueue::AddTask() ERROR: run out of space when trying to add task %u", task);
		for (i32 i = 0; i < MAX_TASKS; ++ i)
		{
			DrxLog("  %i: %u", i, m_taskQueue[i]);
		}
	}
#endif
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::RestartTask()
{
	DRX_ASSERT(m_taskInProgress);
	m_taskInProgress = false;
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::TaskFinished()
{
	DRX_ASSERT(m_taskInProgress);
	DRX_ASSERT(m_numTasks > 0);

	if (m_taskInProgress)
	{
		m_taskInProgress = false;
		for (i32 i = 1; i < MAX_TASKS; ++ i)
		{
			m_taskQueue[i - 1] = m_taskQueue[i];
		}
		m_taskQueue[MAX_TASKS - 1] = eST_None;
		-- m_numTasks;
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::Update()
{
	if ((!m_taskInProgress) && (m_numTasks))
	{
		m_taskInProgress = true;
		DRX_ASSERT(m_taskStartedCB);
		(m_taskStartedCB)(m_taskQueue[0], m_pCBArg);
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::CancelTask(ESessionTask task)
{
	if (m_taskInProgress && (m_taskQueue[0] == task))
	{
		m_taskInProgress = false;
	}

	for (i32 i = 0; i < MAX_TASKS; ++ i)
	{
		if (m_taskQueue[i] == task)
		{
			CancelTaskByIndex(i);
			break;
		}
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::CancelTaskByIndex(i32 index)
{
	m_taskQueue[index] = eST_None;
	-- m_numTasks;
	for (i32 i = (index + 1); i < MAX_TASKS; ++ i)
	{
		m_taskQueue[i - 1] = m_taskQueue[i];
		m_taskQueue[i] = eST_None;
	}
}

//---------------------------------------------------------------------------
inline bool CLobbyTaskQueue::HasTaskInProgress()
{
	return m_taskInProgress;
}

//---------------------------------------------------------------------------
inline i32 CLobbyTaskQueue::GetCurrentQueueSize()
{
	return m_numTasks;
}

//---------------------------------------------------------------------------
inline CLobbyTaskQueue::ESessionTask CLobbyTaskQueue::GetCurrentTask()
{
	if (m_taskInProgress)
	{
		return m_taskQueue[0];
	}
	else
	{
		return eST_None;
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::ClearNonVitalTasks()
{
	// Cancels all tasks not currently running that arent either SessionEnd or Delete (since these have to be done)
	i32k startPos = (m_taskInProgress ? 1 : 0);
	for (i32 i = startPos; (i < MAX_TASKS) && (m_taskQueue[i] != eST_None); ++ i)
	{
		if ((m_taskQueue[i] != eST_SessionEnd) && (m_taskQueue[i] != eST_Delete))
		{
			CancelTaskByIndex(i);
			-- i;
		}
	}
}

//---------------------------------------------------------------------------
inline void CLobbyTaskQueue::ClearInternalSessionTasks()
{
	// Cancels all tasks that aren't valid unless we're in a session
	i32k startPos = (m_taskInProgress ? 1 : 0);
	for (i32 i = startPos; (i < MAX_TASKS) && (m_taskQueue[i] != eST_None); ++ i)
	{
		ESessionTask task = m_taskQueue[i];
		if ((task != eST_Join) && (task != eST_Create) && (task != eST_FindGame) && (task != eST_Unload))
		{
			CancelTaskByIndex(i);
			-- i;
		}
	}
}

#endif // __LOBBY_TASK_QUEUE_H__
