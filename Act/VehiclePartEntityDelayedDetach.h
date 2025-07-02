// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Описание: Subclass of VehiclePartEntity that can be asked to detach at a random point in the future

   -------------------------------------------------------------------------
   История:
   - 14:02:2012:	Created by Andrew Blackwell

*************************************************************************/

#ifndef __VEHICLEPARTENTITYDELAYEDDETACH_H__
#define __VEHICLEPARTENTITYDELAYEDDETACH_H__

//Base Class include
#include "VehiclePartEntity.h"

class CVehiclePartEntityDelayedDetach : public CVehiclePartEntity
{
public:
	IMPLEMENT_VEHICLEOBJECT

	CVehiclePartEntityDelayedDetach();
	virtual ~CVehiclePartEntityDelayedDetach();

	virtual void Update(const float frameTime) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

private:
	float m_detachTimer;
};

#endif //__VEHICLEPARTENTITYDELAYEDDETACH_H__
