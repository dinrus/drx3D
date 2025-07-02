// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/GraphicsPipeline/RenderPassBase.h>


class CComputeRenderPass : public CRenderPassBase
{
public:
	enum EPassFlags
	{
		eFlags_None = 0,
		eFlags_ReflectConstantBuffersFromShader = BIT(0)
	};

	enum EDirtyFlags : u32
	{
		eDirty_ResourceLayout = BIT(0),
		eDirty_Resources = BIT(1),
		eDirty_Technique = BIT(2),

		eDirty_None = 0,
		eDirty_All = eDirty_Technique | eDirty_Resources | eDirty_ResourceLayout
	};

	static_assert(uint(eDirty_ResourceLayout) == uint(CDeviceResourceSetDesc::EDirtyFlags::eDirtyBindPoint), "eDirty_ResourceLayout needs to match CDeviceResourceSetDesc::EDirtyFlags::eDirtyBindPoint");
	static_assert(uint(eDirty_Resources) == uint(CDeviceResourceSetDesc::EDirtyFlags::eDirtyBinding), "eDirty_Resources needs to match CDeviceResourceSetDesc::EDirtyFlags::eDirtyBinding");

public:
	CComputeRenderPass(EPassFlags flags = eFlags_None);

	bool IsDirty() const override final;
	using CRenderPassBase::IsDirty;

	void SetOutputUAV(u32 slot, CTexture* pTexture, ResourceViewHandle resourceViewID = EDefaultResourceViews::UnorderedAccess, ::EShaderStage shaderStages = EShaderStage_Compute);
	void SetOutputUAV(u32 slot, CGpuBuffer* pBuffer, ResourceViewHandle resourceViewID = EDefaultResourceViews::UnorderedAccess, ::EShaderStage shaderStages = EShaderStage_Compute);
	void SetDispatchSize(u32 sizeX, u32 sizeY, u32 sizeZ);
	void SetTechnique(CShader* pShader, const CDrxNameTSCRC& techName, uint64 rtMask);
	void SetFlags(EPassFlags flags);
	void SetTexture(u32 slot, CTexture* pTexture, ResourceViewHandle resourceViewID = EDefaultResourceViews::Default);
	void SetTextureSamplerPair(u32 slot, CTexture* pTex, SamplerStateHandle sampler, ResourceViewHandle resourceViewID = EDefaultResourceViews::Default);
	void SetBuffer(u32 slot, CGpuBuffer* pBuffer);
	void SetSampler(u32 slot, SamplerStateHandle sampler);
	void SetInlineConstantBuffer(u32 slot, CConstantBuffer* pConstantBuffer);
	void SetConstant(const CDrxNameR& paramName, const Vec4 &param);
	void SetConstant(const CDrxNameR& paramName, const Matrix44 &param);
	void SetConstantArray(const CDrxNameR& paramName, const Vec4 params[], u32 numParams);

	void PrepareResourcesForUse(CDeviceCommandListRef RESTRICT_REFERENCE commandList);

	void BeginConstantUpdate();

	void BeginRenderPass(CDeviceCommandListRef RESTRICT_REFERENCE commandList);
	void Dispatch(CDeviceCommandListRef RESTRICT_REFERENCE commandList, ::EShaderStage srvUsage);
	void EndRenderPass(CDeviceCommandListRef RESTRICT_REFERENCE commandList);

	// Executes this pass as if it's the only participant of a compute stage (see CSceneGBufferStage + CSceneRenderPass)
	void Execute(CDeviceCommandListRef RESTRICT_REFERENCE commandList, ::EShaderStage srvUsage = EShaderStage_Compute);

	void Reset();

private:
	EDirtyFlags Compile();

	static bool OnResourceInvalidated(uk pThis, SResourceBindPoint bindPoint, UResourceReference pResource, u32 flags) threadsafe;

	typedef SDeviceObjectHelpers::CShaderConstantUpr ConstantUpr;

private:
	EPassFlags               m_flags;
	EDirtyFlags              m_dirtyMask;
	CShader*                 m_pShader;
	CDrxNameTSCRC            m_techniqueName;
	uint64                   m_rtMask;
	u32                   m_dispatchSizeX;
	u32                   m_dispatchSizeY;
	u32                   m_dispatchSizeZ;
	CDeviceResourceSetDesc   m_resourceDesc;
	CDeviceResourceSetPtr    m_pResourceSet;
	CDeviceResourceLayoutPtr m_pResourceLayout;
	CDeviceComputePSOPtr     m_pPipelineState;
	i32                      m_currentPsoUpdateCount;

	ConstantUpr          m_constantUpr;

	bool                     m_bPendingConstantUpdate;
	bool                     m_bCompiled;

	SProfilingStats          m_profilingStats;
};

DEFINE_ENUM_FLAG_OPERATORS(CComputeRenderPass::EDirtyFlags);

///////////////////////////////////// Inline functions for CComputeRenderPass /////////////////////////////////////

#define ASSIGN_VALUE(dst, src, dirtyFlag) \
  if ((dst) != (src))                     \
    m_dirtyMask |= (dirtyFlag);           \
  (dst) = (src);

inline void CComputeRenderPass::SetDispatchSize(u32 sizeX, u32 sizeY, u32 sizeZ)
{
	m_dispatchSizeX = sizeX;
	m_dispatchSizeY = sizeY;
	m_dispatchSizeZ = sizeZ;
}

inline void CComputeRenderPass::SetTechnique(CShader* pShader, const CDrxNameTSCRC& techName, uint64 rtMask)
{
	ASSIGN_VALUE(m_pShader, pShader, eDirty_Technique);
	ASSIGN_VALUE(m_techniqueName, techName, eDirty_Technique);
	ASSIGN_VALUE(m_rtMask, rtMask, eDirty_Technique);

	if (m_pPipelineState && (!m_pPipelineState->IsValid() || m_currentPsoUpdateCount != m_pPipelineState->GetUpdateCount()))
	{
		m_dirtyMask |= eDirty_Technique;
	}
}

inline void CComputeRenderPass::SetFlags(EPassFlags flags)
{
	ASSIGN_VALUE(m_flags, flags, eDirty_ResourceLayout);
}

inline void CComputeRenderPass::SetInlineConstantBuffer(u32 slot, CConstantBuffer* pConstantBuffer)
{
	if (m_constantUpr.SetTypedConstantBuffer(EConstantBufferShaderSlot(slot), pConstantBuffer, EShaderStage_Compute))
		m_dirtyMask |= eDirty_ResourceLayout;
}

inline void CComputeRenderPass::SetConstant(const CDrxNameR& paramName, const Vec4 &param)
{
	DRX_ASSERT(m_bPendingConstantUpdate);
	m_constantUpr.SetNamedConstant(paramName, param, eHWSC_Compute);
}

inline void CComputeRenderPass::SetConstant(const CDrxNameR& paramName, const Matrix44 &param)
{
	SetConstantArray(paramName, reinterpret_cast<const Vec4*>(param.GetData()), 4);
}

inline void CComputeRenderPass::SetConstantArray(const CDrxNameR& paramName, const Vec4 params[], u32 numParams)
{
	DRX_ASSERT(m_bPendingConstantUpdate);
	m_constantUpr.SetNamedConstantArray(paramName, params, numParams, eHWSC_Compute);
}

#undef ASSIGN_VALUE

// ------------------------------------------------------------------------

inline void CComputeRenderPass::SetOutputUAV(u32 slot, CTexture* pTexture, ResourceViewHandle resourceViewID, ::EShaderStage shaderStages)
{
	m_resourceDesc.SetTexture(slot, pTexture, resourceViewID, shaderStages);
}

inline void CComputeRenderPass::SetOutputUAV(u32 slot, CGpuBuffer* pBuffer, ResourceViewHandle resourceViewID, ::EShaderStage shaderStages)
{
	m_resourceDesc.SetBuffer(slot, pBuffer, resourceViewID, shaderStages);
}

inline void CComputeRenderPass::SetTexture(u32 slot, CTexture* pTexture, ResourceViewHandle resourceViewID)
{
	m_resourceDesc.SetTexture(slot, pTexture, resourceViewID, EShaderStage_Compute);
}

inline void CComputeRenderPass::SetTextureSamplerPair(u32 slot, CTexture* pTex, SamplerStateHandle sampler, ResourceViewHandle resourceViewID)
{
	m_resourceDesc.SetTexture(slot, pTex, resourceViewID, EShaderStage_Compute);
	m_resourceDesc.SetSampler(slot, sampler, EShaderStage_Compute);
}

inline void CComputeRenderPass::SetBuffer(u32 slot, CGpuBuffer* pBuffer)
{
	m_resourceDesc.SetBuffer(slot, pBuffer, EDefaultResourceViews::Default, EShaderStage_Compute);
}

inline void CComputeRenderPass::SetSampler(u32 slot, SamplerStateHandle sampler)
{
	m_resourceDesc.SetSampler(slot, sampler, EShaderStage_Compute);
}
