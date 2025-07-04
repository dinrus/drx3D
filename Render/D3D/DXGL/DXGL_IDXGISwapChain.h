// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DXGL_IDXGISwapChain.h
//  Version:     v1.00
//  Created:     11/10/2013 by Valerio Guagliumi.
//  Описание: Contains a definition of the IDXGISwapChain interface
//               matching the one in the DirectX SDK
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DXGL_IDXGISwapChain_h__
#define __DXGL_IDXGISwapChain_h__

#if !DXGL_FULL_EMULATION
	#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>
#endif //!DXGL_FULL_EMULATION

#if DXGL_FULL_EMULATION
struct IDXGISwapChain : IDXGIDeviceSubObject
#else
struct IDXGISwapChain : CDrxDXGLGIObject
#endif
{
	// IDXGISwapChain interface
	virtual HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval, UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, uk * ppSurface) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput** ppOutput) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) = 0;

	// IDXGIDeviceSubObject interface
	virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, uk * ppDevice) = 0;
};

#endif // __DXGL_IDXGISwapChain_h__
