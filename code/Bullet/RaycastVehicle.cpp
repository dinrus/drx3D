#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/Vehicle/RaycastVehicle.h>

#include <drx3D/Physics/Dynamics/ConstraintSolver/Solve2LinearConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Physics/Dynamics/DynamicsWorld.h>
#include <drx3D/Physics/Dynamics/Vehicle/VehicleRaycaster.h>
#include <drx3D/Physics/Dynamics/Vehicle/WheelInfo.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactConstraint.h>

#define ROLLING_INFLUENCE_FIX

RigidBody& ActionInterface::getFixedBody()
{
	static RigidBody s_fixed(0, 0, 0);
	s_fixed.setMassProps(Scalar(0.), Vec3(Scalar(0.), Scalar(0.), Scalar(0.)));
	return s_fixed;
}

RaycastVehicle::RaycastVehicle(const VehicleTuning& tuning, RigidBody* chassis, VehicleRaycaster* raycaster)
	: m_vehicleRaycaster(raycaster),
	  m_pitchControl(Scalar(0.))
{
	m_chassisBody = chassis;
	m_indexRightAxis = 0;
	m_indexUpAxis = 2;
	m_indexForwardAxis = 1;
	defaultInit(tuning);
}

void RaycastVehicle::defaultInit(const VehicleTuning& tuning)
{
	(void)tuning;
	m_currentVehicleSpeedKmHour = Scalar(0.);
	m_steeringValue = Scalar(0.);
}

RaycastVehicle::~RaycastVehicle()
{
}

//
// basically most of the code is general for 2 or 4 wheel vehicles, but some of it needs to be reviewed
//
WheelInfo& RaycastVehicle::addWheel(const Vec3& connectionPointCS, const Vec3& wheelDirectionCS0, const Vec3& wheelAxleCS, Scalar suspensionRestLength, Scalar wheelRadius, const VehicleTuning& tuning, bool isFrontWheel)
{
	WheelInfoConstructionInfo ci;

	ci.m_chassisConnectionCS = connectionPointCS;
	ci.m_wheelDirectionCS = wheelDirectionCS0;
	ci.m_wheelAxleCS = wheelAxleCS;
	ci.m_suspensionRestLength = suspensionRestLength;
	ci.m_wheelRadius = wheelRadius;
	ci.m_suspensionStiffness = tuning.m_suspensionStiffness;
	ci.m_wheelsDampingCompression = tuning.m_suspensionCompression;
	ci.m_wheelsDampingRelaxation = tuning.m_suspensionDamping;
	ci.m_frictionSlip = tuning.m_frictionSlip;
	ci.m_bIsFrontWheel = isFrontWheel;
	ci.m_maxSuspensionTravelCm = tuning.m_maxSuspensionTravelCm;
	ci.m_maxSuspensionForce = tuning.m_maxSuspensionForce;

	m_wheelInfo.push_back(WheelInfo(ci));

	WheelInfo& wheel = m_wheelInfo[getNumWheels() - 1];

	updateWheelTransformsWS(wheel, false);
	updateWheelTransform(getNumWheels() - 1, false);
	return wheel;
}

const Transform2& RaycastVehicle::getWheelTransform2WS(i32 wheelIndex) const
{
	Assert(wheelIndex < getNumWheels());
	const WheelInfo& wheel = m_wheelInfo[wheelIndex];
	return wheel.m_worldTransform;
}

void RaycastVehicle::updateWheelTransform(i32 wheelIndex, bool interpolatedTransform2)
{
	WheelInfo& wheel = m_wheelInfo[wheelIndex];
	updateWheelTransformsWS(wheel, interpolatedTransform2);
	Vec3 up = -wheel.m_raycastInfo.m_wheelDirectionWS;
	const Vec3& right = wheel.m_raycastInfo.m_wheelAxleWS;
	Vec3 fwd = up.cross(right);
	fwd = fwd.normalize();
	//	up = right.cross(fwd);
	//	up.normalize();

	//rotate around steering over de wheelAxleWS
	Scalar steering = wheel.m_steering;

	Quat steeringOrn(up, steering);  //wheel.m_steering);
	Matrix3x3 steeringMat(steeringOrn);

	Quat rotatingOrn(right, -wheel.m_rotation);
	Matrix3x3 rotatingMat(rotatingOrn);

	Matrix3x3 basis2;
	basis2[0][m_indexRightAxis] = -right[0];
	basis2[1][m_indexRightAxis] = -right[1];
	basis2[2][m_indexRightAxis] = -right[2];

	basis2[0][m_indexUpAxis] = up[0];
	basis2[1][m_indexUpAxis] = up[1];
	basis2[2][m_indexUpAxis] = up[2];

	basis2[0][m_indexForwardAxis] = fwd[0];
	basis2[1][m_indexForwardAxis] = fwd[1];
	basis2[2][m_indexForwardAxis] = fwd[2];

	wheel.m_worldTransform.setBasis(steeringMat * rotatingMat * basis2);
	wheel.m_worldTransform.setOrigin(
		wheel.m_raycastInfo.m_hardPointWS + wheel.m_raycastInfo.m_wheelDirectionWS * wheel.m_raycastInfo.m_suspensionLength);
}

void RaycastVehicle::resetSuspension()
{
	i32 i;
	for (i = 0; i < m_wheelInfo.size(); i++)
	{
		WheelInfo& wheel = m_wheelInfo[i];
		wheel.m_raycastInfo.m_suspensionLength = wheel.getSuspensionRestLength();
		wheel.m_suspensionRelativeVelocity = Scalar(0.0);

		wheel.m_raycastInfo.m_contactNormalWS = -wheel.m_raycastInfo.m_wheelDirectionWS;
		//wheel_info.setContactFriction(Scalar(0.0));
		wheel.m_clippedInvContactDotSuspension = Scalar(1.0);
	}
}

void RaycastVehicle::updateWheelTransformsWS(WheelInfo& wheel, bool interpolatedTransform2)
{
	wheel.m_raycastInfo.m_isInContact = false;

	Transform2 chassisTrans = getChassisWorldTransform();
	if (interpolatedTransform2 && (getRigidBody()->getMotionState()))
	{
		getRigidBody()->getMotionState()->getWorldTransform(chassisTrans);
	}

	wheel.m_raycastInfo.m_hardPointWS = chassisTrans(wheel.m_chassisConnectionPointCS);
	wheel.m_raycastInfo.m_wheelDirectionWS = chassisTrans.getBasis() * wheel.m_wheelDirectionCS;
	wheel.m_raycastInfo.m_wheelAxleWS = chassisTrans.getBasis() * wheel.m_wheelAxleCS;
}

Scalar RaycastVehicle::rayCast(WheelInfo& wheel)
{
	updateWheelTransformsWS(wheel, false);

	Scalar depth = -1;

	Scalar raylen = wheel.getSuspensionRestLength() + wheel.m_wheelsRadius;

	Vec3 rayvector = wheel.m_raycastInfo.m_wheelDirectionWS * (raylen);
	const Vec3& source = wheel.m_raycastInfo.m_hardPointWS;
	wheel.m_raycastInfo.m_contactPointWS = source + rayvector;
	const Vec3& target = wheel.m_raycastInfo.m_contactPointWS;

	Scalar param = Scalar(0.);

	VehicleRaycaster::VehicleRaycasterResult rayResults;

	Assert(m_vehicleRaycaster);

	uk object = m_vehicleRaycaster->castRay(source, target, rayResults);

	wheel.m_raycastInfo.m_groundObject = 0;

	if (object)
	{
		param = rayResults.m_distFraction;
		depth = raylen * rayResults.m_distFraction;
		wheel.m_raycastInfo.m_contactNormalWS = rayResults.m_hitNormalInWorld;
		wheel.m_raycastInfo.m_isInContact = true;

		wheel.m_raycastInfo.m_groundObject = &getFixedBody();  ///@todo for driving on dynamic/movable objects!;
		//wheel.m_raycastInfo.m_groundObject = object;

		Scalar hitDistance = param * raylen;
		wheel.m_raycastInfo.m_suspensionLength = hitDistance - wheel.m_wheelsRadius;
		//clamp on max suspension travel

		Scalar minSuspensionLength = wheel.getSuspensionRestLength() - wheel.m_maxSuspensionTravelCm * Scalar(0.01);
		Scalar maxSuspensionLength = wheel.getSuspensionRestLength() + wheel.m_maxSuspensionTravelCm * Scalar(0.01);
		if (wheel.m_raycastInfo.m_suspensionLength < minSuspensionLength)
		{
			wheel.m_raycastInfo.m_suspensionLength = minSuspensionLength;
		}
		if (wheel.m_raycastInfo.m_suspensionLength > maxSuspensionLength)
		{
			wheel.m_raycastInfo.m_suspensionLength = maxSuspensionLength;
		}

		wheel.m_raycastInfo.m_contactPointWS = rayResults.m_hitPointInWorld;

		Scalar denominator = wheel.m_raycastInfo.m_contactNormalWS.dot(wheel.m_raycastInfo.m_wheelDirectionWS);

		Vec3 chassis_velocity_at_contactPoint;
		Vec3 relpos = wheel.m_raycastInfo.m_contactPointWS - getRigidBody()->getCenterOfMassPosition();

		chassis_velocity_at_contactPoint = getRigidBody()->getVelocityInLocalPoint(relpos);

		Scalar projVel = wheel.m_raycastInfo.m_contactNormalWS.dot(chassis_velocity_at_contactPoint);

		if (denominator >= Scalar(-0.1))
		{
			wheel.m_suspensionRelativeVelocity = Scalar(0.0);
			wheel.m_clippedInvContactDotSuspension = Scalar(1.0) / Scalar(0.1);
		}
		else
		{
			Scalar inv = Scalar(-1.) / denominator;
			wheel.m_suspensionRelativeVelocity = projVel * inv;
			wheel.m_clippedInvContactDotSuspension = inv;
		}
	}
	else
	{
		//put wheel info as in rest position
		wheel.m_raycastInfo.m_suspensionLength = wheel.getSuspensionRestLength();
		wheel.m_suspensionRelativeVelocity = Scalar(0.0);
		wheel.m_raycastInfo.m_contactNormalWS = -wheel.m_raycastInfo.m_wheelDirectionWS;
		wheel.m_clippedInvContactDotSuspension = Scalar(1.0);
	}

	return depth;
}

const Transform2& RaycastVehicle::getChassisWorldTransform() const
{
	/*if (getRigidBody()->getMotionState())
	{
		Transform2 chassisWorldTrans;
		getRigidBody()->getMotionState()->getWorldTransform(chassisWorldTrans);
		return chassisWorldTrans;
	}
	*/

	return getRigidBody()->getCenterOfMassTransform();
}

void RaycastVehicle::updateVehicle(Scalar step)
{
	{
		for (i32 i = 0; i < getNumWheels(); i++)
		{
			updateWheelTransform(i, false);
		}
	}

	m_currentVehicleSpeedKmHour = Scalar(3.6) * getRigidBody()->getLinearVelocity().length();

	const Transform2& chassisTrans = getChassisWorldTransform();

	Vec3 forwardW(
		chassisTrans.getBasis()[0][m_indexForwardAxis],
		chassisTrans.getBasis()[1][m_indexForwardAxis],
		chassisTrans.getBasis()[2][m_indexForwardAxis]);

	if (forwardW.dot(getRigidBody()->getLinearVelocity()) < Scalar(0.))
	{
		m_currentVehicleSpeedKmHour *= Scalar(-1.);
	}

	//
	// simulate suspension
	//

	i32 i = 0;
	for (i = 0; i < m_wheelInfo.size(); i++)
	{
		//Scalar depth;
		//depth =
		rayCast(m_wheelInfo[i]);
	}

	updateSuspension(step);

	for (i = 0; i < m_wheelInfo.size(); i++)
	{
		//apply suspension force
		WheelInfo& wheel = m_wheelInfo[i];

		Scalar suspensionForce = wheel.m_wheelsSuspensionForce;

		if (suspensionForce > wheel.m_maxSuspensionForce)
		{
			suspensionForce = wheel.m_maxSuspensionForce;
		}
		Vec3 impulse = wheel.m_raycastInfo.m_contactNormalWS * suspensionForce * step;
		Vec3 relpos = wheel.m_raycastInfo.m_contactPointWS - getRigidBody()->getCenterOfMassPosition();

		getRigidBody()->applyImpulse(impulse, relpos);
	}

	updateFriction(step);

	for (i = 0; i < m_wheelInfo.size(); i++)
	{
		WheelInfo& wheel = m_wheelInfo[i];
		Vec3 relpos = wheel.m_raycastInfo.m_hardPointWS - getRigidBody()->getCenterOfMassPosition();
		Vec3 vel = getRigidBody()->getVelocityInLocalPoint(relpos);

		if (wheel.m_raycastInfo.m_isInContact)
		{
			const Transform2& chassisWorldTransform = getChassisWorldTransform();

			Vec3 fwd(
				chassisWorldTransform.getBasis()[0][m_indexForwardAxis],
				chassisWorldTransform.getBasis()[1][m_indexForwardAxis],
				chassisWorldTransform.getBasis()[2][m_indexForwardAxis]);

			Scalar proj = fwd.dot(wheel.m_raycastInfo.m_contactNormalWS);
			fwd -= wheel.m_raycastInfo.m_contactNormalWS * proj;

			Scalar proj2 = fwd.dot(vel);

			wheel.m_deltaRotation = (proj2 * step) / (wheel.m_wheelsRadius);
			wheel.m_rotation += wheel.m_deltaRotation;
		}
		else
		{
			wheel.m_rotation += wheel.m_deltaRotation;
		}

		wheel.m_deltaRotation *= Scalar(0.99);  //damping of rotation when not in contact
	}
}

void RaycastVehicle::setSteeringValue(Scalar steering, i32 wheel)
{
	Assert(wheel >= 0 && wheel < getNumWheels());

	WheelInfo& wheelInfo = getWheelInfo(wheel);
	wheelInfo.m_steering = steering;
}

Scalar RaycastVehicle::getSteeringValue(i32 wheel) const
{
	return getWheelInfo(wheel).m_steering;
}

void RaycastVehicle::applyEngineForce(Scalar force, i32 wheel)
{
	Assert(wheel >= 0 && wheel < getNumWheels());
	WheelInfo& wheelInfo = getWheelInfo(wheel);
	wheelInfo.m_engineForce = force;
}

const WheelInfo& RaycastVehicle::getWheelInfo(i32 index) const
{
	Assert((index >= 0) && (index < getNumWheels()));

	return m_wheelInfo[index];
}

WheelInfo& RaycastVehicle::getWheelInfo(i32 index)
{
	Assert((index >= 0) && (index < getNumWheels()));

	return m_wheelInfo[index];
}

void RaycastVehicle::setBrake(Scalar brake, i32 wheelIndex)
{
	Assert((wheelIndex >= 0) && (wheelIndex < getNumWheels()));
	getWheelInfo(wheelIndex).m_brake = brake;
}

void RaycastVehicle::updateSuspension(Scalar deltaTime)
{
	(void)deltaTime;

	Scalar chassisMass = Scalar(1.) / m_chassisBody->getInvMass();

	for (i32 w_it = 0; w_it < getNumWheels(); w_it++)
	{
		WheelInfo& wheel_info = m_wheelInfo[w_it];

		if (wheel_info.m_raycastInfo.m_isInContact)
		{
			Scalar force;
			//	Spring
			{
				Scalar susp_length = wheel_info.getSuspensionRestLength();
				Scalar current_length = wheel_info.m_raycastInfo.m_suspensionLength;

				Scalar length_diff = (susp_length - current_length);

				force = wheel_info.m_suspensionStiffness * length_diff * wheel_info.m_clippedInvContactDotSuspension;
			}

			// Damper
			{
				Scalar projected_rel_vel = wheel_info.m_suspensionRelativeVelocity;
				{
					Scalar susp_damping;
					if (projected_rel_vel < Scalar(0.0))
					{
						susp_damping = wheel_info.m_wheelsDampingCompression;
					}
					else
					{
						susp_damping = wheel_info.m_wheelsDampingRelaxation;
					}
					force -= susp_damping * projected_rel_vel;
				}
			}

			// RESULT
			wheel_info.m_wheelsSuspensionForce = force * chassisMass;
			if (wheel_info.m_wheelsSuspensionForce < Scalar(0.))
			{
				wheel_info.m_wheelsSuspensionForce = Scalar(0.);
			}
		}
		else
		{
			wheel_info.m_wheelsSuspensionForce = Scalar(0.0);
		}
	}
}

struct WheelContactPoint
{
	RigidBody* m_body0;
	RigidBody* m_body1;
	Vec3 m_frictionPositionWorld;
	Vec3 m_frictionDirectionWorld;
	Scalar m_jacDiagABInv;
	Scalar m_maxImpulse;

	WheelContactPoint(RigidBody* body0, RigidBody* body1, const Vec3& frictionPosWorld, const Vec3& frictionDirectionWorld, Scalar maxImpulse)
		: m_body0(body0),
		  m_body1(body1),
		  m_frictionPositionWorld(frictionPosWorld),
		  m_frictionDirectionWorld(frictionDirectionWorld),
		  m_maxImpulse(maxImpulse)
	{
		Scalar denom0 = body0->computeImpulseDenominator(frictionPosWorld, frictionDirectionWorld);
		Scalar denom1 = body1->computeImpulseDenominator(frictionPosWorld, frictionDirectionWorld);
		Scalar relaxation = 1.f;
		m_jacDiagABInv = relaxation / (denom0 + denom1);
	}
};

Scalar calcRollingFriction(WheelContactPoint& contactPoint, i32 numWheelsOnGround);
Scalar calcRollingFriction(WheelContactPoint& contactPoint, i32 numWheelsOnGround)
{
	Scalar j1 = 0.f;

	const Vec3& contactPosWorld = contactPoint.m_frictionPositionWorld;

	Vec3 rel_pos1 = contactPosWorld - contactPoint.m_body0->getCenterOfMassPosition();
	Vec3 rel_pos2 = contactPosWorld - contactPoint.m_body1->getCenterOfMassPosition();

	Scalar maxImpulse = contactPoint.m_maxImpulse;

	Vec3 vel1 = contactPoint.m_body0->getVelocityInLocalPoint(rel_pos1);
	Vec3 vel2 = contactPoint.m_body1->getVelocityInLocalPoint(rel_pos2);
	Vec3 vel = vel1 - vel2;

	Scalar vrel = contactPoint.m_frictionDirectionWorld.dot(vel);

	// calculate j that moves us to zero relative velocity
	j1 = -vrel * contactPoint.m_jacDiagABInv / Scalar(numWheelsOnGround);
	SetMin(j1, maxImpulse);
	SetMax(j1, -maxImpulse);

	return j1;
}

Scalar sideFrictionStiffness2 = Scalar(1.0);
void RaycastVehicle::updateFriction(Scalar timeStep)
{
	//calculate the impulse, so that the wheels don't move sidewards
	i32 numWheel = getNumWheels();
	if (!numWheel)
		return;

	m_forwardWS.resize(numWheel);
	m_axle.resize(numWheel);
	m_forwardImpulse.resize(numWheel);
	m_sideImpulse.resize(numWheel);

	i32 numWheelsOnGround = 0;

	//collapse all those loops into one!
	for (i32 i = 0; i < getNumWheels(); i++)
	{
		WheelInfo& wheelInfo = m_wheelInfo[i];
		class RigidBody* groundObject = (class RigidBody*)wheelInfo.m_raycastInfo.m_groundObject;
		if (groundObject)
			numWheelsOnGround++;
		m_sideImpulse[i] = Scalar(0.);
		m_forwardImpulse[i] = Scalar(0.);
	}

	{
		for (i32 i = 0; i < getNumWheels(); i++)
		{
			WheelInfo& wheelInfo = m_wheelInfo[i];

			class RigidBody* groundObject = (class RigidBody*)wheelInfo.m_raycastInfo.m_groundObject;

			if (groundObject)
			{
				const Transform2& wheelTrans = getWheelTransform2WS(i);

				Matrix3x3 wheelBasis0 = wheelTrans.getBasis();
				m_axle[i] = -Vec3(
					wheelBasis0[0][m_indexRightAxis],
					wheelBasis0[1][m_indexRightAxis],
					wheelBasis0[2][m_indexRightAxis]);

				const Vec3& surfNormalWS = wheelInfo.m_raycastInfo.m_contactNormalWS;
				Scalar proj = m_axle[i].dot(surfNormalWS);
				m_axle[i] -= surfNormalWS * proj;
				m_axle[i] = m_axle[i].normalize();

				m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
				m_forwardWS[i].normalize();

				resolveSingleBilateral(*m_chassisBody, wheelInfo.m_raycastInfo.m_contactPointWS,
									   *groundObject, wheelInfo.m_raycastInfo.m_contactPointWS,
									   Scalar(0.), m_axle[i], m_sideImpulse[i], timeStep);

				m_sideImpulse[i] *= sideFrictionStiffness2;
			}
		}
	}

	Scalar sideFactor = Scalar(1.);
	Scalar fwdFactor = 0.5;

	bool sliding = false;
	{
		for (i32 wheel = 0; wheel < getNumWheels(); wheel++)
		{
			WheelInfo& wheelInfo = m_wheelInfo[wheel];
			class RigidBody* groundObject = (class RigidBody*)wheelInfo.m_raycastInfo.m_groundObject;

			Scalar rollingFriction = 0.f;

			if (groundObject)
			{
				if (wheelInfo.m_engineForce != 0.f)
				{
					rollingFriction = wheelInfo.m_engineForce * timeStep;
				}
				else
				{
					Scalar defaultRollingFrictionImpulse = 0.f;
					Scalar maxImpulse = wheelInfo.m_brake ? wheelInfo.m_brake : defaultRollingFrictionImpulse;
					WheelContactPoint contactPt(m_chassisBody, groundObject, wheelInfo.m_raycastInfo.m_contactPointWS, m_forwardWS[wheel], maxImpulse);
					Assert(numWheelsOnGround > 0);
					rollingFriction = calcRollingFriction(contactPt, numWheelsOnGround);
				}
			}

			//switch between active rolling (throttle), braking and non-active rolling friction (no throttle/break)

			m_forwardImpulse[wheel] = Scalar(0.);
			m_wheelInfo[wheel].m_skidInfo = Scalar(1.);

			if (groundObject)
			{
				m_wheelInfo[wheel].m_skidInfo = Scalar(1.);

				Scalar maximp = wheelInfo.m_wheelsSuspensionForce * timeStep * wheelInfo.m_frictionSlip;
				Scalar maximpSide = maximp;

				Scalar maximpSquared = maximp * maximpSide;

				m_forwardImpulse[wheel] = rollingFriction;  //wheelInfo.m_engineForce* timeStep;

				Scalar x = (m_forwardImpulse[wheel]) * fwdFactor;
				Scalar y = (m_sideImpulse[wheel]) * sideFactor;

				Scalar impulseSquared = (x * x + y * y);

				if (impulseSquared > maximpSquared)
				{
					sliding = true;

					Scalar factor = maximp / Sqrt(impulseSquared);

					m_wheelInfo[wheel].m_skidInfo *= factor;
				}
			}
		}
	}

	if (sliding)
	{
		for (i32 wheel = 0; wheel < getNumWheels(); wheel++)
		{
			if (m_sideImpulse[wheel] != Scalar(0.))
			{
				if (m_wheelInfo[wheel].m_skidInfo < Scalar(1.))
				{
					m_forwardImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
					m_sideImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
				}
			}
		}
	}

	// apply the impulses
	{
		for (i32 wheel = 0; wheel < getNumWheels(); wheel++)
		{
			WheelInfo& wheelInfo = m_wheelInfo[wheel];

			Vec3 rel_pos = wheelInfo.m_raycastInfo.m_contactPointWS -
								m_chassisBody->getCenterOfMassPosition();

			if (m_forwardImpulse[wheel] != Scalar(0.))
			{
				m_chassisBody->applyImpulse(m_forwardWS[wheel] * (m_forwardImpulse[wheel]), rel_pos);
			}
			if (m_sideImpulse[wheel] != Scalar(0.))
			{
				class RigidBody* groundObject = (class RigidBody*)m_wheelInfo[wheel].m_raycastInfo.m_groundObject;

				Vec3 rel_pos2 = wheelInfo.m_raycastInfo.m_contactPointWS -
									 groundObject->getCenterOfMassPosition();

				Vec3 sideImp = m_axle[wheel] * m_sideImpulse[wheel];

#if defined ROLLING_INFLUENCE_FIX  // fix. It only worked if car's up was along Y - VT.
				Vec3 vChassisWorldUp = getRigidBody()->getCenterOfMassTransform().getBasis().getColumn(m_indexUpAxis);
				rel_pos -= vChassisWorldUp * (vChassisWorldUp.dot(rel_pos) * (1.f - wheelInfo.m_rollInfluence));
#else
				rel_pos[m_indexUpAxis] *= wheelInfo.m_rollInfluence;
#endif
				m_chassisBody->applyImpulse(sideImp, rel_pos);

				//apply friction impulse on the ground
				groundObject->applyImpulse(-sideImp, rel_pos2);
			}
		}
	}
}

void RaycastVehicle::debugDraw(IDebugDraw* debugDrawer)
{
	for (i32 v = 0; v < this->getNumWheels(); v++)
	{
		Vec3 wheelColor(0, 1, 1);
		if (getWheelInfo(v).m_raycastInfo.m_isInContact)
		{
			wheelColor.setVal(0, 0, 1);
		}
		else
		{
			wheelColor.setVal(1, 0, 1);
		}

		Vec3 wheelPosWS = getWheelInfo(v).m_worldTransform.getOrigin();

		Vec3 axle = Vec3(
			getWheelInfo(v).m_worldTransform.getBasis()[0][getRightAxis()],
			getWheelInfo(v).m_worldTransform.getBasis()[1][getRightAxis()],
			getWheelInfo(v).m_worldTransform.getBasis()[2][getRightAxis()]);

		//debug wheels (cylinders)
		debugDrawer->drawLine(wheelPosWS, wheelPosWS + axle, wheelColor);
		debugDrawer->drawLine(wheelPosWS, getWheelInfo(v).m_raycastInfo.m_contactPointWS, wheelColor);
	}
}

uk DefaultVehicleRaycaster::castRay(const Vec3& from, const Vec3& to, VehicleRaycasterResult& result)
{
	//	RayResultCallback& resultCallback;

	CollisionWorld::ClosestRayResultCallback rayCallback(from, to);

	m_dynamicsWorld->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit())
	{
		const RigidBody* body = RigidBody::upcast(rayCallback.m_collisionObject);
		if (body && body->hasContactResponse())
		{
			result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
			result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = rayCallback.m_closestHitFraction;
			return (uk )body;
		}
	}
	return 0;
}
