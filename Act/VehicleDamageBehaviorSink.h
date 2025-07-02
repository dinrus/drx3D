// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEDAMAGEBEHAVIORSINK_H__
#define __VEHICLEDAMAGEBEHAVIORSINK_H__

class CVehicle;

//! Sinks the vehicle if on water (typically a boat). This behaviour is applied when the vehicle is destroyed.
class CVehicleDamageBehaviorSink
	: public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleDamageBehaviorSink();
	virtual ~CVehicleDamageBehaviorSink() {}

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void Update(const float deltaTime) override {}

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override { s->Add(*this); }

	void         ChangeSinkingBehavior(bool isSinking);

protected:

	IVehicle* m_pVehicle;

	float     m_formerWaterDensity;
	i32       m_sinkingTimer;
	bool      m_isSinking;
};

#endif
