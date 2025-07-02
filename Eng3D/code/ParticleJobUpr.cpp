// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ParticleJobUpr.h>
#include <drx3D/Eng3D/ParticleUpr.h>
#include <drx3D/Eng3D/ParticleEmitter.h>
#include <drx3D/Eng3D/ParticleComponentRuntime.h>
#include <drx3D/Eng3D/ParticleSystem.h>
#include <drx3D/Eng3D/ParticleProfiler.h>
#include <drx3D/CoreX/Renderer/IGpuParticles.h>

namespace pfx2
{

i32 e_ParticlesJobsPerThread = 16;

ILINE bool EmitterHasDeferred(CParticleEmitter* pEmitter)
{
	return ThreadMode() >= 4 && !pEmitter->GetCEffect()->RenderDeferred.empty();
}

CParticleJobUpr::CParticleJobUpr()
{
	REGISTER_CVAR(e_ParticlesJobsPerThread, 16, 0, "Maximum particle jobs to assign per worker thread");
}

void CParticleJobUpr::AddDeferredRender(CParticleEmitter* pEmitter, const SRenderContext& renderContext)
{
	SDeferredRender render(pEmitter, renderContext);
	m_deferredRenders.push_back(render);
}

void CParticleJobUpr::ScheduleComputeVertices(CParticleComponentRuntime& runtime, CRenderObject* pRenderObject, const SRenderContext& renderContext)
{
	CParticleUpr* pPartUpr = static_cast<CParticleUpr*>(gEnv->pParticleUpr);

	SAddParticlesToSceneJob& job = pPartUpr->GetParticlesToSceneJob(renderContext.m_passInfo);
	if (auto pGpuRuntime = runtime.GetGpuRuntime())
		job.pGpuRuntime = pGpuRuntime;
	else
		job.pVertexCreator = &runtime;
	job.pRenderObject = pRenderObject;
	job.pShaderItem = &pRenderObject->m_pCurrMaterial->GetShaderItem();
	job.nCustomTexId = renderContext.m_renderParams.nTextureID;
	job.aabb = runtime.GetBounds();
}

void CParticleJobUpr::AddUpdateEmitter(CParticleEmitter* pEmitter)
{
	DRX_PFX2_ASSERT(!m_updateState.IsRunning());
	if (ThreadMode() == 0)
	{
		// Update synchronously in main thread
		pEmitter->UpdateParticles();
	}
	else
	{
		// Schedule emitters rendered last frame first
		if (ThreadMode() >= 2 && pEmitter->WasRenderedLastFrame())
		{
			if (EmitterHasDeferred(pEmitter))
				m_emittersDeferred.push_back(pEmitter);
			else
				m_emittersVisible.push_back(pEmitter);
		}
		else if (ThreadMode() < 3 || !pEmitter->IsStable())
			m_emittersInvisible.push_back(pEmitter);
	}
}

void CParticleJobUpr::ScheduleUpdateEmitter(CParticleEmitter* pEmitter)
{
	auto job = [pEmitter]()
	{
		pEmitter->UpdateParticles();
	};
	auto priority = EmitterHasDeferred(pEmitter) ? JobUpr::eHighPriority : JobUpr::eRegularPriority;
	gEnv->pJobUpr->AddLambdaJob("job:pfx2:UpdateEmitter", job, priority, &m_updateState);
}

void CParticleJobUpr::ScheduleUpdates()
{
	// Schedule jobs in a high-priority job
	DRX_PFX2_ASSERT(!m_updateState.IsRunning());
	auto job = [this]()
	{
		Job_ScheduleUpdates();
	};
	gEnv->pJobUpr->AddLambdaJob("job:pfx2:ScheduleUpdates", job, JobUpr::eHighPriority, &m_updateState);
}

void CParticleJobUpr::Job_ScheduleUpdates()
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);

	// Schedule deferred emitters in individual high-priority jobs
	for (auto pEmitter : m_emittersDeferred)
		ScheduleUpdateEmitter(pEmitter);
	m_emittersDeferred.resize(0);

	if (m_emittersVisible.size())
	{
		// Sort fast (visible) emitters by camera Z
		const CCamera& camera = gEnv->p3DEngine->GetRenderingCamera();
		Vec3 sortDir = -camera.GetViewdir();
		stl::sort(m_emittersVisible, [sortDir](const CParticleEmitter* pe)
		{
			return pe->GetLocation().t | sortDir;
		});
		ScheduleUpdateEmitters(m_emittersVisible, JobUpr::eRegularPriority);
	}
	ScheduleUpdateEmitters(m_emittersInvisible, JobUpr::eStreamPriority);
}

void CParticleJobUpr::ScheduleUpdateEmitters(TDynArray<CParticleEmitter*>& emitters, JobUpr::TPriorityLevel priority)
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);

	// Batch updates into jobs
	const uint maxJobs = gEnv->pJobUpr->GetNumWorkerThreads() * e_ParticlesJobsPerThread;
	const uint numJobs = min(emitters.size(), maxJobs);

	uint e = 0;
	for (uint j = 0; j < numJobs; ++j)
	{
		uint e2 = (j + 1) * emitters.size() / numJobs;
		auto emitterGroup = emitters(e, e2 - e);
		e = e2;

		auto job = [emitterGroup]()
		{
			for (auto pEmitter : emitterGroup)
				pEmitter->UpdateParticles();
		};
		gEnv->pJobUpr->AddLambdaJob("job:pfx2:UpdateEmitters", job, priority, &m_updateState);
	}
	DRX_PFX2_ASSERT(e == emitters.size());
}

void CParticleJobUpr::SynchronizeUpdates()
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);
	m_updateState.Wait();
	m_emittersInvisible.resize(0);
	m_emittersVisible.resize(0);
}

void CParticleJobUpr::DeferredRender()
{
	DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);

	for (const SDeferredRender& render : m_deferredRenders)
	{
		SRenderContext renderContext(render.m_rParam, render.m_passInfo);
		renderContext.m_distance = render.m_distance;
		renderContext.m_lightVolumeId = render.m_lightVolumeId;
		renderContext.m_fogVolumeId = render.m_fogVolumeId;
		render.m_pEmitter->RenderDeferred(renderContext);
	}
	m_deferredRenders.clear();
}


}
