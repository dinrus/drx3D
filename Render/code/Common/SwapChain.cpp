// Разработка 2001-2017 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/SwapChain.h>

void CSwapChain::ReadSwapChainSurfaceDesc()
{
	DRX_ASSERT(m_pSwapChain);

	HRESULT hr = S_OK;
	ZeroMemory(&m_surfaceDesc, sizeof(DXGI_SURFACE_DESC));

	//////////////////////////////////////////////////////////////////////////
	// Read Back Buffers Surface description from the Swap Chain
	DXGI_SWAP_CHAIN_DESC backBufferSurfaceDesc = GetDesc();

	m_surfaceDesc.Width = (UINT)backBufferSurfaceDesc.BufferDesc.Width;
	m_surfaceDesc.Height = (UINT)backBufferSurfaceDesc.BufferDesc.Height;
#if defined(SUPPORT_DEVICE_INFO) || DRX_RENDERER_VULKAN
	m_surfaceDesc.Format = backBufferSurfaceDesc.BufferDesc.Format;
	m_surfaceDesc.SampleDesc = backBufferSurfaceDesc.SampleDesc;
#elif DRX_PLATFORM_DURANGO
	m_surfaceDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	m_surfaceDesc.SampleDesc.Count = 1;
	m_surfaceDesc.SampleDesc.Quality = 0;
#endif
#endif
	//////////////////////////////////////////////////////////////////////////
	DRX_ASSERT(!FAILED(hr));
}

DXGI_FORMAT CSwapChain::GetSwapChainFormat()
{
#if DRX_PLATFORM_DURANGO
	DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
#elif defined(USE_SDL2_VIDEO)
	DXGI_FORMAT fmt = DXGI_FORMAT_B8G8R8X8_UNORM;
#else
	DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

	switch (CRendererCVars::CV_r_HDRSwapChain)
	{
	case 0: fmt = DXGI_FORMAT_R8G8B8A8_UNORM /*_SRGB*/; break;
	case 1: fmt = DXGI_FORMAT_R16G16B16A16_UNORM; break;
	case 2: fmt = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
#if defined(__dxgi1_5_h__) || false /* TODO */
	case 3: fmt = DXGI_FORMAT_R10G10B10A2_UNORM; break;
	case 4: fmt = DXGI_FORMAT_R16G16B16A16_UNORM; break;
	case 5: fmt = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	case 6: fmt = DXGI_FORMAT_R8G8B8A8_UNORM;  break;
	case 7: fmt = DXGI_FORMAT_R10G10B10A2_UNORM; break;
	case 8: fmt = DXGI_FORMAT_R16G16B16A16_UNORM; break;
	case 9: fmt = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
#endif
	}
#endif

	return fmt;
}

#if !DRX_PLATFORM_DURANGO
CSwapChain CSwapChain::CreateSwapChain(HWND hWnd, DXGIOutput* pOutput, uint32_t width, uint32_t height, bool isMainContext, bool isFullscreen, bool vsync)
{
	IDXGISwapChain* piSwapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC scDesc;

	scDesc.BufferDesc.Width = width;
	scDesc.BufferDesc.Height = height;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Format = GetSwapChainFormat();
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;

	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	scDesc.BufferCount = CRenderer::CV_r_MaxFrameLatency + 1;
	scDesc.OutputWindow = hWnd;

	// Always create a swapchain for windowed mode as per Microsoft recommendations here: https://msdn.microsoft.com/en-us/library/windows/desktop/bb174579(v=vs.85).aspx
	// Specifically: "We recommend that you create a windowed swap chain and allow the end user to change the swap chain to full screen through SetFullscreenState; that is, do not set the Windowed member of DXGI_SWAP_CHAIN_DESC to FALSE to force the swap chain to be full screen."
	scDesc.Windowed = 1;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#if DRX_RENDERER_VULKAN
	if (!vsync)
		scDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
#endif

#if (DRX_RENDERER_DIRECT3D >= 120)
	const bool bWaitable = (isMainContext && CRenderer::CV_r_D3D12WaitableSwapChain && !isFullscreen);
	if (bWaitable)
	{
		// Set this flag to create a waitable object you can use to ensure rendering does not begin while a
		// frame is still being presented. When this flag is used, the swapchain's latency must be set with
		// the IDXGISwapChain2::SetMaximumFrameLatency API instead of IDXGIDevice1::SetMaximumFrameLatency.
		scDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	}
#endif

	DXGI_RATIONAL desktopRefreshRate = scDesc.BufferDesc.RefreshRate;

	if (isFullscreen)
	{
		DXGI_MODE_DESC match;
		if (SUCCEEDED(pOutput->FindClosestMatchingMode(&scDesc.BufferDesc, &match, gcpRendD3D.DevInfo().Device())))
		{
			scDesc.BufferDesc = match;
			desktopRefreshRate = match.RefreshRate;
		}
	}

#if defined(SUPPORT_DEVICE_INFO)
	HRESULT hr = gcpRendD3D.DevInfo().Factory()->CreateSwapChain(gcpRendD3D.DevInfo().Device(), &scDesc, &piSwapChain);
#elif DRX_PLATFORM_ORBIS
	HRESULT hr = CDrxDXOrbisGIFactory().CreateSwapChain(gcpRendD3D.GetDevice().GetRealDevice(), &scDesc, &piSwapChain);
#else
#error UNKNOWN PLATFORM TRYING TO CREATE SWAP CHAIN
#endif
	DXGISwapChain* pSwapChainRawPtr = nullptr;

	DRX_ASSERT(SUCCEEDED(hr) && piSwapChain != nullptr);
	hr = piSwapChain->QueryInterface(__uuidof(DXGISwapChain), (uk *)&pSwapChainRawPtr);
	DRX_ASSERT(SUCCEEDED(hr) && pSwapChainRawPtr != 0);

	piSwapChain->Release();

	_smart_ptr<DXGISwapChain> pSmartSwapChain;
	pSmartSwapChain.Assign_NoAddRef(pSwapChainRawPtr);

	CSwapChain sc = { std::move(pSmartSwapChain) };
#if defined(SUPPORT_DEVICE_INFO)
	auto refreshRate = isFullscreen ? scDesc.BufferDesc.RefreshRate : desktopRefreshRate;
	sc.m_refreshRateNumerator = refreshRate.Numerator;
	sc.m_refreshRateDenominator = refreshRate.Denominator;
#endif

	return sc;
}
#endif

#if DRX_PLATFORM_DURANGO
#if (DRX_RENDERER_DIRECT3D >= 120)
CSwapChain CSwapChain::CreateSwapChain(IDXGIFactory2ToCall* pDXGIFactory, ID3D12Device* pD3D12Device, CDrxDX12Device* pDX12Device, uint32_t width, uint32_t height)
#else
CSwapChain CSwapChain::CreateSwapChain(IDXGIFactory2ToCall* pDXGIFactory, ID3D11Device* pD3D11Device, uint32_t width, uint32_t height)
#endif
{
	// Create full HD swap chain with backbuffer
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = GetSwapChainFormat();
	swapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGIX_SWAP_CHAIN_MATCH_XBOX360_AND_PC;

	DXGISwapChain* pSwapChain = nullptr;
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("Xbox -- CRenderDisplayContext::CreateSwapChain: CreateSwapChainForCoreWindow()");
#if (DRX_RENDERER_DIRECT3D >= 120)
		IDXGISwapChain1ToCall* pDXGISwapChain = nullptr;
		HRESULT hr = pDXGIFactory->CreateSwapChainForCoreWindow(pD3D12Device, (IUnknown*)gEnv->pWindow, &swapChainDesc, nullptr, &pDXGISwapChain);
		if (hr == S_OK && pDXGISwapChain)
			pSwapChain = CDrxDX12SwapChain::Create(pDX12Device, pDXGISwapChain, &swapChainDesc);
#else
		HRESULT hr = pDXGIFactory->CreateSwapChainForCoreWindow(pD3D11Device, (IUnknown*)gEnv->pWindow, &swapChainDesc, nullptr, &pSwapChain);
#endif

		DRX_ASSERT(SUCCEEDED(hr) && pSwapChain != nullptr);
	}

	return CSwapChain{ pSwapChain };
}
#endif

#if defined(SUPPORT_DEVICE_INFO_USER_DISPLAY_OVERRIDES)
inline tukk GetScanlineOrderNaming(DXGI_MODE_SCANLINE_ORDER v)
{
	switch (v)
	{
	case DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE:
		return "progressive";
	case DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST:
		return "interlaced (upper field first)";
	case DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST:
		return "interlaced (lower field first)";
	default:
		return "unspecified";
	}
}
void CSwapChain::UserOverrideDisplayProperties(DXGI_MODE_DESC& desc)
{
	if (gcpRendD3D->IsFullscreen())
	{
		if (CRenderer::CV_r_overrideRefreshRate > 0)
		{
			DXGI_RATIONAL& refreshRate = desc.RefreshRate;
			if (refreshRate.Denominator)
				gEnv->pLog->Log("Overriding refresh rate to %.2f Hz (was %.2f Hz).", (float)CRenderer::CV_r_overrideRefreshRate, (float)refreshRate.Numerator / (float)refreshRate.Denominator);
			else
				gEnv->pLog->Log("Overriding refresh rate to %.2f Hz (was undefined).", (float)CRenderer::CV_r_overrideRefreshRate);
			refreshRate.Numerator = (u32)(CRenderer::CV_r_overrideRefreshRate * 1000.0f);
			refreshRate.Denominator = 1000;
		}

		if (CRenderer::CV_r_overrideScanlineOrder > 0)
		{
			DXGI_MODE_SCANLINE_ORDER old = desc.ScanlineOrdering;
			DXGI_MODE_SCANLINE_ORDER& so = desc.ScanlineOrdering;
			switch (CRenderer::CV_r_overrideScanlineOrder)
			{
			case 2:
				so = DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST;
				break;
			case 3:
				so = DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST;
				break;
			default:
				so = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
				break;
			}
			gEnv->pLog->Log("Overriding scanline order to %s (was %s).", GetScanlineOrderNaming(so), GetScanlineOrderNaming(old));
		}
	}
}
#endif

void CSwapChain::ResizeSwapChain(uint32_t buffers, uint32_t width, uint32_t height, bool isMainContext, bool bResizeTarget /*= false*/)
{
#if !DRX_PLATFORM_DURANGO 
	// Wait for GPU to finish occupying the resources
	GetDeviceObjectFactory().GetCoreCommandList().GetGraphicsInterface()->ClearState(true);

	m_surfaceDesc.Width = width;
	m_surfaceDesc.Height = height;

#if (DRX_RENDERER_DIRECT3D >= 110) || (DRX_RENDERER_VULKAN >= 10)
	DXGI_SWAP_CHAIN_DESC scDesc = GetDesc();

	scDesc.BufferCount = buffers;
	scDesc.BufferDesc.Width = width;
	scDesc.BufferDesc.Height = height;

#if defined(SUPPORT_DEVICE_INFO_USER_DISPLAY_OVERRIDES)
	if (isMainContext)
		UserOverrideDisplayProperties(scDesc.BufferDesc);
#endif

	//	TODO: Activate once sRGB encoding is gone from the shader
	//	m_devInfo.SwapChainDesc().BufferDesc.Format =
	//		((nNewColDepth / 4) <=  8 ? DXGI_FORMAT_R8G8B8A8_UNORM :
	//		((nNewColDepth / 4) <= 10 ? DXGI_FORMAT_R10G10B10A2_UNORM :
	//		                            DXGI_FORMAT_R16G16B16A16_FLOAT));

	// Resize the hWnd's dimensions associated with the SwapChain (triggers EVENT)
	if (bResizeTarget)
	{
		HRESULT hr = m_pSwapChain->ResizeTarget(&scDesc.BufferDesc);
		DRX_ASSERT(SUCCEEDED(hr));
	}

	// Resize the Resources associated with the SwapChain
	{
		HRESULT hr = m_pSwapChain->ResizeBuffers(scDesc.BufferCount, scDesc.BufferDesc.Width, scDesc.BufferDesc.Height, DXGI_FORMAT_UNKNOWN, scDesc.Flags);
		DRX_ASSERT(SUCCEEDED(hr));

#if (DRX_RENDERER_DIRECT3D >= 120)
		if (scDesc.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
			m_pSwapChain->SetMaximumFrameLatency(CRendererCVars::CV_r_MaxFrameLatency);
#endif
	}
#endif

	ReadSwapChainSurfaceDesc();
#endif
}

HRESULT CSwapChain::Present(u32 syncInterval, u32 flags)
{
	return m_pSwapChain->Present(syncInterval, flags);
}

