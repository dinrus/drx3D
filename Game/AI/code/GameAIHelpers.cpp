// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameAIHelpers.h>
#include <drx3D/AI/IAgent.h>
#include <drx3D/AI/IAIObject.h>
#include <drx3D/AI/IAIActor.h>

CGameAIInstanceBase::CGameAIInstanceBase(EntityId entityID)
{
	Init(entityID);
}

void CGameAIInstanceBase::Init(EntityId entityID)
{
	m_entityID = entityID;

#ifndef RELEASE
	if (IEntity* entity = gEnv->pEntitySystem->GetEntity(entityID))
	{
		m_debugEntityName = entity->GetName();
	}
#endif
}

void CGameAIInstanceBase::SendSignal(tukk signal, IAISignalExtraData* data)
{
	IEntity* entity = gEnv->pEntitySystem->GetEntity(GetEntityID());
	assert(entity);
	if (entity)
	{
		if (IAIObject* aiObject = entity->GetAI())
			if (IAIActor* aiActor = aiObject->CastToIAIActor())
				aiActor->SetSignal(1, signal, NULL, data, 0);
	}
}


// ===========================================================================
//	Send a signal to the AI actor.
//
//	In:		The signal name (NULL is invalid!)
//	In:		Optional signal context data that gets passed to Lua behavior 
//			scripts	(NULL will ignore).
//	In:		Option signal ID (or mode really). Should be a AISIGNAL_xxx
//			value.
//
void CGameAIInstanceBase::SendSignal(
	tukk signal, IAISignalExtraData* data, i32 nSignalID)
{
	IEntity* entity = gEnv->pEntitySystem->GetEntity(GetEntityID());
	assert(entity != NULL);
	if (entity != NULL)
	{
		IAIObject* aiObject = entity->GetAI();
		if (aiObject != NULL)
		{
			IAIActor* aiActor = aiObject->CastToIAIActor();
			if (aiActor != NULL)
			{
				aiActor->SetSignal(nSignalID, signal, NULL, data, 0);
			}
		}
	}
}
