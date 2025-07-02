#include <drx3D/Physics/SoftBody/DeformableContactConstraint.h>
#include<drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>

// ================= static constraints ===================
class ReducedDeformableStaticConstraint : public DeformableStaticConstraint
{
 public:
  ReducedDeformableBody* m_rsb;
  Scalar m_dt;
  Vec3 m_ri;
  Vec3 m_targetPos;
  Vec3 m_impulseDirection;
  Matrix3x3 m_impulseFactorMatrix;
  Scalar m_impulseFactor;
  Scalar m_rhs;
  Scalar m_appliedImpulse;
  Scalar m_erp;

  ReducedDeformableStaticConstraint(ReducedDeformableBody* rsb, 
                                      SoftBody::Node* node,
                                      const Vec3& ri,
                                      const Vec3& x0,
                                      const Vec3& dir,
                                      const ContactSolverInfo& infoGlobal,
                                      Scalar dt);
	// ReducedDeformableStaticConstraint(const ReducedDeformableStaticConstraint& other);
  ReducedDeformableStaticConstraint() {}
  virtual ~ReducedDeformableStaticConstraint() {}

  virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal);
  
  // this calls reduced deformable body's applyFullSpaceImpulse
  virtual void applyImpulse(const Vec3& impulse);

  Vec3 getDeltaVa() const;

  // virtual void applySplitImpulse(const Vec3& impulse) {}
};

// ================= base contact constraints ===================
class ReducedDeformableRigidContactConstraint : public DeformableRigidContactConstraint
{
 public:
  bool m_collideStatic;     // flag for collision with static object
  bool m_collideMultibody;  // flag for collision with multibody

  i32 m_nodeQueryIndex;
  i32 m_solverBodyId;       // for debugging

  ReducedDeformableBody* m_rsb;
  SolverBody* m_solverBody;
  Scalar m_dt;

  Scalar m_appliedNormalImpulse;
  Scalar m_appliedTangentImpulse;
  Scalar m_appliedTangentImpulse2;
  Scalar m_normalImpulseFactor;
  Scalar m_tangentImpulseFactor;
  Scalar m_tangentImpulseFactor2;
  Scalar m_tangentImpulseFactorInv;
  Scalar m_tangentImpulseFactorInv2;
  Scalar m_rhs;
  Scalar m_rhs_tangent;
  Scalar m_rhs_tangent2;
  
  Scalar m_cfm;
  Scalar m_cfm_friction;
  Scalar m_erp;
  Scalar m_erp_friction;
  Scalar m_friction;

  Vec3 m_contactNormalA;     // surface normal for rigid body (opposite direction as impulse)
  Vec3 m_contactNormalB;     // surface normal for reduced deformable body (opposite direction as impulse)
  Vec3 m_contactTangent;     // tangential direction of the relative velocity
  Vec3 m_contactTangent2;    // 2nd tangential direction of the relative velocity
  Vec3 m_relPosA;            // relative position of the contact point for A (rigid)
  Vec3 m_relPosB;            // relative position of the contact point for B
  Matrix3x3 m_impulseFactor;    // total impulse matrix

  Vec3 m_bufferVelocityA;    // velocity at the beginning of the iteration
  Vec3 m_bufferVelocityB;
  Vec3 m_linearComponentNormal;    // linear components for the solver body
  Vec3 m_angularComponentNormal;   // angular components for the solver body
  // since 2nd contact direction only applies to multibody, these components will never be used
  Vec3 m_linearComponentTangent;
  Vec3 m_angularComponentTangent;

  ReducedDeformableRigidContactConstraint(ReducedDeformableBody* rsb, 
                                            const SoftBody::DeformableRigidContact& c, 
                                            const ContactSolverInfo& infoGlobal,
                                            Scalar dt);
	// ReducedDeformableRigidContactConstraint(const ReducedDeformableRigidContactConstraint& other);
  ReducedDeformableRigidContactConstraint() {}
  virtual ~ReducedDeformableRigidContactConstraint() {}

  void setSolverBody(i32k bodyId, SolverBody& solver_body);

  virtual void warmStarting() {}

  virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal);

  void calculateTangentialImpulse(Scalar& deltaImpulse_tangent, 
                                  Scalar& appliedImpulse, 
                                  const Scalar rhs_tangent,
                                  const Scalar tangentImpulseFactorInv,
                                  const Vec3& tangent,
                                  const Scalar lower_limit,
                                  const Scalar upper_limit,
                                  const Vec3& deltaV_rel);

  virtual void applyImpulse(const Vec3& impulse) {}

  virtual void applySplitImpulse(const Vec3& impulse) {} // TODO: may need later

  virtual Vec3 getVa() const;
  virtual Vec3 getDeltaVa() const = 0;
  virtual Vec3 getDeltaVb() const = 0;
};

// ================= node vs rigid constraints ===================
class ReducedDeformableNodeRigidContactConstraint : public ReducedDeformableRigidContactConstraint
{
 public:
  SoftBody::Node* m_node;

  ReducedDeformableNodeRigidContactConstraint(ReducedDeformableBody* rsb, 
                                                const SoftBody::DeformableNodeRigidContact& contact, 
                                                const ContactSolverInfo& infoGlobal,
                                                Scalar dt);
	// ReducedDeformableNodeRigidContactConstraint(const ReducedDeformableNodeRigidContactConstraint& other);
  ReducedDeformableNodeRigidContactConstraint() {}
  virtual ~ReducedDeformableNodeRigidContactConstraint() {}

  virtual void warmStarting();

  // get the velocity of the deformable node in contact
	virtual Vec3 getVb() const;

  // get the velocity change of the rigid body
  virtual Vec3 getDeltaVa() const;

  // get velocity change of the node in contat
  virtual Vec3 getDeltaVb() const;

	// get the split impulse velocity of the deformable face at the contact point
	virtual Vec3 getSplitVb() const;

	// get the velocity change of the input soft body node in the constraint
	virtual Vec3 getDv(const SoftBody::Node*) const;

	// cast the contact to the desired type
	const SoftBody::DeformableNodeRigidContact* getContact() const
	{
		return static_cast<const SoftBody::DeformableNodeRigidContact*>(m_contact);
	}
  
  // this calls reduced deformable body's applyFullSpaceImpulse
  virtual void applyImpulse(const Vec3& impulse);
};

// ================= face vs rigid constraints ===================
class ReducedDeformableFaceRigidContactConstraint : public ReducedDeformableRigidContactConstraint
{
 public:
  SoftBody::Face* m_face;
	bool m_useStrainLimiting;

  ReducedDeformableFaceRigidContactConstraint(ReducedDeformableBody* rsb, 
                                                const SoftBody::DeformableFaceRigidContact& contact, 
                                                const ContactSolverInfo& infoGlobal,
                                                Scalar dt, 
                                                bool useStrainLimiting);
	// ReducedDeformableFaceRigidContactConstraint(const ReducedDeformableFaceRigidContactConstraint& other);
  ReducedDeformableFaceRigidContactConstraint() {}
  virtual ~ReducedDeformableFaceRigidContactConstraint() {}

  // get the velocity of the deformable face at the contact point
	virtual Vec3 getVb() const;

	// get the split impulse velocity of the deformable face at the contact point
	virtual Vec3 getSplitVb() const;

	// get the velocity change of the input soft body node in the constraint
	virtual Vec3 getDv(const SoftBody::Node*) const;

	// cast the contact to the desired type
	const SoftBody::DeformableFaceRigidContact* getContact() const
	{
		return static_cast<const SoftBody::DeformableFaceRigidContact*>(m_contact);
	}

  // this calls reduced deformable body's applyFullSpaceImpulse
  virtual void applyImpulse(const Vec3& impulse);
};