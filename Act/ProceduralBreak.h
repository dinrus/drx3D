// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: basic information for procedural breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __PROCEDURALBREAK_H__
#define __PROCEDURALBREAK_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "DebugBreakage.h"

	#define BREAK_HIERARCHICAL_TRACKING 1

enum ENetBreakOperation
{
	eNBO_Create = 0, // must be first
	eNBO_Create_NoSpawn,
	eNBO_Update,
	eNBO_Remove,
	eNBO_Remove_NoSpawn,
	eNBO_JointBreak,
	// absorbed a breaktype, step
	eNBO_AbsorbStep,
	#if BREAK_HIERARCHICAL_TRACKING
	// performed a post step, flush relevant joint breaks
	eNBO_PostStep,
	// very special op - once we reach it, we skip over it, but bump the frame count
	eNBO_FrameCounterFinished,
	#endif
	eNBO_NUM_OPERATIONS, // must be last
};

static u32k OPS_REFERENCING_ENTS     = BIT(eNBO_Create) | BIT(eNBO_Create_NoSpawn) | BIT(eNBO_Update) | BIT(eNBO_Remove) | BIT(eNBO_Remove_NoSpawn) | BIT(eNBO_JointBreak);
static u32k OPS_CAUSING_ENTS         = BIT(eNBO_Create) | BIT(eNBO_Update) | BIT(eNBO_Remove) | BIT(eNBO_JointBreak);
static u32k OPS_WITH_PARTIDS         = BIT(eNBO_Create) | BIT(eNBO_Create_NoSpawn);
static u32k OPS_WITH_NOSPAWN         = BIT(eNBO_Remove);
	#if BREAK_HIERARCHICAL_TRACKING
static u32k OPS_JOINT_FRAME_COUNTERS = BIT(eNBO_PostStep) | BIT(eNBO_Remove) | BIT(eNBO_Remove_NoSpawn);
static u32k OPS_ADD_OPEN             = BIT(eNBO_Create) | BIT(eNBO_JointBreak);
static u32k OPS_REMOVE_OPEN          = BIT(eNBO_Remove) | BIT(eNBO_Remove_NoSpawn);
	#endif

struct SProceduralSpawnRec
{
	SProceduralSpawnRec()
		: op(eNBO_NUM_OPERATIONS)
		, idxRef(0)
		, idx(0)
		, partid(0)
	{}

	ENetBreakOperation op;
	i32                idxRef;
	i32                idx;
	i32                partid;

	void               SerializeWith(TSerialize ser);
};

struct SJointBreakRec
{
	i32    idxRef;
	i32    id;
	#if BREAK_HIERARCHICAL_TRACKING
	u16 frame;
	#endif
	i32    epicenter;

	void   SerializeWith(TSerialize ser);
};

struct SJointBreakParticleRec
{
	Vec3 vel;
	void SerializeWith(TSerialize ser);
};

template<class T>
struct SProceduralStreamWrapper : public T
{
	SProceduralStreamWrapper() : breakId(-1) {}
	SProceduralStreamWrapper(i32 bid, const T& rec) : T(rec), breakId(bid) {}
	i32 breakId;

	void SerializeWith(TSerialize ser)
	{
		LOGBREAK("SProceduralStreamWrapper, %s", ser.IsReading() ? "Reading:" : "Writing");
		ser.Value("breakId", breakId, 'brId');
		LOGBREAK("breakId: %d", breakId);
		T::SerializeWith(ser);
	}
};

typedef SProceduralStreamWrapper<SProceduralSpawnRec>    SDeclareProceduralSpawnRec;
typedef SProceduralStreamWrapper<SJointBreakRec>         SDeclareJointBreakRec;
typedef SProceduralStreamWrapper<SJointBreakParticleRec> SDeclareJointBreakParticleRec;

struct SProceduralBreak : public IBreakDescriptionInfo
{
	SProceduralBreak() : magicId(0) {}
	i32                              magicId;
	DynArray<SJointBreakRec>         jointBreaks;
	DynArray<SProceduralSpawnRec>    spawnRecs;
	DynArray<SJointBreakParticleRec> gibs;
	void AddProceduralSendables(i32 breakId, INetSendableSink* pSink);
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
