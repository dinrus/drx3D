// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Vehicle part class that spawns a particle effect and attaches it to the vehicle.

   -------------------------------------------------------------------------
   История:
   - 01:09:2010: Created by Paul Slinger

*************************************************************************/

#ifndef __VEHICLEPARTPARTICLEEFFECT_H__
#define __VEHICLEPARTPARTICLEEFFECT_H__

#include "IGameObject.h"
#include "VehiclePartBase.h"

class CVehiclePartParticleEffect : public CVehiclePartBase
{
public:

	IMPLEMENT_VEHICLEOBJECT

	CVehiclePartParticleEffect();

	~CVehiclePartParticleEffect();

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void PostInit() override;
	virtual void Reset() override;
	virtual void Update(const float frameTime) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;
	// PS - May need to implement SetLocalTM(), GetLocalTM(), GetWorldTM() and GetLocalBounds here, but that would require quite a lot of extra storage and calculations.
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;

	IEntity*     GetPartEntity() const;

private:

	void ActivateParticleEffect(bool activate);

	i32              m_id;

	string           m_particleEffectName, m_helperName;

	IParticleEffect* m_pParticleEffect;

	IVehicleHelper*  m_pHelper;
};

#endif //__VEHICLEPARTPARTICLEEFFECT_H__
