// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIAdapter.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for IDXGIAdapter
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLGIADAPTER__
#define __DRXDXGLGIADAPTER__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>

namespace NDrxOpenGL
{
struct SAdapter;
}

class CDrxDXGLGIFactory;
class CDrxDXGLGIOutput;

class CDrxDXGLGIAdapter : public CDrxDXGLGIObject
{
public:
#if DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLGIAdapter, DXGIAdapter)
#endif //DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLGIAdapter, DXGIAdapter1)

	CDrxDXGLGIAdapter(CDrxDXGLGIFactory* pFactory, NDrxOpenGL::SAdapter* pGLAdapter);
	virtual ~CDrxDXGLGIAdapter();

	bool                  Initialize();
	D3D_FEATURE_LEVEL     GetSupportedFeatureLevel();
	NDrxOpenGL::SAdapter* GetGLAdapter();

	// IDXGIObject overrides
	HRESULT GetParent(REFIID riid, uk * ppParent);

	// IDXGIAdapter implementation
	HRESULT EnumOutputs(UINT Output, IDXGIOutput** ppOutput);
	HRESULT GetDesc(DXGI_ADAPTER_DESC* pDesc);
	HRESULT CheckInterfaceSupport(REFGUID InterfaceName, LARGE_INTEGER* pUMDVersion);

	// IDXGIAdapter1 implementation
	HRESULT GetDesc1(DXGI_ADAPTER_DESC1* pDesc);
protected:
	std::vector<_smart_ptr<CDrxDXGLGIOutput>> m_kOutputs;
	_smart_ptr<CDrxDXGLGIFactory>             m_spFactory;

	_smart_ptr<NDrxOpenGL::SAdapter>          m_spGLAdapter;
	DXGI_ADAPTER_DESC                         m_kDesc;
	DXGI_ADAPTER_DESC1                        m_kDesc1;
	D3D_FEATURE_LEVEL                         m_eSupportedFeatureLevel;
};

#endif //__DRXDXGLGIADAPTER__
