// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLESEATACTIONROTATEBONE_H__
#define __VEHICLESEATACTIONROTATEBONE_H__

#include <drx3D/Animation/IDrxAnimation.h>

struct ISkeletonPose;
struct IAnimatedCharacter;

class CVehicleSeatActionRotateBone
	: public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT

private:

public:

	CVehicleSeatActionRotateBone();
	virtual ~CVehicleSeatActionRotateBone();

	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void StartUsing(EntityId passengerId) override;
	virtual void ForceUsage() override {}
	virtual void StopUsing() override;
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override;

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void PostSerialize() override               {}
	virtual void Update(const float deltaTime) override {}

	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override;

	virtual void PrePhysUpdate(const float dt) override;

protected:
	IDefaultSkeleton* GetCharacterModelSkeleton() const;
	void              UpdateSound(const float dt);

	//static tSoundID PlaySound(IEntityAudioComponent& rIEntityAudioComponent, tukk soundName);
	//static void SetSoundParam(IEntityAudioComponent& rIEntityAudioComponent, tSoundID soundID, tukk param, float value);
	//static void StopSound(IEntityAudioComponent& rIEntityAudioComponent, tSoundID soundID);
	IVehicle*                  m_pVehicle;
	IVehicleSeat*              m_pSeat;

	IAnimationOperatorQueuePtr m_poseModifier;

	i32                        m_MoveBoneId;

	Ang3                       m_boneRotation;
	Ang3                       m_prevBoneRotation;
	Quat                       m_boneBaseOrientation;
	Quat                       m_currentMovementRotation;
	Ang3                       m_desiredBoneRotation;
	Ang3                       m_rotationSmoothingRate;

	float                      m_soundRotSpeed;
	float                      m_soundRotLastAppliedSpeed;
	float                      m_soundRotSpeedSmoothRate;
	float                      m_soundRotSpeedSmoothTime;
	float                      m_soundRotSpeedScalar;
	string                     m_soundNameFP;
	string                     m_soundNameTP;
	string                     m_soundParamRotSpeed;

	float                      m_pitchLimitMax;
	float                      m_pitchLimitMin;
	float                      m_settlePitch;
	float                      m_settleDelay;
	float                      m_settleTime;
	float                      m_noDriverTime;

	float                      m_networkSluggishness;

	IAnimatedCharacter*        m_pAnimatedCharacter;

	float                      m_autoAimStrengthMultiplier;
	float                      m_speedMultiplier;
	float                      m_rotationMultiplier;

	//tSoundID m_rotatingSoundID;

	u8 m_hasDriver         : 1;
	u8 m_driverIsLocal     : 1;
	u8 m_settleBoneOnExit  : 1;
	u8 m_useRotationSounds : 1;
	u8 m_stopCurrentSound  : 1;
	u8 m_rotationLockCount : 2;
};

#endif // __VEHICLESEATACTIONROTATEBONE_H__
