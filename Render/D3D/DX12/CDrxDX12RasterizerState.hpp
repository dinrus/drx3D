// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Device/CDrxDX12DeviceChild.hpp>

class CDrxDX12RasterizerState : public CDrxDX12DeviceChild<ID3D11RasterizerStateToImplement>
{
public:
	DX12_OBJECT(CDrxDX12RasterizerState, CDrxDX12DeviceChild<ID3D11RasterizerStateToImplement> );

	static CDrxDX12RasterizerState* Create(const D3D11_RASTERIZER_DESC* pRasterizerDesc);

	const D3D12_RASTERIZER_DESC& GetD3D12RasterizerDesc() const
	{
		return m_Desc12;
	}
	const D3D11_RASTERIZER_DESC& GetD3D11RasterizerDesc() const
	{
		return m_Desc11;
	}

	#pragma region /* ID3D11RasterizerState implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_RASTERIZER_DESC* pDesc) FINALGFX;

	#pragma endregion

protected:
	CDrxDX12RasterizerState(const D3D11_RASTERIZER_DESC& desc11, const D3D12_RASTERIZER_DESC& desc12);

private:
	D3D11_RASTERIZER_DESC m_Desc11;
	D3D12_RASTERIZER_DESC m_Desc12;
};
