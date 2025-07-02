// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/CDrxVKGIObject.hpp>
#include <drx3D/Render/D3D/Vulkan/CDrxVKGIOutput.hpp>

class CDrxVKGIFactory;
class CDrxVKGIAdapter : public CDrxVKGIObject
{
public:
	IMPLEMENT_INTERFACES(CDrxVKGIAdapter)
	static CDrxVKGIAdapter* Create(CDrxVKGIFactory* factory, UINT Adapter);

	virtual ~CDrxVKGIAdapter();

	ILINE CDrxVKGIFactory* GetFactory() const { return m_pFactory; }

	ILINE UINT GetDeviceIndex()   { return m_deviceIndex; }

	#pragma region /* IDXGIAdapter implementation */

	virtual HRESULT STDMETHODCALLTYPE EnumOutputs(
	  /* [in] */ UINT Output,
	  /* [annotation][out][in] */
	  _COM_Outptr_ IDXGIOutput** ppOutput);

	virtual HRESULT STDMETHODCALLTYPE GetDesc(
	  /* [annotation][out] */
	  _Out_ DXGI_ADAPTER_DESC* pDesc)
	{
		return E_FAIL;
	}

	virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(
	  /* [annotation][in] */
	  _In_ REFGUID InterfaceName,
	  /* [annotation][out] */
	  _Out_ LARGE_INTEGER* pUMDVersion)
	{
		return E_FAIL;
	}

	#pragma endregion

	#pragma region /* IDXGIAdapter1 implementation */

	virtual HRESULT STDMETHODCALLTYPE GetDesc1(
	  _Out_ DXGI_ADAPTER_DESC1* pDesc)
	{
		return S_FALSE;
	}

	#pragma endregion

	#pragma endregion

protected:
	CDrxVKGIAdapter(CDrxVKGIFactory* pFactory, UINT index);

private:
	_smart_ptr<CDrxVKGIFactory> m_pFactory;
	UINT m_deviceIndex;
};
