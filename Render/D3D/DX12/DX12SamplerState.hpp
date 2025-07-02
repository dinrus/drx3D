// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12Base.hpp>

namespace NDrxDX12
{

class CSamplerState : public CRefCounted
{
public:
	CSamplerState();
	virtual ~CSamplerState();

	D3D12_SAMPLER_DESC& GetSamplerDesc()
	{
		//		DX12_ASSERT(m_pResource && (m_pResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		return m_unSamplerDesc;
	}
	const D3D12_SAMPLER_DESC& GetSamplerDesc() const
	{
		//		DX12_ASSERT(m_pResource && (m_pResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		return m_unSamplerDesc;
	}

	void                        SetDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& descriptorHandle) { m_DescriptorHandle = descriptorHandle; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const                                              { return m_DescriptorHandle; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_DescriptorHandle;

	union
	{
		D3D12_SAMPLER_DESC m_unSamplerDesc;
	};
};

}
