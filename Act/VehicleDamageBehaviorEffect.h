// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEDAMAGEBEHAVIOREFFECT_H__
#define __VEHICLEDAMAGEBEHAVIOREFFECT_H__

#include <drx3D/Act/ISharedParams.h>

//! Spawn a particle effect
class CVehicleDamageBehaviorEffect : public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT
public:

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void Update(const float deltaTime) override;

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;

protected:

	BEGIN_SHARED_PARAMS(SSharedParams)

	string effectName;

	float damageRatioMin;

	bool  disableAfterExplosion;
	bool  updateFromHelper;

	END_SHARED_PARAMS

	void LoadEffect(IVehicleComponent* pComponent);
	void UpdateEffect(float randomness, float damageRatio);

	IVehicle*             m_pVehicle;

	SSharedParamsConstPtr m_pSharedParams;

	const SDamageEffect*  m_pDamageEffect;

	i32                   m_slot;
};

#endif //__VEHICLEDAMAGEBEHAVIOREFFECT_H__
