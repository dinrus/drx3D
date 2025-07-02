// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/IIndexedMesh.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Phys/IDeferredCollisionEvent.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Entity/IBreakableUpr.h>

class CDelayedPlaneBreak : public IDeferredPhysicsEvent
{
public:
	enum EStatus
	{
		eStatus_NONE,
		eStatus_STARTED,
		eStatus_DONE,
	};

	CDelayedPlaneBreak()
		: m_status(eStatus_NONE)
		, m_count(0)
		, m_iBreakEvent(0)
		, m_idx(0)
		, m_bMeshPrepOnly(false)
		, m_jobState()
	{
	}

	CDelayedPlaneBreak(const CDelayedPlaneBreak& src)
		: m_status(src.m_status)
		, m_islandIn(src.m_islandIn)
		, m_islandOut(src.m_islandOut)
		, m_iBreakEvent(src.m_iBreakEvent)
		, m_idx(src.m_idx)
		, m_count(src.m_count)
		, m_bMeshPrepOnly(src.m_bMeshPrepOnly)
		, m_epc(src.m_epc)
	{
	}

	CDelayedPlaneBreak& operator=(const CDelayedPlaneBreak& src)
	{
		m_status = src.m_status;
		m_islandIn = src.m_islandIn;
		m_islandOut = src.m_islandOut;
		m_iBreakEvent = src.m_iBreakEvent;
		m_idx = src.m_idx;
		m_count = src.m_count;
		m_bMeshPrepOnly = src.m_bMeshPrepOnly;
		m_epc = src.m_epc;
		return *this;
	}

	virtual void              Start()            {}
	virtual void              ExecuteAsJob();
	virtual i32               Result(EventPhys*) { return 0; }
	virtual void              Sync()             { gEnv->GetJobUpr()->WaitForJob(m_jobState); }
	virtual bool              HasFinished()      { return true; }
	virtual DeferredEventType GetType() const    { return PhysCallBack_OnCollision; }
	virtual EventPhys*        PhysicsEvent()     { return &m_epc; }

	virtual void              Execute();

	 i32          m_status;
	SExtractMeshIslandIn  m_islandIn;
	SExtractMeshIslandOut m_islandOut;
	i32                   m_iBreakEvent;
	i32                   m_idx;
	i32                   m_count;

	bool                  m_bMeshPrepOnly;

	EventPhysCollision    m_epc;
	JobUpr::SJobState m_jobState;
};
