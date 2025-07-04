// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/VKCommandList.hpp>

class CDeviceGraphicsCommandInterfaceImpl;
class CDeviceComputeCommandInterfaceImpl;
class CDeviceCopyCommandInterfaceImpl;
class CDeviceRenderPass;
class CDeviceNvidiaCommandInterfaceImpl;

struct SEmpty {};

struct SSharedState
{
	VK_PTR(NDrxVulkan::CCommandList) pCommandList;
};

// deferred binding of descriptor sets
struct SPendingBindings
{
	enum { MaxPendingBindings  = EResourceLayoutSlot_Max + 1};

	u32          validMask;
	u32          dynamicOffsetMask;
	VkDescriptorSet descriptorSets[MaxPendingBindings];
	u32          dynamicOffsets[MaxPendingBindings];

	void Reset()
	{
		validMask = 0;
		dynamicOffsetMask = 0;
	}

	void AppendDescriptorSet(u8 bindSlot, VkDescriptorSet descriptorSet, u32* pDynamicOffset)
	{
		DRX_ASSERT(bindSlot < MaxPendingBindings);

		validMask |= (1u << bindSlot);
		descriptorSets[bindSlot] = descriptorSet;

		if (pDynamicOffset)
		{
			dynamicOffsetMask |= (1u << bindSlot);
			dynamicOffsets[bindSlot] = *pDynamicOffset;
		}
	}
};

struct SCustomState
{
	SPendingBindings pendingBindings;
};

class CDeviceCommandListImpl : public CDeviceCommandListCommon<SSharedState, SCustomState, SCustomState>
{
public:
	~CDeviceCommandListImpl();

	void RequestTransition(NDrxVulkan::CImageResource* pResource, VkImageLayout desiredLayout, VkAccessFlags desiredAccess) const;
	void RequestTransition(NDrxVulkan::CBufferResource* pResource, VkAccessFlags desiredAccess) const;

public:
	ILINE CDeviceGraphicsCommandInterfaceImpl* GetGraphicsInterfaceImpl()
	{
		return (GetVKCommandList()->GetVkListType() & VK_QUEUE_GRAPHICS_BIT) ? reinterpret_cast<CDeviceGraphicsCommandInterfaceImpl*>(this) : nullptr;
	}

	ILINE CDeviceComputeCommandInterfaceImpl* GetComputeInterfaceImpl()
	{
		return (GetVKCommandList()->GetVkListType() & VK_QUEUE_COMPUTE_BIT) ? reinterpret_cast<CDeviceComputeCommandInterfaceImpl*>(this) : nullptr;
	}

	ILINE CDeviceCopyCommandInterfaceImpl* GetCopyInterfaceImpl()
	{
		return (GetVKCommandList()->GetVkListType() & VK_QUEUE_TRANSFER_BIT) ? reinterpret_cast<CDeviceCopyCommandInterfaceImpl*>(this) : nullptr;
	}

	inline CDeviceNvidiaCommandInterfaceImpl* GetNvidiaCommandInterfaceImpl()
	{
		return nullptr;
	}

	void SetProfilerMarker(tukk label);
	void BeginProfilerEvent(tukk label);
	void EndProfilerEvent(tukk label);

	void CeaseCommandListEvent(i32 nPoolId);
	void ResumeCommandListEvent(i32 nPoolId);

	// Helper functions for VK
	NDrxVulkan::CCommandList* GetVKCommandList() const { return m_sharedState.pCommandList; }

protected:
	void ClearStateImpl(bool bOutputMergerOnly) const {}

	void ResetImpl();
	void LockToThreadImpl();
	void CloseImpl();

	void ApplyPendingBindings(VkCommandBuffer vkCommandList, VkPipelineLayout vkPipelineLayout, VkPipelineBindPoint vkPipelineBindPoint, const SPendingBindings& RESTRICT_REFERENCE bindings);

private:
	DynArray<tukk > m_profilerEventStack;
};
	
class CDeviceGraphicsCommandInterfaceImpl : public CDeviceCommandListImpl
{
	friend class CDeviceCommandListImpl;

protected:
	void PrepareUAVsForUseImpl(u32 viewCount, CGpuBuffer** pViews, bool bCompute) const;
	void PrepareRenderPassForUseImpl(CDeviceRenderPass& renderPass) const;
	void PrepareResourcesForUseImpl(u32 bindSlot, CDeviceResourceSet* pResources) const;
	void PrepareResourceForUseImpl(u32 bindSlot, CTexture* pTexture, const ResourceViewHandle TextureView, ::EShaderStage srvUsage) const;
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass) const;
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages) const;
	void PrepareVertexBuffersForUseImpl(u32 numStreams, u32 lastStreamSlot, const CDeviceInputStream* vertexStreams) const;
	void PrepareIndexBufferForUseImpl(const CDeviceInputStream* indexStream) const;
	void BeginResourceTransitionsImpl(u32 numTextures, CTexture** pTextures, EResourceTransitionType type) { /*NOP*/ }

	void BeginRenderPassImpl(const CDeviceRenderPass& renderPass, const D3DRectangle& renderArea);
	void EndRenderPassImpl(const CDeviceRenderPass& renderPass);
	void SetViewportsImpl(u32 vpCount, const D3DViewPort* pViewports);
	void SetScissorRectsImpl(u32 rcCount, const D3DRectangle* pRects);
	void SetPipelineStateImpl(const CDeviceGraphicsPSO* pDevicePSO);
	void SetResourceLayoutImpl(const CDeviceResourceLayout* pResourceLayout);
	void SetResourcesImpl(u32 bindSlot, const CDeviceResourceSet* pResources);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages);
	void SetVertexBuffersImpl(u32 numStreams, u32 lastStreamSlot, const CDeviceInputStream* vertexStreams);
	void SetIndexBufferImpl(const CDeviceInputStream* indexStream); // NOTE: Take care with PSO strip cut/restart value and 32/16 bit indices
	void SetInlineConstantsImpl(u32 bindSlot, u32 constantCount, float* pConstants) { VK_NOT_IMPLEMENTED; }
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
	void BindNullVertexBuffers();
};

class CDeviceComputeCommandInterfaceImpl : public CDeviceCommandListImpl
{
protected:
	void PrepareUAVsForUseImpl(u32 viewCount, CGpuBuffer** pViews) const;
	void PrepareResourcesForUseImpl(u32 bindSlot, CDeviceResourceSet* pResources) const;
	void PrepareInlineConstantBufferForUseImpl(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlots, EShaderStage shaderStages) const;

	void SetPipelineStateImpl(const CDeviceComputePSO* pDevicePSO);
	void SetResourceLayoutImpl(const CDeviceResourceLayout* pResourceLayout);
	void SetResourcesImpl(u32 bindSlot, const CDeviceResourceSet* pResources);
	void SetInlineConstantBufferImpl(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot);
	void SetInlineConstantsImpl(u32 bindSlot, u32 constantCount, float* pConstants) { VK_NOT_IMPLEMENTED; }

	void DispatchImpl(u32 X, u32 Y, u32 Z);

	void ClearUAVImpl(D3DUAV* pView, const FLOAT Values[4], UINT NumRects, const D3D11_RECT* pRects);
	void ClearUAVImpl(D3DUAV* pView, const UINT Values[4], UINT NumRects, const D3D11_RECT* pRects);
};

class CDeviceCopyCommandInterfaceImpl : public CDeviceCommandListImpl
{
public:
	void                         CopyBuffer(NDrxVulkan::CBufferResource* pSrc, NDrxVulkan::CBufferResource* pDst, const SResourceRegionMapping& mapping);                      // GPU-side copy
	void                         CopyImage(NDrxVulkan::CImageResource* pSrc, NDrxVulkan::CImageResource* pDst, const SResourceRegionMapping& mapping);                         // GPU-side copy
	static bool                  FillBuffer(ukk pSrc, NDrxVulkan::CBufferResource* pDst, const SResourceMemoryMapping& mapping);                                       // CPU-fill
	NDrxVulkan::CBufferResource* UploadBuffer(ukk pSrc, NDrxVulkan::CBufferResource* pDst, const SResourceMemoryMapping& mapping, bool bAllowGpu);                     // CPU-fill (+ GPU-side copy if needed)
	void                         UploadImage(ukk pSrc, NDrxVulkan::CImageResource* pDst, const SResourceMemoryMapping& mapping, bool bExt = false);                    // CPU-fill (+ GPU-side copy if needed)
	void                         UploadImage(NDrxVulkan::CBufferResource* pSrc, NDrxVulkan::CImageResource* pDst, const SResourceMemoryMapping& mapping, bool bExt = false);   // GPU-side copy only
	void                         DownloadImage(NDrxVulkan::CImageResource* pSrc, NDrxVulkan::CBufferResource* pDst, const SResourceMemoryMapping& mapping, bool bExt = false); // GPU-side copy only

private: // All function below are implemented inline and forward to one of the functions above

	void CopyBuffer(NDrxVulkan::CBufferResource* pSrc, NDrxVulkan::CBufferResource* pDst)
	{
		SResourceRegionMapping mapping;
		mapping.SourceOffset.Left = 0;
		mapping.DestinationOffset.Left = 0;
		mapping.Extent.Width = pSrc->GetElementCount();
		CopyBuffer(pSrc, pDst, mapping);
	}

	void CopyImage(NDrxVulkan::CImageResource* pSrc, NDrxVulkan::CImageResource* pDst)
	{
		SResourceRegionMapping mapping;
		mapping.SourceOffset.Left = 0;
		mapping.SourceOffset.Top = 0;
		mapping.SourceOffset.Front = 0;
		mapping.SourceOffset.Subresource = 0;
		mapping.DestinationOffset.Left = 0;
		mapping.DestinationOffset.Top = 0;
		mapping.DestinationOffset.Front = 0;
		mapping.DestinationOffset.Subresource = 0;
		mapping.Extent.Width = pSrc->GetWidth();
		mapping.Extent.Height = pSrc->GetHeight();
		mapping.Extent.Depth = pSrc->GetDepth();
		mapping.Extent.Subresources = pSrc->GetMipCount() * pSrc->GetSliceCount();
		CopyImage(pSrc, pDst, mapping);
	}

	NDrxVulkan::CBufferResource* UploadBuffer(ukk pSrc, NDrxVulkan::CBufferResource* pDst, const SResourceMemoryAlignment& alignment, bool bAllowGpu)
	{
		SResourceMemoryMapping mapping;
		mapping.MemoryLayout = alignment;
		mapping.ResourceOffset.Left = 0;
		mapping.ResourceOffset.Subresource = 0;
		mapping.Extent.Width = pDst->GetElementCount();
		return UploadBuffer(pSrc, pDst, mapping, bAllowGpu);
	}

	void UploadImage(ukk pSrc, NDrxVulkan::CImageResource* pDst, const SResourceMemoryAlignment& alignment)
	{
		SResourceMemoryMapping mapping;
		mapping.MemoryLayout = alignment;
		mapping.ResourceOffset.Left = 0;
		mapping.ResourceOffset.Top = 0;
		mapping.ResourceOffset.Front = 0;
		mapping.ResourceOffset.Subresource = 0;
		mapping.Extent.Width = pDst->GetWidth();
		mapping.Extent.Height = pDst->GetHeight();
		mapping.Extent.Depth = pDst->GetDepth();
		mapping.Extent.Subresources = pDst->GetMipCount() * pDst->GetSliceCount();
		UploadImage(pSrc, pDst, mapping);
	}

	template<typename T>
	NDrxVulkan::CBufferResource* UnwrapDeviceBuffer(T* pDeviceBuffer)
	{
		return pDeviceBuffer->GetBuffer();
	}

	template<typename T>
	NDrxVulkan::CBufferResource* UnwrapConstantBuffer(T* pConstantBuffer)
	{
		return pConstantBuffer->m_buffer->GetBuffer();
	}

	template<typename T>
	NDrxVulkan::CImageResource* UnwrapDeviceTexture(T* pDeviceTexture)
	{
		// We use CDeviceTexture::Get2DTexture() on all "kinds" of CDeviceTexture, which is only possible because they are all the same type
		static_assert(
		  std::is_same<D3DLookupTexture, D3DTexture>::value &&
		  std::is_same<D3DCubeTexture, D3DTexture>::value &&
		  std::is_same<D3DVolumeTexture, D3DTexture>::value,
		  "All Vk texture types expected to share the same type");
		return pDeviceTexture->Get2DTexture();
	}

protected:
	void CopyImpl(CDeviceBuffer* pSrc, CDeviceBuffer* pDst)
	{
		CopyBuffer(UnwrapDeviceBuffer(pSrc), UnwrapDeviceBuffer(pDst));
	}

	void CopyImpl(D3DBuffer* pSrc, D3DBuffer* pDst)
	{
		CopyBuffer(pSrc, pDst);
	}

	void CopyImpl(CDeviceTexture* pSrc, CDeviceTexture* pDst)
	{
		CopyImage(UnwrapDeviceTexture(pSrc), UnwrapDeviceTexture(pDst));
	}

	void CopyImpl(CDeviceTexture* pSrc, D3DTexture* pDst)
	{
		CopyImage(UnwrapDeviceTexture(pSrc), pDst);
	}

	void CopyImpl(D3DTexture* pSrc, D3DTexture* pDst)
	{
		CopyImage(pSrc, pDst);
	}

	void CopyImpl(D3DTexture* pSrc, CDeviceTexture* pDst)
	{
		CopyImage(pSrc, UnwrapDeviceTexture(pDst));
	}

	void CopyImpl(CDeviceBuffer* pSrc, CDeviceBuffer* pDst, const SResourceRegionMapping& regionMapping)
	{
		CopyBuffer(UnwrapDeviceBuffer(pSrc), UnwrapDeviceBuffer(pDst), regionMapping);
	}

	void CopyImpl(D3DBuffer* pSrc, D3DBuffer* pDst, const SResourceRegionMapping& regionMapping)
	{
		CopyBuffer(pSrc, pDst, regionMapping);
	}

	void CopyImpl(CDeviceTexture* pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping)
	{
		CopyImage(UnwrapDeviceTexture(pSrc), UnwrapDeviceTexture(pDst), regionMapping);
	}

	void CopyImpl(D3DTexture* pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping)
	{
		CopyImage(pSrc, UnwrapDeviceTexture(pDst), regionMapping);
	}

	void CopyImpl(ukk pSrc, CConstantBuffer* pDst, const SResourceMemoryAlignment& memoryLayout)
	{
		VK_ASSERT(memoryLayout.typeStride == 1);
		SResourceMemoryMapping mapping;
		mapping.MemoryLayout = memoryLayout;
		mapping.ResourceOffset.Left = 0;
		mapping.Extent.Width = memoryLayout.rowStride;
		UploadBuffer(pSrc, UnwrapConstantBuffer(pDst), mapping, true);
	}

	void CopyImpl(ukk pSrc, CDeviceBuffer* pDst, const SResourceMemoryAlignment& memoryLayout)
	{
		UploadBuffer(pSrc, UnwrapDeviceBuffer(pDst), memoryLayout, true);
	}

	void CopyImpl(ukk pSrc, CDeviceTexture* pDst, const SResourceMemoryAlignment& memoryLayout)
	{
		UploadImage(pSrc, UnwrapDeviceTexture(pDst), memoryLayout);
	}

	void CopyImpl(ukk pSrc, CConstantBuffer* pDst, const SResourceMemoryMapping& memoryMapping)
	{
		UploadBuffer(pSrc, UnwrapConstantBuffer(pDst), memoryMapping, true);
	}

	void CopyImpl(ukk pSrc, CDeviceBuffer* pDst, const SResourceMemoryMapping& memoryMapping)
	{
		UploadBuffer(pSrc, UnwrapDeviceBuffer(pDst), memoryMapping, true);
	}

	void CopyImpl(ukk pSrc, CDeviceTexture* pDst, const SResourceMemoryMapping& memoryMapping)
	{
		UploadImage(pSrc, UnwrapDeviceTexture(pDst), memoryMapping);
	}

	void CopyImpl(CDeviceBuffer* pSrc, uk pDst, const SResourceMemoryAlignment& memoryLayout)
	{
		VK_NOT_IMPLEMENTED;
	}

	void CopyImpl(CDeviceTexture* pSrc, uk pDst, const SResourceMemoryAlignment& memoryLayout)
	{
		VK_NOT_IMPLEMENTED;
	}

	void CopyImpl(CDeviceBuffer* pSrc, uk pDst, const SResourceMemoryMapping& memoryMapping)
	{
		VK_NOT_IMPLEMENTED;
	}

	void CopyImpl(CDeviceTexture* pSrc, uk pDst, const SResourceMemoryMapping& memoryMapping)
	{
		VK_NOT_IMPLEMENTED;
	}
};

class CDeviceNvidiaCommandInterfaceImpl : public CDeviceCommandListImpl
{
};
