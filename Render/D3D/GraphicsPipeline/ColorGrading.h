// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CColorGradingController;
struct SColorGradingMergeParams;

class CColorGradingStage : public CGraphicsPipelineStage
{
	typedef stl::aligned_vector<SVF_P3F_C4B_T2F, DRX_PLATFORM_ALIGNMENT> VertexArray;

public:
	CColorGradingStage();

	void Init();
	void Execute();

	void SetStaticColorChart(_smart_ptr<CTexture> pStaticColorChart) { AUTO_LOCK_T(DrxCriticalSectionNonRecursive, m_lock); std::swap(m_pChartStatic, pStaticColorChart); }
	CTexture* GetStaticColorChart()   const { AUTO_LOCK_T(DrxCriticalSectionNonRecursive, m_lock); return m_pChartStatic; }

	CTexture* GetColorChart()         const { return m_pChartToUse; }
	CTexture* GetIdentityColorChart() const { return m_pChartIdentity; }

	// TODO: remove once graphicspipeline=0 mode is not required anymore in D3DColorGradingController
	const std::array<_smart_ptr<CTexture>, 2>& GetMergeLayers() const { return m_pMergeLayers;  }
	CVertexBuffer                              GetSlicesVB()    const;

	bool IsRenderPassesDirty() ;

private:
	void PreparePrimitives(CColorGradingController& controller, const SColorGradingMergeParams& mergeParams);

	DrxCriticalSectionNonRecursive      m_lock;
	
	_smart_ptr<CTexture>                m_pChartIdentity;
	_smart_ptr<CTexture>                m_pChartStatic;
	_smart_ptr<CTexture>                m_pChartToUse;
	std::array<_smart_ptr<CTexture>, 2> m_pMergeLayers;
	buffer_handle_t                     m_slicesVertexBuffer;

	std::vector<CRenderPrimitive>       m_mergeChartsPrimitives;
	CRenderPrimitive                    m_colorGradingPrimitive;

	CPrimitiveRenderPass                m_mergePass;
	CPrimitiveRenderPass                m_colorGradingPass;

	// TODO: remove once r_graphicspipeline=0 is gone
	VertexArray                         m_vecSlicesData;
};
