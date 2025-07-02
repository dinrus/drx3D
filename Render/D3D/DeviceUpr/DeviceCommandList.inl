// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DeviceObjects.h"

#if defined(DRX_RENDERER_GNM)
#include "GNM/DeviceCommandList_GNM.inl"
#endif

////////////////////////////////////////////////////////////////////////////

inline CDeviceGraphicsCommandInterface* CDeviceCommandList::GetGraphicsInterface()
{
	return reinterpret_cast<CDeviceGraphicsCommandInterface*>(GetGraphicsInterfaceImpl());
}

inline CDeviceComputeCommandInterface* CDeviceCommandList::GetComputeInterface()
{
	return reinterpret_cast<CDeviceComputeCommandInterface*>(GetComputeInterfaceImpl());
}

inline CDeviceNvidiaCommandInterface* CDeviceCommandList::GetNvidiaCommandInterface()
{
	return reinterpret_cast<CDeviceNvidiaCommandInterface*>(GetNvidiaCommandInterfaceImpl());
}

inline CDeviceCopyCommandInterface* CDeviceCommandList::GetCopyInterface()
{
	return reinterpret_cast<CDeviceCopyCommandInterface*>(GetCopyInterfaceImpl());
}

inline void CDeviceCommandList::Reset()
{
	ZeroStruct(m_graphicsState.pPipelineState);
	ZeroStruct(m_graphicsState.pResourceLayout);
	ZeroArray(m_graphicsState.pResources);
	m_graphicsState.stencilRef = -1;
	m_graphicsState.vertexStreams = nullptr;
	m_graphicsState.indexStream = nullptr;
	m_graphicsState.requiredResourceBindings = 0;
	m_graphicsState.validResourceBindings = 0;

	ZeroStruct(m_computeState.pPipelineState);
	ZeroStruct(m_computeState.pResourceLayout);
	ZeroArray(m_computeState.pResources);
	m_computeState.requiredResourceBindings = 0;
	m_computeState.validResourceBindings = 0;

	ResetImpl();

#if defined(ENABLE_PROFILING_CODE)
	m_primitiveTypeForProfiling = eptUnknown;
	m_profilingStats.Reset();
#endif

	ClearStateImpl(false);
}

inline void CDeviceCommandList::LockToThread()
{
	LockToThreadImpl();
}

#if defined(ENABLE_PROFILING_CODE)
inline void CDeviceCommandList::BeginProfilingSection()
{
	m_profilingStats.Reset();
}

inline const SProfilingStats& CDeviceCommandList::EndProfilingSection()
{
	return m_profilingStats;
}
#endif

////////////////////////////////////////////////////////////////////////////

inline void CDeviceGraphicsCommandInterface::ClearState(bool bOutputMergerOnly) const
{
	ClearStateImpl(bOutputMergerOnly);
}

inline void CDeviceGraphicsCommandInterface::PrepareUAVsForUse(u32 viewCount, CGpuBuffer** pViews, bool bCompute) const
{
	PrepareUAVsForUseImpl(viewCount, pViews, bCompute);
}

inline void CDeviceGraphicsCommandInterface::PrepareRenderPassForUse(CDeviceRenderPass& renderPass) const
{
	PrepareRenderPassForUseImpl(renderPass);
}

inline void CDeviceGraphicsCommandInterface::PrepareResourceForUse(u32 bindSlot, CTexture* pTexture, const ResourceViewHandle TextureView, ::EShaderStage srvUsage) const
{
	PrepareResourceForUseImpl(bindSlot, pTexture, TextureView, srvUsage);
}

inline void CDeviceGraphicsCommandInterface::PrepareResourcesForUse(u32 bindSlot, CDeviceResourceSet* pResources) const
{
	PrepareResourcesForUseImpl(bindSlot, pResources);
}

inline void CDeviceGraphicsCommandInterface::PrepareInlineConstantBufferForUse(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass) const
{
	PrepareInlineConstantBufferForUseImpl(bindSlot, pBuffer, shaderSlot, shaderClass);
}

inline void CDeviceGraphicsCommandInterface::PrepareInlineConstantBufferForUse(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, ::EShaderStage shaderStages) const
{
	PrepareInlineConstantBufferForUseImpl(bindSlot, pBuffer, shaderSlot, shaderStages);
}

inline void CDeviceGraphicsCommandInterface::PrepareVertexBuffersForUse(u32 streamCount, u32 streamMax, const CDeviceInputStream* vertexStreams) const
{
	PrepareVertexBuffersForUseImpl(streamCount, streamMax, vertexStreams);
}

inline void CDeviceGraphicsCommandInterface::PrepareIndexBufferForUse(const CDeviceInputStream* indexStream) const
{
	PrepareIndexBufferForUseImpl(indexStream);
}

inline void CDeviceGraphicsCommandInterface::BeginResourceTransitions(u32 numTextures, CTexture** pTextures, EResourceTransitionType type)
{
	BeginResourceTransitionsImpl(numTextures, pTextures, type);
}

inline void CDeviceGraphicsCommandInterface::BeginRenderPass(const CDeviceRenderPass& renderPass, const D3DRectangle& renderArea)
{
	BeginRenderPassImpl(renderPass, renderArea);
}

inline void CDeviceGraphicsCommandInterface::EndRenderPass(const CDeviceRenderPass& renderPass)
{
	EndRenderPassImpl(renderPass);
}
inline void CDeviceGraphicsCommandInterface::SetViewports(u32 vpCount, const D3DViewPort* pViewports)
{
	SetViewportsImpl(vpCount, pViewports);
}

inline void CDeviceGraphicsCommandInterface::SetScissorRects(u32 rcCount, const D3DRectangle* pRects)
{
	SetScissorRectsImpl(rcCount, pRects);
}

inline void CDeviceGraphicsCommandInterface::SetPipelineState(const CDeviceGraphicsPSO* pDevicePSO)
{
	if (m_graphicsState.pPipelineState.Set(pDevicePSO))
	{
		SetPipelineStateImpl(pDevicePSO);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numPSOSwitches;
		m_primitiveTypeForProfiling = pDevicePSO->m_PrimitiveTypeForProfiling;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::SetResourceLayout(const CDeviceResourceLayout* pResourceLayout)
{
	if (m_graphicsState.pResourceLayout.Set(pResourceLayout))
	{
		// If a root signature is changed on a command list, all previous root signature bindings
		// become stale and all newly expected bindings must be set before Draw/Dispatch
		ZeroArray(m_graphicsState.pResources);

		m_graphicsState.requiredResourceBindings = pResourceLayout->GetRequiredResourceBindings();
		m_graphicsState.validResourceBindings = 0;  // invalidate all resource bindings

		SetResourceLayoutImpl(pResourceLayout);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numLayoutSwitches;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::SetResources(u32 bindSlot, const CDeviceResourceSet* pResources)
{
	DRX_ASSERT(bindSlot <= EResourceLayoutSlot_Max);
	if (m_graphicsState.pResources[bindSlot].Set(pResources))
	{
		m_graphicsState.validResourceBindings[bindSlot] = pResources->IsValid();

		SetResourcesImpl(bindSlot, pResources);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numResourceSetSwitches;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::SetInlineConstantBuffer(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, EHWShaderClass shaderClass)
{
	// TODO: remove redundant InlineConstantBuffer sets
	//	if (m_pCurrentResources[bindSlot] != ((char*)pBuffer->m_base_ptr + pBuffer->m_offset))
	{
		//		m_pCurrentResources[bindSlot] = ((char*)pBuffer->m_base_ptr + pBuffer->m_offset);

#if _RELEASE
		m_graphicsState.validResourceBindings[bindSlot] = true;
#else
		DRX_ASSERT(pBuffer != nullptr);
		if ((m_graphicsState.validResourceBindings[bindSlot] = (pBuffer != nullptr)))
#endif
			SetInlineConstantBufferImpl(bindSlot, pBuffer, shaderSlot, shaderClass);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numInlineSets;
#endif

	}
}

inline void CDeviceGraphicsCommandInterface::SetInlineConstantBuffer(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, ::EShaderStage shaderStages)
{
	// TODO: remove redundant InlineConstantBuffer sets
	//	if (m_pCurrentResources[bindSlot] != ((char*)pBuffer->m_base_ptr + pBuffer->m_offset))
	{
		//		m_pCurrentResources[bindSlot] = ((char*)pBuffer->m_base_ptr + pBuffer->m_offset);

#if _RELEASE
		m_graphicsState.validResourceBindings[bindSlot] = true;
#else
		DRX_ASSERT(pBuffer != nullptr);
		if ((m_graphicsState.validResourceBindings[bindSlot] = (pBuffer != nullptr)))
#endif
			SetInlineConstantBufferImpl(bindSlot, pBuffer, shaderSlot, shaderStages);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numInlineSets;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::SetInlineConstants(u32 bindSlot, u32 constantCount, float* pConstants)
{
	m_graphicsState.validResourceBindings[bindSlot] = true;
	SetInlineConstantsImpl(bindSlot, constantCount, pConstants);

#if defined(ENABLE_PROFILING_CODE)
	++m_profilingStats.numInlineSets;
#endif
}

inline void CDeviceGraphicsCommandInterface::SetVertexBuffers(u32 streamCount, u32 streamMax, const CDeviceInputStream* vertexStreams)
{
	if (m_graphicsState.vertexStreams.Set(vertexStreams))
	{
		SetVertexBuffersImpl(streamCount, streamMax, vertexStreams);
	}
}

inline void CDeviceGraphicsCommandInterface::SetIndexBuffer(const CDeviceInputStream* indexStream)
{
	if (m_graphicsState.indexStream.Set(indexStream))
	{
		SetIndexBufferImpl(indexStream);
	}
}

inline void CDeviceGraphicsCommandInterface::SetStencilRef(u8 stencilRefValue)
{
	if (m_graphicsState.stencilRef.Set(stencilRefValue))
	{
		SetStencilRefImpl(stencilRefValue);
	}
}

inline void CDeviceGraphicsCommandInterface::SetDepthBias(float constBias, float slopeBias, float biasClamp)
{
	SetDepthBiasImpl(constBias, slopeBias, biasClamp);
}

inline void CDeviceGraphicsCommandInterface::SetDepthBounds(float fMin, float fMax)
{
	SetDepthBoundsImpl(fMin, fMax);
}

inline void CDeviceGraphicsCommandInterface::Draw(u32 VertexCountPerInstance, u32 InstanceCount, u32 StartVertexLocation, u32 StartInstanceLocation)
{
	if (m_graphicsState.validResourceBindings == m_graphicsState.requiredResourceBindings)
	{
		DrawImpl(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

#if defined(ENABLE_PROFILING_CODE)
		int nPrimitives = VertexCountPerInstance;

		switch (m_primitiveTypeForProfiling)
		{
		case eptTriangleList:
			nPrimitives = VertexCountPerInstance / 3;
			assert(VertexCountPerInstance % 3 == 0);
			break;
		case eptTriangleStrip:
			nPrimitives = VertexCountPerInstance - 2;
			assert(VertexCountPerInstance > 2);
			break;
		case eptLineList:
			nPrimitives = VertexCountPerInstance / 2;
			assert(VertexCountPerInstance % 2 == 0);
			break;
		case eptLineStrip:
			nPrimitives = VertexCountPerInstance - 1;
			assert(VertexCountPerInstance > 1);
			break;
		case eptPointList:
			nPrimitives = VertexCountPerInstance;
			assert(VertexCountPerInstance > 0);
			break;
		case ept1ControlPointPatchList:
			nPrimitives = VertexCountPerInstance;
			assert(VertexCountPerInstance > 0);
			break;

	#ifndef _RELEASE
		default:
			assert(0);
	#endif
		}

		m_profilingStats.numPolygons += nPrimitives * InstanceCount;
		m_profilingStats.numDIPs++;
	}
	else
	{
		m_profilingStats.numInvalidDIPs++;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::DrawIndexed(u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, int BaseVertexLocation, u32 StartInstanceLocation)
{
	if (m_graphicsState.validResourceBindings == m_graphicsState.requiredResourceBindings)
	{
		DrawIndexedImpl(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

#if defined(ENABLE_PROFILING_CODE)
		int nPrimitives = IndexCountPerInstance;

		switch (m_primitiveTypeForProfiling)
		{
		case eptTriangleList:
			nPrimitives = IndexCountPerInstance / 3;
			assert(IndexCountPerInstance % 3 == 0);
			break;
		case ept4ControlPointPatchList:
			nPrimitives = IndexCountPerInstance >> 2;
			assert(IndexCountPerInstance % 4 == 0);
			break;
		case ept3ControlPointPatchList:
			nPrimitives = IndexCountPerInstance / 3;
			assert(IndexCountPerInstance % 3 == 0);
			break;
		case eptTriangleStrip:
			nPrimitives = IndexCountPerInstance - 2;
			assert(IndexCountPerInstance > 2);
			break;
		case eptLineList:
			nPrimitives = IndexCountPerInstance >> 1;
			assert(IndexCountPerInstance % 2 == 0);
			break;
#ifdef _DEBUG
		default:
			assert(0);
#endif
		}

		m_profilingStats.numPolygons += nPrimitives * InstanceCount;
		m_profilingStats.numDIPs++;
	}
	else
	{
		m_profilingStats.numInvalidDIPs++;
#endif
	}
}

inline void CDeviceGraphicsCommandInterface::ClearSurface(D3DSurface* pView, const ColorF& color, u32 numRects, const D3D11_RECT* pRects)
{
	ClearSurfaceImpl(pView, (float*)&color, numRects, pRects);
}

inline void CDeviceGraphicsCommandInterface::ClearSurface(D3DDepthSurface* pView, int clearFlags, float depth, u8 stencil, u32 numRects, const D3D11_RECT* pRects)
{
	ClearSurfaceImpl(pView, clearFlags, depth, stencil, numRects, pRects);
}

inline void CDeviceGraphicsCommandInterface::BeginOcclusionQuery(D3DOcclusionQuery* pQuery)
{
	BeginOcclusionQueryImpl(pQuery);
}

inline void CDeviceGraphicsCommandInterface::EndOcclusionQuery(D3DOcclusionQuery* pQuery)
{
	EndOcclusionQueryImpl(pQuery);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CDeviceComputeCommandInterface::PrepareUAVsForUse(u32 viewCount, CGpuBuffer** pViews) const
{
	PrepareUAVsForUseImpl(viewCount, pViews);
}

inline void CDeviceComputeCommandInterface::PrepareResourcesForUse(u32 bindSlot, CDeviceResourceSet* pResources) const
{
	PrepareResourcesForUseImpl(bindSlot, pResources);
}

inline void CDeviceComputeCommandInterface::PrepareInlineConstantBufferForUse(u32 bindSlot, CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot, ::EShaderStage shaderStages) const
{
	PrepareInlineConstantBufferForUseImpl(bindSlot, pBuffer, shaderSlot, shaderStages);
}

inline void CDeviceComputeCommandInterface::SetPipelineState(const CDeviceComputePSO* pDevicePSO)
{
	if (m_computeState.pPipelineState.Set(pDevicePSO))
	{
		SetPipelineStateImpl(pDevicePSO);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numPSOSwitches;
#endif
	}
}

inline void CDeviceComputeCommandInterface::SetResourceLayout(const CDeviceResourceLayout* pResourceLayout)
{
	if (m_computeState.pResourceLayout.Set(pResourceLayout))
	{
		// If a root signature is changed on a command list, all previous root signature bindings
		// become stale and all newly expected bindings must be set before Draw/Dispatch
		ZeroArray(m_computeState.pResources);

		m_computeState.requiredResourceBindings = pResourceLayout->GetRequiredResourceBindings();
		m_computeState.validResourceBindings = 0;

		SetResourceLayoutImpl(pResourceLayout);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numLayoutSwitches;
#endif
	}
}

inline void CDeviceComputeCommandInterface::SetResources(u32 bindSlot, const CDeviceResourceSet* pResources)
{
	DRX_ASSERT(bindSlot <= EResourceLayoutSlot_Max);
	if (m_computeState.pResources[bindSlot].Set(pResources))
	{
		m_computeState.validResourceBindings[bindSlot] = pResources->IsValid();

		SetResourcesImpl(bindSlot, pResources);

#if defined(ENABLE_PROFILING_CODE)
		++m_profilingStats.numResourceSetSwitches;
#endif
	}
}

inline void CDeviceComputeCommandInterface::SetInlineConstantBuffer(u32 bindSlot, const CConstantBuffer* pBuffer, EConstantBufferShaderSlot shaderSlot)
{
	m_computeState.validResourceBindings[bindSlot] = true;
	SetInlineConstantBufferImpl(bindSlot, pBuffer, shaderSlot);
}

inline void CDeviceComputeCommandInterface::SetInlineConstants(u32 bindSlot, u32 constantCount, float* pConstants)
{
	m_computeState.validResourceBindings[bindSlot] = true;
	SetInlineConstantsImpl(bindSlot, constantCount, pConstants);
}

inline void CDeviceComputeCommandInterface::Dispatch(u32 X, u32 Y, u32 Z)
{
	DispatchImpl(X, Y, Z);
}

inline void CDeviceComputeCommandInterface::ClearUAV(D3DUAV* pView, const ColorF& Values, UINT NumRects, const D3D11_RECT* pRects)
{
	ClearUAVImpl(pView, (float*)&Values, NumRects, pRects);
}

inline void CDeviceComputeCommandInterface::ClearUAV(D3DUAV* pView, const ColorI& Values, UINT NumRects, const D3D11_RECT* pRects)
{
	ClearUAVImpl(pView, (UINT*)&Values, NumRects, pRects);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void CDeviceCopyCommandInterface::Copy(CDeviceBuffer* pSrc, CDeviceBuffer* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(D3DBuffer* pSrc, D3DBuffer* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceTexture* pSrc, CDeviceTexture* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceTexture* pSrc, D3DTexture* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(D3DTexture* pSrc, D3DTexture* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(D3DTexture* pSrc, CDeviceTexture* pDst)
{
	CopyImpl(pSrc, pDst);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceBuffer* pSrc, CDeviceBuffer* pDst, const SResourceRegionMapping& regionMapping)
{
	CopyImpl(pSrc, pDst, regionMapping);
}

inline void CDeviceCopyCommandInterface::Copy(D3DBuffer* pSrc, D3DBuffer* pDst, const SResourceRegionMapping& regionMapping)
{
	CopyImpl(pSrc, pDst, regionMapping);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceTexture* pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping)
{
	CopyImpl(pSrc, pDst, regionMapping);
}

inline void CDeviceCopyCommandInterface::Copy(D3DTexture* pSrc, CDeviceTexture* pDst, const SResourceRegionMapping& regionMapping)
{
	CopyImpl(pSrc, pDst, regionMapping);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CConstantBuffer* pDst, const SResourceMemoryAlignment& memoryLayout)
{
	CopyImpl(pSrc, pDst, memoryLayout);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CDeviceBuffer* pDst, const SResourceMemoryAlignment& memoryLayout)
{
	CopyImpl(pSrc, pDst, memoryLayout);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CDeviceTexture* pDst, const SResourceMemoryAlignment& memoryLayout)
{
	CopyImpl(pSrc, pDst, memoryLayout);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CConstantBuffer* pDst, const SResourceMemoryMapping& memoryMapping)
{
	CopyImpl(pSrc, pDst, memoryMapping);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CDeviceBuffer* pDst, const SResourceMemoryMapping& memoryMapping)
{
	CopyImpl(pSrc, pDst, memoryMapping);
}

inline void CDeviceCopyCommandInterface::Copy(const void* pSrc, CDeviceTexture* pDst, const SResourceMemoryMapping& memoryMapping)
{
	CopyImpl(pSrc, pDst, memoryMapping);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceBuffer* pSrc, void* pDst, const SResourceMemoryAlignment& memoryLayout)
{
	CopyImpl(pSrc, pDst, memoryLayout);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceTexture* pSrc, void* pDst, const SResourceMemoryAlignment& memoryLayout)
{
	CopyImpl(pSrc, pDst, memoryLayout);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceBuffer* pSrc, void* pDst, const SResourceMemoryMapping& memoryMapping)
{
	CopyImpl(pSrc, pDst, memoryMapping);
}

inline void CDeviceCopyCommandInterface::Copy(CDeviceTexture* pSrc, void* pDst, const SResourceMemoryMapping& memoryMapping)
{
	CopyImpl(pSrc, pDst, memoryMapping);
}

#if DRX_RENDERER_GNM
	#include "GNM/DeviceCommandList_GNM.inl"
#endif


inline void CDeviceNvidiaCommandInterface::SetModifiedWMode(bool enabled, u32 numViewports, const float* pA, const float* pB)
{
#if defined(USE_NV_API) && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	SetModifiedWModeImpl(enabled, numViewports, pA, pB);
#else
	DRX_ASSERT_MESSAGE(false, "Only supported on DirectX11, PC");
#endif
}