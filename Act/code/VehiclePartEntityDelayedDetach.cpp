// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Описание: Subclass of VehiclePartEntity that can be asked to detach at a random point in the future

   -------------------------------------------------------------------------
   История:
   - 14:02:2012:	Created by Andrew Blackwell

*************************************************************************/

#include <drx3D/Act/StdAfx.h>

//This Include
#include <drx3D/Act/VehiclePartEntityDelayedDetach.h>

//------------------------------------------------------------------------
CVehiclePartEntityDelayedDetach::CVehiclePartEntityDelayedDetach()
	: CVehiclePartEntity()
	, m_detachTimer(-1.0f)
{
}

//------------------------------------------------------------------------
CVehiclePartEntityDelayedDetach::~CVehiclePartEntityDelayedDetach()
{
}

//------------------------------------------------------------------------
void CVehiclePartEntityDelayedDetach::Update(const float frameTime)
{
	CVehiclePartEntity::Update(frameTime);

	if (EntityAttached() && m_detachTimer >= 0.0f)
	{
		m_detachTimer -= frameTime;

		if (m_detachTimer <= 0.0f)
		{
			SVehicleEventParams vehicleEventParams;
			vehicleEventParams.entityId = GetPartEntityId();
			vehicleEventParams.iParam = m_pVehicle->GetEntityId();

			m_pVehicle->BroadcastVehicleEvent(eVE_OnDetachPartEntity, vehicleEventParams);

			m_detachTimer = -1.0f;
		}
	}
}

//------------------------------------------------------------------------
void CVehiclePartEntityDelayedDetach::OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params)
{
	CVehiclePartEntity::OnVehicleEvent(event, params);

	switch (event)
	{
	case eVE_RequestDelayedDetachAllPartEntities:
		{
			//we're a part entity, so want to detach.
			//don't reset timer if all ready set
			if (m_detachTimer < 0.0f && EntityAttached())
			{
				//random time between min + max wait
				m_detachTimer = drx_random(params.fParam, params.fParam2);
			}
			break;
		}

	case eVE_Sleep:
		{
			//if we were scheduled to delay detach, do so now as we won't receive further updates
			if (m_detachTimer >= 0.0f && EntityAttached())
			{
				m_detachTimer = -1.0f;

				SVehicleEventParams vehicleEventParams;

				vehicleEventParams.entityId = GetPartEntityId();
				vehicleEventParams.iParam = m_pVehicle->GetEntityId();

				m_pVehicle->BroadcastVehicleEvent(eVE_OnDetachPartEntity, vehicleEventParams);
			}
			break;
		}
	}
}

//------------------------------------------------------------------------
DEFINE_VEHICLEOBJECT(CVehiclePartEntityDelayedDetach)
