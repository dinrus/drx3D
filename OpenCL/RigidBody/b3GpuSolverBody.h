#ifndef D3_GPU_SOLVER_BODY_H
#define D3_GPU_SOLVER_BODY_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3Matrix3x3.h>

#include <drx3D/Common/b3AlignedAllocator.h>
#include <drx3D/Common/b3TransformUtil.h>

///Until we get other contributions, only use SIMD on Windows, when using Visual Studio 2008 or later, and not double precision
#ifdef D3_USE_SSE
#define USE_SIMD 1
#endif  //

///The b3SolverBody is an internal datastructure for the constraint solver. Only necessary data is packed to increase cache coherence/performance.
D3_ATTRIBUTE_ALIGNED16(struct)
b3GpuSolverBody
{
	D3_DECLARE_ALIGNED_ALLOCATOR();
	//	b3Transform		m_worldTransformUnused;
	b3Vec3 m_deltaLinearVelocity;
	b3Vec3 m_deltaAngularVelocity;
	b3Vec3 m_angularFactor;
	b3Vec3 m_linearFactor;
	b3Vec3 m_invMass;
	b3Vec3 m_pushVelocity;
	b3Vec3 m_turnVelocity;
	b3Vec3 m_linearVelocity;
	b3Vec3 m_angularVelocity;

	union {
		uk m_originalBody;
		i32 m_originalBodyIndex;
	};

	i32 padding[3];

	/*
	void	setWorldTransform(const b3Transform& worldTransform)
	{
		m_worldTransform = worldTransform;
	}

	const b3Transform& getWorldTransform() const
	{
		return m_worldTransform;
	}
	*/
	D3_FORCE_INLINE void getVelocityInLocalPointObsolete(const b3Vec3& rel_pos, b3Vec3& velocity) const
	{
		if (m_originalBody)
			velocity = m_linearVelocity + m_deltaLinearVelocity + (m_angularVelocity + m_deltaAngularVelocity).cross(rel_pos);
		else
			velocity.setVal(0, 0, 0);
	}

	D3_FORCE_INLINE void getAngularVelocity(b3Vec3 & angVel) const
	{
		if (m_originalBody)
			angVel = m_angularVelocity + m_deltaAngularVelocity;
		else
			angVel.setVal(0, 0, 0);
	}

	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	D3_FORCE_INLINE void applyImpulse(const b3Vec3& linearComponent, const b3Vec3& angularComponent, const b3Scalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_deltaAngularVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	D3_FORCE_INLINE void internalApplyPushImpulse(const b3Vec3& linearComponent, const b3Vec3& angularComponent, b3Scalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_pushVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_turnVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	const b3Vec3& getDeltaLinearVelocity() const
	{
		return m_deltaLinearVelocity;
	}

	const b3Vec3& getDeltaAngularVelocity() const
	{
		return m_deltaAngularVelocity;
	}

	const b3Vec3& getPushVelocity() const
	{
		return m_pushVelocity;
	}

	const b3Vec3& getTurnVelocity() const
	{
		return m_turnVelocity;
	}

	////////////////////////////////////////////////
	///some internal methods, don't use them

	b3Vec3& internalGetDeltaLinearVelocity()
	{
		return m_deltaLinearVelocity;
	}

	b3Vec3& internalGetDeltaAngularVelocity()
	{
		return m_deltaAngularVelocity;
	}

	const b3Vec3& internalGetAngularFactor() const
	{
		return m_angularFactor;
	}

	const b3Vec3& internalGetInvMass() const
	{
		return m_invMass;
	}

	void internalSetInvMass(const b3Vec3& invMass)
	{
		m_invMass = invMass;
	}

	b3Vec3& internalGetPushVelocity()
	{
		return m_pushVelocity;
	}

	b3Vec3& internalGetTurnVelocity()
	{
		return m_turnVelocity;
	}

	D3_FORCE_INLINE void internalGetVelocityInLocalPointObsolete(const b3Vec3& rel_pos, b3Vec3& velocity) const
	{
		velocity = m_linearVelocity + m_deltaLinearVelocity + (m_angularVelocity + m_deltaAngularVelocity).cross(rel_pos);
	}

	D3_FORCE_INLINE void internalGetAngularVelocity(b3Vec3 & angVel) const
	{
		angVel = m_angularVelocity + m_deltaAngularVelocity;
	}

	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	D3_FORCE_INLINE void internalApplyImpulse(const b3Vec3& linearComponent, const b3Vec3& angularComponent, const b3Scalar impulseMagnitude)
	{
		//if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent * impulseMagnitude * m_linearFactor;
			m_deltaAngularVelocity += angularComponent * (impulseMagnitude * m_angularFactor);
		}
	}

	void writebackVelocity()
	{
		//if (m_originalBody>=0)
		{
			m_linearVelocity += m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;

			//m_originalBody->setCompanionId(-1);
		}
	}

	void writebackVelocityAndTransform(b3Scalar timeStep, b3Scalar splitImpulseTurnErp)
	{
		(void)timeStep;
		if (m_originalBody)
		{
			m_linearVelocity += m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;

			//correct the position/orientation based on push/turn recovery
			b3Transform newTransform;
			if (m_pushVelocity[0] != 0.f || m_pushVelocity[1] != 0 || m_pushVelocity[2] != 0 || m_turnVelocity[0] != 0.f || m_turnVelocity[1] != 0 || m_turnVelocity[2] != 0)
			{
				//	b3Quat orn = m_worldTransform.getRotation();
				//				b3TransformUtil::integrateTransform(m_worldTransform,m_pushVelocity,m_turnVelocity*splitImpulseTurnErp,timeStep,newTransform);
				//				m_worldTransform = newTransform;
			}
			//m_worldTransform.setRotation(orn);
			//m_originalBody->setCompanionId(-1);
		}
	}
};

#endif  //D3_SOLVER_BODY_H
