// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: playback of generic breaking events
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Entity/IBreakableUpr.h>

#if !NET_USE_SIMPLE_BREAKAGE

	#include <drx3D/Act/GenericPlaybackListener.h>

CGenericPlaybackListener::CGenericPlaybackListener() : m_spawnIdx(0), m_pBreakage(0)
{
}

void CGenericPlaybackListener::Perform(const SSimulateRemoveEntityParts& param, INetBreakagePlaybackPtr pBreakage)
{
	DRX_ASSERT(!m_pBreakage);
	m_pBreakage = pBreakage;
	DRX_ASSERT(m_spawnIdx == 0);

	EventPhysRemoveEntityParts event;
	event.pEntity = param.ent.Find();
	if (!event.pEntity)
		return;
	event.iForeignData = event.pEntity->GetiForeignData();
	event.pForeignData = event.pEntity->GetForeignData(event.iForeignData);
	gEnv->pEntitySystem->GetBreakableUpr()->FakePhysicsEvent(&event);
}

bool CGenericPlaybackListener::AcceptUpdateMesh(const EventPhysUpdateMesh* pEvent)
{
	return false;
}

bool CGenericPlaybackListener::AcceptCreateEntityPart(const EventPhysCreateEntityPart* pEvent)
{
	return false;
}

bool CGenericPlaybackListener::AcceptRemoveEntityParts(const EventPhysRemoveEntityParts* pEvent)
{
	return false;
}

bool CGenericPlaybackListener::AcceptJointBroken(const EventPhysJointBroken* pEvent)
{
	return false;
}

void CGenericPlaybackListener::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	DRX_ASSERT(m_pBreakage != NULL);
	m_pBreakage->SpawnedEntity(m_spawnIdx++, pEntity->GetId());
}

void CGenericPlaybackListener::OnRemove(IEntity* pEntity)
{
}

void CGenericPlaybackListener::EndEvent(INetContext* pCtx)
{
	DRX_ASSERT(m_pBreakage != NULL);
	m_pBreakage = 0;
	m_spawnIdx = 0;
}

void CGenericPlaybackListener::OnStartFrame()
{
}

bool CGenericPlaybackListener::OnEndFrame()
{
	DRX_ASSERT(false);
	return false;
}

void CGenericPlaybackListener::OnTimeout()
{
}

/*
   void CGenericPlaybackListener::Perform( const SSimulateCreateEntityPart& param, INetBreakagePlayback * pBreakage )
   {
   DRX_ASSERT(!m_pBreakage);
   m_pBreakage = pBreakage;
   DRX_ASSERT(m_spawnIdx == 0);

   EventPhysCreateEntityPart event;
   event.bInvalid = param.bInvalid;
   event.breakAngImpulse = param.breakAngImpulse;
   event.breakImpulse = param.breakImpulse;
   for (i32 i=0; i<2; i++)
   {
    event.cutDirLoc[i] = param.cutDirLoc[i];
    event.cutPtLoc[i] = param.cutPtLoc[i];
   }
   event.cutRadius = param.cutRadius;
   event.pEntNew = param.entNew.Find();
   event.pEntity = param.entSrc.Find();
   if (!event.pEntity)
    return;
   event.iForeignData = event.pEntity->GetiForeignData();
   event.pForeignData = event.pEntity->GetForeignData();
   event.nTotParts = param.nTotParts;
   event.partidNew = param.partidNew;
   event.partidSrc = param.partidSrc;
   gEnv->pEntitySystem->GetBreakableUpr()->FakePhysicsEvent(&event);
   }
 */

#endif // !NET_USE_SIMPLE_BREAKAGE
