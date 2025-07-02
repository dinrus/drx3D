// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12InputLayout.hpp>
#include <drx3D/Render/../DeviceUpr/DeviceFormats.h> // DeviceFormats::GetSize

CDrxDX12InputLayout* CDrxDX12InputLayout::Create(CDrxDX12Device* pDevice, const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs11, UINT NumElements, ukk pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength)
{
	ID3D12Device* pDevice12 = pDevice->GetD3D12Device();
	CDrxDX12InputLayout* pResult = DX12_NEW_RAW(CDrxDX12InputLayout());
	pResult->m_Descriptors.resize(NumElements);

	for (UINT i = 0; i < NumElements; ++i)
	{
		pResult->m_SemanticNames.push_back(pInputElementDescs11[i].SemanticName);
	}

	for (UINT i = 0; i < NumElements; ++i)
	{
		D3D12_INPUT_ELEMENT_DESC& desc12 = pResult->m_Descriptors[i];
		ZeroMemory(&desc12, sizeof(D3D12_INPUT_ELEMENT_DESC));

		desc12.SemanticName = pResult->m_SemanticNames[i].c_str();
		desc12.SemanticIndex = pInputElementDescs11[i].SemanticIndex;
		desc12.Format = pInputElementDescs11[i].Format;
		desc12.InputSlot = pInputElementDescs11[i].InputSlot;
		desc12.AlignedByteOffset = pInputElementDescs11[i].AlignedByteOffset;
		desc12.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(pInputElementDescs11[i].InputSlotClass);
		desc12.InstanceDataStepRate = pInputElementDescs11[i].InstanceDataStepRate;
	}

	return pResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12InputLayout::CDrxDX12InputLayout()
	: Super(nullptr, nullptr)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CDrxDX12InputLayout::GetInputSize(UINT inputSlot) const
{
	size_t size = 0;

	for (UINT i = 0; i < m_Descriptors.size(); ++i)
		if (m_Descriptors[i].InputSlot == inputSlot)
			size += DeviceFormats::GetStride(m_Descriptors[i].Format);

	return size;
}
