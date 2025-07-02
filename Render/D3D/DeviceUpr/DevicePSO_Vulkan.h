// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/DeviceUpr/DevicePSO.h>

////////////////////////////////////////////////////////////////////////////
class CDeviceGraphicsPSO_Vulkan : public CDeviceGraphicsPSO
{
public:
	CDeviceGraphicsPSO_Vulkan(CDevice* pDevice)
		: m_pDevice(pDevice)
		, m_pipeline(VK_NULL_HANDLE)
	{}

	~CDeviceGraphicsPSO_Vulkan();

	virtual EInitResult Init(const CDeviceGraphicsPSODesc& desc) final;

	const VkPipeline&   GetVkPipeline() const { return m_pipeline; }

protected:
	CDevice*   m_pDevice;
	VkPipeline m_pipeline;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDeviceComputePSO_Vulkan : public CDeviceComputePSO
{
public:
	CDeviceComputePSO_Vulkan(CDevice* pDevice)
		: m_pDevice(pDevice)
		, m_pipeline(VK_NULL_HANDLE)
	{}

	~CDeviceComputePSO_Vulkan();

	virtual bool      Init(const CDeviceComputePSODesc& desc) final;

	const VkPipeline& GetVkPipeline() const { return m_pipeline; }

protected:
	CDevice*   m_pDevice;
	VkPipeline m_pipeline;
};
