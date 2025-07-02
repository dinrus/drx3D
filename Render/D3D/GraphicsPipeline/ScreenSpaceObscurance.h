// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CScreenSpaceObscuranceStage : public CGraphicsPipelineStage
{
public:
	void Init();
	void Execute();

private:
	CStretchRectPass  m_passCopyFromESRAM;
	CFullscreenPass   m_passObscurance;
	CFullscreenPass   m_passFilter;
	CStretchRectPass  m_passAlbedoDownsample0;
	CStretchRectPass  m_passAlbedoDownsample1;
	CStretchRectPass  m_passAlbedoDownsample2;
	CGaussianBlurPass m_passAlbedoBlur;
};
