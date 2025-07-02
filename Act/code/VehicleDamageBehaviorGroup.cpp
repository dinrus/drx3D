// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleDamageBehaviorGroup.h>

#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleDamagesGroup.h>

CVehicleDamageBehaviorGroup::CVehicleDamageBehaviorGroup()
	: m_pVehicle(nullptr)
{}

//------------------------------------------------------------------------
bool CVehicleDamageBehaviorGroup::Init(IVehicle* pVehicle, const CVehicleParams& table)
{
	m_pVehicle = (CVehicle*) pVehicle;

	CVehicleParams groupParams = table.findChild("Group");
	if (!groupParams)
		return false;

	if (!groupParams.haveAttr("name"))
		return false;

	m_damageGroupName = groupParams.getAttr("name");
	return !m_damageGroupName.empty();
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorGroup::Reset()
{
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorGroup::OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams)
{
	if (CVehicleDamagesGroup* pDamagesGroup = m_pVehicle->GetDamagesGroup(m_damageGroupName.c_str()))
	{
		pDamagesGroup->OnDamageEvent(event, behaviorParams);
	}
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorGroup::Serialize(TSerialize ser, EEntityAspects aspects)
{
}

//------------------------------------------------------------------------
void CVehicleDamageBehaviorGroup::Update(const float deltaTime)
{
}

void CVehicleDamageBehaviorGroup::GetMemoryUsage(IDrxSizer* s) const
{
	s->AddObject(this, sizeof(*this));
	s->AddObject(m_damageGroupName);
}

DEFINE_VEHICLEOBJECT(CVehicleDamageBehaviorGroup);
