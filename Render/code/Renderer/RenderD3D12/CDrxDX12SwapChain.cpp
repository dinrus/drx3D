// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12SwapChain.hpp>

#include <drx3D/Render/DX12/Device/CDrxDX12Device.hpp>
#include <drx3D/Render/DX12/Device/CDrxDX12DeviceContext.hpp>
#include <drx3D/Render/DX12/Resource/CDrxDX12Resource.hpp>
#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture1D.hpp>
#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture2D.hpp>
#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture3D.hpp>

#include <drx3D/Render/DX12/API/DX12Device.hpp>
#include <drx3D/Render/DX12/API/DX12SwapChain.hpp>
#include <drx3D/Render/DX12/API/DX12Resource.hpp>
#include <drx3D/Render/Dx12/API/DX12AsyncCommandQueue.hpp>

CDrxDX12SwapChain* CDrxDX12SwapChain::Create(CDrxDX12Device* pDevice, IDXGIFactory4ToCall* factory, DXGI_SWAP_CHAIN_DESC* pDesc)
{
	CDrxDX12DeviceContext* pDeviceContext = pDevice->GetDeviceContext();
	NDrxDX12::CSwapChain* pDX12SwapChain = NDrxDX12::CSwapChain::Create(pDeviceContext->GetCoreGraphicsCommandListPool(), factory, pDesc);

	if (pDX12SwapChain)
	{
		return DX12_NEW_RAW(CDrxDX12SwapChain(pDevice, pDX12SwapChain));
	}

	return nullptr;
}

CDrxDX12SwapChain* CDrxDX12SwapChain::Create(CDrxDX12Device* pDevice, IDXGISwapChain1ToCall* swapchain, DXGI_SWAP_CHAIN_DESC1* pDesc)
{
	CDrxDX12DeviceContext* pDeviceContext = pDevice->GetDeviceContext();
	NDrxDX12::CSwapChain* pDX12SwapChain = NDrxDX12::CSwapChain::Create(pDeviceContext->GetCoreGraphicsCommandListPool(), swapchain, pDesc);

	if (pDX12SwapChain)
	{
		return DX12_NEW_RAW(CDrxDX12SwapChain(pDevice, pDX12SwapChain));
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12SwapChain::CDrxDX12SwapChain(CDrxDX12Device* pDevice, NDrxDX12::CSwapChain* pSwapChain)
	: Super()
	, m_pDevice(pDevice)
	, m_pDX12SwapChain(pSwapChain)
{
	DX12_FUNC_LOG

	m_pBuffers.resize(m_pDX12SwapChain->GetBackBufferCount());
	memset(m_pBuffers.data(), 0, m_pBuffers.size() * sizeof(m_pBuffers[0]));
}

CDrxDX12SwapChain::~CDrxDX12SwapChain()
{
	DX12_FUNC_LOG

	m_pDX12SwapChain->Release();
}

/* IDXGIDeviceSubObject implementation */

/* IDXGISwapChain implementation */

HRESULT STDMETHODCALLTYPE CDrxDX12SwapChain::Present(
  UINT SyncInterval,
  UINT Flags)
{
	DX12_FUNC_LOG

	UINT Buffer = m_pDX12SwapChain->GetCurrentBackbufferIndex();
	NDrxDX12::CResource& dx12Resource = m_pDX12SwapChain->GetBackBuffer(Buffer);

	if (m_pBuffers[Buffer])
	{
		// HACK: transfer state from "outside" CResource to "inside" CResource
		dx12Resource.SetState         (m_pBuffers[Buffer]->GetDX12Resource().GetState         ());
		dx12Resource.SetAnnouncedState(m_pBuffers[Buffer]->GetDX12Resource().GetAnnouncedState());
	}

	m_pDevice->GetDeviceContext()->Finish(m_pDX12SwapChain);

	if (m_pBuffers[Buffer])
	{
		// HACK: transfer state from "inside" CResource to "outside" CResource
		m_pBuffers[Buffer]->GetDX12Resource().SetState         (dx12Resource.GetState         ());
		m_pBuffers[Buffer]->GetDX12Resource().SetAnnouncedState(dx12Resource.GetAnnouncedState());
	}

	DX12_LOG(g_nPrintDX12, "------------------------------------------------ PRESENT ------------------------------------------------");
	m_pDX12SwapChain->Present(SyncInterval, Flags);

	return m_pDX12SwapChain->GetLastPresentReturnValue();
}

HRESULT STDMETHODCALLTYPE CDrxDX12SwapChain::GetBuffer(
  UINT Buffer,
  _In_ REFIID riid,
  _Outvoid** ppSurface)
{
	DX12_FUNC_LOG
	if (riid == __uuidof(ID3D11Texture2D))
	{
		if (m_pBuffers[Buffer])
		{
			m_pBuffers[Buffer]->AddRef();
			*ppSurface = m_pBuffers[Buffer];
			return S_OK;
		}

		const NDrxDX12::CResource& dx12Resource = m_pDX12SwapChain->GetBackBuffer(Buffer);
		switch (dx12Resource.GetDesc().Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_BUFFER:
			*ppSurface = CDrxDX12Buffer::Create(m_pDevice, this, dx12Resource.GetD3D12Resource());
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			*ppSurface = CDrxDX12Texture1D::Create(m_pDevice, this, dx12Resource.GetD3D12Resource());
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			*ppSurface = m_pBuffers[Buffer] = CDrxDX12Texture2D::Create(m_pDevice, this, dx12Resource.GetD3D12Resource());
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			*ppSurface = CDrxDX12Texture3D::Create(m_pDevice, this, dx12Resource.GetD3D12Resource());
			break;
		case D3D12_RESOURCE_DIMENSION_UNKNOWN:
		default:
			*ppSurface = nullptr;
			DX12_ASSERT(0, "Not implemented!");
			break;
		}
	}
	else
	{
		*ppSurface = nullptr;
		DX12_ASSERT(0, "Not implemented!");
		return -1;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxDX12SwapChain::ResizeBuffers(
  UINT BufferCount,
  UINT Width,
  UINT Height,
  DXGI_FORMAT NewFormat,
  UINT SwapChainFlags)
{
	DX12_FUNC_LOG

	UINT64 FrameFenceValues[CMDQUEUE_NUM];
	UINT64 FrameFenceNulls[CMDQUEUE_NUM] = { 0ULL, 0ULL, 0ULL };

	// Submit pending barriers in case the we try to resize in the middle of a frame
	m_pDX12SwapChain->UnblockBuffers(m_pDevice->GetDeviceContext()->GetCoreGraphicsCommandList());

	// Submit pending command-lists in case there are left-overs, make sure it's flushed to and executed on the hardware
	m_pDevice->FlushAndWaitForGPU();
	m_pDevice->GetDeviceContext()->GetCoreGraphicsCommandListPool().GetFences().GetLastCompletedFenceValues(FrameFenceValues);

	// Give up the buffers, they are passed to the release heap now
	m_pDX12SwapChain->ForfeitBuffers();

	// Clear the release heap of all pending releases, the buffers are gone now
	m_pDevice->GetDX12Device()->FlushReleaseHeap(FrameFenceValues, FrameFenceNulls);

	HRESULT res = m_pDX12SwapChain->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
	if (res == S_OK)
	{
		m_pDX12SwapChain->AcquireBuffers();
	}

	m_pBuffers.resize(m_pDX12SwapChain->GetBackBufferCount());
	memset(m_pBuffers.data(), 0, m_pBuffers.size() * sizeof(m_pBuffers[0]));

	return res;
}

/* IDXGISwapChain1 implementation */

/* IDXGISwapChain2 implementation */

/* IDXGISwapChain3 implementation */
