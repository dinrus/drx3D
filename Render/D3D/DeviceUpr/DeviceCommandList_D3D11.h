// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CDeviceGraphicsCommandInterfaceImpl;
class CDeviceComputeCommandInterfaceImpl;
class CDeviceNvidiaCommandInterfaceImpl;

class CDeviceCopyCommandInterfaceImpl;
class CDeviceRenderPass;

struct SSharedState
{
	SCachedValue<uk> shader[eHWSC_Num];
	SCachedValue<ID3D11ShaderResourceView*>           shaderResourceView[eHWSC_Num][MAX_TMU];
	SCachedValue<ID3D11SamplerState*>                 samplerState[eHWSC_Num][MAX_TMU];
	SCachedValue<uint64>                              constantBuffer[eHWSC_Num][eConstantBufferShaderSlot_Count];

	std::array<std::array<u8, MAX_TMU>, eHWSC_Num> srvs;
	std::array<std::array<u8, MAX_TMU>, eHWSC_Num> samplers;

	std::array<u8, eHWSC_Num>                      numSRVs;
	std::array<u8, eHWSC_Num>                      numSamplers;

	EShaderStage validShaderStages;
};

struct SCustomGraphicsState
{
	SCachedValue<_smart_ptr<ID3D11DepthStencilState>> depthStencilState;
	SCachedValue<_smart_ptr<ID3D11RasterizerState>>   rasterizerState;
	u32                                 rasterizerStateIndex;
	SCachedValue<_smart_ptr<ID3D11BlendState>>        blendState;
	SCachedValue<ID3D11InputLayout*>       inputLayout;
	SCachedValue<D3D11_PRIMITIVE_TOPOLOGY> topology;

	float depthConstBias;
	float depthSlopeBias;
	float depthBiasClamp;

	bool bRasterizerStateDirty;
	bool bDepthStencilStateDirty;
};

struct SCustomComputeState
{
	std::bitset<D3D11_PS_CS_UAV_REGISTER_COUNT> boundUAVs;
};

class CDeviceCommandListImpl : public CDeviceCommandListCommon<SSharedState, SCustomGraphicsState, SCustomComputeState>
{
public:
	CDeviceGraphicsCommandInterfaceImpl* GetGraphicsInterfaceImpl()
	{
		return reinterpret_cast<CDeviceGraphicsCommandInterfaceImpl*>(this);
	}

	CDeviceComputeCommandInterfaceImpl* GetComputeInterfaceImpl()
	{
		return reinterpret_cast<CDeviceComputeCommandInterfaceImpl*>(this);
	}

	CDeviceNvidiaCommandInterfaceImpl* GetNvidiaCommandInterfaceImpl();
	CDeviceCopyCommandInterfaceImpl* GetCopyInterfaceImpl()
	{
		return reinterpret_cast<CDeviceCopyCommandInterfaceImpl*>(this);
	}

	void SetProfilerMarker(tukk label);
	void BeginProfilerEvent(tukk label);
	void EndProfilerEvent(tukk label);

protected:
	void ClearStateImpl(bool bOutputMergerOnly) const;

	void ResetImpl();
	void LockToThreadImpl() {}
	void CloseImpl()        {}
};

////////////////////////////////////////////////////////////////////////////

class CDeviceGraphicsCommandInterfaceImpl : public CDeviceCommandListImpl
{
protected:
	void PrepareUAVsForUseImpl(u32 viewCount, CGpuBuffer** pViews, bool bCompute) const {}
	void PrepareRenderPassForUseImpl(CDeviceRenderPass& renderPass) const {}
	void PrepareResourceForUseImpl(u32 bindSlot, CTexture* pTexture, const ResourceViewHandle TextureView, ::EShaderStage srvUsage) const                              {}
	void PrepareResourcesForUseImpl(u32 bindSlot, CDeviceResourceSet* pResources) const                                                                                {}
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass) const         {}
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages) const          {}
	void PrepareVertexBuffersForUseImpl(u32 numStreams, u32 lastStreamSlot, const CDeviceInputStream* vertexStreams) const                                          {}
	void PrepareIndexBufferForUseImpl(const CDeviceInputStream* indexStream) const                                                                                        {}
	void BeginResourceTransitionsImpl(u32 numTextures, CTexture** pTextures, EResourceTransitionType type)                                                             {}

	void BeginRenderPassImpl(const CDeviceRenderPass& renderPass, const D3DRectangle& renderArea);
	void EndRenderPassImpl(const CDeviceRenderPass& renderPass) {}
	void SetViewportsImpl(u32 vpCount, const D3DViewPort* pViewports);
	void SetScissorRectsImpl(u32 rcCount, const D3DRectangle* pRects);
	void SetPipelineStateImpl(const CDeviceGraphicsPSO* pDevicePSO);
	void SetResourceLayoutImpl(const CDeviceResourceLayout* pResourceLayout) {}
	void SetResourcesImpl(u32 bindSlot, const CDeviceResourceSet* pResources);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages);
	void SetVertexBuffersImpl(u32 numStreams, u32 lastStreamSlot, const CDeviceInputStream* vertexStreams);
	void SetIndexBufferImpl(const CDeviceInputStream* indexStream); // NOTE: Take care with PSO strip cut/restart value and 32/16 bit indices
	void SetInlineConstantsImpl(u32 bindSlot, u32 constantCount, float* pConstants) {}
	void SetStencilRefImpl(u8 stencilRefValue);
	void SetDepthBiasImpl(float constBias, float slopeBias, float biasClamp);
	void SetDepthBoundsImpl(float fMin, float fMax);

	void DrawImpl(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation);
	void DrawIndexedImpl(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, i32 BaseVertexLocation, u32 StartInstanceLocation);

	void ClearSurfaceImpl(D3DSurface* pView, const FLOAT Color[4], UINT NumRects, const D3D11_RECT* pRects);
	void ClearSurfaceImpl(D3DDepthSurface* pView, i32 clearFlags, float depth, u8 stencil, u32 numRects, const D3D11_RECT* pRects);

	void BeginOcclusionQueryImpl(D3DOcclusionQuery* pQuery);
	void EndOcclusionQueryImpl(D3DOcclusionQuery* pQuery);

protected:
	void SetResources_RequestedByShaderOnly(const CDeviceResourceSet* pResources);
	void SetResources_All(const CDeviceResourceSet* pResources);

	void ApplyDepthStencilState();
	void ApplyRasterizerState();
};

////////////////////////////////////////////////////////////////////////////

class CDeviceComputeCommandInterfaceImpl : public CDeviceCommandListImpl
{
protected:
	void PrepareUAVsForUseImpl(u32 viewCount, CGpuBuffer** pViews) const {}
	void PrepareResourcesForUseImpl(u32 bindSlot, CDeviceResourceSet* pResources) const {}
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlots, ::EShaderStage shaderStages) const {}

	void SetPipelineStateImpl(const CDeviceComputePSO* pDevicePSO);
	void SetResourceLayoutImpl(const CDeviceResourceLayout* pResourceLayout) {}
	void SetResourcesImpl(u32 bindSlot, const CDeviceResourceSet* pResources);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot);
	void SetInlineConstantsImpl(u32 bindSlot, u32 constantCount, float* pConstants);

	void DispatchImpl(u32 X, u32 Y, u32 Z);

	void ClearUAVImpl(D3DUAV* pView, const FLOAT Values[4], UINT NumRects, const D3D11_RECT* pRects);
	void ClearUAVImpl(D3DUAV* pView, const UINT Values[4], UINT NumRects, const D3D11_RECT* pRects);
};

////////////////////////////////////////////////////////////////////////////

class CDeviceNvidiaCommandInterfaceImpl : public CDeviceCommandListImpl
{
protected:
	void SetModifiedWModeImpl(bool enabled, uint32_t numViewports, const float* pA, const float* pB);
};


////////////////////////////////////////////////////////////////////////////

class CDeviceCopyCommandInterfaceImpl : public CDeviceCommandListImpl
{
protected:
	void CopyImpl(CDeviceBuffer*  pSrc, CDeviceBuffer*  pDst);
	void CopyImpl(D3DBuffer*      pSrc, D3DBuffer*      pDst);
	void CopyImpl(CDeviceTexture* pSrc, CDeviceTexture* pDst);
	void CopyImpl(CDeviceTexture* pSrc, D3DTexture*     pDst);
	void CopyImpl(D3DTexture*     pSrc, D3DTexture*     pDst);
	void CopyImpl(D3DTexture*     pSrc, CDeviceTexture* pDst);

	void CopyImpl(CDeviceBuffer*  pSrc, CDeviceBuffer*  pDst, const SResourceRegionMapping& regionMapping);
	void CopyImpl(D3DBuffer*      pSrc, D3DBuffer*      pDst, const SResourceRegionMapping& regionMapping);
	void CopyImpl(CDeviceTexture* pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping);
	void CopyImpl(D3DTexture*     pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping);

	void CopyImpl(ukk pSrc, CConstantBuffer* pDst, const SResourceMemoryAlignment& memoryLayout);
	void CopyImpl(ukk pSrc, CDeviceBuffer*   pDst, const SResourceMemoryAlignment& memoryLayout);
	void CopyImpl(ukk pSrc, CDeviceTexture*  pDst, const SResourceMemoryAlignment& memoryLayout);

	void CopyImpl(ukk pSrc, CConstantBuffer* pDst, const SResourceMemoryMapping& memoryMapping);
	void CopyImpl(ukk pSrc, CDeviceBuffer*   pDst, const SResourceMemoryMapping& memoryMapping);
	void CopyImpl(ukk pSrc, CDeviceTexture*  pDst, const SResourceMemoryMapping& memoryMapping);

	void CopyImpl(CDeviceBuffer*  pSrc, uk pDst, const SResourceMemoryAlignment& memoryLayout);
	void CopyImpl(CDeviceTexture* pSrc, uk pDst, const SResourceMemoryAlignment& memoryLayout);

	void CopyImpl(CDeviceBuffer*  pSrc, uk pDst, const SResourceMemoryMapping& memoryMapping);
	void CopyImpl(CDeviceTexture* pSrc, uk pDst, const SResourceMemoryMapping& memoryMapping);
};

