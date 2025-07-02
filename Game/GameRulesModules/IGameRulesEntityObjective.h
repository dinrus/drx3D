// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for an entity based objective

	-------------------------------------------------------------------------
	История:
	- 20:10:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _IGAME_RULES_ENTITY_OBJECTIVE_H_
#define _IGAME_RULES_ENTITY_OBJECTIVE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameObject.h>
#include <drx3D/Network/SerializeFwd.h>

class IGameRulesEntityObjective
{
public:
	virtual ~IGameRulesEntityObjective() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void Update(float frameTime) = 0;

	virtual void OnStartGame() = 0;

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) = 0;

	virtual bool IsComplete(i32 teamId) = 0;

	virtual void AddEntityId(i32 type, EntityId entityId, i32 index, bool isNewEntity, const CTimeValue &timeAdded) = 0;
	virtual void RemoveEntityId(i32 type, EntityId entityId) = 0;
	virtual void ClearEntities(i32 type) = 0;
	virtual bool IsEntityFinished(i32 type, i32 index) = 0;
	virtual bool CanRemoveEntity(i32 type, i32 index) = 0;

	virtual void OnHostMigration(bool becomeServer) = 0;		// Host migration

	virtual void OnTimeTillRandomChangeUpdated(i32 type, float fPercLiveSpan) = 0;

	virtual bool IsPlayerEntityUsingObjective(EntityId playerId) = 0;
};

#endif // _IGAME_RULES_ENTITY_OBJECTIVE_H_
