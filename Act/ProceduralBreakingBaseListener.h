// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: common listener code for procedural breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __PROCEDURALBREAKBASELISTENER_H__
#define __PROCEDURALBREAKBASELISTENER_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "IBreakReplicatorListener.h"
	#include "IProceduralBreakType.h"
	#include <drx3D/CoreX/Containers/VectorMap.h>

void DumpSpawnRecs(const DynArray<SProceduralSpawnRec>& recs, const DynArray<SJointBreakRec>& joints, tukk title);

class CProceduralBreakingBaseListener : public IBreakReplicatorListener
{
public:
	CProceduralBreakingBaseListener(IProceduralBreakTypePtr pBreakType);
	~CProceduralBreakingBaseListener();
	bool            AcceptUpdateMesh(const EventPhysUpdateMesh* pEvent);
	bool            AcceptCreateEntityPart(const EventPhysCreateEntityPart* pEvent);
	bool            AcceptRemoveEntityParts(const EventPhysRemoveEntityParts* pEvent);
	bool            AcceptJointBroken(const EventPhysJointBroken* pEvent);

	void            OnSpawn(IEntity* pEntity, SEntitySpawnParams& params);
	void            OnRemove(IEntity* pEntity);

	void            EndEvent(INetContext* pCtx);

	virtual void    OnPostStep();
	virtual void    OnStartFrame();
	virtual bool    OnEndFrame();

	virtual void    OnTimeout();

	CObjectSelector GetObjectSelector(i32 idx);

	tukk     GetName()
	{
		if (m_name.empty())
			m_name.Format("ProceduralBreaking.%s.%s", GetListenerName(), m_pBreakType->GetName());
		return m_name.c_str();
	}

protected:
	IProceduralBreakType* GetBreakType()           { return m_pBreakType; }
	void                  DisallowAdditions()      { m_allowAdditions = false; }
	i32                   GetNumEmptySteps() const { return m_numEmptySteps; }

private:
	VectorMap<EntityId, i32> m_localEntityIds;
	IProceduralBreakTypePtr  m_pBreakType;
	#if BREAK_HIERARCHICAL_TRACKING
	i32                      m_physTime;
	i32                      m_activityThisStep;
	i32                      m_numEmptySteps;
	#endif
	ENetBreakOperation       m_curBreakOp;
	i32                      m_curBreakId;
	i32                      m_curBreakPart;
	bool                     m_allowAdditions;
	string                   m_name; // usually empty

	struct SJustSpawnedObject
	{
		ENetBreakOperation op;
		i32                idxRef;
		EntityId           to;
		i32                virtId;
	};
	std::vector<SJustSpawnedObject> m_justSpawnedObjects;

	virtual i32         GotBreakOp(ENetBreakOperation op, i32 idxRef, i32 partid, EntityId to) = 0;
	virtual void        FinishBreakOp(ENetBreakOperation op, i32 idxRef, EntityId to, i32 virtId) = 0;
	virtual void        GotJointBreak(i32 idxRef, i32 id, i32 epicenter) = 0;
	virtual void        GotJointBreakParticle(const EventPhysCreateEntityPart* pEvent) = 0;
	virtual void        Complete() = 0;
	virtual bool        AllowComplete() = 0;
	virtual tukk GetListenerName() = 0;

	bool                AcceptIfRelated(IPhysicalEntity* pEnt, ENetBreakOperation breakOp, i32 partid);
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
