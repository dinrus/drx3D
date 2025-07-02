// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Device/CDrxDX12DeviceChild.hpp>
#include <drx3D/Render/DX12/API/DX12Shader.hpp>

class CDrxDX12Shader : public CDrxDX12DeviceChild<ID3D11DeviceChildToImplement>
{
public:
	DX12_OBJECT(CDrxDX12Shader, CDrxDX12DeviceChild<ID3D11DeviceChildToImplement> );

	static CDrxDX12Shader* Create(CDrxDX12Device* pDevice, ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage);

	virtual ~CDrxDX12Shader();

	NDrxDX12::CShader* GetDX12Shader() const
	{
		return m_pShader;
	}

	const D3D12_SHADER_BYTECODE& GetD3D12ShaderBytecode() const
	{
		return m_ShaderBytecode12;
	}

protected:
	CDrxDX12Shader(ukk pShaderBytecode, SIZE_T BytecodeLength);

private:
	DX12_PTR(NDrxDX12::CShader) m_pShader;

	uint8_t*              m_pShaderBytecodeData;

	D3D12_SHADER_BYTECODE m_ShaderBytecode12;
};
