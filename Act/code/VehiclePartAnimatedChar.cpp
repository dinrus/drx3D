// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a part for vehicles which uses animated characters

   -------------------------------------------------------------------------
   История:
   - 24:08:2011: Created by Richard Semmens

*************************************************************************/
#include <drx3D/Act/StdAfx.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IVehicleSystem.h>

#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehiclePartBase.h>
#include <drx3D/Act/VehiclePartAnimated.h>
#include <drx3D/Act/VehiclePartAnimatedChar.h>
#include <drx3D/Act/VehicleUtils.h>

//#pragma optimize("", off)
//#pragma inline_depth(0)

//------------------------------------------------------------------------
CVehiclePartAnimatedChar::CVehiclePartAnimatedChar()
{
}

//------------------------------------------------------------------------
CVehiclePartAnimatedChar::~CVehiclePartAnimatedChar()
{
}

//------------------------------------------------------------------------
void CVehiclePartAnimatedChar::Physicalize()
{
	if (m_pSharedParameters->m_isPhysicalized && GetEntity()->GetPhysics())
	{
		if (m_slot != -1)
			GetEntity()->UnphysicalizeSlot(m_slot);

		SEntityPhysicalizeParams params;
		params.mass = m_pSharedParameters->m_mass;
		params.density = m_pSharedParameters->m_density;

		//TODO: Check civcar for 'proper' slot allocation
		params.nSlot = 0; //m_slot;
		params.type = PE_LIVING;
		GetEntity()->PhysicalizeSlot(m_slot, params); // always returns -1 for chars

		if (m_pCharInstance)
		{
			FlagSkeleton(m_pCharInstance->GetISkeletonPose(), m_pCharInstance->GetIDefaultSkeleton());
		}

		m_pVehicle->RequestPhysicalization(this, false);
	}

	GetEntity()->SetSlotFlags(m_slot, GetEntity()->GetSlotFlags(m_slot) | ENTITY_SLOT_IGNORE_PHYSICS);
}

DEFINE_VEHICLEOBJECT(CVehiclePartAnimatedChar);
