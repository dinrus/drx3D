// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements water ripple generation for boats and similar vehicles

   -------------------------------------------------------------------------
   История:
   - 04:06:2012: Created by Benito G.R.

*************************************************************************/
#ifndef __VEHICLEPAR_RIPPLEGENERATOR_H__
#define __VEHICLEPAR_RIPPLEGENERATOR_H__

#include "VehiclePartBase.h"

class CVehicle;

class CVehiclePartWaterRipplesGenerator
	: public CVehiclePartBase
{
	IMPLEMENT_VEHICLEOBJECT

public:

	CVehiclePartWaterRipplesGenerator();
	virtual ~CVehiclePartWaterRipplesGenerator();

	// IVehiclePart
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void PostInit() override;
	virtual void Update(const float frameTime) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override
	{
		s->Add(*this);
		CVehiclePartBase::GetMemoryUsageInternal(s);
	}
	// ~IVehiclePart

private:

	Vec3      m_localOffset;
	float     m_waterRipplesScale;
	float     m_waterRipplesStrength;
	float     m_minMovementSpeed;
	bool      m_onlyMovingForward;
};

#endif //__VEHICLEPAR_RIPPLEGENERATOR_H__
