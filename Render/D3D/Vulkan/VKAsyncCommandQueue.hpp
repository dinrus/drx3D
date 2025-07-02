// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Thread/IThreadUpr.h>
#include <drx3D/Plugins/concqueue/concqueue.hpp>
#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

namespace NDrxVulkan
{

class CCommandListPool;

class CAsyncCommandQueue : public IThread
{
public:
	static i32k MAX_FRAMES_GPU_LAG = 1;   // Maximum number of frames GPU can lag behind CPU. TODO: tie to cvar

	CAsyncCommandQueue();
	~CAsyncCommandQueue();

	bool IsSynchronous();
	void Init(CCommandListPool* pCommandListPool);
	void Clear();
	void Flush(UINT64 lowerBoundFenceValue = ~0ULL);
	void FlushNextPresent();
	void SignalStop() { m_bStopRequested = true; }

	// Equates to the number of pending Present() calls
	i32  GetQueuedFramesCount() const { return m_QueuedFramesCounter; }

	void Present(
		VkSwapchainKHR SwapChain, UINT Index,
		INT NumPendingWSemaphores, VkSemaphore const* pPendingWSemaphoreHeap,
		VkResult* pPresentResult);
	void ResetCommandList(
		CCommandList* pCommandList);
	void ExecuteCommandLists(
		UINT NumCommandLists, VkCommandBuffer const* ppCommandLists,
		INT NumPendingWSemaphores, VkSemaphore const* pPendingWSemaphoreHeap, VkPipelineStageFlags const* pPendingWStageMaskHeap,
		INT NumPendingSSemaphores, VkSemaphore const* pPendingSSemaphoreHeap,
		VkFence PendingFence, UINT64 PendingFenceValue);

#ifdef VK_LINKEDADAPTER
	void   SyncAdapters(VkFence pFence, const UINT64 Value);
#endif

private:

	enum eTaskType
	{
		eTT_ExecuteCommandList,
		eTT_ResetCommandList,
		eTT_PresentBackbuffer,
		eTT_SyncAdapters
	};

	struct STaskArgs
	{
		CCommandListPool* pCommandListPool;
		 i32*     QueueFramesCounter;
	};

	struct SExecuteCommandlist
	{
		VkCommandBuffer CommandList;
		INT             NumWaitableSemaphores;
		INT             NumSignalableSemaphores;
		VkSemaphore     WaitableSemaphores[64];
		VkPipelineStageFlags StageMasks[64];
		VkSemaphore     SignalableSemaphores[64];
		VkFence         Fence;
		UINT64          FenceValue;

		void            Process(const STaskArgs& args);
	};

	struct SResetCommandlist
	{
		CCommandList* CommandList;

		void          Process(const STaskArgs& args);
	};

#ifdef VK_LINKEDADAPTER
	struct SSyncAdapters
	{
		VkNaryFence* Fence;
		UINT64           FenceValue;

		void             Process(const STaskArgs& args);
	};
#endif

	struct SPresentBackbuffer
	{
		VkSwapchainKHR   SwapChain;
		INT              NumWaitableSemaphores;
		VkSemaphore      WaitableSemaphores[64];
		u32           Index;
		VkResult*        pPresentResult;

		void             Process(const STaskArgs& args);
	};

	struct SSubmissionTask
	{
		eTaskType type;

		union
		{
			SExecuteCommandlist ExecuteCommandList;
			SResetCommandlist   ResetCommandList;

#ifdef VK_LINKEDADAPTER
			SSyncAdapters       SyncAdapters;
#endif

			SPresentBackbuffer  PresentBackbuffer;
		} Data;

		template<typename TaskType>
		void Process(const STaskArgs& args)
		{
			TaskType* pTask = reinterpret_cast<TaskType*>(&Data);
			pTask->Process(args);
		}
	};

	template<typename TaskType>
	void AddTask(SSubmissionTask& task)
	{
		if (!IsSynchronous())
		{
			m_TaskQueue.enqueue(task);
			m_TaskEvent.Release();
		}
		else
		{
			Flush();
			STaskArgs taskArgs = { m_pCmdListPool, &m_QueuedFramesCounter };
			task.Process<TaskType>(taskArgs);
		}
	}

	void ThreadEntry() override;

	 i32                            m_QueuedFramesCounter;
	 bool                           m_bStopRequested;
	 bool                           m_bSleeping;

	CCommandListPool*                       m_pCmdListPool;
	ConcQueue<UnboundMPSC, SSubmissionTask> m_TaskQueue;
	DrxFastSemaphore                        m_TaskEvent;
};

}
