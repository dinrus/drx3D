// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/PrimitiveRenderPass.h>
#include <drx3D/Render/RendElements/FlareSoftOcclusionQuery.h>

class CRenderView;

class CLensOpticsStage : public CGraphicsPipelineStage
{
public:
	void      Init();
	void      Execute();

	bool      HasContent() const { return m_primitivesRendered>0; }

private:
	void      UpdateOcclusionQueries(SRenderViewInfo* pViewInfo, i32 viewInfoCount);

	CPrimitiveRenderPass  m_passLensOptics;
	CSoftOcclusionUpr m_softOcclusionUpr;

	i32             m_occlusionUpdateFrame = -1;
	i32             m_primitivesRendered   =  0;
};
