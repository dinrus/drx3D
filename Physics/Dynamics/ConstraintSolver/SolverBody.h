#ifndef DRX3D_SOLVER_BODY_H
#define DRX3D_SOLVER_BODY_H

class RigidBody;
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>

#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

///Until we get other contributions, only use SIMD on Windows, when using Visual Studio 2008 or later, and not double precision
#ifdef DRX3D_USE_SSE
#define USE_SIMD 1
#endif  //

#ifdef USE_SIMD

struct SimdScalar
{
	SIMD_FORCE_INLINE SimdScalar()
	{
	}

	SIMD_FORCE_INLINE SimdScalar(float fl)
		: m_vec128(_mm_set1_ps(fl))
	{
	}

	SIMD_FORCE_INLINE SimdScalar(__m128 v128)
		: m_vec128(v128)
	{
	}
	union {
		__m128 m_vec128;
		float m_floats[4];
		i32 m_ints[4];
		Scalar m_unusedPadding;
	};
	SIMD_FORCE_INLINE __m128 get128()
	{
		return m_vec128;
	}

	SIMD_FORCE_INLINE const __m128 get128() const
	{
		return m_vec128;
	}

	SIMD_FORCE_INLINE void set128(__m128 v128)
	{
		m_vec128 = v128;
	}

	SIMD_FORCE_INLINE operator __m128()
	{
		return m_vec128;
	}
	SIMD_FORCE_INLINE operator const __m128() const
	{
		return m_vec128;
	}

	SIMD_FORCE_INLINE operator float() const
	{
		return m_floats[0];
	}
};

///@brief Return the elementwise product of two SimdScalar
SIMD_FORCE_INLINE SimdScalar
operator*(const SimdScalar& v1, const SimdScalar& v2)
{
	return SimdScalar(_mm_mul_ps(v1.get128(), v2.get128()));
}

///@brief Return the elementwise product of two SimdScalar
SIMD_FORCE_INLINE SimdScalar
operator+(const SimdScalar& v1, const SimdScalar& v2)
{
	return SimdScalar(_mm_add_ps(v1.get128(), v2.get128()));
}

#else
#define SimdScalar Scalar
#endif

///The SolverBody is an internal datastructure for the constraint solver. Only necessary data is packed to increase cache coherence/performance.
ATTRIBUTE_ALIGNED16(struct)
SolverBody
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();
	Transform2 m_worldTransform;
	Vec3 m_deltaLinearVelocity;
	Vec3 m_deltaAngularVelocity;
	Vec3 m_angularFactor;
	Vec3 m_linearFactor;
	Vec3 m_invMass;
	Vec3 m_pushVelocity;
	Vec3 m_turnVelocity;
	Vec3 m_linearVelocity;
	Vec3 m_angularVelocity;
	Vec3 m_externalForceImpulse;
	Vec3 m_externalTorqueImpulse;

	RigidBody* m_originalBody;
	void setWorldTransform(const Transform2& worldTransform2)
	{
		m_worldTransform = worldTransform2;
	}

	const Transform2& getWorldTransform() const
	{
		return m_worldTransform;
	}

	SIMD_FORCE_INLINE void getVelocityInLocalPointNoDelta(const Vec3& rel_pos, Vec3& velocity) const
	{
		if (m_originalBody)
			velocity = m_linearVelocity + m_externalForceImpulse + (m_angularVelocity + m_externalTorqueImpulse).cross(rel_pos);
		else
			velocity.setVal(0, 0, 0);
	}

	SIMD_FORCE_INLINE void getVelocityInLocalPointObsolete(const Vec3& rel_pos, Vec3& velocity) const
	{
		if (m_originalBody)
			velocity = m_linearVelocity + m_deltaLinearVelocity + (m_angularVelocity + m_deltaAngularVelocity).cross(rel_pos);
		else
			velocity.setVal(0, 0, 0);
	}

	SIMD_FORCE_INLINE void getAngularVelocity(Vec3 & angVel) const
	{
		if (m_originalBody)
			angVel = m_angularVelocity + m_deltaAngularVelocity;
		else
			angVel.setVal(0, 0, 0);
	}

	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	SIMD_FORCE_INLINE void applyImpulse(const Vec3& linearComponent, const Vec3& angularComponent, const Scalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_deltaAngularVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	SIMD_FORCE_INLINE void internalApplyPushImpulse(const Vec3& linearComponent, const Vec3& angularComponent, Scalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_pushVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_turnVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	const Vec3& getDeltaLinearVelocity() const
	{
		return m_deltaLinearVelocity;
	}

	const Vec3& getDeltaAngularVelocity() const
	{
		return m_deltaAngularVelocity;
	}

	const Vec3& getPushVelocity() const
	{
		return m_pushVelocity;
	}

	const Vec3& getTurnVelocity() const
	{
		return m_turnVelocity;
	}

	////////////////////////////////////////////////
	///some internal methods, don't use them

	Vec3& internalGetDeltaLinearVelocity()
	{
		return m_deltaLinearVelocity;
	}

	Vec3& internalGetDeltaAngularVelocity()
	{
		return m_deltaAngularVelocity;
	}

	const Vec3& internalGetAngularFactor() const
	{
		return m_angularFactor;
	}

	const Vec3& internalGetInvMass() const
	{
		return m_invMass;
	}

	void internalSetInvMass(const Vec3& invMass)
	{
		m_invMass = invMass;
	}

	Vec3& internalGetPushVelocity()
	{
		return m_pushVelocity;
	}

	Vec3& internalGetTurnVelocity()
	{
		return m_turnVelocity;
	}

	SIMD_FORCE_INLINE void internalGetVelocityInLocalPointObsolete(const Vec3& rel_pos, Vec3& velocity) const
	{
		velocity = m_linearVelocity + m_deltaLinearVelocity + (m_angularVelocity + m_deltaAngularVelocity).cross(rel_pos);
	}

	SIMD_FORCE_INLINE void internalGetAngularVelocity(Vec3 & angVel) const
	{
		angVel = m_angularVelocity + m_deltaAngularVelocity;
	}

	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	SIMD_FORCE_INLINE void internalApplyImpulse(const Vec3& linearComponent, const Vec3& angularComponent, const Scalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_deltaAngularVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	void writebackVelocity()
	{
		if (m_originalBody)
		{
			m_linearVelocity += m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;

			//m_originalBody->setCompanionId(-1);
		}
	}

	void writebackVelocityAndTransform2(Scalar timeStep, Scalar splitImpulseTurnErp)
	{
		(void)timeStep;
		if (m_originalBody)
		{
			m_linearVelocity += m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;

			//correct the position/orientation based on push/turn recovery
			Transform2 newTransform2;
			if (m_pushVelocity[0] != 0.f || m_pushVelocity[1] != 0 || m_pushVelocity[2] != 0 || m_turnVelocity[0] != 0.f || m_turnVelocity[1] != 0 || m_turnVelocity[2] != 0)
			{
				//	Quat orn = m_worldTransform.getRotation();
				Transform2Util::integrateTransform(m_worldTransform, m_pushVelocity, m_turnVelocity * splitImpulseTurnErp, timeStep, newTransform2);
				m_worldTransform = newTransform2;
			}
			//m_worldTransform.setRotation(orn);
			//m_originalBody->setCompanionId(-1);
		}
	}
};

#endif  //DRX3D_SOLVER_BODY_H
