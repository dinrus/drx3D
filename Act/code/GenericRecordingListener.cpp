// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GenericRecordingListener.h>
#include <drx3D/Act/SimulateCreateEntityPart.h>
#include <drx3D/Act/SimulateRemoveEntityParts.h>
#include <drx3D/Act/BreakReplicator.h>

CGenericRecordingListener::CGenericRecordingListener() : m_pDef(0), m_pInfo(0)
{
}

bool CGenericRecordingListener::AcceptJointBroken(const EventPhysJointBroken* pEvt)
{
	GameWarning("CBreakReplicator::AcceptJointBroken: cannot generically replicate broken joints");
	return false;
}

bool CGenericRecordingListener::AcceptUpdateMesh(const EventPhysUpdateMesh* pEvent)
{
	GameWarning("CBreakReplicator::AcceptUpdateMesh: cannot generically replicate meshes");
	return false;
}

bool CGenericRecordingListener::AcceptCreateEntityPart(const EventPhysCreateEntityPart* pEvent)
{
	GameWarning("CBreakReplicator::AcceptUpdateMesh: cannot generically replicate part creation");
	return false;
}

bool CGenericRecordingListener::AcceptRemoveEntityParts(const EventPhysRemoveEntityParts* pEvent)
{
	return false;
}

void CGenericRecordingListener::EndEvent(INetContext* pCtx)
{
	if (m_spawned.empty())
		return;

	DRX_ASSERT(m_pDef);
	DRX_ASSERT(m_pInfo != NULL);
	SNetBreakDescription def;
	def.pMessagePayload = m_pInfo;
	def.pEntities = &m_spawned[0];
	def.nEntities = m_spawned.size();
	pCtx->LogBreak(def);

	m_pDef = 0;
	m_pInfo = 0;
	m_spawned.resize(0);
}

void CGenericRecordingListener::OnRemove(IEntity* pEntity)
{
	for (i32 i = 0; i < m_spawned.size(); i++)
		if (m_spawned[i] == pEntity->GetId())
			m_spawned[i] = 0;
}

void CGenericRecordingListener::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	m_spawned.push_back(pEntity->GetId());
}
