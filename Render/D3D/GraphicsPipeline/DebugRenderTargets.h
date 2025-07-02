// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/PrimitiveRenderPass.h>

class CDebugRenderTargetsStage : public CGraphicsPipelineStage
{
public:
	static tukk showRenderTargetHelp;

	void Execute();

	void OnShowRenderTargetsCmd(IConsoleCmdArgs* pArgs);

private:

	struct SRenderTargetInfo
	{
		CTexture* pTexture      = nullptr;
		Vec4      channelWeight = Vec4(1.0f);
		bool      bFiltered     = false;
		bool      bRGBKEncoded  = false;
		bool      bAliased      = false;
		i32       slice         = -1;
		i32       mip           = -1;
	};

	void ResetRenderTargetList();
	void ExecuteShowTargets();

private:
	CPrimitiveRenderPass          m_debugPass;
	std::vector<CRenderPrimitive> m_debugPrimitives;
	i32                           m_primitiveCount = 0;

	std::vector<SRenderTargetInfo> m_renderTargetList;
	bool                           m_bShowList;
	i32                            m_columnCount;
};