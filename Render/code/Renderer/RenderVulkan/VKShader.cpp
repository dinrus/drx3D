// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/VKShader.hpp>
#include <drx3D/Render/VkSpirvHelper.hpp>


namespace NDrxVulkan
{

	CShader::CShader(CDevice* pDevice)
		: CDeviceObject(pDevice)
		, m_shaderHandle(VK_NULL_HANDLE)
	{}
	
	CShader::~CShader()
	{
		vkDestroyShaderModule(GetDevice()->GetVkDevice(), m_shaderHandle, nullptr);
	}

	VkResult CShader::Init(ukk pShaderCode, size_t shaderCodeSize)
	{
		bool stripReflections = false;
		if (CRendererCVars::CV_r_VkShaderCompiler && strcmp(CRendererCVars::CV_r_VkShaderCompiler->GetString(), STR_VK_SHADER_COMPILER_GLSLANG) == 0)
			stripReflections = true;

		std::vector<u32> shaderCode((shaderCodeSize + 3) / 4);
		memcpy(shaderCode.data(), pShaderCode, shaderCodeSize);
		if (stripReflections)
		{
			bool success = VkSpvHelper::StripGoogleExtensionsFromShader(shaderCode);
			DRX_ASSERT(success == true);
			shaderCodeSize = static_cast<uint32_t>(shaderCode.size());
		}

		DRX_ASSERT(shaderCodeSize > 0);

		VkShaderModuleCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.flags = 0;
		shaderCreateInfo.pCode = reinterpret_cast<u32k*>(shaderCode.data());
		shaderCreateInfo.codeSize = shaderCodeSize;
		shaderCreateInfo.pNext = nullptr;

		return vkCreateShaderModule(GetDevice()->GetVkDevice(), &shaderCreateInfo, nullptr, &m_shaderHandle);
	}
}
