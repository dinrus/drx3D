#ifndef DRX3D_REDUCED_SOFT_BODY_H
#define DRX3D_REDUCED_SOFT_BODY_H

#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/Transform2.h>

// Reduced deformable body is a simplified deformable object embedded in a rigid frame.
class ReducedDeformableBody : public SoftBody
{
 public:
  //
  //  Typedefs
  //
  typedef AlignedObjectArray<Vec3> TVStack;
  // typedef AlignedObjectArray<Matrix3x3> tBlockDiagMatrix;
  typedef AlignedObjectArray<Scalar> tDenseArray;
  typedef AlignedObjectArray<AlignedObjectArray<Scalar> > tDenseMatrix;

 private:
  // flag to turn off the reduced modes
  bool m_rigidOnly;

  // Flags for transform. Once transform is applied, users cannot scale the mesh or change its total mass.
  bool m_transform_lock;

  // scaling factors
  Scalar m_rhoScale;         // mass density scale
  Scalar m_ksScale;          // stiffness scale

  // projection matrix
  tDenseMatrix m_projPA;        // Eqn. 4.11 from Rahul Sheth's thesis
  tDenseMatrix m_projCq;
  tDenseArray m_STP;
  tDenseArray m_MrInvSTP;

  TVStack m_localMomentArm; // Sq + x0

  Vec3 m_internalDeltaLinearVelocity;
  Vec3 m_internalDeltaAngularVelocity;
  tDenseArray m_internalDeltaReducedVelocity;
  
  Vec3 m_linearVelocityFromReduced;  // contribution to the linear velocity from reduced velocity
  Vec3 m_angularVelocityFromReduced; // contribution to the angular velocity from reduced velocity
  Vec3 m_internalDeltaAngularVelocityFromReduced;

 protected:
  // rigid frame
  Scalar m_mass;          // total mass of the rigid frame
  Scalar m_inverseMass;   // inverse of the total mass of the rigid frame
  Vec3 m_linearVelocity;
  Vec3 m_angularVelocity;
  Scalar m_linearDamping;    // linear damping coefficient
  Scalar m_angularDamping;    // angular damping coefficient
  Vec3 m_linearFactor;
  Vec3 m_angularFactor;
  // Vec3 m_invInertiaLocal;
  Matrix3x3 m_invInertiaLocal;
  Transform2 m_rigidTransform2World;
  Matrix3x3 m_invInertiaTensorWorldInitial;
  Matrix3x3 m_invInertiaTensorWorld;
  Matrix3x3 m_interpolateInvInertiaTensorWorld;
  Vec3 m_initialCoM;  // initial center of mass (original of the m_rigidTransform2World)

  // damping
  Scalar m_dampingAlpha;
  Scalar m_dampingBeta;

 public:
  //
  //  Fields
  //

  // reduced space
  i32 m_nReduced;
  i32 m_nFull;
  tDenseMatrix m_modes;														// modes of the reduced deformable model. Each inner array is a mode, outer array size = n_modes
  tDenseArray m_reducedDofs;				   // Reduced degree of freedom
  tDenseArray m_reducedDofsBuffer;     // Reduced degree of freedom at t^n
  tDenseArray m_reducedVelocity;		   // Reduced velocity array
  tDenseArray m_reducedVelocityBuffer; // Reduced velocity array at t^n
  tDenseArray m_reducedForceExternal;          // reduced external force
  tDenseArray m_reducedForceElastic;           // reduced internal elastic force
  tDenseArray m_reducedForceDamping;           // reduced internal damping force
  tDenseArray m_eigenvalues;		// eigenvalues of the reduce deformable model
  tDenseArray m_Kr;	// reduced stiffness matrix
  
  // full space
  TVStack m_x0;					     				 // Rest position
  tDenseArray m_nodalMass;           // Mass on each node
  AlignedObjectArray<i32> m_fixedNodes; // index of the fixed nodes
  i32 m_nodeIndexOffset;             // offset of the node index needed for contact solver when there are multiple reduced deformable body in the world.

  // contacts
  AlignedObjectArray<i32> m_contactNodesList;

  //
  // Api
  //
  ReducedDeformableBody(SoftBodyWorldInfo* worldInfo, i32 node_count, const Vec3* x, const Scalar* m);

  ~ReducedDeformableBody() {}

  //
  // initializing helpers
  //
  void internalInitialization();

  void setReducedModes(i32 num_modes, i32 full_size);

  void setMassProps(const tDenseArray& mass_array);

  void setInertiaProps();

  void setRigidVelocity(const Vec3& v);

  void setRigidAngularVelocity(const Vec3& omega);

  void setStiffnessScale(const Scalar ks);

  void setMassScale(const Scalar rho);

  void setFixedNodes(i32k n_node);

  void setDamping(const Scalar alpha, const Scalar beta);

  void disableReducedModes(const bool rigid_only);

  virtual void setTotalMass(Scalar mass, bool fromfaces = false);

  //
  // various internal updates
  //
  virtual void transformTo(const Transform2& trs);
  virtual void transform(const Transform2& trs);
  // caution: 
  // need to use scale before using transform, because the scale is performed in the local frame 
  // (i.e., may have some rotation already, but the m_rigidTransform2World doesn't have this info)
  virtual void scale(const Vec3& scl);

 private:
  void updateRestNodalPositions();

  void updateInitialInertiaTensor(const Matrix3x3& rotation);

  void updateLocalInertiaTensorFromNodes();

  void updateInertiaTensor();

  void updateModesByRotation(const Matrix3x3& rotation);
 
 public:
  void updateLocalMomentArm();

  void predictIntegratedTransform2(Scalar dt, Transform2& predictedTransform2);

  // update the external force projection matrix 
  void updateExternalForceProjectMatrix(bool initialized);

  void endOfTimeStepZeroing();

  void applyInternalVelocityChanges();

  //
  // position and velocity update related
  //

  // compute reduced degree of freedoms
  void updateReducedDofs(Scalar solverdt);

  // compute reduced velocity update (for explicit time stepping)
  void updateReducedVelocity(Scalar solverdt);

  // map to full degree of freedoms
  void mapToFullPosition(const Transform2& ref_trans);

  // compute full space velocity from the reduced velocity
  void mapToFullVelocity(const Transform2& ref_trans);

  // compute total angular momentum
  const Vec3 computeTotalAngularMomentum() const;

  // get a single node's full space velocity from the reduced velocity
  const Vec3 computeNodeFullVelocity(const Transform2& ref_trans, i32 n_node) const;

  // get a single node's all delta velocity
  const Vec3 internalComputeNodeDeltaVelocity(const Transform2& ref_trans, i32 n_node) const;

  //
  // rigid motion related
  //
  void applyDamping(Scalar timeStep);

  void applyCentralImpulse(const Vec3& impulse);

  void applyTorqueImpulse(const Vec3& torque);

  void proceedToTransform2(Scalar dt, bool end_of_time_step);

  //
  // force related
  //

  // apply impulse to the rigid frame
  void internalApplyRigidImpulse(const Vec3& impulse, const Vec3& rel_pos);

  // apply impulse to nodes in the full space
  void internalApplyFullSpaceImpulse(const Vec3& impulse, const Vec3& rel_pos, i32 n_node, Scalar dt);

  // apply nodal external force in the full space
  void applyFullSpaceNodalForce(const Vec3& f_ext, i32 n_node);

  // apply gravity to the rigid frame
  void applyRigidGravity(const Vec3& gravity, Scalar dt);

  // apply reduced elastic force
  void applyReducedElasticForce(const tDenseArray& reduce_dofs);

  // apply reduced damping force
  void applyReducedDampingForce(const tDenseArray& reduce_vel);

  // calculate the impulse factor
  virtual Matrix3x3 getImpulseFactor(i32 n_node);

  // get relative position from a node to the CoM of the rigid frame
  Vec3 getRelativePos(i32 n_node);

  //
  // accessors
  //
  bool isReducedModesOFF() const;
  Scalar getTotalMass() const;
  Transform2& getRigidTransform();
  const Vec3& getLinearVelocity() const;
	const Vec3& getAngularVelocity() const;

  #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
  void clampVelocity(Vec3& v) const {
      v.setX(
          fmax(-DRX3D_CLAMP_VELOCITY_TO,
                fmin(DRX3D_CLAMP_VELOCITY_TO, v.getX()))
      );
      v.setY(
          fmax(-DRX3D_CLAMP_VELOCITY_TO,
                fmin(DRX3D_CLAMP_VELOCITY_TO, v.getY()))
      );
      v.setZ(
          fmax(-DRX3D_CLAMP_VELOCITY_TO,
                fmin(DRX3D_CLAMP_VELOCITY_TO, v.getZ()))
      );
  }
  #endif
};

#endif // DRX3D_REDUCED_SOFT_BODY_H