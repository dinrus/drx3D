// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEDAMAGEBEHAVIORMOVEMENTNOTIFICATION_H__
#define __VEHICLEDAMAGEBEHAVIORMOVEMENTNOTIFICATION_H__

class CVehicle;

//! Sends a notification to the active movement type on Hit and Repair.
class CVehicleDamageBehaviorMovementNotification
	: public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleDamageBehaviorMovementNotification();
	virtual ~CVehicleDamageBehaviorMovementNotification() {}

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override                                         { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override {}
	virtual void Update(const float deltaTime) override                     {}

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}

	virtual void GetMemoryUsage(IDrxSizer* s) const override                                     { s->Add(*this); }

protected:

	IVehicle* m_pVehicle;

	bool      m_isSteeringInvolved;
	bool      m_isFatal;
	bool      m_isDamageAlwaysFull;

	i32       m_param;
};

#endif
