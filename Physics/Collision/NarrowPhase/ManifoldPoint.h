
#ifndef DRX3D_MANIFOLD_CONTACT_POINT_H
#define DRX3D_MANIFOLD_CONTACT_POINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

#ifdef PFX_USE_FREE_VECTORMATH
#include <physics_effects/base_level/solver/pfx_constraint_row.h>
typedef sce::PhysicsEffects::PfxConstraintRow ConstraintRow;
#else
// Don't change following order of parameters
ATTRIBUTE_ALIGNED16(struct)
ConstraintRow
{
	Scalar m_normal[3];
	Scalar m_rhs;
	Scalar m_jacDiagInv;
	Scalar m_lowerLimit;
	Scalar m_upperLimit;
	Scalar m_accumImpulse;
};
typedef ConstraintRow PfxConstraintRow;
#endif  //PFX_USE_FREE_VECTORMATH

enum ContactPointFlags
{
	DRX3D_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED = 1,
	DRX3D_CONTACT_FLAG_HAS_CONTACT_CFM = 2,
	DRX3D_CONTACT_FLAG_HAS_CONTACT_ERP = 4,
	DRX3D_CONTACT_FLAG_CONTACT_STIFFNESS_DAMPING = 8,
	DRX3D_CONTACT_FLAG_FRICTION_ANCHOR = 16,
};

/// ManifoldContactPoint collects and maintains persistent contactpoints.
/// used to improve stability and performance of rigidbody dynamics response.
class ManifoldPoint
{
public:
	ManifoldPoint()
		: m_userPersistentData(0),
		  m_contactPointFlags(0),
		  m_appliedImpulse(0.f),
		  m_prevRHS(0.f),
		  m_appliedImpulseLateral1(0.f),
		  m_appliedImpulseLateral2(0.f),
		  m_contactMotion1(0.f),
		  m_contactMotion2(0.f),
		  m_contactCFM(0.f),
		  m_contactERP(0.f),
		  m_frictionCFM(0.f),
		  m_lifeTime(0)
	{
	}

	ManifoldPoint(const Vec3& pointA, const Vec3& pointB,
					const Vec3& normal,
					Scalar distance) : m_localPointA(pointA),
										 m_localPointB(pointB),
										 m_positionWorldOnB(0,0,0),
										 m_positionWorldOnA(0,0,0),
										 m_normalWorldOnB(normal),
										 m_distance1(distance),
										 m_combinedFriction(Scalar(0.)),
										 m_combinedRollingFriction(Scalar(0.)),
										 m_combinedSpinningFriction(Scalar(0.)),
										 m_combinedRestitution(Scalar(0.)),
										 m_partId0(-1),
										 m_partId1(-1),
										 m_index0(-1),
										 m_index1(-1),
										 m_userPersistentData(0),
										 m_contactPointFlags(0),
										 m_appliedImpulse(0.f),
										 m_prevRHS(0.f),
										 m_appliedImpulseLateral1(0.f),
										 m_appliedImpulseLateral2(0.f),
										 m_contactMotion1(0.f),
										 m_contactMotion2(0.f),
										 m_contactCFM(0.f),
										 m_contactERP(0.f),
										 m_frictionCFM(0.f),
										 m_lifeTime(0),
										 m_lateralFrictionDir1(0,0,0),
										 m_lateralFrictionDir2(0,0,0)
	{
	}

	Vec3 m_localPointA;
	Vec3 m_localPointB;
	Vec3 m_positionWorldOnB;
	///m_positionWorldOnA is redundant information, see getPositionWorldOnA(), but for clarity
	Vec3 m_positionWorldOnA;
	Vec3 m_normalWorldOnB;

	Scalar m_distance1;
	Scalar m_combinedFriction;
	Scalar m_combinedRollingFriction;   //torsional friction orthogonal to contact normal, useful to make spheres stop rolling forever
	Scalar m_combinedSpinningFriction;  //torsional friction around contact normal, useful for grasping objects
	Scalar m_combinedRestitution;

	//BP mod, store contact triangles.
	i32 m_partId0;
	i32 m_partId1;
	i32 m_index0;
	i32 m_index1;

	mutable uk m_userPersistentData;
	//bool			m_lateralFrictionInitialized;
	i32 m_contactPointFlags;

	Scalar m_appliedImpulse;
	Scalar m_prevRHS;
	Scalar m_appliedImpulseLateral1;
	Scalar m_appliedImpulseLateral2;
	Scalar m_contactMotion1;
	Scalar m_contactMotion2;

	union {
		Scalar m_contactCFM;
		Scalar m_combinedContactStiffness1;
	};

	union {
		Scalar m_contactERP;
		Scalar m_combinedContactDamping1;
	};

	Scalar m_frictionCFM;

	i32 m_lifeTime;  //lifetime of the contactpoint in frames

	Vec3 m_lateralFrictionDir1;
	Vec3 m_lateralFrictionDir2;

	Scalar getDistance() const
	{
		return m_distance1;
	}
	i32 getLifeTime() const
	{
		return m_lifeTime;
	}

	const Vec3& getPositionWorldOnA() const
	{
		return m_positionWorldOnA;
		//				return m_positionWorldOnB + m_normalWorldOnB * m_distance1;
	}

	const Vec3& getPositionWorldOnB() const
	{
		return m_positionWorldOnB;
	}

	void setDistance(Scalar dist)
	{
		m_distance1 = dist;
	}

	///this returns the most recent applied impulse, to satisfy contact constraints by the constraint solver
	Scalar getAppliedImpulse() const
	{
		return m_appliedImpulse;
	}
};

#endif  //DRX3D_MANIFOLD_CONTACT_POINT_H
