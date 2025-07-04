// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: handle failure of playback of events
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __NULLSTREAM_H__
#define __NULLSTREAM_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "IBreakPlaybackStream.h"

class CNullStream : public IBreakPlaybackStream
{
	virtual bool                        AttemptAbsorb(const IProceduralBreakTypePtr& pBT)                       { return false; }
	virtual bool                        GotExplosiveObjectState(const SExplosiveObjectState* state)             { return false; }
	virtual bool                        GotProceduralSpawnRec(const SProceduralSpawnRec* rec)                   { return false; }
	virtual bool                        GotJointBreakRec(const SJointBreakRec* rec)                             { return false; }
	virtual bool                        GotJointBreakParticleRec(const SJointBreakParticleRec* rec)             { return false; }
	virtual IBreakReplicatorListenerPtr Playback(CBreakReplicator* pReplicator, INetBreakagePlaybackPtr pBreak) { return 0; }
	virtual bool                        SetMagicId(i32 id)                                                      { return false; }
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
