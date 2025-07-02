// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12Shader.hpp>

#if DRX_PLATFORM_DURANGO
#include <d3d11shader_x.h>
#include <d3dcompiler_x.h>
#else
#include <d3d11shader.h>
#include <d3dcompiler.h>
#endif

CDrxDX12Shader* CDrxDX12Shader::Create(CDrxDX12Device* pDevice, ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage)
{
	CDrxDX12Shader* pResult = DX12_NEW_RAW(CDrxDX12Shader(pShaderBytecode, BytecodeLength));
	pResult->m_pShader = NDrxDX12::CShader::CreateFromD3D11(pDevice->GetDX12Device(), pResult->GetD3D12ShaderBytecode());

	return pResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12Shader::CDrxDX12Shader(ukk pShaderBytecode, SIZE_T BytecodeLength)
	: Super(nullptr, nullptr)
	, m_pShaderBytecodeData(NULL)
{
	if (pShaderBytecode && BytecodeLength)
	{
		m_pShaderBytecodeData = new uint8_t[BytecodeLength];
		memcpy(m_pShaderBytecodeData, pShaderBytecode, BytecodeLength);

		m_ShaderBytecode12.pShaderBytecode = m_pShaderBytecodeData;
		m_ShaderBytecode12.BytecodeLength = BytecodeLength;
	}
	else
	{
		m_ShaderBytecode12.pShaderBytecode = NULL;
		m_ShaderBytecode12.BytecodeLength = 0;
	}
}

CDrxDX12Shader::~CDrxDX12Shader()
{
	if (m_pShaderBytecodeData)
	{
		delete[] m_pShaderBytecodeData;
	}
}
