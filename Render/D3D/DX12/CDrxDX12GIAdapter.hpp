// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/CDrxDX12Object.hpp>
#include <drx3D/Render/CDrxDX12GIFactory.hpp>

class CDrxDX12GIAdapter : public CDrxDX12GIObject<IDXGIAdapter3ToImplement>
{
public:
	DX12_OBJECT(CDrxDX12GIAdapter, CDrxDX12GIObject<IDXGIAdapter3ToImplement>);

	static CDrxDX12GIAdapter* Create(CDrxDX12GIFactory* factory, UINT Adapter);

	ILINE CDrxDX12GIFactory*   GetFactory    () const { return m_pFactory; }
	ILINE IDXGIAdapter3ToCall* GetDXGIAdapter() const { return m_pDXGIAdapter3; }

	#pragma region /* IDXGIAdapter implementation */

	VIRTUALGFX HRESULT STDMETHODCALLTYPE EnumOutputs(
	  /* [in] */ UINT Output,
	  /* [annotation][out][in] */
	  _COM_Outptr_ IDXGIOutput** ppOutput) FINALGFX;

	VIRTUALGFX HRESULT STDMETHODCALLTYPE GetDesc(
	  /* [annotation][out] */
	  _Out_ DXGI_ADAPTER_DESC* pDesc) FINALGFX
	{
		return m_pDXGIAdapter3->GetDesc(pDesc);
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(
	  /* [annotation][in] */
	  _In_ REFGUID InterfaceName,
	  /* [annotation][out] */
	  _Out_ LARGE_INTEGER* pUMDVersion) FINALGFX
	{
		return m_pDXGIAdapter3->CheckInterfaceSupport(InterfaceName, pUMDVersion);
	}

	#pragma endregion

	#pragma region /* IDXGIAdapter1 implementation */

	VIRTUALGFX HRESULT STDMETHODCALLTYPE GetDesc1(
	  _Out_ DXGI_ADAPTER_DESC1* pDesc) FINALGFX
	{
		return m_pDXGIAdapter3->GetDesc1(pDesc);
	}

	#pragma endregion

	#pragma region /* IDXGIAdapter2 implementation */
	#if defined(__dxgi1_2_h__) || defined(__d3d11_x_h__)

	VIRTUALGFX HRESULT STDMETHODCALLTYPE GetDesc2(
	  /* [annotation][out] */
	  _Out_ DXGI_ADAPTER_DESC2* pDesc) FINALGFX
	{
		return m_pDXGIAdapter3->GetDesc2(pDesc);
	}

	#endif
	#pragma endregion

	#pragma region /* IDXGIAdapter3 implementation */
	#ifdef __dxgi1_4_h__

	VIRTUALGFX HRESULT STDMETHODCALLTYPE RegisterHardwareContentProtectionTeardownStatusEvent(
	  /* [annotation][in] */
	  _In_ HANDLE hEvent,
	  /* [annotation][out] */
	  _Out_ DWORD* pdwCookie) FINALGFX
	{
		return m_pDXGIAdapter3->RegisterHardwareContentProtectionTeardownStatusEvent(hEvent, pdwCookie);
	}

	VIRTUALGFX void STDMETHODCALLTYPE UnregisterHardwareContentProtectionTeardownStatus(
	  /* [annotation][in] */
	  _In_ DWORD dwCookie) FINALGFX
	{
		return m_pDXGIAdapter3->UnregisterHardwareContentProtectionTeardownStatus(dwCookie);
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE QueryVideoMemoryInfo(
	  /* [annotation][in] */
	  _In_ UINT NodeIndex,
	  /* [annotation][in] */
	  _In_ DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
	  /* [annotation][out] */
	  _Out_ DXGI_QUERY_VIDEO_MEMORY_INFO* pVideoMemoryInfo) FINALGFX
	{
		return m_pDXGIAdapter3->QueryVideoMemoryInfo(NodeIndex, MemorySegmentGroup, pVideoMemoryInfo);
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE SetVideoMemoryReservation(
	  /* [annotation][in] */
	  _In_ UINT NodeIndex,
	  /* [annotation][in] */
	  _In_ DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
	  /* [annotation][in] */
	  _In_ UINT64 Reservation) FINALGFX
	{
		return m_pDXGIAdapter3->SetVideoMemoryReservation(NodeIndex, MemorySegmentGroup, Reservation);
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE RegisterVideoMemoryBudgetChangeNotificationEvent(
	  /* [annotation][in] */
	  _In_ HANDLE hEvent,
	  /* [annotation][out] */
	  _Out_ DWORD* pdwCookie) FINALGFX
	{
		return m_pDXGIAdapter3->RegisterVideoMemoryBudgetChangeNotificationEvent(hEvent, pdwCookie);
	}

	VIRTUALGFX void STDMETHODCALLTYPE UnregisterVideoMemoryBudgetChangeNotification(
	  /* [annotation][in] */
	  _In_ DWORD dwCookie) FINALGFX
	{
		return m_pDXGIAdapter3->UnregisterVideoMemoryBudgetChangeNotification(dwCookie);
	}

	#endif
	#pragma endregion

protected:
	CDrxDX12GIAdapter(CDrxDX12GIFactory* pFactory, IDXGIAdapter3ToCall* pAdapter);

private:
	CDrxDX12GIFactory*   m_pFactory;
	IDXGIAdapter3ToCall* m_pDXGIAdapter3;
};
