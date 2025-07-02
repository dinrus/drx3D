// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/DeviceUpr/DeviceResourceSet.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: optimize compiled resource layout for cache friendliness
struct CDeviceResourceSet_DX11 : CDeviceResourceSet
{
	static ukk const InvalidPointer;

	struct SCompiledConstantBuffer
	{
		D3DBuffer* pBuffer;
		u32 offset;
		u32 size;
		uint64 code;
		i32 slot;
	};

	struct SCompiledBufferSRV
	{
		ID3D11ShaderResourceView* pSrv;
		i32 slot;
	};

	struct SCompiledUAV
	{
		ID3D11UnorderedAccessView* pUav;
		CSubmissionQueue_DX11::SHADER_TYPE shaderType;
		i32 slot;
	};

	CDeviceResourceSet_DX11(CDeviceResourceSet::EFlags flags)
		: CDeviceResourceSet(flags)
	{}

	virtual bool UpdateImpl(const CDeviceResourceSetDesc& desc, CDeviceResourceSetDesc::EDirtyFlags dirtyFlags);

	void         ClearCompiledData();

	// set via reflection from shader
	std::array<std::array<ID3D11ShaderResourceView*, MAX_TMU>, eHWSC_Num>                  compiledTextureSRVs;
	std::array<std::array<ID3D11SamplerState*, MAX_TMU>, eHWSC_Num>                        compiledSamplers;

	// set directly
	std::array<std::array<SCompiledConstantBuffer, eConstantBufferShaderSlot_Count>, eHWSC_Num> compiledCBs;
	std::array<u8, eHWSC_Num> numCompiledCBs;

	std::array<std::array<SCompiledBufferSRV, ResourceSetBufferCount>, eHWSC_Num> compiledBufferSRVs;
	std::array<u8, eHWSC_Num> numCompiledBufferSRVs;

	std::array<SCompiledUAV, 8> compiledUAVs;
	u8 numCompiledUAVs;
};
class CDeviceResourceLayout_DX11 : public CDeviceResourceLayout
{
public:
	CDeviceResourceLayout_DX11(UsedBindSlotSet requiredResourceBindings)
		: CDeviceResourceLayout(requiredResourceBindings)
	{}
};
