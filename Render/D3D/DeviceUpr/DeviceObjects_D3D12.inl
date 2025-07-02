// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

class CDeviceTimestampGroup : public CDeviceTimestampGroup_Base<CDeviceTimestampGroup>
{
public:
	CDeviceTimestampGroup();
	~CDeviceTimestampGroup();
	
	void Init();

	void BeginMeasurement();
	void EndMeasurement();

	u32 IssueTimestamp(CDeviceCommandList* pCommandList);
	bool ResolveTimestamps();

	uint64 GetTime(u32 timestamp)
	{
		timestamp -= m_groupIndex * kMaxTimestamps;

		return m_timeValues[timestamp];
	}

	float GetTimeMS(u32 timestamp0, u32 timestamp1)
	{
		timestamp0 -= m_groupIndex * kMaxTimestamps;
		timestamp1 -= m_groupIndex * kMaxTimestamps;
		
		uint64 duration = std::max(m_timeValues[timestamp0], m_timeValues[timestamp1]) - std::min(m_timeValues[timestamp0], m_timeValues[timestamp1]);
		return duration / (float)(m_frequency / 1000);
	}

protected:
	u32                              m_numTimestamps;
	u32                              m_groupIndex;

	DeviceFenceHandle                   m_fence;
	UINT64                              m_frequency;
	std::array<uint64, kMaxTimestamps>  m_timeValues;

	bool                                m_measurable;

protected:
	static bool                         s_reservedGroups[4];
};

////////////////////////////////////////////////////////////////////////////
class CDeviceRenderPass final : public CDeviceRenderPass_Base
{
private:
	virtual bool UpdateImpl(const CDeviceRenderPassDesc& passDesc) override;

public:
	std::array<ID3D11RenderTargetView*, CDeviceRenderPassDesc::MaxRendertargetCount> m_RenderTargetViews;
	int                                                                              m_RenderTargetCount;
	ID3D11DepthStencilView*                                                          m_pDepthStencilView;
};

////////////////////////////////////////////////////////////////////////////
template<typename T>
static inline D3D11_BIND_FLAG ConvertToDX11BindFlags(const T& desc)
{
	DRX_ASSERT_MESSAGE((desc & (CDeviceObjectFactory::BIND_RENDER_TARGET | CDeviceObjectFactory::BIND_DEPTH_STENCIL)) != (CDeviceObjectFactory::BIND_RENDER_TARGET | CDeviceObjectFactory::BIND_DEPTH_STENCIL), "RenderTarget and DepthStencil can't be requested together!");

	// *INDENT-OFF*
	return D3D11_BIND_FLAG(
		((desc & CDeviceObjectFactory::BIND_VERTEX_BUFFER   ) ? D3D11_BIND_VERTEX_BUFFER              : 0) |
		((desc & CDeviceObjectFactory::BIND_INDEX_BUFFER    ) ? D3D11_BIND_INDEX_BUFFER               : 0) |
		((desc & CDeviceObjectFactory::BIND_CONSTANT_BUFFER ) ? D3D11_BIND_CONSTANT_BUFFER            : 0) |
		((desc & CDeviceObjectFactory::BIND_SHADER_RESOURCE ) ? D3D11_BIND_SHADER_RESOURCE            : 0) |
	//	((desc & CDeviceObjectFactory::BIND_STREAM_OUTPUT   ) ? D3D11_BIND_STREAM_OUTPUT              : 0) |
		((desc & CDeviceObjectFactory::BIND_RENDER_TARGET   ) ? D3D11_BIND_RENDER_TARGET              : 0) |
		((desc & CDeviceObjectFactory::BIND_DEPTH_STENCIL   ) ? D3D11_BIND_DEPTH_STENCIL              : 0) |
		((desc & CDeviceObjectFactory::BIND_UNORDERED_ACCESS) ? D3D11_BIND_UNORDERED_ACCESS           : 0));
	// *INDENT-ON*
}

template<typename T>
static inline D3D11_RESOURCE_MISC_FLAG ConvertToDX11MiscFlags(const T& desc)
{
	// D3D11_RESOURCE_MISC_RESOURCE_CLAMP and D3D11_RESOURCE_MISC_GENERATE_MIPS are mutually exclusive.
	// *INDENT-OFF*
	return D3D11_RESOURCE_MISC_FLAG(
		((desc & CDeviceObjectFactory::USAGE_AUTOGENMIPS ) ? D3D11_RESOURCE_MISC_GENERATE_MIPS          :
		((desc & CDeviceObjectFactory::USAGE_STREAMING   ) ? D3D11_RESOURCE_MISC_RESOURCE_CLAMP         : 0)  |
		((desc & CDeviceObjectFactory::USAGE_LODABLE     ) ? D3D11_RESOURCE_MISC_RESOURCE_CLAMP         : 0)) |
		((desc & CDeviceObjectFactory::USAGE_STRUCTURED  ) ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED      : 0)  |
		((desc & CDeviceObjectFactory::USAGE_RAW         ) ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 0)  |
		((desc & CDeviceObjectFactory::USAGE_INDIRECTARGS) ? D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS      : 0)  |
		((desc & CDeviceObjectFactory::USAGE_UAV_OVERLAP ) ? D3D11_RESOURCE_MISC_UAV_OVERLAP            : 0)  |
		((desc & CDeviceObjectFactory::USAGE_HIFREQ_HEAP ) ? D3D11_RESOURCE_MISC_HIFREQ_HEAP            : 0));
	// *INDENT-ON*
}

template<typename T>
static inline D3D11_CPU_ACCESS_FLAG ConvertToDX11CPUAccessFlags(const T& desc)
{
	// *INDENT-OFF*
	return D3D11_CPU_ACCESS_FLAG(
		((desc & CDeviceObjectFactory::USAGE_CPU_WRITE) ? D3D11_CPU_ACCESS_WRITE : 0)  |
		((desc & CDeviceObjectFactory::USAGE_CPU_READ ) ? D3D11_CPU_ACCESS_READ  : 0));
	// *INDENT-ON*
}

template<typename T>
static inline D3D11_USAGE ConvertToDX11Usage(const T& desc)
{
	DRX_ASSERT_MESSAGE((desc & (CDeviceObjectFactory::USAGE_CPU_READ | CDeviceObjectFactory::USAGE_CPU_WRITE)) != (CDeviceObjectFactory::USAGE_CPU_READ | CDeviceObjectFactory::USAGE_CPU_WRITE), "CPU Read and Write can't be requested together!");

	// *INDENT-OFF*
	return D3D11_USAGE(
		((desc & CDeviceObjectFactory::USAGE_CPU_READ ) ? D3D11_USAGE_STAGING :
		((desc & CDeviceObjectFactory::USAGE_CPU_WRITE) ? D3D11_USAGE_DYNAMIC :
		                                                  D3D11_USAGE_DEFAULT)));
	// *INDENT-ON*
}

static inline D3D11_SUBRESOURCE_DATA* ConvertToDX11Data(u32 numSubs, const STexturePayload* pSrc, D3D11_SUBRESOURCE_DATA* pDst)
{
	if (!pSrc || !pSrc->m_pSysMemSubresourceData)
		return nullptr;

	for (u32 i = 0; i < numSubs; ++i)
	{
		pDst[i].pSysMem = pSrc->m_pSysMemSubresourceData[i].m_pSysMem;
		pDst[i].SysMemPitch = pSrc->m_pSysMemSubresourceData[i].m_sSysMemAlignment.rowStride;
		pDst[i].SysMemSlicePitch = pSrc->m_pSysMemSubresourceData[i].m_sSysMemAlignment.planeStride;

#if DRX_PLATFORM_ORBIS
		pDst[i].SysMemTileMode = (D3D11_TEXTURE_TILE_MODE)pSrc.m_eSysMemTileMode;
#endif
	}

	// Terminator
	pDst[numSubs].pSysMem = nullptr;
	pDst[numSubs].SysMemPitch = 0;
	pDst[numSubs].SysMemSlicePitch = 0;

	return pDst;
}

////////////////////////////////////////////////////////////////////////////
// Low-level resource management API (TODO: remove D3D-dependency by abstraction)

inline D3DResource* CDeviceObjectFactory::GetNullResource(D3D11_RESOURCE_DIMENSION eType)
{
	assert(m_NullResources[eType]);
	m_NullResources[eType]->AddRef();
	return m_NullResources[eType];
}

////////////////////////////////////////////////////////////////////////////

inline CDeviceCommandListRef CDeviceObjectFactory::GetCoreCommandList() const
{
	// Sanity check
	DRX_ASSERT(
		m_pCoreCommandList->m_sharedState.pCommandList == m_pDX12Scheduler->GetCommandList(CMDQUEUE_GRAPHICS) ||
		m_pCoreCommandList->m_sharedState.pCommandList == nullptr
	);

	return *m_pCoreCommandList.get();
}

inline NDrxDX12::CDevice* GetDevice()
{
	return GetDeviceObjectFactory().GetDX12Device();
}

inline NDrxDX12::CCommandScheduler* GetScheduler()
{
	return GetDeviceObjectFactory().GetDX12Scheduler();
}

inline static D3D12_SHADER_VISIBILITY GetShaderVisibility(::EShaderStage shaderStages)
{
	extern D3D12_SHADER_VISIBILITY shaderVisibility[eHWSC_Num + 1];

	EHWShaderClass shaderClass = eHWSC_Num;
	if (IsPowerOfTwo(shaderStages)) // only bound to a single shader stage?
	{
		shaderClass = EHWShaderClass(countTrailingZeros32(shaderStages));
	}

	return shaderVisibility[shaderClass];
};

inline static CD3DX12_DESCRIPTOR_RANGE GetDescriptorRange(SResourceBindPoint bindPoint, int descriptorIndex)
{
	extern D3D12_DESCRIPTOR_RANGE_TYPE mapDescriptorRange[size_t(SResourceBindPoint::ESlotType::Count)];

	CD3DX12_DESCRIPTOR_RANGE result;
	result.BaseShaderRegister = bindPoint.slotNumber;
	result.NumDescriptors = 1;
	result.OffsetInDescriptorsFromTableStart = descriptorIndex;
	result.RegisterSpace = 0;
	result.RangeType = mapDescriptorRange[size_t(bindPoint.slotType)];

	return result;
}

#define GET_DX12_SHADER_VIEW(uniformBufferOrTexture, rView)    reinterpret_cast<CDrxDX12ShaderResourceView*>((uniformBufferOrTexture)->LookupSRV(rView))
#define GET_DX12_UNORDERED_VIEW(uniformBufferOrTexture, rView) reinterpret_cast<CDrxDX12UnorderedAccessView*>((uniformBufferOrTexture)->LookupUAV(rView))
#define GET_DX12_DEPTHSTENCIL_VIEW(dsTarget, dView)            reinterpret_cast<CDrxDX12DepthStencilView*>((dsTarget)->LookupDSV(dView));
#define GET_DX12_RENDERTARGET_VIEW(rTarget, rView)             reinterpret_cast<CDrxDX12RenderTargetView*>((rTarget)->LookupRTV(rView))
#define GET_DX12_SAMPLERSTATE(uniformSamplerState)             reinterpret_cast<CDrxDX12SamplerState*>(CDeviceObjectFactory::LookupSamplerState(uniformSamplerState).second)

#define GET_DX12_TEXTURE_RESOURCE(uniformTexture)              reinterpret_cast<CDrxDX12Resource<ID3D11Resource>*>((uniformTexture)->GetDevTexture()->GetBaseTexture())
#define GET_DX12_BUFFER_RESOURCE(uniformBuffer)                reinterpret_cast<CDrxDX12Resource<ID3D11Resource>*>((uniformBuffer).GetDevBuffer()->GetBaseBuffer())
#define GET_DX12_CONSTANTBUFFER_RESOURCE(constantBuffer)       reinterpret_cast<CDrxDX12Buffer*>((constantBuffer)->GetD3D())
