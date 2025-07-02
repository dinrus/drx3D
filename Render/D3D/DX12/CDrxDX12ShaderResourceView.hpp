// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Resource/CDrxDX12View.hpp>

class CDrxDX12ShaderResourceView : public CDrxDX12View<ID3D11ShaderResourceViewToImplement>
{
public:
	DX12_OBJECT(CDrxDX12ShaderResourceView, CDrxDX12View<ID3D11ShaderResourceViewToImplement> );

	static CDrxDX12ShaderResourceView* Create(CDrxDX12Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc);

	#pragma region /* ID3D11ShaderResourceView implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) FINALGFX;

	#pragma endregion

	template<class T>
	void BeginResourceStateTransition(T* pCmdList)
	{
		GetDX12View().GetDX12Resource().VerifyBackBuffer();
		pCmdList->BeginResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	template<class T>
	void EndResourceStateTransition(T* pCmdList)
	{
		GetDX12View().GetDX12Resource().VerifyBackBuffer();
		pCmdList->EndResourceStateTransition(GetDX12View().GetDX12Resource(), GetDX12View(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

protected:
	CDrxDX12ShaderResourceView(ID3D11Resource* pResource11, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc11, ID3D12Resource* pResource12, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc12);

private:
	D3D11_SHADER_RESOURCE_VIEW_DESC m_Desc11;
};
