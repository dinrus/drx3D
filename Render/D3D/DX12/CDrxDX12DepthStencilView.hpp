// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Resource/CDrxDX12View.hpp>

class CDrxDX12DepthStencilView : public CDrxDX12View<ID3D11DepthStencilViewToImplement>
{
public:
	DX12_OBJECT(CDrxDX12DepthStencilView, CDrxDX12View<ID3D11DepthStencilViewToImplement> );

	static CDrxDX12DepthStencilView* Create(CDrxDX12Device* pDevice, ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc);

	#pragma region /* ID3D11DepthStencilView implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) FINALGFX;

	#pragma endregion

	template<class T>
	void BeginResourceStateTransition(T* pCmdList)
	{
		pCmdList->BeginResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), m_Desc11.Flags & D3D11_DSV_READ_ONLY_DEPTH ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	template<class T>
	void EndResourceStateTransition(T* pCmdList)
	{
		pCmdList->EndResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), m_Desc11.Flags & D3D11_DSV_READ_ONLY_DEPTH ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

protected:
	CDrxDX12DepthStencilView(ID3D11Resource* pResource11, const D3D11_DEPTH_STENCIL_VIEW_DESC& rDesc11, ID3D12Resource* pResource12, const D3D12_DEPTH_STENCIL_VIEW_DESC& rDesc12);

	CDrxDX12DepthStencilView(ID3D11Resource* pResource11, const D3D11_DEPTH_STENCIL_VIEW_DESC& rDesc11, ID3D12Resource* pResource12);

private:
	D3D11_DEPTH_STENCIL_VIEW_DESC m_Desc11;
};
