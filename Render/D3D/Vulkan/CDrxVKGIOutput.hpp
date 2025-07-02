// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/CDrxVKGIObject.hpp>

class CDrxVKGIAdapter;
class CDrxVKGIOutput : public CDrxVKGIObject
{
public:
	IMPLEMENT_INTERFACES(CDrxVKGIOutput)
	static CDrxVKGIOutput* Create(CDrxVKGIAdapter* Adapter, UINT Output);

	virtual ~CDrxVKGIOutput();

	#pragma region /* IDXGIOutput implementation */

	virtual HRESULT STDMETHODCALLTYPE GetDesc(
	  /* [annotation][out] */
	  _Out_ DXGI_OUTPUT_DESC* pDesc)
	{
		return E_FAIL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDisplayModeList(
	  /* [in] */ DXGI_FORMAT EnumFormat,
	  /* [in] */ UINT Flags,
	  /* [annotation][out][in] */
	  _Inout_ UINT* pNumModes,
	  /* [annotation][out] */
	  _Out_writes_to_opt_(*pNumModes, *pNumModes)  DXGI_MODE_DESC* pDesc)
	{
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE FindClosestMatchingMode(
	  /* [annotation][in] */
	  _In_ const DXGI_MODE_DESC* pModeToMatch,
	  /* [annotation][out] */
	  _Out_ DXGI_MODE_DESC* pClosestMatch,
	  /* [annotation][in] */
	  _In_opt_ IUnknown* pConcernedDevice);

	virtual HRESULT STDMETHODCALLTYPE WaitForVBlank(void) final
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE TakeOwnership(
	  /* [annotation][in] */
	  _In_ IUnknown* pDevice,
	  BOOL Exclusive) final;

	virtual void STDMETHODCALLTYPE ReleaseOwnership(void) final
	{
		
	}

	virtual HRESULT STDMETHODCALLTYPE GetGammaControlCapabilities(
	  /* [annotation][out] */
	  _Out_ DXGI_GAMMA_CONTROL_CAPABILITIES* pGammaCaps) final
	{
		return S_FALSE; 
	}

	virtual HRESULT STDMETHODCALLTYPE SetGammaControl(
	  /* [annotation][in] */
	  _In_ const DXGI_GAMMA_CONTROL* pArray) final
	{
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE GetGammaControl(
	  /* [annotation][out] */
	  _Out_ DXGI_GAMMA_CONTROL* pArray) final
	{
		return S_FALSE; 
	}

	virtual HRESULT STDMETHODCALLTYPE SetDisplaySurface(
	  /* [annotation][in] */
	  _In_ IDXGISurface* pScanoutSurface) final
	{
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData(
	  /* [annotation][in] */
	  _In_ IDXGISurface* pDestination) final
	{
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(
	  /* [annotation][out] */
	  _Out_ DXGI_FRAME_STATISTICS* pStats) final
	{
		return S_FALSE;
	}

	#pragma endregion

protected:
	CDrxVKGIOutput(CDrxVKGIAdapter* pAdapter);

private:
	_smart_ptr<CDrxVKGIAdapter> m_pAdapter;

};
