// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/D3D/Vulkan/VKAsyncCommandQueue.hpp>
#include <drx3D/Render/D3D/Vulkan/VKCommandList.hpp>
#include <drx3D/Render/DriverD3D.h>

#ifdef VK_LINKEDADAPTER
	#include <drx3D/Render/Redirections/VkDevice.inl"
#endif

namespace NDrxVulkan
{

void CAsyncCommandQueue::SExecuteCommandlist::Process(const STaskArgs& args)
{
	VkSubmitInfo Info;
	ZeroStruct(Info);

	// *INDENT-OFF*
	Info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	Info.waitSemaphoreCount   = NumWaitableSemaphores;
	Info.pWaitSemaphores      = WaitableSemaphores;
	Info.pWaitDstStageMask    = StageMasks;
	Info.commandBufferCount   = 1;
	Info.pCommandBuffers      = &CommandList;
	Info.signalSemaphoreCount = NumSignalableSemaphores;
	Info.pSignalSemaphores    = SignalableSemaphores;
	// *INDENT-ON*

	VkResult res = vkQueueSubmit(
		args.pCommandListPool->GetVkCommandQueue(),
		1,
		&Info,
		Fence);
	
	args.pCommandListPool->SetSignalledFenceValue(FenceValue);
}

void CAsyncCommandQueue::SResetCommandlist::Process(const STaskArgs& args)
{
	CommandList->Reset();
}

#ifdef VK_LINKEDADAPTER
void CAsyncCommandQueue::SSyncAdapters::Process(const STaskArgs& args)
{
	BroadcastableVkCommandQueue<2>* broadcastCQ = (BroadcastableVkCommandQueue<2>*)(args.pCommandListPool->GetVkCommandQueue());
	broadcastCQ->SyncAdapters(Fence, FenceValue);
}
#endif

void CAsyncCommandQueue::SPresentBackbuffer::Process(const STaskArgs& args)
{
	VkPresentInfoKHR Info;
	ZeroStruct(Info);

	// *INDENT-OFF*
	Info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	Info.waitSemaphoreCount = NumWaitableSemaphores;
	Info.pWaitSemaphores    = WaitableSemaphores;
	Info.swapchainCount     = 1u;
	Info.pSwapchains        = &SwapChain;
	Info.pImageIndices      = &Index;
	// *INDENT-ON*

	*pPresentResult = vkQueuePresentKHR(args.pCommandListPool->GetVkCommandQueue(), &Info);

	DrxInterlockedDecrement(args.QueueFramesCounter);
}

bool CAsyncCommandQueue::IsSynchronous()
{
	return !(CRenderer::CV_r_VkSubmissionThread & BIT(m_pCmdListPool->GetVkQueueType()));
}

CAsyncCommandQueue::CAsyncCommandQueue()
	: m_pCmdListPool(nullptr)
	, m_QueuedFramesCounter(0)
	, m_bStopRequested(false)
	, m_TaskEvent(INT_MAX, 0)
{
}

CAsyncCommandQueue::~CAsyncCommandQueue()
{
	Clear();
}

void CAsyncCommandQueue::Init(CCommandListPool* pCommandListPool)
{
	m_pCmdListPool = pCommandListPool;
	m_QueuedFramesCounter = 0;
	m_bStopRequested = false;
	m_bSleeping = true;

	GetISystem()->GetIThreadUpr()->SpawnThread(this, "Vk AsyncCommandQueue");
}

void CAsyncCommandQueue::Clear()
{
	SignalStop();
	Flush();
	m_TaskEvent.Release();

	GetISystem()->GetIThreadUpr()->JoinThread(this, eJM_Join);

	m_pCmdListPool = nullptr;
}

void CAsyncCommandQueue::ExecuteCommandLists(
	UINT NumCommandLists, VkCommandBuffer const* ppCommandLists,
	INT NumPendingWSemaphores, VkSemaphore const* pPendingWSemaphoreHeap, VkPipelineStageFlags const* pPendingWStageMaskHeap,
	INT NumPendingSSemaphores, VkSemaphore const* pPendingSSemaphoreHeap,
	VkFence PendingFence, UINT64 PendingFenceValue
)
{
	for (i32 i = 0; i < NumCommandLists; ++i)
	{
		SSubmissionTask task;
		ZeroStruct(task);

		task.type = eTT_ExecuteCommandList;
		task.Data.ExecuteCommandList.CommandList = ppCommandLists[i];
		task.Data.ExecuteCommandList.Fence = PendingFence;
		task.Data.ExecuteCommandList.FenceValue = PendingFenceValue;

		if (i == (0))
		{
			task.Data.ExecuteCommandList.NumWaitableSemaphores = NumPendingWSemaphores;
			memcpy(task.Data.ExecuteCommandList.WaitableSemaphores, pPendingWSemaphoreHeap, NumPendingWSemaphores * sizeof(VkSemaphore));
			memcpy(task.Data.ExecuteCommandList.StageMasks, pPendingWStageMaskHeap, NumPendingWSemaphores * sizeof(VkSemaphore));
		}
		
		if (i == (NumCommandLists - 1))
		{
			task.Data.ExecuteCommandList.NumSignalableSemaphores = NumPendingSSemaphores;
			memcpy(task.Data.ExecuteCommandList.SignalableSemaphores, pPendingSSemaphoreHeap, NumPendingSSemaphores * sizeof(VkSemaphore));
		}

		AddTask<SExecuteCommandlist>(task);
	}
}

void CAsyncCommandQueue::ResetCommandList(CCommandList* pCommandList)
{
	SSubmissionTask task;
	ZeroStruct(task);

	task.type = eTT_ResetCommandList;
	task.Data.ResetCommandList.CommandList = pCommandList;

	AddTask<SResetCommandlist>(task);
}

#ifdef VK_LINKEDADAPTER
void CAsyncCommandQueue::SyncAdapters(VkFence pFence, const UINT64 FenceValue)
{
	SSubmissionTask task;
	ZeroStruct(task);

	task.type = eTT_SyncAdapters;
	task.Data.SyncAdapters.Fence = pFence;
	task.Data.SyncAdapters.FenceValue = FenceValue;

	AddTask<SSyncAdapters>(task);
}
#endif

void CAsyncCommandQueue::Present(
	VkSwapchainKHR SwapChain, UINT Index,
	INT NumPendingWSemaphores, VkSemaphore const* pPendingWSemaphoreHeap,
	VkResult* pPresentResult)
{
	DrxInterlockedIncrement(&m_QueuedFramesCounter);

	SSubmissionTask task;
	ZeroStruct(task);

	task.type = eTT_PresentBackbuffer;
	task.Data.PresentBackbuffer.SwapChain = SwapChain;
	task.Data.PresentBackbuffer.Index = Index;
	task.Data.PresentBackbuffer.pPresentResult = pPresentResult;

	task.Data.ExecuteCommandList.NumWaitableSemaphores = NumPendingWSemaphores;
	memcpy(task.Data.ExecuteCommandList.WaitableSemaphores, pPendingWSemaphoreHeap, NumPendingWSemaphores * sizeof(VkSemaphore));

	AddTask<SPresentBackbuffer>(task);

	{
		while (m_QueuedFramesCounter > MAX_FRAMES_GPU_LAG)
		{
			DrxMT::DrxYieldThread();
		}
	}
}

void CAsyncCommandQueue::Flush(UINT64 lowerBoundFenceValue)
{
	if (lowerBoundFenceValue != (~0ULL))
	{
		while (lowerBoundFenceValue > m_pCmdListPool->GetSignalledFenceValue())
		{
			DrxMT::DrxYieldThread();
		}
	}
	else
	{
		while (!m_bSleeping)
		{
			DrxMT::DrxYieldThread();
		}
	}
}

void CAsyncCommandQueue::FlushNextPresent()
{
	i32k numQueuedFrames = m_QueuedFramesCounter;
	if (numQueuedFrames > 0)
	{
		while (numQueuedFrames == m_QueuedFramesCounter)
		{
			DrxMT::DrxYieldThread();
		}
	}
}

void CAsyncCommandQueue::ThreadEntry()
{
	const STaskArgs taskArgs = { m_pCmdListPool, &m_QueuedFramesCounter };
	SSubmissionTask task;

	while (!m_bStopRequested)
	{
		m_TaskEvent.Acquire();

		if (m_TaskQueue.dequeue(task))
		{
			switch (task.type)
			{
			case eTT_ExecuteCommandList:
				task.Process<SExecuteCommandlist>(taskArgs);
				break;
			case eTT_ResetCommandList:
				task.Process<SResetCommandlist>(taskArgs);
				break;
#ifdef VK_LINKEDADAPTER
			case eTT_SyncAdapters:
				task.Process<SSyncAdapters>(taskArgs);
				break;
#endif
			case eTT_PresentBackbuffer:
				task.Process<SPresentBackbuffer>(taskArgs);
				break;
			}
		}
	}
}

}
