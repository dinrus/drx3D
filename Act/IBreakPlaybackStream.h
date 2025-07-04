// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: handle playback of events
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __IBREAKPLAYBACKSTREAM_H__
#define __IBREAKPLAYBACKSTREAM_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

class CBreakReplicator;
struct SExplosiveObjectState;
struct SProceduralSpawnRec;
struct SJointBreakRec;
struct SJointBreakParticleRec;

struct IBreakPlaybackStream : public _reference_target_t
{
	virtual bool                        AttemptAbsorb(const IProceduralBreakTypePtr& pBT) = 0;

	virtual bool                        GotExplosiveObjectState(const SExplosiveObjectState* state) = 0;
	virtual bool                        GotProceduralSpawnRec(const SProceduralSpawnRec* rec) = 0;
	virtual bool                        GotJointBreakRec(const SJointBreakRec* rec) = 0;
	virtual bool                        GotJointBreakParticleRec(const SJointBreakParticleRec* rec) = 0;
	virtual bool                        SetMagicId(i32 id) = 0;
	virtual IBreakReplicatorListenerPtr Playback(CBreakReplicator* pReplicator, INetBreakagePlaybackPtr pBreak) = 0;
};
typedef _smart_ptr<IBreakPlaybackStream> IBreakPlaybackStreamPtr;

#endif // ! NET_USE_SIMPLE_BREAKAGE

#endif
