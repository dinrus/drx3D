// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.


#pragma once
#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

namespace NDrxVulkan
{
	class CShader : public CDeviceObject
	{
	public:
		CShader(CDevice* pDevice);
		virtual ~CShader();

		VkResult Init(ukk pShaderCode, size_t shaderCodeSize);
		VkShaderModule GetVulkanShader() const { return m_shaderHandle; }
		void DebugSetName(tukk szName) { /*TODO*/ }

	private:
		VkShaderModule m_shaderHandle;
	};

}
