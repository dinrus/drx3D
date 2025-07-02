// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Range.h>

//  предварительные объявления
class CGraphicsPipelineStage;
class CGraphicsPipelineStateLocalCache;
class CStandardGraphicsPipeline;
class CSceneRenderPass;
class CRenderView;
class CCVarUpdateRecorder;
struct SRenderViewInfo;
class CGraphicsPipeline;
class CRenderOutput;

enum class GraphicsPipelinePassType : std::uint8_t
{
	renderPass,
	resolve,
};

struct SGraphicsPipelinePassContext
{
	SGraphicsPipelinePassContext() = default;

	SGraphicsPipelinePassContext(CRenderView* renderView, CSceneRenderPass* pSceneRenderPass, EShaderTechniqueID technique, u32 filter, u32 excludeFilter)
		: pSceneRenderPass(pSceneRenderPass)
		, batchFilter(filter)
		, batchExcludeFilter(excludeFilter)
		, pRenderView(renderView)
		, techniqueID(technique)
	{

		;
	}

	SGraphicsPipelinePassContext(GraphicsPipelinePassType type, CRenderView* renderView, CSceneRenderPass* pSceneRenderPass) 
		: type(type), pSceneRenderPass(pSceneRenderPass), pRenderView(renderView)
	{}

	GraphicsPipelinePassType type = GraphicsPipelinePassType::renderPass;

	CSceneRenderPass*  pSceneRenderPass;

	u32             batchFilter;
	u32             batchExcludeFilter;

	ERenderListID      renderListId = EFSLIST_INVALID;

	// Stage ID of a scene stage (EStandardGraphicsPipelineStage)
	u32 stageID = 0;
	// Pass ID, in case a stage has several different scene passes
	u32 passID = 0;

	u32 renderItemGroup;
	u32 profilerSectionIndex;

	// rend items
	CRenderView* pRenderView;

	std::vector<TRect_tpl<u16>> resolveScreenBounds;

	// Uses the range if rendItems is empty, otherwise uses the rendItems vector
	TRange<i32>        rendItems;

	bool               renderNearest = false;
	EShaderTechniqueID techniqueID;

	std::map<struct IRenderNode*, IRenderer::SDrawCallCountInfo>* pDrawCallInfoPerNode = nullptr;
	std::map<struct IRenderMesh*, IRenderer::SDrawCallCountInfo>* pDrawCallInfoPerMesh = nullptr;
};

class CGraphicsPipelineStage
{
public:
	virtual ~CGraphicsPipelineStage() {}

	// Allocate resources needed by the graphics pipeline stage
	virtual void Init()                                    {}
	// Change internal values and resources when the render-resolution has changed (or the underlying resource dimensions)
	// If the resources are considering sub-view/rectangle support then interpret CRendererResources::s_resourceWidth/Height
	virtual void Resize(i32 renderWidth, i32 renderHeight) {}
	// Update stage before actual rendering starts (called every "RenderScene")
	virtual void Update()                                  {}

	// Reset any cvar dependent states.
	virtual void OnCVarsChanged(const CCVarUpdateRecorder& cvarUpdater) {}

	// If this stage should be updated based on the given flags
	virtual bool IsStageActive(EShaderRenderingFlags flags) const { return true; }

public:
	void                             SetRenderView(CRenderView* pRenderView) { m_pRenderView = pRenderView; }

	CRenderView*                     RenderView() const { return m_pRenderView; }
	const SRenderViewport&           GetViewport() const;

	const SRenderViewInfo&           GetCurrentViewInfo() const;

	CGraphicsPipeline&               GetGraphicsPipeline()       { assert(m_pGraphicsPipeline); return *m_pGraphicsPipeline; }
	const CGraphicsPipeline&         GetGraphicsPipeline() const { assert(m_pGraphicsPipeline); return *m_pGraphicsPipeline; }

	CStandardGraphicsPipeline&       GetStdGraphicsPipeline();
	const CStandardGraphicsPipeline& GetStdGraphicsPipeline() const;

	const CRenderOutput&             GetRenderOutput() const;
	CRenderOutput&                   GetRenderOutput();

protected:
	void                             ClearDeviceOutputState();

protected:
	friend class CGraphicsPipeline;

	u32             m_stageID;

private:
	CGraphicsPipeline* m_pGraphicsPipeline = nullptr;
	CRenderView*       m_pRenderView = nullptr;
};
