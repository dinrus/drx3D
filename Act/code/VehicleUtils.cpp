// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements several utility functions for vehicles

   -------------------------------------------------------------------------
   История:

*************************************************************************/
#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/VehicleUtils.h>

#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/PersistantDebug.h>

#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleAnimation.h>
#include <drx3D/Act/VehicleCVars.h>
#include <drx3D/Act/VehicleComponent.h>
#include <drx3D/Act/VehicleHelper.h>
#include <drx3D/Act/VehicleSeat.h>
#include <drx3D/Act/VehicleSeatGroup.h>
#include <drx3D/Act/VehicleSeatActionWeapons.h>

//------------------------------------------------------------------------
void VehicleUtils::DumpDamageBehaviorEvent(const SVehicleDamageBehaviorEventParams& params)
{
#if ENABLE_VEHICLE_DEBUG
	DrxLog("=================================");
	DrxLog("SVehicleDamageBehaviorEventParams");
	DrxLog("localPos (%.2f, %.2f, %.2f)", params.localPos.x, params.localPos.y, params.localPos.z);
	DrxLog("shooterId: %u", params.shooterId);
	DrxLog("radius: %.2f", params.radius);
	DrxLog("hitValue: %.2f", params.hitValue);
	DrxLog("componentDamageRatio: %.2f", params.componentDamageRatio);
	DrxLog("randomness: %.1f", params.randomness);

	if (params.pVehicleComponent)
		DrxLog("pVehicleComponent: <%s>", ((CVehicleComponent*)params.pVehicleComponent)->GetName().c_str());
#endif
}

//------------------------------------------------------------------------
void VehicleUtils::DrawTM(const Matrix34& tm, tukk name, bool clear)
{
#if ENABLE_VEHICLE_DEBUG
	IPersistantDebug* pDebug = CDrxAction::GetDrxAction()->GetIPersistantDebug();
	pDebug->Begin(name, clear);

	const static ColorF red(1, 0, 0, 1);
	const static ColorF green(0, 1, 0, 1);
	const static ColorF blue(0, 0, 1, 1);

	float timeout = 0.1f;
	float radius = 1.5f;

	pDebug->AddDirection(tm.GetTranslation(), radius, tm.GetColumn(0), red, timeout);
	pDebug->AddDirection(tm.GetTranslation(), radius, tm.GetColumn(1), green, timeout);
	pDebug->AddDirection(tm.GetTranslation(), radius, tm.GetColumn(2), blue, timeout);
#endif
}

//------------------------------------------------------------------------
void VehicleUtils::DumpSlots(IVehicle* pVehicle)
{
#if ENABLE_VEHICLE_DEBUG
	if (VehicleCVars().v_debugdraw)
	{
		IEntity* pEntity = pVehicle->GetEntity();

		for (i32 i = 0; i < pEntity->GetSlotCount(); ++i)
		{
			SEntitySlotInfo info;
			if (pEntity->GetSlotInfo(i, info))
			{
				if (info.pCharacter)
					DrxLog("slot %i: <char> %s", i, info.pCharacter->GetFilePath());
				else if (info.pStatObj)
					DrxLog("slot %i: <statobj> %s", i, info.pStatObj->GetGeoName());
				else if (info.pParticleEmitter)
					DrxLog("slot %i: <particle> %s", i, info.pParticleEmitter->GetName());
				else
					DrxLog("slot %i: <UNKNOWN>", i);
			}
		}
	}
#endif
}
