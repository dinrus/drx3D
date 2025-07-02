// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Device/CDrxDX12DeviceChild.hpp>

class CDrxDX12BlendState : public CDrxDX12DeviceChild<ID3D11BlendStateToImplement>
{
public:
	DX12_OBJECT(CDrxDX12BlendState, CDrxDX12DeviceChild<ID3D11BlendStateToImplement> );

	static CDrxDX12BlendState* Create(const D3D11_BLEND_DESC* pBlendStateDesc);

	const D3D12_BLEND_DESC& GetD3D12BlendDesc() const
	{
		return m_Desc12;
	}

	#pragma region /* ID3D11BlendState implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_BLEND_DESC* pDesc) FINALGFX;

	#pragma endregion

protected:
	CDrxDX12BlendState(const D3D11_BLEND_DESC& desc11, const D3D12_BLEND_DESC& desc12);

private:
	D3D11_BLEND_DESC m_Desc11;

	D3D12_BLEND_DESC m_Desc12;
};
