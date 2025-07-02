// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12SamplerState.hpp>

CDrxDX12SamplerState* CDrxDX12SamplerState::Create(const D3D11_SAMPLER_DESC* pSamplerDesc)
{
	D3D12_SAMPLER_DESC desc12;
	desc12.Filter = static_cast<D3D12_FILTER>(pSamplerDesc->Filter);
	desc12.AddressU = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(pSamplerDesc->AddressU);
	desc12.AddressV = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(pSamplerDesc->AddressV);
	desc12.AddressW = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(pSamplerDesc->AddressW);
	desc12.MipLODBias = pSamplerDesc->MipLODBias;
	desc12.MaxAnisotropy = pSamplerDesc->MaxAnisotropy;
	desc12.ComparisonFunc = static_cast<D3D12_COMPARISON_FUNC>(pSamplerDesc->ComparisonFunc);
	memcpy(desc12.BorderColor, pSamplerDesc->BorderColor, 4 * sizeof(float));
	desc12.MinLOD = pSamplerDesc->MinLOD;
	desc12.MaxLOD = pSamplerDesc->MaxLOD;

	return DX12_NEW_RAW(CDrxDX12SamplerState(*pSamplerDesc, desc12));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region /* ID3D11SamplerState implementation */

void STDMETHODCALLTYPE CDrxDX12SamplerState::GetDesc(
  _Out_ D3D11_SAMPLER_DESC* pDesc)
{
	if (pDesc)
	{
		*pDesc = m_Desc11;
	}
}

#pragma endregion
