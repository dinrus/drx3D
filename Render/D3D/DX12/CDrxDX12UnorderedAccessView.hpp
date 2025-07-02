// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Resource/CDrxDX12View.hpp>

class CDrxDX12UnorderedAccessView : public CDrxDX12View<ID3D11UnorderedAccessViewToImplement>
{
public:
	DX12_OBJECT(CDrxDX12UnorderedAccessView, CDrxDX12View<ID3D11UnorderedAccessViewToImplement> );

	static CDrxDX12UnorderedAccessView* Create(CDrxDX12Device* pDevice, ID3D11Resource* pResource11, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc11);

	#pragma region /* ID3D11UnorderedAccessView implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc) FINALGFX;

	#pragma endregion

	template<class T>
	void BeginResourceStateTransition(T* pCmdList)
	{
		pCmdList->BeginResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	template<class T>
	void EndResourceStateTransition(T* pCmdList)
	{
		pCmdList->EndResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

protected:
	CDrxDX12UnorderedAccessView(ID3D11Resource* pResource11, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc11, ID3D12Resource* pResource12, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc12);

private:
	D3D11_UNORDERED_ACCESS_VIEW_DESC m_Desc11;
};
