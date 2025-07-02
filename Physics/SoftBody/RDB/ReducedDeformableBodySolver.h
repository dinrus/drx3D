#ifndef DRX3D_REDUCED_DEFORMABLE_BODY_DYNAMICS_WORLD_H
#define DRX3D_REDUCED_DEFORMABLE_BODY_DYNAMICS_WORLD_H

#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableContactConstraint.h>

class ReducedDeformableBody;

class ReducedDeformableBodySolver : public DeformableBodySolver
{
 protected:
  bool m_ascendOrder;
  Scalar m_dampingAlpha;
  Scalar m_dampingBeta;

  Vec3 m_gravity;

  void predictReduceDeformableMotion(Scalar solverdt);

  void applyExplicitForce(Scalar solverdt);

 public:
  AlignedObjectArray<AlignedObjectArray<ReducedDeformableStaticConstraint> > m_staticConstraints;
  AlignedObjectArray<AlignedObjectArray<ReducedDeformableNodeRigidContactConstraint> > m_nodeRigidConstraints;
  AlignedObjectArray<AlignedObjectArray<ReducedDeformableFaceRigidContactConstraint> > m_faceRigidConstraints;
  
  ReducedDeformableBodySolver();
  ~ReducedDeformableBodySolver() {}

  virtual void setGravity(const Vec3& gravity);

  virtual SolverTypes getSolverType() const
  {
    return REDUCED_DEFORMABLE_SOLVER;
  }

  // resize/clear data structures
	virtual void reinitialize(const AlignedObjectArray<SoftBody*>& bodies, Scalar dt);

  virtual void predictMotion(Scalar solverdt);

  virtual void applyTransforms(Scalar timeStep);

  // set up contact constraints
	virtual void setConstraints(const ContactSolverInfo& infoGlobal);

  // solve all constraints (fixed and contact)
  virtual Scalar solveContactConstraints(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal);

  // apply all the delta velocities
  virtual void deformableBodyInternalWriteBack();

  // virtual void setProjection() {}

  // virtual void setLagrangeMultiplier() {}

  // virtual void setupDeformableSolve(bool implicit);

};

#endif // DRX3D_REDUCED_DEFORMABLE_BODY_DYNAMICS_WORLD_H