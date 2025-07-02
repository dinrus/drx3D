// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSwapChain.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for IDXGISwapChain
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSWAPCHAIN__
#define __DRXDXGLSWAPCHAIN__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLBase.hpp>
#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>

namespace NDrxOpenGL
{
class CDeviceContextProxy;
}

class CDrxDXGLDevice;
class CDrxDXGLTexture2D;

class CDrxDXGLSwapChain : public CDrxDXGLGIObject
{
public:
#if DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLSwapChain, DXGIDeviceSubObject)
#endif //DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLSwapChain, DXGISwapChain)

	CDrxDXGLSwapChain(CDrxDXGLDevice* pDevice, const DXGI_SWAP_CHAIN_DESC& kDesc);
	~CDrxDXGLSwapChain();

	bool Initialize();

	// IDXGISwapChain implementation
	HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval, UINT Flags);
	HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, uk * ppSurface);
	HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget);
	HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget);
	HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc);
	HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters);
	HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput** ppOutput);
	HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats);
	HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount);

	// IDXGIDeviceSubObject implementation
	HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, uk * ppDevice) { return E_NOTIMPL; }
protected:
	bool                      UpdateTexture(bool bSetPixelFormat);
protected:
	_smart_ptr<CDrxDXGLDevice>    m_spDevice;
	_smart_ptr<CDrxDXGLTexture2D> m_spBackBufferTexture;
	DXGI_SWAP_CHAIN_DESC          m_kDesc;
};

#endif //__DRXDXGLSWAPCHAIN__
