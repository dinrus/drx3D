// Разработка 2001-2017 DinrusPro / Dinrus Group. РНЦП Динрус. 

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DX11CommandListFence.hpp>
#include <drx3D/Render/DriverD3D.h>

namespace NDrxDX11
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCommandListFence::CCommandListFence(CDevice* device)
	: m_pDevice(device)
	, m_CurrentValue(0)
	, m_SubmittedValue(0)
	, m_LastCompletedValue(0)
{
}

//---------------------------------------------------------------------------------------------------------------------
CCommandListFence::~CCommandListFence()
{
	for (i32 f = 0, n = m_Fence.size(); f < n; ++f)
	{
		m_Fence[f]->Release();
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool CCommandListFence::Init()
{
	D3D11_QUERY_DESC FenceInfo;

	ZeroStruct(FenceInfo);

	FenceInfo.Query = D3D11_QUERY_EVENT;
	FenceInfo.MiscFlags = 0; // D3D11_QUERY_MISC_PREDICATEHINT;

	for (i32 f = 0, n = m_Fence.size(); f < n; ++f)
	{
		ID3D11Query* fence;
		if (S_OK != m_pDevice->GetD3D11Device()->CreateQuery(&FenceInfo, &fence))
		{
			DX11_ERROR("Could not create fence object!");
			return false;
		}

		m_Fence[f] = fence;
	}

	return true;
}

void CCommandListFence::WaitForFence(UINT64 fenceValue)
{
	if (!IsCompleted(fenceValue))
	{
		DX11_LOG(DX11_FENCE_ANALYZER, "Waiting CPU for fence: %lld (is %lld currently)", fenceValue, ProbeCompletion());
		{
			if (m_LastCompletedValue < m_SubmittedValue)
			{
				BOOL bQuery = false;

				i32k size  = i32(m_Fence.size());
				i32k start = i32((m_LastCompletedValue + 1) % size);
				i32k end   = i32((m_SubmittedValue     + 1) % size);
				i32k last  = i32((m_SubmittedValue     + 0) % size);

				m_FenceAccessLock.RLock();

				// TODO: optimize
				if (end > start)
				{
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fence[last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
				}
				else
				{
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fence[size - 1], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fence[last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
				}

				m_FenceAccessLock.RUnlock();

			}
		}
		DX11_LOG(DX11_FENCE_ANALYZER, "Completed CPU on fence: %lld", ProbeCompletion());

		AdvanceCompletion();
	}
}

UINT64 CCommandListFence::AdvanceCompletion() threadsafe_const
{
	// Check current completed fence
	UINT64 currentCompletedValue = ProbeCompletion();

	if (m_LastCompletedValue < currentCompletedValue)
	{
		DX11_LOG(DX11_FENCE_ANALYZER, "Completed fence(s): %lld to %lld", m_LastCompletedValue + 1, currentCompletedValue);

		BOOL bQuery = false;

		i32k size  = i32(m_Fence.size());
		i32k start = i32((m_LastCompletedValue  + 1) % size);
		i32k end   = i32((currentCompletedValue + 1) % size);

		m_FenceAccessLock.WLock();

		// TODO: optimize
		if (end > start)
		{
			for (i32 f = start, n = end; f < n; ++f)
				gcpRendD3D->GetDeviceContext().GetData(m_Fence[f], (uk )&bQuery, sizeof(BOOL), 0);
		}
		else
		{
			for (i32 f = start, n = size; f < n; ++f)
				gcpRendD3D->GetDeviceContext().GetData(m_Fence[f], (uk )&bQuery, sizeof(BOOL), 0);
			for (i32 f = 0, n = end; f < n; ++f)
				gcpRendD3D->GetDeviceContext().GetData(m_Fence[f], (uk )&bQuery, sizeof(BOOL), 0);
		}

		m_FenceAccessLock.WUnlock();
	}

#ifdef DX11_IN_ORDER_TERMINATION
	DX11_ASSERT(m_LastCompletedValue <= currentCompletedValue, "Getting new fence which is older than the last!");
	// We do not allow smaller fences being submitted, and fences always complete in-order, no max() neccessary
	m_LastCompletedValue = currentCompletedValue;
#else
	// CLs may terminate in any order. Is it higher than last known completed fence? If so, update it!
	MaxFenceValue(m_LastCompletedValue, currentCompletedValue);
#endif

	return currentCompletedValue;
}

UINT64 CCommandListFence::ProbeCompletion() threadsafe_const
{
	// Check current completed fence
	UINT64 currentCompletedValue = m_LastCompletedValue;

	if (m_LastCompletedValue < m_SubmittedValue)
	{
		BOOL bQuery = false;

		i32k size  = i32(m_Fence.size());
		i32k start = i32(m_LastCompletedValue + 1);
		i32k end   = i32(m_SubmittedValue     + 1);
			
		for (i32 i = start; i < end; ++i)
		{
			if ((gcpRendD3D->GetDeviceContext().GetData(m_Fence[i % size], (uk )&bQuery, sizeof(BOOL), D3D11_ASYNC_GETDATA_DONOTFLUSH)) != S_OK)
				break;

			++currentCompletedValue;
		}
	}

	return currentCompletedValue;
}

//---------------------------------------------------------------------------------------------------------------------
CCommandListFenceSet::CCommandListFenceSet(CDevice* device)
	: m_pDevice(device)
{
	// *INDENT-OFF*
	m_LastCompletedValues[CMDQUEUE_IMMEDIATE] = 0;
	m_SubmittedValues    [CMDQUEUE_IMMEDIATE] = 0;
	m_SignalledValues    [CMDQUEUE_IMMEDIATE] = 0;
	m_CurrentValues      [CMDQUEUE_IMMEDIATE] = 0;
	// *INDENT-ON*
}

//---------------------------------------------------------------------------------------------------------------------
CCommandListFenceSet::~CCommandListFenceSet()
{
	for (i32 i = 0; i < CMDQUEUE_NUM; ++i)
	{
		for (i32 f = 0, n = m_Fences[i].size(); f < n; ++f)
		{
//			while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[i][f], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
			m_Fences[i][f]->Release();
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool CCommandListFenceSet::Init()
{
	D3D11_QUERY_DESC FenceInfo;

	ZeroStruct(FenceInfo);

	FenceInfo.Query = D3D11_QUERY_EVENT;
	FenceInfo.MiscFlags = 0; // D3D11_QUERY_MISC_PREDICATEHINT;

	for (i32 i = 0; i < CMDQUEUE_NUM; ++i)
	{
		for (i32 f = 0, n = m_Fences[i].size(); f < n; ++f)
		{
			ID3D11Query* fence;
			if (S_OK != m_pDevice->GetD3D11Device()->CreateQuery(&FenceInfo, &fence))
			{
				DX11_ERROR("Could not create fence object!");
				return false;
			}

			m_Fences[i][f] = fence;
		}
	}

	return true;
}

void CCommandListFenceSet::SignalFence(const UINT64 fenceValue, i32k id) threadsafe_const
{
	DX11_ERROR("Fences can't be signaled by the CPU under DX11");
}

void CCommandListFenceSet::WaitForFence(const UINT64 fenceValue, i32k id) threadsafe_const
{
	DX11_LOG(DX11_FENCE_ANALYZER, "Waiting CPU for fence %s: %lld (is %lld currently)",
	       id == CMDQUEUE_IMMEDIATE ? "imm" : "def",
	       fenceValue,
	       ProbeCompletion(id));

	{
		if (m_LastCompletedValues[id] < fenceValue)
		{
			DX11_ASSERT(m_CurrentValues  [id] >= fenceValue, "Fence to be tested not allocated!");
			DX11_ASSERT(m_SubmittedValues[id] >= fenceValue, "Fence to be tested not submitted!");
			DX11_ASSERT(m_SignalledValues[id] >= fenceValue, "Fence to be tested not signalled!");

			BOOL bQuery = false;

			i32k size  = i32(m_Fences[id].size());
			i32k start = i32((m_LastCompletedValues[id] + 1) % size);
			i32k end   = i32((fenceValue                + 1) % size);
			i32k last  = i32((fenceValue                + 0) % size);

			// NOTE: optimal
			if (end > start)
			{
				while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
			}
			else
			{
				while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][size - 1], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
				while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
			}

		}
	}

	DX11_LOG(DX11_FENCE_ANALYZER, "Completed CPU on fence %s: %lld",
	       id == CMDQUEUE_IMMEDIATE ? "imm" : "def",
	       ProbeCompletion(id));

	AdvanceCompletion(id);
}

void CCommandListFenceSet::WaitForFence(const UINT64 (&fenceValues)[CMDQUEUE_NUM]) threadsafe_const
{
	// TODO: the pool which waits for the fence can be omitted (in-order-execution)
	DX11_LOG(DX11_FENCE_ANALYZER, "Waiting CPU for fences: [%lld] (is [%lld] currently)",
	       fenceValues[CMDQUEUE_IMMEDIATE],
	       ProbeCompletion(CMDQUEUE_IMMEDIATE));

	{
		for (i32 id = 0; id < CMDQUEUE_NUM; ++id)
		{
			if (m_LastCompletedValues[id] < fenceValues[id])
			{
				DX11_ASSERT(m_CurrentValues  [id] >= fenceValues[id], "Fence to be tested not allocated!");
				DX11_ASSERT(m_SubmittedValues[id] >= fenceValues[id], "Fence to be tested not submitted!");
				DX11_ASSERT(m_SignalledValues[id] >= fenceValues[id], "Fence to be tested not signalled!");

				BOOL bQuery = false;

				i32k size  = i32(m_Fences[id].size());
				i32k start = i32((m_LastCompletedValues[id] + 1) % size);
				i32k end   = i32((fenceValues          [id] + 1) % size);
				i32k last  = i32((fenceValues          [id] + 0) % size);

				// NOTE: optimal
				if (end > start)
				{
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
				}
				else
				{
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][size - 1], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
					while ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][last - 0], (uk )&bQuery, sizeof(BOOL), 0)) != S_OK);
				}
			}
		}
	}

	DX11_LOG(DX11_FENCE_ANALYZER, "Completed CPU on fences: [%lld]",
	       ProbeCompletion(CMDQUEUE_IMMEDIATE));

	AdvanceCompletion();
}

UINT64 CCommandListFenceSet::ProbeCompletion(i32k id) threadsafe_const
{
	// Check current completed fence
	UINT64 currentCompletedValue = m_LastCompletedValues[id];

	if (m_LastCompletedValues[id] < m_SignalledValues[id])
	{
		BOOL bQuery = false;

		i32k size  = i32(m_Fences[id].size());
		i32k start = i32(m_LastCompletedValues[id] + 1);
		i32k end   = i32(m_SignalledValues    [id] + 1);
			
		for (i32 i = start; i < end; ++i)
		{
			if ((gcpRendD3D->GetDeviceContext().GetData(m_Fences[id][i % size], (uk )&bQuery, sizeof(BOOL), D3D11_ASYNC_GETDATA_DONOTFLUSH)) != S_OK)
				break;

			++currentCompletedValue;
		}
	}

	return currentCompletedValue;
}

}
