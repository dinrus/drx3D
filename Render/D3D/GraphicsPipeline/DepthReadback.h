// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CDepthReadbackStage : public CGraphicsPipelineStage
{
public:
	~CDepthReadbackStage() override
	{
		ReleaseResources();
	}

	void Init() override;
	void Execute();

	// See IRenderer::PinOcclusionBuffer
	float* PinOcclusionBuffer(Matrix44A& camera) threadsafe
	{
		TAutoLock lock(m_lock);
		SResult* pResult;
		if (m_lastPinned < kMaxResults)
		{
			DRX_ASSERT(m_numPinned != 0);
			pResult = m_result + m_lastPinned;
		}
		else if (m_lastResult < kMaxResults)
		{
			DRX_ASSERT(m_numPinned == 0);
			pResult = m_result + m_lastResult;
			m_lastPinned = m_lastResult;
		}
		else
		{
			return nullptr;
		}

		++m_numPinned;
		camera = pResult->camera;
		return pResult->data;
	}

	void UnpinOcclusionBuffer() threadsafe
	{
		TAutoLock lock(m_lock);
		DRX_ASSERT(m_lastPinned < kMaxResults && m_numPinned != 0);
		if (!--m_numPinned)
		{
			m_lastPinned = kMaxResults;
		}
	}

private:
	enum EConfigurationFlags : u8
	{
		kNone         = 0,
		kMultiRes     = BIT(0),
		kReverseDepth = BIT(1),
		kNativeDepth  = BIT(2),
		kMSAA         = BIT(3),
	};

	struct SReadback
	{
		Matrix44A           camera;
		CFullscreenPass     pass;
		float               zNear;
		float               zFar;
		EConfigurationFlags flags;

		bool                bIssued    : 1;
		bool                bCompleted : 1;
		bool                bReceived  : 1;
	};

	struct SResult
	{
		float     data[CULL_SIZEX * CULL_SIZEY];
		Matrix44A camera;
	};

	using TLock = DrxCriticalSectionNonRecursive;
	using TAutoLock = DrxAutoCriticalSectionNoRecursive;

	bool      IsReadbackRequired();
	CTexture* GetInputTexture(EConfigurationFlags& flags);
	bool      CreateResources(u32 sourceWidth, u32 sourceHeight);
	void      ReleaseResources();
	void      ConfigurePasses(CTexture* pSource, EConfigurationFlags flags);
	void      ExecutePasses(float sourceWidth, float sourceHeight, float sampledWidth, float sampledHeight);
	void      ReadbackLatestData();

	static u32k kMaxDownsamplePasses = DRX_ARRAY_COUNT(CRendererResources::s_ptexLinearDepthDownSample);
	static u32k kMaxReadbackPasses = DRX_ARRAY_COUNT(CRendererResources::s_ptexLinearDepthReadBack);
	static u32k kMaxResults = 2;

	const ICVar*        m_pCheckOcclusion = nullptr;       // e_CheckOcclusion
	const ICVar*        m_pStatObjRenderTasks = nullptr;   // e_StatObjBufferRenderTasks
	const ICVar*        m_pCoverageBufferReproj = nullptr; // e_CoverageBufferReproj

	u32              m_resourceWidth = 0;
	u32              m_resourceHeight = 0;
	i32               m_sourceTexId = 0;
	EConfigurationFlags m_resourceFlags;

	u32              m_downsamplePassCount = 0;
	u32              m_readbackIndex = 0;

	CFullscreenPass     m_downsamplePass[kMaxDownsamplePasses];
	SReadback           m_readback[kMaxReadbackPasses];

	TLock               m_lock;
	u32              m_lastResult = kMaxResults;
	u32              m_lastPinned = kMaxResults;
	u32              m_numPinned = 0;
	SResult             m_result[kMaxResults];
};
