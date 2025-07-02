// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CToneMappingStage : public CGraphicsPipelineStage
{
public:
	void Execute();
	void ExecuteDebug();
	void ExecuteFixedExposure(CTexture* pColorTex, CTexture* pDepthTex);
	void DisplayDebugInfo();

private:
	_smart_ptr<CTexture> m_pColorChartTex;
	CFullscreenPass      m_passToneMapping;
	CFullscreenPass      m_passFixedExposureToneMapping;
};
