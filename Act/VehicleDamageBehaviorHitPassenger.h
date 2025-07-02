// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEDAMAGEHITPASSENGER_H__
#define __VEHICLEDAMAGEHITPASSENGER_H__

class CVehicle;

//! Applies damage to all actors seating in the vehicle for every hit the vehicle receives
class CVehicleDamageBehaviorHitPassenger
	: public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT

public:

	CVehicleDamageBehaviorHitPassenger();
	virtual ~CVehicleDamageBehaviorHitPassenger() {}

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override                                           {}
	virtual void Release() override                                         { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override {}
	virtual void Update(const float deltaTime) override                     {}

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}

	virtual void GetMemoryUsage(IDrxSizer* s) const override;

protected:

	IVehicle* m_pVehicle;

	i32       m_damage;
	bool      m_isDamagePercent;
};

#endif
