// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Device/CDrxDX12DeviceChild.hpp>

class CDrxDX12DepthStencilState : public CDrxDX12DeviceChild<ID3D11DepthStencilStateToImplement>
{
public:
	DX12_OBJECT(CDrxDX12DepthStencilState, CDrxDX12DeviceChild<ID3D11DepthStencilStateToImplement> );

	static CDrxDX12DepthStencilState* Create(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc);

	const D3D12_DEPTH_STENCIL_DESC& GetD3D12DepthStencilDesc() const
	{
		return m_Desc12;
	}

	#pragma region /* ID3D11DepthStencilState implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_DEPTH_STENCIL_DESC* pDesc) FINALGFX;

	#pragma endregion

protected:
	CDrxDX12DepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc11, const D3D12_DEPTH_STENCIL_DESC& desc12);

private:
	D3D11_DEPTH_STENCIL_DESC m_Desc11;
	D3D12_DEPTH_STENCIL_DESC m_Desc12;
};
