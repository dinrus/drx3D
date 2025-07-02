#ifndef DRX3D_TRANSFORM_UTIL_H
#define DRX3D_TRANSFORM_UTIL_H

#include <drx3D/Maths/Linear/Transform2.h>
#define ANGULAR_MOTION_THRESHOLD Scalar(0.5) * SIMD_HALF_PI

SIMD_FORCE_INLINE Vec3 AabbSupport(const Vec3& halfExtents, const Vec3& supportDir)
{
	return Vec3(supportDir.x() < Scalar(0.0) ? -halfExtents.x() : halfExtents.x(),
					 supportDir.y() < Scalar(0.0) ? -halfExtents.y() : halfExtents.y(),
					 supportDir.z() < Scalar(0.0) ? -halfExtents.z() : halfExtents.z());
}

/// Utils related to temporal transforms
class Transform2Util
{
public:
	static void integrateTransform(const Transform2& curTrans, const Vec3& linvel, const Vec3& angvel, Scalar timeStep, Transform2& predictedTransform2)
	{
		predictedTransform2.setOrigin(curTrans.getOrigin() + linvel * timeStep);
		//	#define QUATERNION_DERIVATIVE
#ifdef QUATERNION_DERIVATIVE
		Quat predictedOrn = curTrans.getRotation();
		predictedOrn += (angvel * predictedOrn) * (timeStep * Scalar(0.5));
		predictedOrn.safeNormalize();
#else
		//Exponential map
		//google for "Practical Parameterization of Rotations Using the Exponential Map", F. Sebastian Grassia

		Vec3 axis;
		Scalar fAngle2 = angvel.length2();
		Scalar fAngle = 0;
		if (fAngle2 > SIMD_EPSILON)
		{
			fAngle = Sqrt(fAngle2);
		}

		//limit the angular motion
		if (fAngle * timeStep > ANGULAR_MOTION_THRESHOLD)
		{
			fAngle = ANGULAR_MOTION_THRESHOLD / timeStep;
		}

		if (fAngle < Scalar(0.001))
		{
			// use Taylor's expansions of sync function
			axis = angvel * (Scalar(0.5) * timeStep - (timeStep * timeStep * timeStep) * (Scalar(0.020833333333)) * fAngle * fAngle);
		}
		else
		{
			// sync(fAngle) = sin(c*fAngle)/t
			axis = angvel * (Sin(Scalar(0.5) * fAngle * timeStep) / fAngle);
		}
		Quat dorn(axis.x(), axis.y(), axis.z(), Cos(fAngle * timeStep * Scalar(0.5)));
		Quat orn0 = curTrans.getRotation();

		Quat predictedOrn = dorn * orn0;
		predictedOrn.safeNormalize();
#endif
		if (predictedOrn.length2() > SIMD_EPSILON)
		{
			predictedTransform2.setRotation(predictedOrn);
		}
		else
		{
			predictedTransform2.setBasis(curTrans.getBasis());
		}
	}

	static void calculateVelocityQuat(const Vec3& pos0, const Vec3& pos1, const Quat& orn0, const Quat& orn1, Scalar timeStep, Vec3& linVel, Vec3& angVel)
	{
		linVel = (pos1 - pos0) / timeStep;
		Vec3 axis;
		Scalar angle;
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

	static void calculateDiffAxisAngleQuaternion(const Quat& orn0, const Quat& orn1a, Vec3& axis, Scalar& angle)
	{
		Quat orn1 = orn0.nearest(orn1a);
		Quat dorn = orn1 * orn0.inverse();
		angle = dorn.getAngle();
		axis = Vec3(dorn.x(), dorn.y(), dorn.z());
		axis[3] = Scalar(0.);
		//check for axis length
		Scalar len = axis.length2();
		if (len < SIMD_EPSILON * SIMD_EPSILON)
			axis = Vec3(Scalar(1.), Scalar(0.), Scalar(0.));
		else
			axis /= Sqrt(len);
	}

	static void calculateVelocity(const Transform2& transform0, const Transform2& transform1, Scalar timeStep, Vec3& linVel, Vec3& angVel)
	{
		linVel = (transform1.getOrigin() - transform0.getOrigin()) / timeStep;
		Vec3 axis;
		Scalar angle;
		calculateDiffAxisAngle(transform0, transform1, axis, angle);
		angVel = axis * angle / timeStep;
	}

	static void calculateDiffAxisAngle(const Transform2& transform0, const Transform2& transform1, Vec3& axis, Scalar& angle)
	{
		Matrix3x3 dmat = transform1.getBasis() * transform0.getBasis().inverse();
		Quat dorn;
		dmat.getRotation(dorn);

		///floating point inaccuracy can lead to w component > 1..., which breaks
		dorn.normalize();

		angle = dorn.getAngle();
		axis = Vec3(dorn.x(), dorn.y(), dorn.z());
		axis[3] = Scalar(0.);
		//check for axis length
		Scalar len = axis.length2();
		if (len < SIMD_EPSILON * SIMD_EPSILON)
			axis = Vec3(Scalar(1.), Scalar(0.), Scalar(0.));
		else
			axis /= Sqrt(len);
	}
};

///The ConvexSeparatingDistanceUtil can help speed up convex collision detection
///by conservatively updating a cached separating distance/vector instead of re-calculating the closest distance
class ConvexSeparatingDistanceUtil
{
	Quat m_ornA;
	Quat m_ornB;
	Vec3 m_posA;
	Vec3 m_posB;

	Vec3 m_separatingNormal;

	Scalar m_boundingRadiusA;
	Scalar m_boundingRadiusB;
	Scalar m_separatingDistance;

public:
	ConvexSeparatingDistanceUtil(Scalar boundingRadiusA, Scalar boundingRadiusB)
		: m_boundingRadiusA(boundingRadiusA),
		  m_boundingRadiusB(boundingRadiusB),
		  m_separatingDistance(0.f)
	{
	}

	Scalar getConservativeSeparatingDistance()
	{
		return m_separatingDistance;
	}

	void updateSeparatingDistance(const Transform2& transA, const Transform2& transB)
	{
		const Vec3& toPosA = transA.getOrigin();
		const Vec3& toPosB = transB.getOrigin();
		Quat toOrnA = transA.getRotation();
		Quat toOrnB = transB.getRotation();

		if (m_separatingDistance > 0.f)
		{
			Vec3 linVelA, angVelA, linVelB, angVelB;
			Transform2Util::calculateVelocityQuat(m_posA, toPosA, m_ornA, toOrnA, Scalar(1.), linVelA, angVelA);
			Transform2Util::calculateVelocityQuat(m_posB, toPosB, m_ornB, toOrnB, Scalar(1.), linVelB, angVelB);
			Scalar maxAngularProjectedVelocity = angVelA.length() * m_boundingRadiusA + angVelB.length() * m_boundingRadiusB;
			Vec3 relLinVel = (linVelB - linVelA);
			Scalar relLinVelocLength = relLinVel.dot(m_separatingNormal);
			if (relLinVelocLength < 0.f)
			{
				relLinVelocLength = 0.f;
			}

			Scalar projectedMotion = maxAngularProjectedVelocity + relLinVelocLength;
			m_separatingDistance -= projectedMotion;
		}

		m_posA = toPosA;
		m_posB = toPosB;
		m_ornA = toOrnA;
		m_ornB = toOrnB;
	}

	void initSeparatingDistance(const Vec3& separatingVector, Scalar separatingDistance, const Transform2& transA, const Transform2& transB)
	{
		m_separatingDistance = separatingDistance;

		if (m_separatingDistance > 0.f)
		{
			m_separatingNormal = separatingVector;

			const Vec3& toPosA = transA.getOrigin();
			const Vec3& toPosB = transB.getOrigin();
			Quat toOrnA = transA.getRotation();
			Quat toOrnB = transB.getRotation();
			m_posA = toPosA;
			m_posB = toPosB;
			m_ornA = toOrnA;
			m_ornB = toOrnB;
		}
	}
};

#endif  //DRX3D_TRANSFORM_UTIL_H
