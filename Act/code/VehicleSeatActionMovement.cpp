// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat action which handle the vehicle movement

   -------------------------------------------------------------------------
   История:
   - 20:10:2006: Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeat.h>
#include <drx3D/Act/VehicleSeatActionMovement.h>

//------------------------------------------------------------------------
CVehicleSeatActionMovement::CVehicleSeatActionMovement()
	: m_delayedStop(0.0f),
	m_actionForward(0.0f),
	m_pVehicle(NULL),
	m_pSeat(NULL)
{
}

//------------------------------------------------------------------------
CVehicleSeatActionMovement::~CVehicleSeatActionMovement()
{
}

//------------------------------------------------------------------------
bool CVehicleSeatActionMovement::Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table)
{
	return Init(pVehicle, pSeat);
}

//------------------------------------------------------------------------
bool CVehicleSeatActionMovement::Init(IVehicle* pVehicle, IVehicleSeat* pSeat)
{
	IVehicleMovement* pMovement = pVehicle->GetMovement();
	if (!pMovement)
		return false;

	m_pVehicle = pVehicle;
	m_pSeat = pSeat;

	return true;
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::Reset()
{
	DRX_ASSERT(m_pVehicle);

	m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);

	m_actionForward = 0.0f;
	m_delayedStop = 0.0f;

	if (IVehicleMovement* pMovement = m_pVehicle->GetMovement())
		pMovement->Reset();
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::StartUsing(EntityId passengerId)
{
	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();
	DRX_ASSERT(pActorSystem);

	IActor* pActor = pActorSystem->GetActor(passengerId);
	DRX_ASSERT(pActor);

	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	DRX_ASSERT(pMovement);

	if (!pMovement)
		return;

	if (m_delayedStop >= 0.0f)
	{
		m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);
		m_delayedStop = 0.0f;

		pMovement->StopDriving();
	}

	pMovement->StartDriving(passengerId);
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::StopUsing()
{
	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();
	DRX_ASSERT(pActorSystem);

	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	if (!pMovement)
		return;

	DRX_ASSERT(m_pSeat);

	// default to continuing for a bit
	m_delayedStop = 0.8f;

	IActor* pActor = pActorSystem->GetActor(m_pSeat->GetPassenger());

	if (pActor && pActor->IsPlayer())
	{
		// if stopped already don't go anywhere
		IPhysicalEntity* pPhys = m_pVehicle->GetEntity()->GetPhysics();
		pe_status_dynamics status;
		if (pPhys && pPhys->GetStatus(&status))
		{
			if (status.v.GetLengthSquared() < 25.0f)
				m_delayedStop = 0.0f;
		}

		if (m_actionForward > 0.0f)
			m_delayedStop = 1.5f;

		if (pMovement->GetMovementType() == IVehicleMovement::eVMT_Air)
			m_delayedStop *= 2.0f;

		m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_AlwaysUpdate);

		// prevent full pedal being kept pressed, but give it a bit
		pMovement->OnAction(eVAI_MoveForward, eAAM_OnPress, 0.1f);
	}
	else
	{
		if (pMovement->GetMovementType() == IVehicleMovement::eVMT_Air)
		{
			m_delayedStop = 0.0f;
			m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_AlwaysUpdate);
		}
		else
		{
			pMovement->StopDriving();
		}
	}
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::OnAction(const TVehicleActionId actionId, i32 activationMode, float value)
{
	if (actionId == eVAI_MoveForward)
		m_actionForward = value;
	else if (actionId == eVAI_MoveBack)
		m_actionForward = -value;

	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	DRX_ASSERT(pMovement);

	pMovement->OnAction(actionId, activationMode, value);
}

//------------------------------------------------------------------------
void CVehicleSeatActionMovement::Update(const float deltaTime)
{
	IVehicleMovement* pMovement = m_pVehicle->GetMovement();
	DRX_ASSERT(pMovement);

	bool isReadyToStopEngine = true;

	if (isReadyToStopEngine)
	{
		if (m_delayedStop > 0.0f)
			m_delayedStop -= deltaTime;

		if (m_delayedStop <= 0.0f)
		{
			m_pVehicle->SetObjectUpdate(this, IVehicle::eVOU_NoUpdate);
			pMovement->StopDriving();
		}
	}
}

DEFINE_VEHICLEOBJECT(CVehicleSeatActionMovement);
