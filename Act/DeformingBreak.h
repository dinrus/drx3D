// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: deforming breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __DEFORMINGBREAK_H__
#define __DEFORMINGBREAK_H__

#pragma once

#include "ProceduralBreak.h"
#include "ActionGame.h"
#include "IProceduralBreakType.h"

void LogDeformPhysicalEntity(tukk from, IPhysicalEntity* pEnt, const Vec3& p, const Vec3& n, float energy);

#if !NET_USE_SIMPLE_BREAKAGE

struct SDeformingBreakParams
{
	SDeformingBreakParams() : breakId(-1) {}
	SDeformingBreakParams(i32 bid, const SBreakEvent& bev) : breakId(bid), breakEvent(bev)
	{
		breakEvent.iState = eBES_Generated;
		breakEvent.time = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	}
	i32         breakId;
	SBreakEvent breakEvent;

	void        SerializeWith(TSerialize ser);
};

struct SDeformingBreak : public SProceduralBreak
{
	DynArray<SBreakEvent> breakEvents;

	virtual void GetAffectedRegion(AABB& pos);
	virtual void AddSendables(INetSendableSink* pSink, i32 brkId);
};

class CDeformingBreak : public IProceduralBreakType
{
public:
	CDeformingBreak(const SBreakEvent& be);

	bool                         AttemptAbsorb(const IProceduralBreakTypePtr& pBT);
	void                         AbsorbStep();
	i32                          GetVirtualId(IPhysicalEntity* pEnt);
	CObjectSelector              GetObjectSelector(i32 idx);
	_smart_ptr<SProceduralBreak> CompleteSend();
	bool                         GotExplosiveObjectState(const SExplosiveObjectState* pState);
	void                         PreparePlayback();
	void                         BeginPlayback(bool hasJointBreaks);
	tukk                  GetName()                                                                                       { return "DeformingBreak"; }
	void                         PatchRecording(DynArray<SProceduralSpawnRec>& spawnRecs, DynArray<SJointBreakRec>& jointBreaks) {}

	virtual bool                 AllowComplete(const SProceduralBreakRecordingState& state);

private:
	DynArray<SBreakEvent> m_bes;
	IPhysicalEntity*      m_pPhysEnt;
	i32                   m_absorbIdx;

	void PreparePlaybackForEvent(i32 event);

	void PrepareSlot(i32 idx);

	bool IsOurStatObj(IRenderNode* rn)
	{
		if (m_bes[0].itype == PHYS_FOREIGN_ID_STATIC)
			return m_bes[0].hash == CObjectSelector::CalculateHash(rn);
		else
			return false;
	}
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
