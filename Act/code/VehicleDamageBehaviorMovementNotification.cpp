// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleDamageBehaviorMovementNotification.h>

#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>

//------------------------------------------------------------------------
CVehicleDamageBehaviorMovementNotification::CVehicleDamageBehaviorMovementNotification()
	: m_pVehicle(nullptr)
	, m_isSteeringInvolved(false)
	, m_isFatal(true)
	, m_isDamageAlwaysFull(false)
	, m_param(0)
{}

//------------------------------------------------------------------------
bool CVehicleDamageBehaviorMovementNotification::Init(IVehicle* pVehicle, const CVehicleParams& table)
{
	DRX_ASSERT(pVehicle);
	m_pVehicle = pVehicle;

	m_param = 0;

	if (CVehicleParams notificationParams = table.findChild("MovementNotification"))
	{
		notificationParams.getAttr("isSteering", m_isSteeringInvolved);
		notificationParams.getAttr("isFatal", m_isFatal);
		notificationParams.getAttr("param", m_param);
		notificationParams.getAttr("isDamageAlwaysFull", m_isDamageAlwaysFull);
	}

	return true;
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorMovementNotification::Reset()
{
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorMovementNotification::OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams)
{
	if (event == eVDBE_Hit || event == eVDBE_VehicleDestroyed || event == eVDBE_Repair)
	{
		IVehicleMovement::EVehicleMovementEvent eventType;

		if (event == eVDBE_Repair)
			eventType = IVehicleMovement::eVME_Repair;
		else if (m_isSteeringInvolved)
			eventType = IVehicleMovement::eVME_DamageSteering;
		else
			eventType = IVehicleMovement::eVME_Damage;

		SVehicleMovementEventParams params;
		if (m_isDamageAlwaysFull)
		{
			if (event != eVDBE_Repair)
				params.fValue = 1.0f;
			else
				params.fValue = 0.0f;
		}
		else
		{
			if (event != eVDBE_Repair)
				params.fValue = min(1.0f, behaviorParams.componentDamageRatio);
			else
			{
				// round down to nearest 0.25 (ensures movement object is sent zero damage on fully repairing vehicle).
				params.fValue = ((i32)(behaviorParams.componentDamageRatio * 4)) / 4.0f;
			}
		}

		params.vParam = behaviorParams.localPos;
		params.bValue = m_isFatal;
		params.iValue = m_param;
		params.pComponent = behaviorParams.pVehicleComponent;

		m_pVehicle->GetMovement()->OnEvent(eventType, params);
	}
}

DEFINE_VEHICLEOBJECT(CVehicleDamageBehaviorMovementNotification);
