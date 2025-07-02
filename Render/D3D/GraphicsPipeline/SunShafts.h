// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CSunShaftsStage : public CGraphicsPipelineStage
{
public:
	void      Init();
	void      Execute();

	bool      IsActive();
	CTexture* GetFinalOutputRT();
	void      GetCompositionParams(Vec4& params0, Vec4& params1);

private:
	CTexture* GetTempOutputRT();
	i32       GetDownscaledTargetsIndex();

private:
	CFullscreenPass m_passShaftsMask;
	CFullscreenPass m_passShaftsGen0;
	CFullscreenPass m_passShaftsGen1;
};
