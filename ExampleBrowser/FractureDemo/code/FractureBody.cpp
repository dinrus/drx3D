#include "../FractureBody.h"
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Dynamics/DynamicsWorld.h>

void FractureBody::recomputeConnectivity(CollisionWorld* world)
{
	m_connections.clear();
	//@todo use the AABB tree to avoid N^2 checks

	if (getCollisionShape()->isCompound())
	{
		CompoundShape* compound = (CompoundShape*)getCollisionShape();
		for (i32 i = 0; i < compound->getNumChildShapes(); i++)
		{
			for (i32 j = i + 1; j < compound->getNumChildShapes(); j++)
			{
				struct MyContactResultCallback : public CollisionWorld::ContactResultCallback
				{
					bool m_connected;
					Scalar m_margin;
					MyContactResultCallback() : m_connected(false), m_margin(0.05)
					{
					}
					virtual Scalar addSingleResult(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1)
					{
						if (cp.getDistance() <= m_margin)
							m_connected = true;
						return 1.f;
					}
				};

				MyContactResultCallback result;

				CollisionObject2 obA;
				obA.setWorldTransform(compound->getChildTransform(i));
				obA.setCollisionShape(compound->getChildShape(i));
				CollisionObject2 obB;
				obB.setWorldTransform(compound->getChildTransform(j));
				obB.setCollisionShape(compound->getChildShape(j));
				world->contactPairTest(&obA, &obB, result);
				if (result.m_connected)
				{
					Connection tmp;
					tmp.m_childIndex0 = i;
					tmp.m_childIndex1 = j;
					tmp.m_childShape0 = compound->getChildShape(i);
					tmp.m_childShape1 = compound->getChildShape(j);
					tmp.m_strength = 1.f;  //??
					m_connections.push_back(tmp);
				}
			}
		}
	}
}

CompoundShape* FractureBody::shiftTransformDistributeMass(CompoundShape* boxCompound, Scalar mass, Transform2& shift)
{
	Vec3 principalInertia;

	Scalar* masses = new Scalar[boxCompound->getNumChildShapes()];
	for (i32 j = 0; j < boxCompound->getNumChildShapes(); j++)
	{
		//evenly distribute mass
		masses[j] = mass / boxCompound->getNumChildShapes();
	}

	return shiftTransform(boxCompound, masses, shift, principalInertia);
}

CompoundShape* FractureBody::shiftTransform(CompoundShape* boxCompound, Scalar* masses, Transform2& shift, Vec3& principalInertia)
{
	Transform2 principal;

	boxCompound->calculatePrincipalAxisTransform(masses, principal, principalInertia);

	///create a new compound with world transform/center of mass properly aligned with the principal axis

	///non-recursive compound shapes perform better

#ifdef USE_RECURSIVE_COMPOUND

	CompoundShape* newCompound = new CompoundShape();
	newCompound->addChildShape(principal.inverse(), boxCompound);
	newBoxCompound = newCompound;
	//m_collisionShapes.push_back(newCompound);

	//DefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	//RigidBody::RigidBodyConstructionInfo rbInfo(mass,myMotionState,newCompound,principalInertia);

#else
#ifdef CHANGE_COMPOUND_INPLACE
	newBoxCompound = boxCompound;
	for (i32 i = 0; i < boxCompound->getNumChildShapes(); i++)
	{
		Transform2 newChildTransform = principal.inverse() * boxCompound->getChildTransform(i);
		///updateChildTransform is really slow, because it re-calculates the AABB each time. todo: add option to disable this update
		boxCompound->updateChildTransform(i, newChildTransform);
	}
	bool isDynamic = (mass != 0.f);
	Vec3 localInertia(0, 0, 0);
	if (isDynamic)
		boxCompound->calculateLocalInertia(mass, localInertia);

#else
	///creation is faster using a new compound to store the shifted children
	CompoundShape* newBoxCompound = new CompoundShape();
	for (i32 i = 0; i < boxCompound->getNumChildShapes(); i++)
	{
		Transform2 newChildTransform = principal.inverse() * boxCompound->getChildTransform(i);
		///updateChildTransform is really slow, because it re-calculates the AABB each time. todo: add option to disable this update
		newBoxCompound->addChildShape(newChildTransform, boxCompound->getChildShape(i));
	}

#endif

#endif  //USE_RECURSIVE_COMPOUND

	shift = principal;
	return newBoxCompound;
}
