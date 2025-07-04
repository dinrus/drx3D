// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: common listener code for procedural breaks
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/

#include <drx3D/Act/StdAfx.h>

#if !NET_USE_SIMPLE_BREAKAGE

	#include <drx3D/Act/ProceduralBreakingBaseListener.h>
	#include <drx3D/Act/NetworkCVars.h>

CProceduralBreakingBaseListener::CProceduralBreakingBaseListener(IProceduralBreakTypePtr pBreakType)
	: m_pBreakType(pBreakType)
	#if BREAK_HIERARCHICAL_TRACKING
	, m_physTime(gEnv->pPhysicalWorld->GetiPhysicsTime())
	, m_activityThisStep(1)
	, m_numEmptySteps(0)
	#endif
	, m_curBreakOp(eNBO_Create)
	, m_curBreakId(-1)
	, m_curBreakPart(-1)
	, m_allowAdditions(true)
{
}

CProceduralBreakingBaseListener::~CProceduralBreakingBaseListener()
{
	if (CNetworkCVars::Get().BreakageLog)
	{
		DrxLogAlways("[brk] ~CProceduralBreakingBaseListener() %p", this);
		for (VectorMap<EntityId, i32>::iterator it = m_localEntityIds.begin(); it != m_localEntityIds.end(); ++it)
		{
			i32 phys = -100;
			if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(it->first))
			{
				++phys;
				if (IPhysicalEntity* pPhys = pEntity->GetPhysics())
					phys = pPhys->GetType();
			}
			DrxLogAlways("[brk] pbrk[%d]: %.8x phys=%d", it->second, it->first, phys);
		}
	}
}

bool CProceduralBreakingBaseListener::AcceptUpdateMesh(const EventPhysUpdateMesh* pEvent)
{
	return AcceptIfRelated(pEvent->pEntity, eNBO_Update, -1);
}

bool CProceduralBreakingBaseListener::AcceptCreateEntityPart(const EventPhysCreateEntityPart* pEvent)
{
	return AcceptIfRelated(pEvent->pEntity, eNBO_Create, pEvent->partidSrc);
}

bool CProceduralBreakingBaseListener::AcceptRemoveEntityParts(const EventPhysRemoveEntityParts* pEvent)
{
	return AcceptIfRelated(pEvent->pEntity, eNBO_Remove, -1);
}

bool CProceduralBreakingBaseListener::AcceptJointBroken(const EventPhysJointBroken* pEvent)
{
	bool related = AcceptIfRelated(pEvent->pEntity[0], eNBO_JointBreak, -1);
	if (related && 0 == (m_pBreakType->flags & ePBTF_NoJointBreaks))
		GotJointBreak(m_curBreakId, pEvent->idJoint, pEvent->partidEpicenter);
	return related;
}

void CProceduralBreakingBaseListener::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	i32 virtId = GotBreakOp(m_curBreakOp, m_curBreakId, m_curBreakPart, pEntity->GetId());
	if (virtId >= 0)
	{
		if (CNetworkCVars::Get().BreakageLog)
			DrxLogAlways("[brk] spawned entity %.8x at index virtId: %d", pEntity->GetId(), virtId);
		pEntity->SetFlags(pEntity->GetFlags() & ~(ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY));
		params.nFlags &= ~(ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY);

		IGameObject* pGameObject = CDrxAction::GetDrxAction()->GetIGameObjectSystem()->CreateGameObjectForEntity(pEntity->GetId());
		pGameObject->ActivateExtension("BreakRepGameObject");
		m_localEntityIds.insert(std::make_pair(pEntity->GetId(), virtId));
	}
	SJustSpawnedObject jso = { m_curBreakOp, m_curBreakId, pEntity->GetId(), virtId };
	m_justSpawnedObjects.push_back(jso);
	#if BREAK_HIERARCHICAL_TRACKING
	m_activityThisStep++;
	#endif
}

void CProceduralBreakingBaseListener::OnRemove(IEntity* pEntity)
{
}

void CProceduralBreakingBaseListener::EndEvent(INetContext* pCtx)
{
	if (m_justSpawnedObjects.empty())
	{
		if (BIT(m_curBreakOp) & OPS_WITH_NOSPAWN)
			GotBreakOp((ENetBreakOperation)(m_curBreakOp + 1), m_curBreakId, m_curBreakPart, 0);
	}
	else
	{
		for (size_t i = 0; i < m_justSpawnedObjects.size(); i++)
		{
			SJustSpawnedObject o = m_justSpawnedObjects[i];
			FinishBreakOp(o.op, o.idxRef, o.to, o.virtId);
		}
		m_justSpawnedObjects.resize(0);
	}
}

void CProceduralBreakingBaseListener::OnPostStep()
{
}

void CProceduralBreakingBaseListener::OnStartFrame()
{
}

bool CProceduralBreakingBaseListener::OnEndFrame()
{
	#if BREAK_HIERARCHICAL_TRACKING
	i32 physTime = gEnv->pPhysicalWorld->GetiPhysicsTime();
	if (physTime != m_physTime)
	{
		if (m_activityThisStep)
			m_numEmptySteps = 0;
		else
			m_numEmptySteps++;
		m_activityThisStep = 0;
		GotBreakOp(eNBO_PostStep, -1, -1, ~0);
		m_physTime = physTime;
	}
	#endif
	bool canStop = AllowComplete();
	if (canStop)
	{
		Complete();
		return false;
	}
	return true;
}

void CProceduralBreakingBaseListener::OnTimeout()
{
	Complete();
}

bool CProceduralBreakingBaseListener::AcceptIfRelated(IPhysicalEntity* pEnt, ENetBreakOperation breakOp, i32 partid)
{
	if (!m_allowAdditions)
		return false;
	i32 virtId = -m_pBreakType->GetVirtualId(pEnt);
	if (virtId == 0)
	{
		if ((m_pBreakType->flags & ePBTF_ChainBreaking) != 0)
		{
			if (pEnt->GetiForeignData() == PHYS_FOREIGN_ID_ENTITY)
			{
				IEntity* pEntity = (IEntity*)pEnt->GetForeignData(PHYS_FOREIGN_ID_ENTITY);
				if (!pEntity)
					return false;
				VectorMap<EntityId, i32>::iterator iter = m_localEntityIds.find(pEntity->GetId());
				if (iter == m_localEntityIds.end())
					return false;
				virtId = iter->second;
			}
			else
				return false;
		}
		else
			return false;
	}
	m_curBreakOp = breakOp;
	m_curBreakId = virtId;
	m_curBreakPart = partid;
	return true;
}

CObjectSelector CProceduralBreakingBaseListener::GetObjectSelector(i32 idx)
{
	if (idx < 0)
		return m_pBreakType->GetObjectSelector(-idx);
	else
	{
		for (VectorMap<EntityId, i32>::iterator iter = m_localEntityIds.begin(); iter != m_localEntityIds.end(); ++iter)
		{
			if (iter->second == idx)
				return CObjectSelector(iter->first);
		}
	}
	return CObjectSelector();
}

void DumpSpawnRecs(const DynArray<SProceduralSpawnRec>& recs, const DynArray<SJointBreakRec>& joints, tukk title)
{
	if (CNetworkCVars::Get().BreakageLog)
	{
		DrxLog("[brk] -- spawn records: %s --------------------------------------------", title);
		for (size_t i = 0; i < joints.size(); i++)
		{
	#if BREAK_HIERARCHICAL_TRACKING
			DrxLog("[brk]   .break %d on %d epipart=%d @ frame %d", i32(joints[i].id), i32(joints[i].idxRef), joints[i].epicenter, i32(joints[i].frame));
	#else
			DrxLog("[brk]   .break %d on %d epipart=%d", i32(joints[i].id), i32(joints[i].idxRef), joints[i].epicenter);
	#endif
		}
	#if BREAK_HIERARCHICAL_TRACKING
		i32 frameno = 0;
	#endif
		for (size_t i = 0; i < recs.size(); i++)
		{
			tukk name = "!!!!!!";
			switch (recs[i].op)
			{
	#if BREAK_HIERARCHICAL_TRACKING
			case eNBO_FrameCounterFinished:
				name = "xxxxxx";
				break;
			case eNBO_PostStep:
				name = "postep";
				break;
	#endif
			case eNBO_NUM_OPERATIONS:
	#if BREAK_HIERARCHICAL_TRACKING
				frameno = -1;
	#endif
				name = "......";
				break;
			case eNBO_Create:
				name = "create";
				break;
			case eNBO_Create_NoSpawn:
				name = "cre0te";
				break;
			case eNBO_Remove:
				name = "remove";
				break;
			case eNBO_Remove_NoSpawn:
				name = "rem0ve";
				break;
			case eNBO_JointBreak:
				name = "jbreak";
				break;
			case eNBO_Update:
				name = "update";
				break;
			case eNBO_AbsorbStep:
				name = "absorb";
				break;
			}
			char buffer[256];
			tuk p = buffer;
			p += sprintf(p, " [%03d] %s", (i32)i, name);
			if (BIT(recs[i].op) & OPS_REFERENCING_ENTS)
				p += sprintf(p, " on %d", i32(recs[i].idxRef));
			if (BIT(recs[i].op) & OPS_WITH_PARTIDS)
				p += sprintf(p, " part %d", recs[i].partid);
			if (BIT(recs[i].op) & OPS_CAUSING_ENTS)
				p += sprintf(p, " -> %d", i32(recs[i].idx));
	#if BREAK_HIERARCHICAL_TRACKING
			if (frameno >= 0 && (BIT(recs[i].op) & OPS_JOINT_FRAME_COUNTERS))
			{
				frameno++;
				p += sprintf(p, " frame=%d", frameno);
			}
	#endif
			DrxLog("[brk] %s", buffer);
		}
		DrxLog("[brk] -- end records  : %s --------------------------------------------", title);
	}
}

#endif // !NET_USE_SIMPLE_BREAKAGE
