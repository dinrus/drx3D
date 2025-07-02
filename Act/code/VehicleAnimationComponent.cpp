// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements vehicle animation using Mannequin

   -------------------------------------------------------------------------
   История:
   - 06:02:2012: Created by Tom Berry

*************************************************************************/
#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeat.h>

#include <drx3D/Act/IDrxMannequin.h>

CVehicleAnimationComponent::CVehicleAnimationComponent()
	:
	m_vehicle(NULL),
	m_pADBVehicle(NULL),
	m_pADBPassenger(NULL),
	m_pAnimContext(NULL),
	m_pActionController(NULL)
{
}

CVehicleAnimationComponent::~CVehicleAnimationComponent()
{
	DeleteActionController();
}

void CVehicleAnimationComponent::Initialise(CVehicle& vehicle, const CVehicleParams& mannequinTable)
{
	DeleteActionController();

	m_vehicle = &vehicle;

	tukk vehicleControllerDef = mannequinTable.getAttr("controllerDef");
	tukk vehicleADB = mannequinTable.getAttr("vehicleADB");
	tukk passengerADB = mannequinTable.getAttr("passengerADB");
	tukk tag = mannequinTable.getAttr("tag");

	IMannequin& mannequinSys = gEnv->pGameFramework->GetMannequinInterface();

	m_pADBVehicle = mannequinSys.GetAnimationDatabaseUpr().Load(vehicleADB);
	m_pADBPassenger = mannequinSys.GetAnimationDatabaseUpr().Load(passengerADB);
	const SControllerDef* pControllerDef = mannequinSys.GetAnimationDatabaseUpr().LoadControllerDef(vehicleControllerDef);

	if (m_pADBVehicle && pControllerDef)
	{
		m_pAnimContext = new SAnimationContext(*pControllerDef);
		m_pActionController = mannequinSys.CreateActionController(vehicle.GetEntity(), *m_pAnimContext);

		if (tag)
		{
			m_typeTag = pControllerDef->m_tags.Find(tag);
			if (m_typeTag != TAG_ID_INVALID)
			{
				m_pAnimContext->state.Set(m_typeTag, true);
			}
		}

		//--- Create scopes via the parts?
		i32k numParts = vehicle.GetPartCount();
		for (i32 p = 0; p < numParts; p++)
		{
			IVehiclePart* pPart = vehicle.GetPart(p);
			TagID partContext = pControllerDef->m_scopeContexts.Find(pPart->GetName());

			if (partContext != TAG_ID_INVALID)
			{
				i32k slot = pPart->GetSlot();

				if (slot >= 0)
				{
					ICharacterInstance* pSlotChar = vehicle.GetEntity()->GetCharacter(slot);
					if (pSlotChar)
					{
						m_pActionController->SetScopeContext(partContext, *vehicle.GetEntity(), pSlotChar, m_pADBVehicle);
					}
				}
			}
		}
	}
}

void CVehicleAnimationComponent::Reset()
{
	if (m_pActionController)
	{
		m_pActionController->Flush();
	}

	if (m_pAnimContext)
	{
		m_pAnimContext->state.Clear();
		if (m_typeTag != TAG_ID_INVALID)
		{
			m_pAnimContext->state.Set(m_typeTag, true);
		}
	}
}

void CVehicleAnimationComponent::Update(float timePassed)
{
	if (m_pActionController)
	{
		m_pActionController->Update(timePassed);
	}
}

void CVehicleAnimationComponent::AttachPassengerScope(CVehicleSeat* pSeat, EntityId entID, bool attach)
{
	DRX_ASSERT(m_pActionController);

	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();
	DRX_ASSERT(pActorSystem);

	IActor* pActor = pActorSystem->GetActor(entID);
	DRX_ASSERT(pActor);

	const SControllerDef& contDef = m_pActionController->GetContext().controllerDef;
	TagID targetScopeContext = contDef.m_scopeContexts.Find(pSeat->GetName());

	if (targetScopeContext != TAG_ID_INVALID)
	{
		IAnimatedCharacter* pAnimChar = pActor ? pActor->GetAnimatedCharacter() : NULL;
		if (pAnimChar)
		{
			if (attach)
			{
				pAnimChar->SetMovementControlMethods(eMCM_Animation, eMCM_Animation);
				pAnimChar->RequestPhysicalColliderMode(eColliderMode_Disabled, eColliderModeLayer_Game, "CVehicle::AttachPassengerScope");
			}
			else
			{
				pAnimChar->SetMovementControlMethods(eMCM_Entity, eMCM_Entity);
				pAnimChar->RequestPhysicalColliderMode(eColliderMode_Undefined, eColliderModeLayer_Game, "CVehicle::AttachPassengerScope");
			}
		}

		u32 numScopes = m_pActionController->GetTotalScopes();

		if (pActor && pActor->GetAnimatedCharacter()->GetActionController())
		{
			m_pActionController->SetSlaveController(*pActor->GetAnimatedCharacter()->GetActionController(), targetScopeContext, attach, m_pADBPassenger);
		}
		else
		{
			if (attach && pActor)
			{
				m_pActionController->SetScopeContext(targetScopeContext, *pActor->GetEntity(), pActor->GetEntity()->GetCharacter(0), m_pADBPassenger);
			}
			else
			{
				m_pActionController->ClearScopeContext(targetScopeContext);
			}
		}
	}

	TagID userTagID = contDef.m_tags.Find(pSeat->GetName());
	if (userTagID != TAG_ID_INVALID)
	{
		m_pAnimContext->state.Set(userTagID, attach);
	}
}

void CVehicleAnimationComponent::DeleteActionController()
{
	SAFE_RELEASE(m_pActionController);
	SAFE_DELETE(m_pAnimContext);
}
