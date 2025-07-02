// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLE_DAMAGE_BEHAVIOR_AUDIO_FEEDBACK_H__
#define __VEHICLE_DAMAGE_BEHAVIOR_AUDIO_FEEDBACK_H__

#include <drx3D/Game/Audio/AudioTypes.h>
#include <drx3D/Act/IVehicleSystem.h>

//! Implements a damage behavior that plays sounds when the vehicle is hit (different for first and third person).
class CVehicleDamageBehaviorAudioFeedback
	: public IVehicleDamageBehavior
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleDamageBehaviorAudioFeedback();
	virtual ~CVehicleDamageBehaviorAudioFeedback();

	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override {}
	virtual void Release() override { delete this; }

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override {}
	virtual void Update(const float deltaTime) override {}

	virtual void OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams) override {}
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override { s->Add(*this); }

protected:
	TAudioSignalID GetSignal() const;

	TAudioSignalID m_firstPersonSignal;
	TAudioSignalID m_thirdPersonSignal;
	EntityId m_vehicleId;
};

#endif //__VEHICLE_DAMAGE_BEHAVIOR_AUDIO_FEEDBACK_H__
