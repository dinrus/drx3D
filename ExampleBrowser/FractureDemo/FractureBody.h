
#ifndef DRX3D_FRACTURE_BODY
#define DRX3D_FRACTURE_BODY

class CollisionShape;
class DynamicsWorld;
class CollisionWorld;
class CompoundShape;
class ManifoldPoint;

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

#define CUSTOM_FRACTURE_TYPE (RigidBody::CO_USER_TYPE + 1)

struct Connection
{
	CollisionShape* m_childShape0;
	CollisionShape* m_childShape1;
	i32 m_childIndex0;
	i32 m_childIndex1;
	Scalar m_strength;
};

class FractureBody : public RigidBody
{
	//connections
public:
	DynamicsWorld* m_world;
	AlignedObjectArray<Scalar> m_masses;
	AlignedObjectArray<Connection> m_connections;

	FractureBody(const RigidBodyConstructionInfo& constructionInfo, DynamicsWorld* world)
		: RigidBody(constructionInfo),
		  m_world(world)
	{
		m_masses.push_back(constructionInfo.m_mass);
		m_internalType = CUSTOM_FRACTURE_TYPE + CO_RIGID_BODY;
	}

	///RigidBody constructor for backwards compatibility.
	///To specify friction (etc) during rigid body construction, please use the other constructor (using RigidBodyConstructionInfo)
	FractureBody(Scalar mass, MotionState* motionState, CollisionShape* collisionShape, const Vec3& localInertia, Scalar* masses, i32 numMasses, DynamicsWorld* world)
		: RigidBody(mass, motionState, collisionShape, localInertia),
		  m_world(world)
	{
		for (i32 i = 0; i < numMasses; i++)
			m_masses.push_back(masses[i]);

		m_internalType = CUSTOM_FRACTURE_TYPE + CO_RIGID_BODY;
	}

	void recomputeConnectivity(CollisionWorld* world);

	static CompoundShape* shiftTransform(CompoundShape* boxCompound, Scalar* masses, Transform2& shift, Vec3& principalInertia);

	static CompoundShape* shiftTransformDistributeMass(CompoundShape* boxCompound, Scalar mass, Transform2& shift);

	static bool collisionCallback(ManifoldPoint& cp, const CollisionObject2* colObj0, i32 partId0, i32 index0, const CollisionObject2* colObj1, i32 partId1, i32 index1);
};

void fractureCallback(DynamicsWorld* world, Scalar timeStep);
void glueCallback(DynamicsWorld* world, Scalar timeStep);

#endif  //DRX3D_FRACTURE_BODY
