// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/GraphicsPipeline/RenderPassBase.h>

struct VrProjectionInfo;

class CSceneRenderPass : public CRenderPassBase
{
	friend class CRenderPassScheduler;

public:
	enum EPassFlags
	{
		ePassFlags_None                         = 0,
		ePassFlags_RenderNearest                = BIT(0),
		ePassFlags_ReverseDepth                 = BIT(1),
		ePassFlags_UseVrProjectionState         = BIT(2),
		ePassFlags_RequireVrProjectionConstants = BIT(3),
		ePassFlags_VrProjectionPass             = ePassFlags_UseVrProjectionState | ePassFlags_RequireVrProjectionConstants // for convenience
	};

	CSceneRenderPass();

	void SetupPassContext(u32 stageID, u32 stagePassID, EShaderTechniqueID technique, u32 filter, ERenderListID renderList = EFSLIST_GENERAL, u32 excludeFilter = 0, bool drawCompiledRenderObject = true);
	void SetPassResources(CDeviceResourceLayoutPtr pResourceLayout, CDeviceResourceSetPtr pPerPassResources);
	void SetRenderTargets(CTexture* pDepthTarget, CTexture* pColorTarget0, CTexture* pColorTarget1 = NULL, CTexture* pColorTarget2 = NULL, CTexture* pColorTarget3 = NULL);
	void ExchangeRenderTarget(u32 slot, CTexture* pNewColorTarget, ResourceViewHandle hRenderTargetView = EDefaultResourceViews::RenderTarget);
	void ExchangeDepthTarget(CTexture* pNewDepthTarget, ResourceViewHandle hDepthStencilView = EDefaultResourceViews::DepthStencil);
	void SetFlags(EPassFlags flags)  { m_passFlags = flags; }
	void SetViewport(const D3DViewPort& viewport);
	void SetViewport(const SRenderViewport& viewport);
	void SetDepthBias(float constBias, float slopeBias, float biasClamp);

	void BeginExecution();
	void EndExecution();
	void Execute();

	void DrawRenderItems(CRenderView* pRenderView, ERenderListID list, i32 listStart = -1, i32 listEnd = -1);
	void DrawTransparentRenderItems(CRenderView* pRenderView, ERenderListID list);

	// Called from rendering backend (has to be threadsafe)
	void                PrepareRenderPassForUse(CDeviceCommandListRef RESTRICT_REFERENCE commandList);
	void                BeginRenderPass(CDeviceCommandListRef RESTRICT_REFERENCE commandList, bool bNearest) const;
	void                EndRenderPass(CDeviceCommandListRef RESTRICT_REFERENCE commandList, bool bNearest) const;

	void                ResolvePass(CDeviceCommandListRef RESTRICT_REFERENCE commandList, const std::vector<TRect_tpl<u16>>& screenBounds) const;

	u32              GetStageID()             const { return m_stageID; }
	u32              GetPassID()              const { return m_passID; }
	u32              GetNumRenderItemGroups() const { return m_numRenderItemGroups; }
	EPassFlags          GetFlags()               const { return m_passFlags; }
	const D3DViewPort&  GetViewport(bool n)      const { return m_viewPort[n]; }
	const D3DRectangle& GetScissorRect()         const { return m_scissorRect; }
	
	CDeviceResourceLayoutPtr   GetResourceLayout() const { return m_pResourceLayout; }
	const CDeviceRenderPassPtr GetRenderPass()     const { return m_pRenderPass; }
	ERenderListID GetRenderList()                  const { return m_renderList; }

protected:
	static bool OnResourceInvalidated(uk pThis, SResourceBindPoint bindPoint, UResourceReference pResource, u32 flags) threadsafe;

protected:
	CDeviceRenderPassDesc    m_renderPassDesc;
	CDeviceRenderPassPtr     m_pRenderPass;
	D3DViewPort              m_viewPort[2];
	D3DRectangle             m_scissorRect;
	CDeviceResourceLayoutPtr m_pResourceLayout;
	CDeviceResourceSetPtr    m_pPerPassResourceSet;

	EShaderTechniqueID       m_technique;
	u32                   m_stageID : 16;
	u32                   m_passID  : 16;
	u32                   m_batchFilter;
	u32                   m_excludeFilter;
	EPassFlags               m_passFlags;
	ERenderListID            m_renderList;

	u32                   m_numRenderItemGroups;
	u32                   m_profilerSectionIndex;

	float                    m_depthConstBias;
	float                    m_depthSlopeBias;
	float                    m_depthBiasClamp;

	std::vector<SGraphicsPipelinePassContext> m_passContexts;

protected:
	static i32               s_recursionCounter;  // For asserting Begin/EndExecution get called on pass
};

DEFINE_ENUM_FLAG_OPERATORS(CSceneRenderPass::EPassFlags)
