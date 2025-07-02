#ifndef DRX3D_CONSTRAINT_SOLVER_H
#define DRX3D_CONSTRAINT_SOLVER_H

#include <drx3D/Maths/Linear/Scalar.h>

class PersistentManifold;
class RigidBody;
class CollisionObject2;
class TypedConstraint;
struct ContactSolverInfo;
struct BroadphaseProxy;
class IDebugDraw;
class StackAlloc;
class Dispatcher;
/// ConstraintSolver provides solver interface

enum ConstraintSolverType
{
	DRX3D_SEQUENTIAL_IMPULSE_SOLVER = 1,
	DRX3D_MLCP_SOLVER = 2,
	DRX3D_NNCG_SOLVER = 4,
	DRX3D_MULTIBODY_SOLVER = 8,
	DRX3D_BLOCK_SOLVER = 16,
};

class ConstraintSolver
{
public:
	virtual ~ConstraintSolver() {}

	virtual void prepareSolve(i32 /* numBodies */, i32 /* numManifolds */) { ; }

	///solve a group of constraints
	virtual Scalar solveGroup(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& info, class IDebugDraw* debugDrawer, Dispatcher* dispatcher) = 0;

	virtual void allSolved(const ContactSolverInfo& /* info */, class IDebugDraw* /* debugDrawer */) { ; }

	///clear internal cached data and reset random seed
	virtual void reset() = 0;

	virtual ConstraintSolverType getSolverType() const = 0;
};

#endif  //DRX3D_CONSTRAINT_SOLVER_H
