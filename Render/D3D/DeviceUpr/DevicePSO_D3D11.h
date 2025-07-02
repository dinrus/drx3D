// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/DeviceUpr/DevicePSO.h>

class CDeviceGraphicsPSO_DX11 : public CDeviceGraphicsPSO
{
public:
	CDeviceGraphicsPSO_DX11();

	virtual EInitResult Init(const CDeviceGraphicsPSODesc& psoDesc) final;

	_smart_ptr<ID3D11RasterizerState>                 m_pRasterizerState;
	u32                                            m_RasterizerStateIndex;
	_smart_ptr<ID3D11BlendState>                      m_pBlendState;
	_smart_ptr<ID3D11DepthStencilState>               m_pDepthStencilState;
	_smart_ptr<ID3D11InputLayout>                     m_pInputLayout;

	std::array<uk , eHWSC_Num>                      m_pDeviceShaders;

	std::array<std::array<u8, MAX_TMU>, eHWSC_Num> m_Samplers;
	std::array<std::array<u8, MAX_TMU>, eHWSC_Num> m_SRVs;

	std::array<u8, eHWSC_Num>                      m_NumSamplers;
	std::array<u8, eHWSC_Num>                      m_NumSRVs;

	// Do we still need these?
	uint64           m_ShaderFlags_RT;
	u32           m_ShaderFlags_MD;
	u32           m_ShaderFlags_MDV;

	D3DPrimitiveType m_PrimitiveTopology;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDeviceComputePSO_DX11 : public CDeviceComputePSO
{
public:
	CDeviceComputePSO_DX11();

	virtual bool Init(const CDeviceComputePSODesc& psoDesc) final;

	std::array<uk , eHWSC_Num> m_pDeviceShaders;
};