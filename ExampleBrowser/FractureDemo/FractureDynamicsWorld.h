#ifndef _DRX3D_FRACTURE_DYNAMICS_WORLD_H
#define _DRX3D_FRACTURE_DYNAMICS_WORLD_H

#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

class FractureBody;
class CompoundShape;
class Transform2;

///The btFractureDynamicsWorld class enabled basic glue and fracture of objects.
///If/once this implementation is stablized/tested we might merge it into DiscreteDynamicsWorld and remove the class.
class FractureDynamicsWorld : public DiscreteDynamicsWorld
{
	AlignedObjectArray<FractureBody*> m_fractureBodies;

	bool m_fracturingMode;

	FractureBody* addNewBody(const Transform2& oldTransform, Scalar* masses, CompoundShape* oldCompound);

	void breakDisconnectedParts(FractureBody* fracObj);

public:
	FractureDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration);

	virtual void addRigidBody(RigidBody* body);

	virtual void removeRigidBody(RigidBody* body);

	void solveConstraints(ContactSolverInfo& solverInfo);

	///either fracture or glue (!fracture)
	void setFractureMode(bool fracture)
	{
		m_fracturingMode = fracture;
	}

	bool getFractureMode() const { return m_fracturingMode; }

	///normally those callbacks are called internally by the 'solveConstraints'
	void glueCallback();

	///normally those callbacks are called internally by the 'solveConstraints'
	void fractureCallback();
};

#endif  //_DRX3D_FRACTURE_DYNAMICS_WORLD_H
