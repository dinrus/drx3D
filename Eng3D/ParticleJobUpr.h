// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/ParticleCommon.h>

class CRenderObject;

namespace pfx2
{

class CParticleEmitter;
class CParticleComponentRuntime;
struct SRenderContext;

// Schedules particle update and render jobs
class CParticleJobUpr
{
public:
	struct SDeferredRender
	{
		SDeferredRender(CParticleEmitter* pEmitter, const SRenderContext& renderContext)
			: m_pEmitter(pEmitter)
			, m_rParam(renderContext.m_renderParams)
			, m_passInfo(renderContext.m_passInfo)
			, m_distance(renderContext.m_distance)
			, m_lightVolumeId(renderContext.m_lightVolumeId)
			, m_fogVolumeId(renderContext.m_fogVolumeId) {}
		CParticleEmitter*  m_pEmitter;
		SRendParams        m_rParam;
		SRenderingPassInfo m_passInfo;
		float              m_distance;
		u16             m_lightVolumeId;
		u16             m_fogVolumeId;
	};

public:
	CParticleJobUpr();
	void AddUpdateEmitter(CParticleEmitter* pEmitter);
	void ScheduleUpdateEmitter(CParticleEmitter* pEmitter);
	void AddDeferredRender(CParticleEmitter* pEmitter, const SRenderContext& renderContext);
	void ScheduleComputeVertices(CParticleComponentRuntime& runtime, CRenderObject* pRenderObject, const SRenderContext& renderContext);
	void ScheduleUpdates();
	void SynchronizeUpdates();
	void DeferredRender();

private:

	void Job_ScheduleUpdates();
	void ScheduleUpdateEmitters(TDynArray<CParticleEmitter*>& emitters, JobUpr::TPriorityLevel priority);

	TDynArray<CParticleEmitter*> m_emittersDeferred;
	TDynArray<CParticleEmitter*> m_emittersVisible;
	TDynArray<CParticleEmitter*> m_emittersInvisible;
	TDynArray<SDeferredRender>   m_deferredRenders;
	JobUpr::SJobState        m_updateState;
};

}

