#ifndef D3_TRANSFORM_UTIL_H
#define D3_TRANSFORM_UTIL_H

#include <drx3D/Common/b3Transform.h>
#define D3_ANGULAR_MOTION_THRESHOLD b3Scalar(0.5) * D3_HALF_PI

D3_FORCE_INLINE b3Vec3 b3AabbSupport(const b3Vec3& halfExtents, const b3Vec3& supportDir)
{
	return b3MakeVector3(supportDir.getX() < b3Scalar(0.0) ? -halfExtents.getX() : halfExtents.getX(),
						 supportDir.getY() < b3Scalar(0.0) ? -halfExtents.getY() : halfExtents.getY(),
						 supportDir.getZ() < b3Scalar(0.0) ? -halfExtents.getZ() : halfExtents.getZ());
}

/// Utils related to temporal transforms
class b3TransformUtil
{
public:
	static void integrateTransform(const b3Transform& curTrans, const b3Vec3& linvel, const b3Vec3& angvel, b3Scalar timeStep, b3Transform& predictedTransform)
	{
		predictedTransform.setOrigin(curTrans.getOrigin() + linvel * timeStep);
		//	#define QUATERNION_DERIVATIVE
#ifdef QUATERNION_DERIVATIVE
		b3Quat predictedOrn = curTrans.getRotation();
		predictedOrn += (angvel * predictedOrn) * (timeStep * b3Scalar(0.5));
		predictedOrn.normalize();
#else
		//Exponential map
		//google for "Practical Parameterization of Rotations Using the Exponential Map", F. Sebastian Grassia

		b3Vec3 axis;
		b3Scalar fAngle = angvel.length();
		//limit the angular motion
		if (fAngle * timeStep > D3_ANGULAR_MOTION_THRESHOLD)
		{
			fAngle = D3_ANGULAR_MOTION_THRESHOLD / timeStep;
		}

		if (fAngle < b3Scalar(0.001))
		{
			// use Taylor's expansions of sync function
			axis = angvel * (b3Scalar(0.5) * timeStep - (timeStep * timeStep * timeStep) * (b3Scalar(0.020833333333)) * fAngle * fAngle);
		}
		else
		{
			// sync(fAngle) = sin(c*fAngle)/t
			axis = angvel * (b3Sin(b3Scalar(0.5) * fAngle * timeStep) / fAngle);
		}
		b3Quat dorn(axis.getX(), axis.getY(), axis.getZ(), b3Cos(fAngle * timeStep * b3Scalar(0.5)));
		b3Quat orn0 = curTrans.getRotation();

		b3Quat predictedOrn = dorn * orn0;
		predictedOrn.normalize();
#endif
		predictedTransform.setRotation(predictedOrn);
	}

	static void calculateVelocityQuaternion(const b3Vec3& pos0, const b3Vec3& pos1, const b3Quat& orn0, const b3Quat& orn1, b3Scalar timeStep, b3Vec3& linVel, b3Vec3& angVel)
	{
		linVel = (pos1 - pos0) / timeStep;
		b3Vec3 axis;
		b3Scalar angle;
		if (orn0 != orn1)
		{
			calculateDiffAxisAngleQuaternion(orn0, orn1, axis, angle);
			angVel = axis * angle / timeStep;
		}
		else
		{
			angVel.setVal(0, 0, 0);
		}
	}

	static void calculateDiffAxisAngleQuaternion(const b3Quat& orn0, const b3Quat& orn1a, b3Vec3& axis, b3Scalar& angle)
	{
		b3Quat orn1 = orn0.nearest(orn1a);
		b3Quat dorn = orn1 * orn0.inverse();
		angle = dorn.getAngle();
		axis = b3MakeVector3(dorn.getX(), dorn.getY(), dorn.getZ());
		axis[3] = b3Scalar(0.);
		//check for axis length
		b3Scalar len = axis.length2();
		if (len < D3_EPSILON * D3_EPSILON)
			axis = b3MakeVector3(b3Scalar(1.), b3Scalar(0.), b3Scalar(0.));
		else
			axis /= b3Sqrt(len);
	}

	static void calculateVelocity(const b3Transform& transform0, const b3Transform& transform1, b3Scalar timeStep, b3Vec3& linVel, b3Vec3& angVel)
	{
		linVel = (transform1.getOrigin() - transform0.getOrigin()) / timeStep;
		b3Vec3 axis;
		b3Scalar angle;
		calculateDiffAxisAngle(transform0, transform1, axis, angle);
		angVel = axis * angle / timeStep;
	}

	static void calculateDiffAxisAngle(const b3Transform& transform0, const b3Transform& transform1, b3Vec3& axis, b3Scalar& angle)
	{
		b3Matrix3x3 dmat = transform1.getBasis() * transform0.getBasis().inverse();
		b3Quat dorn;
		dmat.getRotation(dorn);

		///floating point inaccuracy can lead to w component > 1..., which breaks
		dorn.normalize();

		angle = dorn.getAngle();
		axis = b3MakeVector3(dorn.getX(), dorn.getY(), dorn.getZ());
		axis[3] = b3Scalar(0.);
		//check for axis length
		b3Scalar len = axis.length2();
		if (len < D3_EPSILON * D3_EPSILON)
			axis = b3MakeVector3(b3Scalar(1.), b3Scalar(0.), b3Scalar(0.));
		else
			axis /= b3Sqrt(len);
	}
};

///The b3ConvexSeparatingDistanceUtil can help speed up convex collision detection
///by conservatively updating a cached separating distance/vector instead of re-calculating the closest distance
class b3ConvexSeparatingDistanceUtil
{
	b3Quat m_ornA;
	b3Quat m_ornB;
	b3Vec3 m_posA;
	b3Vec3 m_posB;

	b3Vec3 m_separatingNormal;

	b3Scalar m_boundingRadiusA;
	b3Scalar m_boundingRadiusB;
	b3Scalar m_separatingDistance;

public:
	b3ConvexSeparatingDistanceUtil(b3Scalar boundingRadiusA, b3Scalar boundingRadiusB)
		: m_boundingRadiusA(boundingRadiusA),
		  m_boundingRadiusB(boundingRadiusB),
		  m_separatingDistance(0.f)
	{
	}

	b3Scalar getConservativeSeparatingDistance()
	{
		return m_separatingDistance;
	}

	void updateSeparatingDistance(const b3Transform& transA, const b3Transform& transB)
	{
		const b3Vec3& toPosA = transA.getOrigin();
		const b3Vec3& toPosB = transB.getOrigin();
		b3Quat toOrnA = transA.getRotation();
		b3Quat toOrnB = transB.getRotation();

		if (m_separatingDistance > 0.f)
		{
			b3Vec3 linVelA, angVelA, linVelB, angVelB;
			b3TransformUtil::calculateVelocityQuaternion(m_posA, toPosA, m_ornA, toOrnA, b3Scalar(1.), linVelA, angVelA);
			b3TransformUtil::calculateVelocityQuaternion(m_posB, toPosB, m_ornB, toOrnB, b3Scalar(1.), linVelB, angVelB);
			b3Scalar maxAngularProjectedVelocity = angVelA.length() * m_boundingRadiusA + angVelB.length() * m_boundingRadiusB;
			b3Vec3 relLinVel = (linVelB - linVelA);
			b3Scalar relLinVelocLength = relLinVel.dot(m_separatingNormal);
			if (relLinVelocLength < 0.f)
			{
				relLinVelocLength = 0.f;
			}

			b3Scalar projectedMotion = maxAngularProjectedVelocity + relLinVelocLength;
			m_separatingDistance -= projectedMotion;
		}

		m_posA = toPosA;
		m_posB = toPosB;
		m_ornA = toOrnA;
		m_ornB = toOrnB;
	}

	void initSeparatingDistance(const b3Vec3& separatingVector, b3Scalar separatingDistance, const b3Transform& transA, const b3Transform& transB)
	{
		m_separatingDistance = separatingDistance;

		if (m_separatingDistance > 0.f)
		{
			m_separatingNormal = separatingVector;

			const b3Vec3& toPosA = transA.getOrigin();
			const b3Vec3& toPosB = transB.getOrigin();
			b3Quat toOrnA = transA.getRotation();
			b3Quat toOrnB = transB.getRotation();
			m_posA = toPosA;
			m_posB = toPosB;
			m_ornA = toOrnA;
			m_ornB = toOrnB;
		}
	}
};

#endif  //D3_TRANSFORM_UTIL_H
