// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSwapChain.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for IDXGISwapChain
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/CDrxDXGLGIOutput.hpp>
#include <drx3D/Render/CDrxDXGLSwapChain.hpp>
#include <drx3D/Render/CDrxDXGLTexture2D.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>
#include <drx3D/Render/Implementation/GLContext.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>

CDrxDXGLSwapChain::CDrxDXGLSwapChain(CDrxDXGLDevice* pDevice, const DXGI_SWAP_CHAIN_DESC& kDesc)
	: m_spDevice(pDevice)
{
	DXGL_INITIALIZE_INTERFACE(DXGIDeviceSubObject)
	DXGL_INITIALIZE_INTERFACE(DXGISwapChain)

	m_kDesc = kDesc;
}

CDrxDXGLSwapChain::~CDrxDXGLSwapChain()
{
}

bool CDrxDXGLSwapChain::Initialize()
{
	if (!m_kDesc.Windowed &&
	    FAILED(SetFullscreenState(TRUE, NULL)))
		return false;

	return UpdateTexture(true);
}

bool CDrxDXGLSwapChain::UpdateTexture(bool bSetPixelFormat)
{
	// Create a dummy texture that represents the default back buffer
	D3D11_TEXTURE2D_DESC kBackBufferDesc;
	kBackBufferDesc.Width = m_kDesc.BufferDesc.Width;
	kBackBufferDesc.Height = m_kDesc.BufferDesc.Height;
	kBackBufferDesc.MipLevels = 1;
	kBackBufferDesc.ArraySize = 1;
	kBackBufferDesc.Format = m_kDesc.BufferDesc.Format;
	kBackBufferDesc.SampleDesc = m_kDesc.SampleDesc;
	kBackBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	kBackBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET; // Default back buffer can only be bound as render target
	kBackBufferDesc.CPUAccessFlags = 0;
	kBackBufferDesc.MiscFlags = 0;

	NDrxOpenGL::SDefaultFrameBufferTexturePtr spBackBufferTex(NDrxOpenGL::CreateBackBufferTexture(kBackBufferDesc));
	m_spBackBufferTexture = new CDrxDXGLTexture2D(kBackBufferDesc, spBackBufferTex, m_spDevice);

#if DXGL_FULL_EMULATION
	if (bSetPixelFormat)
	{
		NDrxOpenGL::CDevice* pDevice(m_spDevice->GetGLDevice());
		NDrxOpenGL::TNativeDisplay kNativeDisplay(NULL);
		NDrxOpenGL::TWindowContext kCustomWindowContext(NULL);
		if (!NDrxOpenGL::GetNativeDisplay(kNativeDisplay, m_kDesc.OutputWindow) ||
		    !NDrxOpenGL::CreateWindowContext(kCustomWindowContext, pDevice->GetFeatureSpec(), pDevice->GetPixelFormatSpec(), kNativeDisplay))
			return false;

		spBackBufferTex->SetCustomWindowContext(kCustomWindowContext);
	}
#endif //DXGL_FULL_EMULATION

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGISwapChain implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLSwapChain::Present(UINT SyncInterval, UINT Flags)
{
	NDrxOpenGL::CDevice* pDevice(m_spDevice->GetGLDevice());
#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::CContext* pContext(pDevice->GetCurrentContext());
#else
	NDrxOpenGL::CContext* pContext(pDevice->ReserveContext());
#endif
	if (pContext == NULL)
		return E_FAIL;

	NDrxOpenGL::SDefaultFrameBufferTexture* pGLBackBufferTexture(static_cast<NDrxOpenGL::SDefaultFrameBufferTexture*>(m_spBackBufferTexture->GetGLTexture()));

#if DXGL_FULL_EMULATION
	const NDrxOpenGL::TWindowContext& kWindowContext(
	  pGLBackBufferTexture->m_kCustomWindowContext != NULL ?
	  pGLBackBufferTexture->m_kCustomWindowContext :
	  pDevice->GetDefaultWindowContext());
	pContext->SetWindowContext(kWindowContext);
#else
	const NDrxOpenGL::TWindowContext& kWindowContext(pDevice->GetDefaultWindowContext());
#endif

	pGLBackBufferTexture->Flush(pContext);
	HRESULT kResult(pDevice->Present(kWindowContext) ? S_OK : E_FAIL);

#if !OGL_SINGLE_CONTEXT
	pDevice->ReleaseContext();
#endif
	return kResult;
}

HRESULT CDrxDXGLSwapChain::GetBuffer(UINT Buffer, REFIID riid, uk * ppSurface)
{
	if (Buffer == 0 && riid == __uuidof(ID3D11Texture2D))
	{
		m_spBackBufferTexture->AddRef();
		CDrxDXGLTexture2D::ToInterface(reinterpret_cast<ID3D11Texture2D**>(ppSurface), m_spBackBufferTexture.get());
		return S_OK;
	}
	DXGL_TODO("Support more than one swap chain buffer if required");
	return E_FAIL;
}

HRESULT CDrxDXGLSwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
	NDrxOpenGL::SFrameBufferSpec kFrameBufferSpec;
	if (!SwapChainDescToFrameBufferSpec(kFrameBufferSpec, m_kDesc))
		return E_FAIL;

	NDrxOpenGL::SOutput* pGLOutput(pTarget == NULL ? NULL : CDrxDXGLGIOutput::FromInterface(pTarget)->GetGLOutput());
	return m_spDevice->GetGLDevice()->SetFullScreenState(kFrameBufferSpec, Fullscreen == TRUE, pGLOutput) ? S_OK : E_FAIL;
}

HRESULT CDrxDXGLSwapChain::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
{
	DXGL_NOT_IMPLEMENTED;
	return E_FAIL;
}

HRESULT CDrxDXGLSwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLSwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags)
{
	if (
	  Format == m_kDesc.BufferDesc.Format &&
	  Width == m_kDesc.BufferDesc.Width &&
	  Height == m_kDesc.BufferDesc.Height &&
	  BufferCount == m_kDesc.BufferCount &&
	  SwapChainFlags == m_kDesc.Flags)
		return S_OK; // Nothing to do

	if (BufferCount == m_kDesc.BufferCount)
	{
		m_kDesc.BufferDesc.Format = Format;
		m_kDesc.BufferDesc.Width = Width;
		m_kDesc.BufferDesc.Height = Height;
		m_kDesc.Flags = SwapChainFlags;

		if (UpdateTexture(false))
			return S_OK;
	}

	return E_FAIL;
}

HRESULT CDrxDXGLSwapChain::ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters)
{
	NDrxOpenGL::SDisplayMode kDisplayMode;
	if (!NDrxOpenGL::GetDisplayMode(&kDisplayMode, *pNewTargetParameters) ||
	    m_spDevice->GetGLDevice()->ResizeTarget(kDisplayMode))
		return E_FAIL;
	return S_OK;
}

HRESULT CDrxDXGLSwapChain::GetContainingOutput(IDXGIOutput** ppOutput)
{
	DXGL_NOT_IMPLEMENTED;
	return E_FAIL;
}

HRESULT CDrxDXGLSwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
{
	DXGL_NOT_IMPLEMENTED;
	return E_FAIL;
}

HRESULT CDrxDXGLSwapChain::GetLastPresentCount(UINT* pLastPresentCount)
{
	DXGL_NOT_IMPLEMENTED;
	return E_FAIL;
}
