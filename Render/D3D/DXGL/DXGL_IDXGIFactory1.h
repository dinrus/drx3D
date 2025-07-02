// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DXGL_IDXGIFactory1.h
//  Version:     v1.00
//  Created:     11/10/2013 by Valerio Guagliumi.
//  Описание: Contains a definition of the IDXGIFactory1 interface
//               matching the one in the DirectX SDK
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DXGL_IDXGIFactory1_h__
#define __DXGL_IDXGIFactory1_h__

#if !DXGL_FULL_EMULATION
	#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>
#endif //!DXGL_FULL_EMULATION

struct IDXGIFactory1
#if DXGL_FULL_EMULATION
	: IDXGIFactory
#else
	: CDrxDXGLGIObject
#endif
{
	// IDXGIFactory interface
	virtual HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) = 0;
	virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND* pWindowHandle) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter) = 0;

	// IDXGIFactory1 interface
	virtual HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT Adapter, IDXGIAdapter1** ppAdapter) = 0;
	virtual BOOL STDMETHODCALLTYPE    IsCurrent() = 0;
};

#endif // __DXGL_IDXGIFactory1_h__
