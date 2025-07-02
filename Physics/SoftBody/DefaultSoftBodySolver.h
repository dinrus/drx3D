#ifndef DRX3D_SOFT_BODY_DEFAULT_SOLVER_H
#define DRX3D_SOFT_BODY_DEFAULT_SOLVER_H

#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/SoftBodySolverVertexBuffer.h>
struct CollisionObject2Wrapper;

class DefaultSoftBodySolver : public SoftBodySolver
{
protected:
	/** Variable to define whether we need to update solver constants on the next iteration */
	bool m_updateSolverConstants;

	AlignedObjectArray<SoftBody *> m_softBodySet;

public:
	DefaultSoftBodySolver();

	virtual ~DefaultSoftBodySolver();

	virtual SolverTypes getSolverType() const
	{
		return DEFAULT_SOLVER;
	}

	virtual bool checkInitialized();

	virtual void updateSoftBodies();

	virtual void optimize(AlignedObjectArray<SoftBody *> &softBodies, bool forceUpdate = false);

	virtual void copyBackToSoftBodies(bool bMove = true);

	virtual void solveConstraints(Scalar solverdt);

	virtual void predictMotion(Scalar solverdt);

	virtual void copySoftBodyToVertexBuffer(const SoftBody *const softBody, VertexBufferDescriptor *vertexBuffer);

	virtual void processCollision(SoftBody *, const CollisionObject2Wrapper *);

	virtual void processCollision(SoftBody *, SoftBody *);
};

#endif  // #ifndef DRX3D_ACCELERATED_SOFT_BODY_CPU_SOLVER_H
