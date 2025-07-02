// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements an entity class for detached parts

   -------------------------------------------------------------------------
   История:
   - 26:10:2005: Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/IViewSystem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/AI/IAgent.h>

#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehiclePartDetachedEntity.h>
#include <drx3D/Act/VehiclePartBase.h>

namespace VPDE
{
void RegisterEvents(IGameObjectExtension& goExt, IGameObject& gameObject)
{
	i32k eventID = eGFE_OnCollision;
	gameObject.UnRegisterExtForEvents(&goExt, NULL, 0);
	gameObject.RegisterExtForEvents(&goExt, &eventID, 1);
}
}

//------------------------------------------------------------------------
CVehiclePartDetachedEntity::~CVehiclePartDetachedEntity()
{
	GetGameObject()->EnablePhysicsEvent(false, eEPE_OnCollisionLogged);
}

//------------------------------------------------------------------------
bool CVehiclePartDetachedEntity::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	InitVehiclePart(pGameObject);

	return true;
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::PostInit(IGameObject* pGameObject)
{
	VPDE::RegisterEvents(*this, *pGameObject);
}

//------------------------------------------------------------------------
bool CVehiclePartDetachedEntity::ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params)
{
	ResetGameObject();

	VPDE::RegisterEvents(*this, *pGameObject);

	InitVehiclePart(pGameObject);

	return true;
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::InitVehiclePart(IGameObject* pGameObject)
{
	assert(pGameObject);

	// Set so we receive render events (when GO is set to update due to being visible), allowing last seen timer to reset
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_VisibleAndInRange);

	if (IPhysicalEntity* pPhysics = GetEntity()->GetPhysics())
	{
		pe_params_flags physFlags;

		if (pPhysics->GetParams(&physFlags))
		{
			physFlags.flags |= pef_log_collisions;
			pPhysics->SetParams(&physFlags);

			pGameObject->EnablePhysicsEvent(true, eEPE_OnCollisionLogged);
		}
	}

	m_timeUntilStartIsOver = 10.0f;
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::Update(SEntityUpdateContext& ctx, i32 slot)
{
	const float frameTime = ctx.fFrameTime;

	switch (slot)
	{
	case 0:
		{
			if (m_timeUntilStartIsOver > 0.0f)
				m_timeUntilStartIsOver -= frameTime;
		}
		break;
	}
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::ProcessEvent(const SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_RESET)
	{
		gEnv->pEntitySystem->RemoveEntity(GetEntity()->GetId());
	}
}

uint64 CVehiclePartDetachedEntity::GetEventMask() const 
{
	return ENTITY_EVENT_BIT(ENTITY_EVENT_RESET);
}

//------------------------------------------------------------------------
void CVehiclePartDetachedEntity::HandleEvent(const SGameObjectEvent& event)
{
	if (event.event == eGFE_OnCollision)
	{
		if (m_timeUntilStartIsOver <= 0.0f)
		{
			const EventPhysCollision* pCollision = static_cast<const EventPhysCollision*>(event.ptr);
			IEntitySystem* pES = gEnv->pEntitySystem;
			IEntity* pE1 = pES->GetEntityFromPhysics(pCollision->pEntity[0]);
			IEntity* pE2 = pES->GetEntityFromPhysics(pCollision->pEntity[1]);
			IEntity* pCollider = pE1 == GetEntity() ? pE2 : pE1;

			if (pCollider)
			{
				//OnCollision( pCollider->GetId(), pCollision->pt, pCollision->n );
				IEntity* pEntity = GetEntity();
				DRX_ASSERT(pEntity);

				i32 slotCount = pEntity->GetSlotCount();
				if (slotCount > 1)
				{
					for (i32 i = 1; i < slotCount; i++)
					{
						if (pEntity->IsSlotValid(i))
							pEntity->FreeSlot(i);
					}
				}
			}
		}
	}
}

void CVehiclePartDetachedEntity::FullSerialize(TSerialize ser)
{
	bool hasGeom = false;
	if (ser.IsWriting())
	{
		if (GetEntity()->GetStatObj(0))
			hasGeom = true;
	}
	ser.Value("hasGeometry", hasGeom);
	if (hasGeom)
	{
		if (ser.IsWriting())
		{
			gEnv->p3DEngine->SaveStatObj(GetEntity()->GetStatObj(0), ser);
			if (GetEntity()->GetPhysics())
				GetEntity()->GetPhysics()->GetStateSnapshot(ser);
		}
		else if (ser.IsReading())
		{
			IStatObj* pStatObj = gEnv->p3DEngine->LoadStatObj(ser);
			if (pStatObj)
			{
				GetEntity()->SetStatObj(pStatObj, 0, true, 200.0f);
				SEntityPhysicalizeParams physicsParams;
				if (!pStatObj->GetPhysicalProperties(physicsParams.mass, physicsParams.density))
					physicsParams.mass = 200.0f;
				physicsParams.type = PE_RIGID;
				physicsParams.nFlagsOR &= pef_log_collisions;
				physicsParams.nSlot = 0;
				GetEntity()->Physicalize(physicsParams);
				if (GetEntity()->GetPhysics())
					GetEntity()->GetPhysics()->SetStateFromSnapshot(ser);
			}
		}
	}
}
