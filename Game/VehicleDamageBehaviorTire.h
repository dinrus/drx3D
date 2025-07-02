// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEDAMAGEBEHAVIORTIRE_H__
#define __VEHICLEDAMAGEBEHAVIORTIRE_H__

#include <drx3D/Act/IVehicleSystem.h>

class CVehicle;

//! Makes a wheel disappear with particle effects and a physics impulse.
class CVehicleDamageBehaviorBlowTire
	: public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleDamageBehaviorBlowTire();
	virtual ~CVehicleDamageBehaviorBlowTire() {}

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void Update(const float deltaTime) override;

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override;

protected:
	void Activate(bool activate);

	IVehicle* m_pVehicle;
	string m_component;
	bool m_isActive;
};

#endif
