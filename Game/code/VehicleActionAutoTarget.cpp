// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/VehicleActionAutoTarget.h>

tukk CVehicleActionAutoTarget::m_name = "VehicleActionAutoTarget";

CVehicleActionAutoTarget::CVehicleActionAutoTarget() : m_pVehicle(NULL), m_RegisteredWithAutoAimUpr(false)
{
}

CVehicleActionAutoTarget::~CVehicleActionAutoTarget()
{
	m_pVehicle->UnregisterVehicleEventListener(this);
}

bool CVehicleActionAutoTarget::Init( IVehicle* pVehicle, const CVehicleParams& table )
{
	m_pVehicle = pVehicle;
	pVehicle->RegisterVehicleEventListener(this, "AutoTarget");

	IEntity* pEntity = pVehicle->GetEntity();
	if( ICharacterInstance* pCharacter = pEntity->GetCharacter(0) )
	{
		IDefaultSkeleton& rIDefaultSkeleton = pCharacter->GetIDefaultSkeleton();
		{
			tukk primaryBoneName;
			table.getAttr("PrimaryBoneName", &primaryBoneName);
			m_autoAimParams.primaryBoneId = rIDefaultSkeleton.GetJointIDByName(primaryBoneName);

			tukk secondaryBoneName;
			table.getAttr("SecondaryBoneName", &secondaryBoneName);
			m_autoAimParams.secondaryBoneId = rIDefaultSkeleton.GetJointIDByName(secondaryBoneName);

			tukk physicsBoneName;
			table.getAttr("PhysicsBoneName", &physicsBoneName);
			m_autoAimParams.physicsBoneId = rIDefaultSkeleton.GetJointIDByName(physicsBoneName);

			m_autoAimParams.hasSkeleton = true;
		}
	}

	table.getAttr("FallbackOffset", m_autoAimParams.fallbackOffset);
	table.getAttr("InnerRadius", m_autoAimParams.innerRadius);
	table.getAttr("OuterRadius", m_autoAimParams.outerRadius);
	table.getAttr("SnapRadius", m_autoAimParams.snapRadius);
	table.getAttr("SnapRadiusTagged", m_autoAimParams.snapRadiusTagged);

	return true;
}

void CVehicleActionAutoTarget::Reset()
{
	if(m_RegisteredWithAutoAimUpr)
	{
		g_pGame->GetAutoAimUpr().UnregisterAutoaimTarget(m_pVehicle->GetEntityId());
		m_RegisteredWithAutoAimUpr = false;
	}
}

void CVehicleActionAutoTarget::Release()
{
	Reset();

	delete this;
}

i32 CVehicleActionAutoTarget::OnEvent(i32 eventType, SVehicleEventParams& eventParams)
{
	return 1;
}

void CVehicleActionAutoTarget::OnVehicleEvent( EVehicleEvent event, const SVehicleEventParams& params )
{
	if( (event == eVE_Destroyed || event == eVE_PassengerExit) && m_RegisteredWithAutoAimUpr)
	{
		//No longer a target
		g_pGame->GetAutoAimUpr().UnregisterAutoaimTarget(m_pVehicle->GetEntityId());
		m_RegisteredWithAutoAimUpr = false;
	}
	else if(event == eVE_PassengerEnter && !m_RegisteredWithAutoAimUpr)
	{
		IActor* pDriver = m_pVehicle->GetDriver();
		if(pDriver && !pDriver->IsClient()) //No need to aim at yourself
		{
			g_pGame->GetAutoAimUpr().RegisterAutoaimTargetObject(m_pVehicle->GetEntityId(), m_autoAimParams);
			m_RegisteredWithAutoAimUpr = true;
		}
	}
}

i32 IVehicleAction::OnEvent( i32 eventType, SVehicleEventParams& eventParams )
{
	return 0;
}

DEFINE_VEHICLEOBJECT(CVehicleActionAutoTarget);
