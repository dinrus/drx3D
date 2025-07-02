// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Thread/IThreadUpr.h>
#include <drx3D/Plugins/concqueue/concqueue.hpp>
#include <drx3D/Render/DX12Base.hpp>

namespace NDrxDX12
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

	void Present(IDXGISwapChain3ToCall* pSwapChain, HRESULT* pPresentResult, UINT SyncInterval, UINT Flags, const DXGI_SWAP_CHAIN_DESC& Desc, UINT bufferIndex);
	void ResetCommandList(CCommandList* pCommandList);
	void ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
	void Signal(ID3D12Fence* pFence, const UINT64 Value);
	void Wait(ID3D12Fence* pFence, const UINT64 Value);
	void Wait(ID3D12Fence** pFences, const UINT64(&Values)[CMDQUEUE_NUM]);

#ifdef DX12_LINKEDADAPTER
	void SyncAdapters(ID3D12Fence* pFence, const UINT64 Value);
#endif

private:

	enum eTaskType
	{
		eTT_ExecuteCommandList,
		eTT_ResetCommandList,
		eTT_PresentBackbuffer,
		eTT_SignalFence,
		eTT_WaitForFence,
		eTT_WaitForFences,
		eTT_SyncAdapters
	};

	struct STaskArgs
	{
		CCommandListPool*   pCommandListPool;
		 i32*       QueueFramesCounter;
	};

	struct SExecuteCommandlist
	{
		ID3D12CommandList* pCommandList;

		void               Process(const STaskArgs& args);
	};

	struct SResetCommandlist
	{
		CCommandList* pCommandList;

		void          Process(const STaskArgs& args);
	};

	struct SSignalFence
	{
		ID3D12Fence* pFence;
		UINT64       FenceValue;

		void         Process(const STaskArgs& args);
	};

	struct SWaitForFence
	{
		ID3D12Fence* pFence;
		UINT64       FenceValue;

		void         Process(const STaskArgs& args);
	};

	struct SWaitForFences
	{
		ID3D12Fence** pFences;
		UINT64        FenceValues[CMDQUEUE_NUM];

		void          Process(const STaskArgs& args);
	};

#ifdef DX12_LINKEDADAPTER
	struct SSyncAdapters
	{
		ID3D12Fence* pFence;
		UINT64       FenceValue;

		void         Process(const STaskArgs& args);
	};
#endif

	struct SPresentBackbuffer
	{
		IDXGISwapChain3ToCall*      pSwapChain;
		HRESULT*                    pPresentResult;
		UINT                        SyncInterval;
		UINT                        Flags;
		const DXGI_SWAP_CHAIN_DESC* Desc;

		void                        Process(const STaskArgs& args);
	};

	struct SSubmissionTask
	{
		eTaskType type;

		union
		{
			SExecuteCommandlist ExecuteCommandList;
			SResetCommandlist   ResetCommandList;
			SSignalFence        SignalFence;
			SWaitForFence       WaitForFence;
			SWaitForFences      WaitForFences;

#ifdef DX12_LINKEDADAPTER
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
