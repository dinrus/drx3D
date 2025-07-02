#ifndef DRX3D_RAYCASTVEHICLE_H
#define DRX3D_RAYCASTVEHICLE_H

#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/Vehicle/VehicleRaycaster.h>
class DynamicsWorld;
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/Vehicle/WheelInfo.h>
#include <drx3D/Physics/Dynamics/ActionInterface.h>

//class VehicleTuning;

///rayCast vehicle, very special constraint that turn a rigidbody into a vehicle.
class RaycastVehicle : public ActionInterface
{
	AlignedObjectArray<Vec3> m_forwardWS;
	AlignedObjectArray<Vec3> m_axle;
	AlignedObjectArray<Scalar> m_forwardImpulse;
	AlignedObjectArray<Scalar> m_sideImpulse;

	///backwards compatibility
	i32 m_userConstraintType;
	i32 m_userConstraintId;

public:
	class VehicleTuning
	{
	public:
		VehicleTuning()
			: m_suspensionStiffness(Scalar(5.88)),
			  m_suspensionCompression(Scalar(0.83)),
			  m_suspensionDamping(Scalar(0.88)),
			  m_maxSuspensionTravelCm(Scalar(500.)),
			  m_frictionSlip(Scalar(10.5)),
			  m_maxSuspensionForce(Scalar(6000.))
		{
		}
		Scalar m_suspensionStiffness;
		Scalar m_suspensionCompression;
		Scalar m_suspensionDamping;
		Scalar m_maxSuspensionTravelCm;
		Scalar m_frictionSlip;
		Scalar m_maxSuspensionForce;
	};

private:
	VehicleRaycaster* m_vehicleRaycaster;
	Scalar m_pitchControl;
	Scalar m_steeringValue;
	Scalar m_currentVehicleSpeedKmHour;

	RigidBody* m_chassisBody;

	i32 m_indexRightAxis;
	i32 m_indexUpAxis;
	i32 m_indexForwardAxis;

	void defaultInit(const VehicleTuning& tuning);

public:
	//constructor to create a car from an existing rigidbody
	RaycastVehicle(const VehicleTuning& tuning, RigidBody* chassis, VehicleRaycaster* raycaster);

	virtual ~RaycastVehicle();

	//ActionInterface interface
	virtual void updateAction(CollisionWorld* collisionWorld, Scalar step)
	{
		(void)collisionWorld;
		updateVehicle(step);
	}

	//ActionInterface interface
	void debugDraw(IDebugDraw* debugDrawer);

	const Transform2& getChassisWorldTransform() const;

	Scalar rayCast(WheelInfo& wheel);

	virtual void updateVehicle(Scalar step);

	void resetSuspension();

	Scalar getSteeringValue(i32 wheel) const;

	void setSteeringValue(Scalar steering, i32 wheel);

	void applyEngineForce(Scalar force, i32 wheel);

	const Transform2& getWheelTransform2WS(i32 wheelIndex) const;

	void updateWheelTransform(i32 wheelIndex, bool interpolatedTransform2 = true);

	//	void	setRaycastWheelInfo( i32 wheelIndex , bool isInContact, const Vec3& hitPoint, const Vec3& hitNormal,Scalar depth);

	WheelInfo& addWheel(const Vec3& connectionPointCS0, const Vec3& wheelDirectionCS0, const Vec3& wheelAxleCS, Scalar suspensionRestLength, Scalar wheelRadius, const VehicleTuning& tuning, bool isFrontWheel);

	inline i32 getNumWheels() const
	{
		return i32(m_wheelInfo.size());
	}

	AlignedObjectArray<WheelInfo> m_wheelInfo;

	const WheelInfo& getWheelInfo(i32 index) const;

	WheelInfo& getWheelInfo(i32 index);

	void updateWheelTransformsWS(WheelInfo& wheel, bool interpolatedTransform2 = true);

	void setBrake(Scalar brake, i32 wheelIndex);

	void setPitchControl(Scalar pitch)
	{
		m_pitchControl = pitch;
	}

	void updateSuspension(Scalar deltaTime);

	virtual void updateFriction(Scalar timeStep);

	inline RigidBody* getRigidBody()
	{
		return m_chassisBody;
	}

	const RigidBody* getRigidBody() const
	{
		return m_chassisBody;
	}

	inline i32 getRightAxis() const
	{
		return m_indexRightAxis;
	}
	inline i32 getUpAxis() const
	{
		return m_indexUpAxis;
	}

	inline i32 getForwardAxis() const
	{
		return m_indexForwardAxis;
	}

	///Worldspace forward vector
	Vec3 getForwardVector() const
	{
		const Transform2& chassisTrans = getChassisWorldTransform();

		Vec3 forwardW(
			chassisTrans.getBasis()[0][m_indexForwardAxis],
			chassisTrans.getBasis()[1][m_indexForwardAxis],
			chassisTrans.getBasis()[2][m_indexForwardAxis]);

		return forwardW;
	}

	///Velocity of vehicle (positive if velocity vector has same direction as foward vector)
	Scalar getCurrentSpeedKmHour() const
	{
		return m_currentVehicleSpeedKmHour;
	}

	virtual void setCoordinateSystem(i32 rightIndex, i32 upIndex, i32 forwardIndex)
	{
		m_indexRightAxis = rightIndex;
		m_indexUpAxis = upIndex;
		m_indexForwardAxis = forwardIndex;
	}

	///backwards compatibility
	i32 getUserConstraintType() const
	{
		return m_userConstraintType;
	}

	void setUserConstraintType(i32 userConstraintType)
	{
		m_userConstraintType = userConstraintType;
	};

	void setUserConstraintId(i32 uid)
	{
		m_userConstraintId = uid;
	}

	i32 getUserConstraintId() const
	{
		return m_userConstraintId;
	}
};

class DefaultVehicleRaycaster : public VehicleRaycaster
{
	DynamicsWorld* m_dynamicsWorld;

public:
	DefaultVehicleRaycaster(DynamicsWorld* world)
		: m_dynamicsWorld(world)
	{
	}

	virtual uk castRay(const Vec3& from, const Vec3& to, VehicleRaycasterResult& result);
};

#endif  //DRX3D_RAYCASTVEHICLE_H
