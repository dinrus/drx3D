#ifndef DRX3D_DEFORMABLE_BODY_SOLVERS_H
#define DRX3D_DEFORMABLE_BODY_SOLVERS_H

#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/DeformableBackwardEulerObjective.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Physics/SoftBody/ConjugateResidual.h>
#include <drx3D/Physics/SoftBody/ConjugateGradient.h>
struct CollisionObject2Wrapper;
// class DeformableBackwardEulerObjective;
// class DeformableMultiBodyDynamicsWorld;

class DeformableBodySolver : public SoftBodySolver
{
	typedef AlignedObjectArray<Vec3> TVStack;

protected:
	i32 m_numNodes;                                                // total number of deformable body nodes
	TVStack m_dv;                                                  // v_{n+1} - v_n
	TVStack m_backup_dv;                                           // backed up dv
	TVStack m_ddv;                                                 // incremental dv
	TVStack m_residual;                                            // rhs of the linear solve
	AlignedObjectArray<SoftBody*> m_softBodies;                // all deformable bodies
	TVStack m_backupVelocity;                                      // backed up v, equals v_n for implicit, equals v_{n+1}^* for explicit
	Scalar m_dt;                                                 // dt
	ConjugateGradient<DeformableBackwardEulerObjective> m_cg;  // CG solver
	ConjugateResidual<DeformableBackwardEulerObjective> m_cr;  // CR solver
	bool m_implicit;                                               // use implicit scheme if true, explicit scheme if false
	i32 m_maxNewtonIterations;                                     // max number of newton iterations
	Scalar m_newtonTolerance;                                    // stop newton iterations if f(x) < m_newtonTolerance
	bool m_lineSearch;                                             // If true, use newton's method with line search under implicit scheme
	bool m_reducedSolver;																					 // flag for reduced soft body solver
public:
	// handles data related to objective function
	DeformableBackwardEulerObjective* m_objective;
	bool m_useProjection;

	DeformableBodySolver();

	virtual ~DeformableBodySolver();

	virtual SolverTypes getSolverType() const
	{
		return DEFORMABLE_SOLVER;
	}

	// update soft body normals
	virtual void updateSoftBodies();

	virtual Scalar solveContactConstraints(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal);

	// solve the momentum equation
	virtual void solveDeformableConstraints(Scalar solverdt);

	// set gravity (get from deformable world)
	virtual void setGravity(const Vec3& gravity)
	{
		// for full deformable object, we don't store gravity in the solver
		// this function is overriden in the reduced deformable object
	}

	// resize/clear data structures
	virtual void reinitialize(const AlignedObjectArray<SoftBody*>& softBodies, Scalar dt);

	// set up contact constraints
	virtual void setConstraints(const ContactSolverInfo& infoGlobal);

	// add in elastic forces and gravity to obtain v_{n+1}^* and calls predictDeformableMotion
	virtual void predictMotion(Scalar solverdt);

	// move to temporary position x_{n+1}^* = x_n + dt * v_{n+1}^*
	// x_{n+1}^* is stored in m_q
	void predictDeformableMotion(SoftBody* psb, Scalar dt);

	// save the current velocity to m_backupVelocity
	void backupVelocity();

	// set m_dv and m_backupVelocity to desired value to prepare for momentum solve
	virtual void setupDeformableSolve(bool implicit);

	// set the current velocity to that backed up in m_backupVelocity
	void revertVelocity();

	// set velocity to m_dv + m_backupVelocity
	void updateVelocity();

	// update the node count
	bool updateNodes();

	// calculate the change in dv resulting from the momentum solve
	void computeStep(TVStack& ddv, const TVStack& residual);

	// calculate the change in dv resulting from the momentum solve when line search is turned on
	Scalar computeDescentStep(TVStack& ddv, const TVStack& residual, bool verbose = false);

	virtual void copySoftBodyToVertexBuffer(const SoftBody* const softBody, VertexBufferDescriptor* vertexBuffer) {}

	// process collision between deformable and rigid
	virtual void processCollision(SoftBody* softBody, const CollisionObject2Wrapper* collisionObjectWrap)
	{
		softBody->defaultCollisionHandler(collisionObjectWrap);
	}

	// process collision between deformable and deformable
	virtual void processCollision(SoftBody* softBody, SoftBody* otherSoftBody)
	{
		softBody->defaultCollisionHandler(otherSoftBody);
	}

	// If true, implicit time stepping scheme is used.
	// Otherwise, explicit time stepping scheme is used
	void setImplicit(bool implicit);

	// If true, newton's method with line search is used when implicit time stepping scheme is turned on
	void setLineSearch(bool lineSearch);

	// set temporary position x^* = x_n + dt * v
	// update the deformation gradient at position x^*
	void updateState();

	// set dv = dv + scale * ddv
	void updateDv(Scalar scale = 1);

	// set temporary position x^* = x_n + dt * v^*
	void updateTempPosition();

	// save the current dv to m_backup_dv;
	void backupDv();

	// set dv to the backed-up value
	void revertDv();

	// set dv = dv + scale * ddv
	// set v^* = v_n + dv
	// set temporary position x^* = x_n + dt * v^*
	// update the deformation gradient at position x^*
	void updateEnergy(Scalar scale);

	// calculates the appropriately scaled kinetic energy in the system, which is
	// 1/2 * dv^T * M * dv
	// used in line search
	Scalar kineticEnergy();

	// add explicit force to the velocity in the objective class
	virtual void applyExplicitForce();

	// execute position/velocity update and apply anchor constraints in the integrateTransforms from the Dynamics world
	virtual void applyTransforms(Scalar timeStep);

	virtual void setStrainLimiting(bool opt)
	{
		m_objective->m_projection.m_useStrainLimiting = opt;
	}

	virtual void setPreconditioner(i32 opt)
	{
		switch (opt)
		{
			case DeformableBackwardEulerObjective::Mass_preconditioner:
				m_objective->m_preconditioner = m_objective->m_massPreconditioner;
				break;

			case DeformableBackwardEulerObjective::KKT_preconditioner:
				m_objective->m_preconditioner = m_objective->m_KKTPreconditioner;
				break;
			
			default:
				Assert(false);
				break;
		}
	}

	virtual AlignedObjectArray<DeformableLagrangianForce*>* getLagrangianForceArray()
	{
		return &(m_objective->m_lf);
	}

	virtual const AlignedObjectArray<SoftBody::Node*>* getIndices()
	{
		return m_objective->getIndices();
	}

	virtual void setProjection()
	{
		m_objective->m_projection.setProjection();
	}

	virtual void setLagrangeMultiplier()
	{
		m_objective->m_projection.setLagrangeMultiplier();
	}

	virtual bool isReducedSolver()
	{
		return m_reducedSolver;
	}
	
	virtual void deformableBodyInternalWriteBack() {}

	// unused functions
	virtual void optimize(AlignedObjectArray<SoftBody*>& softBodies, bool forceUpdate = false) {}
	virtual void solveConstraints(Scalar dt) {}
	virtual bool checkInitialized() { return true; }
	virtual void copyBackToSoftBodies(bool bMove = true) {}
};

#endif /* DeformableBodySolver_h */
