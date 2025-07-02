// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

struct SSamplerState;

namespace NDrxVulkan
{
class CSampler : public CDeviceObject
{
public:
	CSampler(CDevice* pDevice) : CDeviceObject(pDevice) {}
	CSampler(CSampler&&) = default;
	CSampler& operator=(CSampler&&) = default;
	virtual ~CSampler() override;

	VkResult Init(const SSamplerState& state);

	VkSampler GetHandle() const { return m_sampler; }

private:
	virtual void Destroy() override; // Uses CDevice::DeferDestruction

	CAutoHandle<VkSampler> m_sampler;
};
}
