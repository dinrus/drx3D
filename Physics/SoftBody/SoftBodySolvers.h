#ifndef DRX3D_SOFT_BODY_SOLVERS_H
#define DRX3D_SOFT_BODY_SOLVERS_H

#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>

class SoftBodyTriangleData;
class SoftBodyLinkData;
class SoftBodyVertexData;
class VertexBufferDescriptor;
class CollisionObject2;
class SoftBody;

class SoftBodySolver
{
public:
	enum SolverTypes
	{
		DEFAULT_SOLVER,
		CPU_SOLVER,
		CL_SOLVER,
		CL_SIMD_SOLVER,
		DX_SOLVER,
		DX_SIMD_SOLVER,
		DEFORMABLE_SOLVER,
		REDUCED_DEFORMABLE_SOLVER
	};

protected:
	i32 m_numberOfPositionIterations;
	i32 m_numberOfVelocityIterations;
	// Simulation timescale
	float m_timeScale;

public:
	SoftBodySolver() : m_numberOfPositionIterations(10),
						 m_timeScale(1)
	{
		m_numberOfVelocityIterations = 0;
		m_numberOfPositionIterations = 5;
	}

	virtual ~SoftBodySolver()
	{
	}

	/**
	 * Return the type of the solver.
	 */
	virtual SolverTypes getSolverType() const = 0;

	/** Ensure that this solver is initialized. */
	virtual bool checkInitialized() = 0;

	/** Optimize soft bodies in this solver. */
	virtual void optimize(AlignedObjectArray<SoftBody *> &softBodies, bool forceUpdate = false) = 0;

	/** Copy necessary data back to the original soft body source objects. */
	virtual void copyBackToSoftBodies(bool bMove = true) = 0;

	/** Predict motion of soft bodies into next timestep */
	virtual void predictMotion(Scalar solverdt) = 0;

	/** Solve constraints for a set of soft bodies */
	virtual void solveConstraints(Scalar solverdt) = 0;

	/** Perform necessary per-step updates of soft bodies such as recomputing normals and bounding boxes */
	virtual void updateSoftBodies() = 0;

	/** Process a collision between one of the world's soft bodies and another collision object */
	virtual void processCollision(SoftBody *, const struct CollisionObject2Wrapper *) = 0;

	/** Process a collision between two soft bodies */
	virtual void processCollision(SoftBody *, SoftBody *) = 0;

	/** Set the number of velocity constraint solver iterations this solver uses. */
	virtual void setNumberOfPositionIterations(i32 iterations)
	{
		m_numberOfPositionIterations = iterations;
	}

	/** Get the number of velocity constraint solver iterations this solver uses. */
	virtual i32 getNumberOfPositionIterations()
	{
		return m_numberOfPositionIterations;
	}

	/** Set the number of velocity constraint solver iterations this solver uses. */
	virtual void setNumberOfVelocityIterations(i32 iterations)
	{
		m_numberOfVelocityIterations = iterations;
	}

	/** Get the number of velocity constraint solver iterations this solver uses. */
	virtual i32 getNumberOfVelocityIterations()
	{
		return m_numberOfVelocityIterations;
	}

	/** Return the timescale that the simulation is using */
	float getTimeScale()
	{
		return m_timeScale;
	}

#if 0
	/**
	 * Add a collision object to be used by the indicated softbody.
	 */
	virtual void addCollisionObject2ForSoftBody( i32 clothIdentifier, CollisionObject2 *collisionObject ) = 0;
#endif
};

/** 
 * Class to manage movement of data from a solver to a given target.
 * This version is abstract. Subclasses will have custom pairings for different combinations.
 */
class SoftBodySolverOutput
{
protected:
public:
	SoftBodySolverOutput()
	{
	}

	virtual ~SoftBodySolverOutput()
	{
	}

	/** Output current computed vertex data to the vertex buffers for all cloths in the solver. */
	virtual void copySoftBodyToVertexBuffer(const SoftBody *const softBody, VertexBufferDescriptor *vertexBuffer) = 0;
};

#endif  // #ifndef DRX3D_SOFT_BODY_SOLVERS_H
