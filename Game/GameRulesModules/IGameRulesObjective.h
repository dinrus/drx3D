// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a player objective
	-------------------------------------------------------------------------
	История:
	- 21:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _IGAME_RULES_OBJECTIVE_H_
#define _IGAME_RULES_OBJECTIVE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/IGameObject.h>
#include <drx3D/Network/SerializeFwd.h>
#include <drx3D/Game/GameRulesTypes.h>

struct SInteractionInfo;

class IGameRulesObjective
{
public:
	virtual ~IGameRulesObjective() {}

	virtual void Init(XmlNodeRef xml) = 0;
	virtual void Update(float frameTime) = 0;

	virtual void OnStartGame() = 0;
	virtual void OnStartGamePost() = 0;

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) = 0;

	virtual void Enable(i32 teamId, bool enable) = 0;
	virtual bool IsComplete(i32 teamId) = 0;

	virtual void OnHostMigration(bool becomeServer) = 0;

	virtual bool SuddenDeathTest() const = 0;
	virtual bool CheckForInteractionWithEntity(EntityId interactId, EntityId playerId, SInteractionInfo& interactionInfo) = 0;
	virtual bool CheckIsPlayerEntityUsingObjective(EntityId playerId) = 0;

	virtual ESpawnEntityState GetObjectiveEntityState(EntityId entityId) = 0;
};

#endif // _IGAME_RULES_OBJECTIVE_H_
