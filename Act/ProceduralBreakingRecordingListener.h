// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: recording of procedural breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __PROCEDURALBREAKRECORDINGLISTENER_H__
#define __PROCEDURALBREAKRECORDINGLISTENER_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "ProceduralBreakingBaseListener.h"

class CProceduralBreakingRecordingListener : public CProceduralBreakingBaseListener
{
public:
	CProceduralBreakingRecordingListener(IProceduralBreakTypePtr pBreakType);
	~CProceduralBreakingRecordingListener();

	bool AttemptAbsorb(const IProceduralBreakTypePtr& pBT);

private:
	virtual void        GotJointBreak(i32 idxRef, i32 id, i32 epicenter);
	virtual void        GotJointBreakParticle(const EventPhysCreateEntityPart* pEvent);
	virtual i32         GotBreakOp(ENetBreakOperation op, i32 idxRef, i32 partid, EntityId to);
	virtual void        FinishBreakOp(ENetBreakOperation op, i32 idxRef, EntityId to, i32 virtId);
	virtual void        Complete();
	i32                 GetEntRef(EntityId id);
	virtual bool        AllowComplete();
	virtual tukk GetListenerName() { return "Recording"; }

	DynArray<EntityId>               m_ents;
	DynArray<SJointBreakRec>         m_jointBreaks;
	DynArray<SProceduralSpawnRec>    m_spawnRecs;
	DynArray<SJointBreakParticleRec> m_gibs;                  // An unclean attempt to sync the initial particle velocities
	std::map<i32, i32>               m_jointBreaksForIds;
	#if BREAK_HIERARCHICAL_TRACKING
	u16                           m_frame;
	std::set<i32>                    m_openIds;
	#else
	bool                             m_gotRemove;
	#endif

	i32 m_orderId;
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
