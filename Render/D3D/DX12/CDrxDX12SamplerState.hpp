// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Device/CDrxDX12DeviceChild.hpp>

#include <drx3D/Render/DX12/API/DX12SamplerState.hpp>

class CDrxDX12SamplerState : public CDrxDX12DeviceChild<ID3D11SamplerStateToImplement>
{
public:
	DX12_OBJECT(CDrxDX12SamplerState, CDrxDX12DeviceChild<ID3D11SamplerStateToImplement> );

	static CDrxDX12SamplerState* Create(const D3D11_SAMPLER_DESC* pSamplerDesc);

	NDrxDX12::CSamplerState&     GetDX12SamplerState()
	{
		return m_DX12SamplerState;
	}

	#pragma region /* ID3D11SamplerState implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_SAMPLER_DESC* pDesc) FINALGFX;

	#pragma endregion

protected:
	CDrxDX12SamplerState(const D3D11_SAMPLER_DESC& desc11, const D3D12_SAMPLER_DESC& desc12)
		: Super(nullptr, nullptr)
		, m_Desc11(desc11)
	{
		m_DX12SamplerState.GetSamplerDesc() = desc12;
	}

private:
	D3D11_SAMPLER_DESC      m_Desc11;

	NDrxDX12::CSamplerState m_DX12SamplerState;
};
