// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12RasterizerState.hpp>

CDrxDX12RasterizerState* CDrxDX12RasterizerState::Create(const D3D11_RASTERIZER_DESC* pRasterizerDesc)
{
	D3D12_RASTERIZER_DESC desc12;
	ZeroMemory(&desc12, sizeof(D3D11_RASTERIZER_DESC));

	desc12.AntialiasedLineEnable = pRasterizerDesc->AntialiasedLineEnable;
	desc12.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	desc12.CullMode = static_cast<D3D12_CULL_MODE>(pRasterizerDesc->CullMode);
	desc12.DepthBias = pRasterizerDesc->DepthBias;
	desc12.DepthBiasClamp = pRasterizerDesc->DepthBiasClamp;
	desc12.DepthClipEnable = pRasterizerDesc->DepthClipEnable;
	desc12.FillMode = static_cast<D3D12_FILL_MODE>(pRasterizerDesc->FillMode);
	desc12.ForcedSampleCount = 0;
	desc12.FrontCounterClockwise = pRasterizerDesc->FrontCounterClockwise;
	desc12.MultisampleEnable = pRasterizerDesc->MultisampleEnable;
	desc12.SlopeScaledDepthBias = pRasterizerDesc->SlopeScaledDepthBias;

	return DX12_NEW_RAW(CDrxDX12RasterizerState(*pRasterizerDesc, desc12));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12RasterizerState::CDrxDX12RasterizerState(const D3D11_RASTERIZER_DESC& desc11, const D3D12_RASTERIZER_DESC& desc12)
	: Super(nullptr, nullptr)
	, m_Desc11(desc11)
	, m_Desc12(desc12)
{

}

#pragma region /* ID3D11RasterizerState implementation */

void STDMETHODCALLTYPE CDrxDX12RasterizerState::GetDesc(
  _Out_ D3D11_RASTERIZER_DESC* pDesc)
{
	if (pDesc)
	{
		*pDesc = m_Desc11;
	}
}

#pragma endregion
