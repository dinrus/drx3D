// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleDamageBehaviorHitPassenger.h>

#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/IGameRulesSystem.h>

//------------------------------------------------------------------------
CVehicleDamageBehaviorHitPassenger::CVehicleDamageBehaviorHitPassenger()
	: m_pVehicle(nullptr)
	, m_damage(0)
	, m_isDamagePercent(false)
{}

//------------------------------------------------------------------------
bool CVehicleDamageBehaviorHitPassenger::Init(IVehicle* pVehicle, const CVehicleParams& table)
{
	m_pVehicle = pVehicle;

	CVehicleParams hitPassTable = table.findChild("HitPassenger");
	if (!hitPassTable)
		return false;

	if (!hitPassTable.getAttr("damage", m_damage))
		return false;

	if (!hitPassTable.getAttr("isDamagePercent", m_isDamagePercent))
		m_isDamagePercent = false;

	return true;
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorHitPassenger::OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams)
{
	if (event != eVDBE_Hit)
		return;

	IActorSystem* pActorSystem = gEnv->pGameFramework->GetIActorSystem();
	DRX_ASSERT(pActorSystem);

	TVehicleSeatId lastSeatId = m_pVehicle->GetLastSeatId();
	for (TVehicleSeatId seatId = FirstVehicleSeatId; seatId <= lastSeatId; ++seatId)
	{
		if (IVehicleSeat* pSeat = m_pVehicle->GetSeatById(seatId))
		{
			EntityId passengerId = pSeat->GetPassenger();
			if (IActor* pActor = pActorSystem->GetActor(passengerId))
			{
				i32 damage = m_damage;

				if (m_isDamagePercent)
				{
					damage = (i32)(pActor->GetMaxHealth() * float(m_damage) / 100.f);
				}

				if (gEnv->bServer)
				{
					if (IGameRules* pGameRules = gEnv->pGameFramework->GetIGameRulesSystem()->GetCurrentGameRules())
					{
						HitInfo hit;

						hit.targetId = pActor->GetEntityId();
						hit.shooterId = behaviorParams.shooterId;
						hit.weaponId = 0;
						hit.damage = (float)damage;
						hit.type = 0;

						pGameRules->ServerHit(hit);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorHitPassenger::GetMemoryUsage(IDrxSizer* s) const
{
	s->Add(*this);
}

DEFINE_VEHICLEOBJECT(CVehicleDamageBehaviorHitPassenger);
