// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDeviceChild.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11DeviceChild
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLDEVICECHILD__
#define __DRXDXGLDEVICECHILD__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLBase.hpp>

class CDrxDXGLDevice;

class CDrxDXGLDeviceChild : public CDrxDXGLBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDeviceChild, D3D11DeviceChild)

	CDrxDXGLDeviceChild(CDrxDXGLDevice* pDevice = NULL);
	virtual ~CDrxDXGLDeviceChild();

	void SetDevice(CDrxDXGLDevice* pDevice);

	// ID3D11DeviceChild implementation
	void STDMETHODCALLTYPE    GetDevice(ID3D11Device** ppDevice);
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData);
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, ukk pData);
	HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);

#if !DXGL_FULL_EMULATION
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLDeviceChild>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLBase::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
protected:
	CDrxDXGLDevice*              m_pDevice;
	CDrxDXGLPrivateDataContainer m_kPrivateDataContainer;
};

#if !DXGL_FULL_EMULATION
struct ID3D11Counter : CDrxDXGLDeviceChild {};
struct ID3D11ClassLinkage : CDrxDXGLDeviceChild {};
#endif //!DXGL_FULL_EMULATION

#endif //__DRXDXGLDEVICECHILD__
