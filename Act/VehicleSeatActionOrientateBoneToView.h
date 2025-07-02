// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLESEATACTIONORIENTATEBONETOVIEW_H__
#define __VEHICLESEATACTIONORIENTATEBONETOVIEW_H__

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Entity/IEntityComponent.h>

struct ISkeletonPose;
struct IAnimatedCharacter;

class CVehicleSeatActionOrientateBoneToView
	: public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT

private:

public:
	CVehicleSeatActionOrientateBoneToView();

	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void StartUsing(EntityId passengerId) override;
	virtual void ForceUsage() override                                                               {}
	virtual void StopUsing() override;
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override {}

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override                          {}
	virtual void PostSerialize() override                                                            {}
	virtual void Update(const float deltaTime) override                                              {}

	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override     {}

	virtual void PrePhysUpdate(const float dt) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override;

protected:
	Ang3              GetDesiredViewAngles(const Vec3& lookPos, const Vec3& aimPos) const;
	Vec3              GetDesiredAimPosition() const;
	Vec3              GetCurrentLookPosition() const;

	IDefaultSkeleton* GetCharacterModelSkeleton() const;
	ISkeletonPose*    GetSkeleton() const;

	IVehicle*                  m_pVehicle;
	IVehicleSeat*              m_pSeat;

	IAnimationOperatorQueuePtr m_poseModifier;

	i32                        m_MoveBoneId;
	i32                        m_LookBoneId;

	float                      m_Sluggishness;

	Ang3                       m_BoneOrientationAngles;
	Ang3                       m_BoneSmoothingRate;
	Quat                       m_BoneBaseOrientation;

	IAnimatedCharacter*        m_pAnimatedCharacter;
};

#endif // __VEHICLESEATACTIONORIENTATEBONETOVIEW_H__
