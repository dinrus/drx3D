// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/ActionGame.h>
#include <drx3D/Act/DelayedPlaneBreak.h>
#include <drx3D/CoreX/ParticleSys/ParticleParams.h>
#include <drx3D/Entity/IBreakableUpr.h>

#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

DECLARE_JOB("DelayedPlaneBreak", TDelayedPlaneBreak, CDelayedPlaneBreak::Execute);

void CDelayedPlaneBreak::ExecuteAsJob()
{
	TDelayedPlaneBreak job;
	job.SetClassInstance(this);
	job.RegisterJobState(&m_jobState);
	job.SetPriorityLevel(JobUpr::eRegularPriority);
	job.SetBlocking();
	job.Run();
}
void CDelayedPlaneBreak::Execute()
{
	m_islandIn.bCreateIsle = (m_islandIn.processFlags & ePlaneBreak_AutoSmash) == 0;
	if (!m_bMeshPrepOnly)
		gEnv->pEntitySystem->GetBreakableUpr()->ExtractPlaneMeshIsland(m_islandIn, m_islandOut);
	else
	{
		m_islandOut.pStatObj = m_islandIn.pStatObj;
		m_islandOut.pStatObj->GetIndexedMesh(true);
	}

	// cppcheck-suppress thisSubtraction
	CDelayedPlaneBreak* pdpb = this - m_idx;
	if (!(m_islandIn.pStatObj->GetFlags() & STATIC_OBJECT_CLONE) &&
	    (m_islandOut.pStatObj->GetFlags() & STATIC_OBJECT_CLONE))
		for (i32 i = 0; i < m_count; ++i)
			if (pdpb[i].m_status != eStatus_NONE &&
			    i != m_idx &&
			    pdpb[i].m_islandOut.pStatObj == m_islandIn.pStatObj &&
			    pdpb[i].m_epc.pEntity[1] == m_epc.pEntity[1] &&
			    pdpb[i].m_epc.partid[1] == m_epc.partid[1])
			{
				IStatObj* oldSrc = pdpb[i].m_islandIn.pStatObj;
				pdpb[i].m_islandIn.pStatObj = m_islandOut.pStatObj;
				pdpb[i].m_islandOut.pStatObj = m_islandOut.pStatObj;
				m_islandOut.pStatObj->AddRef();
				oldSrc->Release();
			}
	m_status = eStatus_DONE;
}
