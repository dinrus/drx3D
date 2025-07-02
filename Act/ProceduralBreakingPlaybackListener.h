// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: playback of procedural breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __PROCEDURALBREAKPLAYBACKLISTENER_H__
#define __PROCEDURALBREAKPLAYBACKLISTENER_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "ProceduralBreakingBaseListener.h"
	#include "IBreakPlaybackStream.h"

class CProceduralBreakingPlaybackStream;
class CDumpOnExitRoutine;

class CProceduralBreakingPlaybackListener : public CProceduralBreakingBaseListener
{
	friend class CDumpOnExitRoutine;

public:
	CProceduralBreakingPlaybackListener(_smart_ptr<CProceduralBreakingPlaybackStream> pStream, INetBreakagePlaybackPtr pBreakage);

	void Start();

	bool AttemptAbsorb(const IProceduralBreakTypePtr& pBT)
	{
		return false;
	}

private:
	void                CompleteStepFrom(DynArray<SProceduralSpawnRec>::iterator it);

	virtual void        GotJointBreak(i32 idxRef, i32 id, i32 epicenter) {}
	virtual void        GotJointBreakParticle(const EventPhysCreateEntityPart* pEvent);
	virtual i32         GotBreakOp(ENetBreakOperation op, i32 idxRef, i32 partid, EntityId to);
	virtual void        FinishBreakOp(ENetBreakOperation op, i32 idxRef, EntityId to, i32 virtId);
	virtual void        Complete();
	virtual bool        AllowComplete();
	virtual tukk GetListenerName() { return "Playback"; }

	void                UpdateJointBreaks();

	#if BREAK_HIERARCHICAL_TRACKING
	u16        m_playbackFrame;
	std::set<i32> m_createdObjects;
	void IncrementFrame();
	#endif
	INetBreakagePlaybackPtr                       m_pBreakage;
	_smart_ptr<CProceduralBreakingPlaybackStream> m_pStream;
	i32 m_gibIdx;
};

class CDumpOnExitRoutine
{
public:
	CDumpOnExitRoutine(CProceduralBreakingPlaybackListener* pL, tukk desc) : m_pL(pL), m_desc(desc) {}
	~CDumpOnExitRoutine();
private:
	tukk                          m_desc;
	CProceduralBreakingPlaybackListener* m_pL;
};

class CProceduralBreakingPlaybackStream : public IBreakPlaybackStream
{
	friend class CProceduralBreakingPlaybackListener;
	friend class CDumpOnExitRoutine;

public:
	CProceduralBreakingPlaybackStream(IProceduralBreakTypePtr pBreakType);
	~CProceduralBreakingPlaybackStream();

	bool                                AttemptAbsorb(const IProceduralBreakTypePtr& pBT);

	virtual bool                        GotExplosiveObjectState(const SExplosiveObjectState* state);
	virtual bool                        GotProceduralSpawnRec(const SProceduralSpawnRec* rec);
	virtual bool                        GotJointBreakRec(const SJointBreakRec* rec);
	virtual bool                        GotJointBreakParticleRec(const SJointBreakParticleRec* rec);
	virtual bool                        SetMagicId(i32 id);

	virtual IBreakReplicatorListenerPtr Playback(CBreakReplicator* pReplicator, INetBreakagePlaybackPtr pBreak);

private:
	IProceduralBreakTypePtr          m_pBreakType;
	DynArray<SJointBreakRec>         m_jointBreaks;
	DynArray<SProceduralSpawnRec>    m_spawnRecs;
	DynArray<SJointBreakParticleRec> m_gibs;                  // An attempt to sync the initial particle velocities
	i32                              m_magicId;
};

inline CDumpOnExitRoutine::~CDumpOnExitRoutine() { DumpSpawnRecs(m_pL->m_pStream->m_spawnRecs, m_pL->m_pStream->m_jointBreaks, m_desc); }

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
