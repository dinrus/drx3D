// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/GpuParticleUpr.h>
#include <drx3D/Render/GpuParticleFeatureFactory.h>
#include <drx3D/Render/GpuParticleComponentRuntime.h>

#include <algorithm>

namespace gpu_pfx2
{

static i32k kMaxRuntimes = 4096;

CUpr::CUpr()
	: m_readback(kMaxRuntimes)
	, m_counter(kMaxRuntimes)
	, m_scratch(kMaxRuntimes)
	, m_numRuntimesReadback(0)
{
}

IParticleComponentRuntime* CUpr::CreateParticleContainer(const SComponentParams& params, TConstArray<IParticleFeature*> features)
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);
	DrxAutoLock<DrxCriticalSection> lock(m_cs);

	auto* pRuntime = new CParticleComponentRuntime(params, features);
	GetWriteRuntimes().push_back(pRuntime);
	return pRuntime;
}

void CUpr::RenderThreadUpdate(CRenderView* pRenderView)
{
	const bool bAsynchronousCompute = CRenderer::CV_r_D3D12AsynchronousCompute & BIT((eStage_ComputeParticles - eStage_FIRST_ASYNC_COMPUTE)) ? true : false;
	const bool bReadbackBoundingBox = CRenderer::CV_r_GpuParticlesConstantRadiusBoundingBoxes ? false : true;

	if (!CRenderer::CV_r_GpuParticles)
		return;

	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);
	PROFILE_LABEL_SCOPE("GPU_PARTICLES");

	if (!m_readback.IsDeviceBufferAllocated())
	{
		m_readback.CreateDeviceBuffer();
		m_counter.CreateDeviceBuffer();
		m_scratch.CreateDeviceBuffer();

		{
			// Full clear
			const ColorI nulls = { 0, 0, 0, 0 };

			CClearSurfacePass::Execute(&m_counter.GetBuffer(), nulls);
			CClearSurfacePass::Execute(&m_scratch.GetBuffer(), nulls);
		}

		// initialize readback staging buffer
		m_counter.Readback(kMaxRuntimes);
		m_readback.Readback(kMaxRuntimes);
	}

	// only at this point, GPU resources are actually freed.
	ProcessResources();

	if (u32 numRuntimes = u32(GetReadRuntimes().size()))
	{
		gpu_pfx2::SUpdateContext context;
		context.pRenderView = pRenderView;
		context.pCounterBuffer = &m_counter.GetBuffer();
		context.pScratchBuffer = &m_scratch.GetBuffer();
		context.pReadbackBuffer = &m_readback.GetBuffer();
		context.deltaTime = gEnv->pTimer->GetFrameTime();

		{
			for (auto& pRuntime : GetReadRuntimes())
			{
				pRuntime->Initialize();
			}
		}

		{
			PROFILE_LABEL_SCOPE("ADD REMOVE NEWBORNS & UPDATE");

			const uint* pCounter = m_counter.Map(m_numRuntimesReadback);
			const SReadbackData* pData = nullptr;

			if (bReadbackBoundingBox)
				pData = m_readback.Map(m_numRuntimesReadback);

			for (u32 i = 0; i < numRuntimes; ++i)
			{
				// TODO: convert to array of command-lists pattern
				// TODO: profile single command list vs. multiple command lists
				SScopedComputeCommandList pComputeInterface(bAsynchronousCompute);

				auto& pRuntime = GetReadRuntimes()[i];
				context.pRuntime = pRuntime;

				if (pData)
					pRuntime->SetBoundsFromUpr(pData);

				pRuntime->SetCounterFromUpr(pCounter);
				pRuntime->SetUprSlot(i);
				pRuntime->AddRemoveParticles(context, pComputeInterface);
				pRuntime->UpdateParticles(context, pComputeInterface);
				pRuntime->CalculateBounds(context, pComputeInterface);
			}

			m_counter.Unmap();
			if (pData)
				m_readback.Unmap();
		}

		{
			PROFILE_LABEL_SCOPE("READBACK");

			m_counter.Readback(numRuntimes);
			if (bReadbackBoundingBox)
				m_readback.Readback(numRuntimes);
			m_numRuntimesReadback = numRuntimes;
		}
	}
}

void CUpr::RenderThreadPreUpdate(CRenderView* pRenderView)
{
	if (u32 numRuntimes = u32(GetReadRuntimes().size()))
	{
		std::vector<CGpuBuffer*> UAVs;

		UAVs.reserve(numRuntimes);
		for (auto& pRuntime : GetReadRuntimes())
			UAVs.emplace_back(&pRuntime->PrepareForUse());

		// Prepare particle buffers which have been used in the compute shader for vertex use
		GetDeviceObjectFactory().GetCoreCommandList().GetGraphicsInterface()->PrepareUAVsForUse(numRuntimes, &UAVs[0], false);
	}
}

void CUpr::RenderThreadPostUpdate(CRenderView* pRenderView)
{
	if (u32 numRuntimes = u32(GetReadRuntimes().size()))
	{
		std::vector<CGpuBuffer*> UAVs;

		UAVs.reserve(numRuntimes);
		for (auto& pRuntime : GetReadRuntimes())
			UAVs.emplace_back(&pRuntime->PrepareForUse());

		// Prepare particle buffers which have been used in the vertex shader for compute use
		GetDeviceObjectFactory().GetCoreCommandList().GetGraphicsInterface()->PrepareUAVsForUse(numRuntimes, &UAVs[0], true);

		{
			// Minimal clear
			const ColorI nulls = { 0, 0, 0, 0 };

#if (DRX_RENDERER_DIRECT3D >= 111)
			const UINT numRanges = 1;
			const D3D11_RECT uavRange = { 0, 0, numRuntimes, 0 };

			gcpRendD3D->GetGraphicsPipeline().GetOrCreateUtilityPass<CClearRegionPass>()->Execute(&m_counter.GetBuffer(), nulls, numRanges, &uavRange);
			gcpRendD3D->GetGraphicsPipeline().GetOrCreateUtilityPass<CClearRegionPass>()->Execute(&m_scratch.GetBuffer(), nulls, numRanges, &uavRange);
#else
			CClearSurfacePass::Execute(&m_counter.GetBuffer(), nulls);
			CClearSurfacePass::Execute(&m_scratch.GetBuffer(), nulls);
#endif
		}
	}
}

gpu::CBitonicSort* CUpr::GetBitonicSort()
{
	if (!m_pBitonicSort)
		m_pBitonicSort = std::unique_ptr<gpu::CBitonicSort>(new gpu::CBitonicSort());
	return m_pBitonicSort.get();
}

void CUpr::ProcessResources()
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);
	DrxAutoLock<DrxCriticalSection> lock(m_cs);

	PROFILE_LABEL_SCOPE("PROCESS RESOURCES");

	GetReadRuntimes() = GetWriteRuntimes();
	GetWriteRuntimes().erase(
	  std::remove_if(
	    GetWriteRuntimes().begin(),
	    GetWriteRuntimes().end(),
	    [](const _smart_ptr<CParticleComponentRuntime>& runtime)
		{
			return runtime->UseCount() <= 2;
	  }),
	  GetWriteRuntimes().end());

	m_particleFeatureGpuInterfaces.erase(
	  std::remove_if(
	    m_particleFeatureGpuInterfaces.begin(),
	    m_particleFeatureGpuInterfaces.end(),
	    [](const _smart_ptr<CFeature>& feature)
		{
			return feature->Unique();
	  }),
	  m_particleFeatureGpuInterfaces.end());

	// initialize needed constant buffers and SRVs, upload data to gpu
	for (auto& feature : m_particleFeatureGpuInterfacesInitialization)
	{
		feature->Initialize();
	}

	m_particleFeatureGpuInterfacesInitialization.resize(0);
}

IParticleFeature* CUpr::CreateParticleFeature(EGpuFeatureType feature)
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);
	DrxAutoLock<DrxCriticalSection> lock(m_cs);

	CFeature* result = m_gpuInterfaceFactory.CreateInstance(feature);
	if (result)
	{
		m_particleFeatureGpuInterfaces.push_back(result);
		m_particleFeatureGpuInterfacesInitialization.push_back(result);
	}
	return result;
}

void CUpr::CleanupResources()
{
	GetWriteRuntimes().clear();
	ProcessResources();

	if (m_readback.IsDeviceBufferAllocated())
	{
		m_readback.FreeDeviceBuffer();
		m_counter.FreeDeviceBuffer();
	}
		
	if (GetWriteRuntimes().size() > 0)
		DrxFatalError("There are still GPU runtimes living past the render thread.");
}

}
