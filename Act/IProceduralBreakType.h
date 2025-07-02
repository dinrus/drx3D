// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: handle different methods of procedurally breaking something
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __IPROCEDURALBREAKTYPE_H__
#define __IPROCEDURALBREAKTYPE_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "ProceduralBreak.h"
	#include "ObjectSelector.h"

struct SExplosiveObjectState;

enum EProceduralBreakTypeFlags
{
	ePBTF_NoJointBreaks = BIT(0),
	ePBTF_ChainBreaking = BIT(1)
};

struct IProceduralBreakType;
typedef _smart_ptr<IProceduralBreakType> IProceduralBreakTypePtr;

struct SProceduralBreakRecordingState
{
	SProceduralBreakRecordingState() : gotRemove(false), numEmptySteps(0) {}
	bool gotRemove;
	i32  numEmptySteps;
};

struct IProceduralBreakType : public _reference_target_t
{
	u8k flags;
	const char  type;

	IProceduralBreakType(u8 f, char t) : flags(f), type(t) {}

	virtual bool                         AttemptAbsorb(const IProceduralBreakTypePtr& pBT) = 0;
	virtual void                         AbsorbStep() = 0;
	virtual i32                          GetVirtualId(IPhysicalEntity* pEnt) = 0;
	virtual bool                         GotExplosiveObjectState(const SExplosiveObjectState* pState) = 0;
	virtual _smart_ptr<SProceduralBreak> CompleteSend() = 0;
	virtual void                         PreparePlayback() = 0;
	virtual void                         BeginPlayback(bool hasJointBreaks) = 0;
	virtual CObjectSelector              GetObjectSelector(i32 idx) = 0;
	virtual tukk                  GetName() = 0;

	virtual void                         PatchRecording(DynArray<SProceduralSpawnRec>& spawnRecs, DynArray<SJointBreakRec>& jointBreaks) = 0;

	virtual bool                         AllowComplete(const SProceduralBreakRecordingState& state) = 0;

	virtual bool                         SendOnlyOnClientJoin() { return false; }
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
