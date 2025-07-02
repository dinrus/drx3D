// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if VK_USE_DXGI

#include <drx3D/Render/D3D/Vulkan/CDrxVKGIObject.hpp>

class CDrxVKGIAdapter_DXGI;

class CDrxVKGIOutput_DXGI : public CDrxVKGIOutput
{
public:
	static CDrxVKGIOutput_DXGI* Create(CDrxVKGIAdapter_DXGI* Adapter, _smart_ptr<IDXGIOutputToCall> pNativeDxgiOutput, UINT Output);

	virtual HRESULT STDMETHODCALLTYPE GetDisplayModeList(
		/* [in] */ DXGI_FORMAT EnumFormat,
		/* [in] */ UINT Flags,
		/* [annotation][out][in] */
		_Inout_ UINT* pNumModes,
		/* [annotation][out] */
		_Out_writes_to_opt_(*pNumModes, *pNumModes)  DXGI_MODE_DESC* pDesc) final;

	virtual HRESULT STDMETHODCALLTYPE FindClosestMatchingMode(
		/* [annotation][in] */
		_In_ const DXGI_MODE_DESC* pModeToMatch,
		/* [annotation][out] */
		_Out_ DXGI_MODE_DESC* pClosestMatch,
		/* [annotation][in] */
		_In_opt_ IUnknown* pConcernedDevice) final;

	virtual HRESULT STDMETHODCALLTYPE GetDesc(
		/* [annotation][out] */
		_Out_ DXGI_OUTPUT_DESC* pDesc);

protected:
	CDrxVKGIOutput_DXGI(CDrxVKGIAdapter_DXGI* pAdapter, _smart_ptr<IDXGIOutputToCall> pNativeDxgiOutput, UINT Output);

private:
	_smart_ptr<IDXGIOutputToCall> m_pNativeDxgiOutput;
};

#endif
