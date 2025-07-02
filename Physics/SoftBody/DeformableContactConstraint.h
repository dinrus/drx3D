#ifndef DRX3D_DEFORMABLE_CONTACT_CONSTRAINT_H
#define DRX3D_DEFORMABLE_CONTACT_CONSTRAINT_H
#include <drx3D/Physics/SoftBody/SoftBody.h>

// DeformableContactConstraint is an abstract class specifying the method that each type of contact constraint needs to implement
class DeformableContactConstraint
{
public:
	// True if the friction is static
	// False if the friction is dynamic
	bool m_static;
	const ContactSolverInfo* m_infoGlobal;

	// normal of the contact
	Vec3 m_normal;

	DeformableContactConstraint(const Vec3& normal, const ContactSolverInfo& infoGlobal) : m_static(false), m_normal(normal), m_infoGlobal(&infoGlobal)
	{
	}

	DeformableContactConstraint(bool isStatic, const Vec3& normal, const ContactSolverInfo& infoGlobal) : m_static(isStatic), m_normal(normal), m_infoGlobal(&infoGlobal)
	{
	}

	DeformableContactConstraint() : m_static(false) {}

	DeformableContactConstraint(const DeformableContactConstraint& other)
		: m_static(other.m_static), m_normal(other.m_normal), m_infoGlobal(other.m_infoGlobal)
	{
	}

	virtual ~DeformableContactConstraint() {}

	// solve the constraint with inelastic impulse and return the error, which is the square of normal component of velocity diffrerence
	// the constraint is solved by calculating the impulse between object A and B in the contact and apply the impulse to both objects involved in the contact
	virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal) = 0;

	// get the velocity of the object A in the contact
	virtual Vec3 getVa() const = 0;

	// get the velocity of the object B in the contact
	virtual Vec3 getVb() const = 0;

	// get the velocity change of the soft body node in the constraint
	virtual Vec3 getDv(const SoftBody::Node*) const = 0;

	// apply impulse to the soft body node and/or face involved
	virtual void applyImpulse(const Vec3& impulse) = 0;

	// scale the penetration depth by erp
	virtual void setPenetrationScale(Scalar scale) = 0;
};

//
// Constraint that a certain node in the deformable objects cannot move
class DeformableStaticConstraint : public DeformableContactConstraint
{
public:
	SoftBody::Node* m_node;

	DeformableStaticConstraint(SoftBody::Node* node, const ContactSolverInfo& infoGlobal) : m_node(node), DeformableContactConstraint(false, Vec3(0, 0, 0), infoGlobal)
	{
	}
	DeformableStaticConstraint() {}
	DeformableStaticConstraint(const DeformableStaticConstraint& other)
		: m_node(other.m_node), DeformableContactConstraint(other)
	{
	}

	virtual ~DeformableStaticConstraint() {}

	virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal)
	{
		return 0;
	}

	virtual Vec3 getVa() const
	{
		return Vec3(0, 0, 0);
	}

	virtual Vec3 getVb() const
	{
		return Vec3(0, 0, 0);
	}

	virtual Vec3 getDv(const SoftBody::Node* n) const
	{
		return Vec3(0, 0, 0);
	}

	virtual void applyImpulse(const Vec3& impulse) {}
	virtual void setPenetrationScale(Scalar scale) {}
};

//
// Anchor Constraint between rigid and deformable node
class DeformableNodeAnchorConstraint : public DeformableContactConstraint
{
public:
	const SoftBody::DeformableNodeRigidAnchor* m_anchor;

	DeformableNodeAnchorConstraint(const SoftBody::DeformableNodeRigidAnchor& c, const ContactSolverInfo& infoGlobal);
	DeformableNodeAnchorConstraint(const DeformableNodeAnchorConstraint& other);
	DeformableNodeAnchorConstraint() {}
	virtual ~DeformableNodeAnchorConstraint()
	{
	}
	virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal);

	// object A is the rigid/multi body, and object B is the deformable node/face
	virtual Vec3 getVa() const;
	// get the velocity of the deformable node in contact
	virtual Vec3 getVb() const;
	virtual Vec3 getDv(const SoftBody::Node* n) const
	{
		return Vec3(0, 0, 0);
	}
	virtual void applyImpulse(const Vec3& impulse);

	virtual void setPenetrationScale(Scalar scale) {}
};

//
// Constraint between rigid/multi body and deformable objects
class DeformableRigidContactConstraint : public DeformableContactConstraint
{
public:
	Vec3 m_total_normal_dv;
	Vec3 m_total_tangent_dv;
	Scalar m_penetration;
	Scalar m_total_split_impulse;
	bool m_binding;
	const SoftBody::DeformableRigidContact* m_contact;

	DeformableRigidContactConstraint(const SoftBody::DeformableRigidContact& c, const ContactSolverInfo& infoGlobal);
	DeformableRigidContactConstraint(const DeformableRigidContactConstraint& other);
	DeformableRigidContactConstraint() : m_binding(false) {}
	virtual ~DeformableRigidContactConstraint()
	{
	}

	// object A is the rigid/multi body, and object B is the deformable node/face
	virtual Vec3 getVa() const;

	// get the split impulse velocity of the deformable face at the contact point
	virtual Vec3 getSplitVb() const = 0;

	// get the split impulse velocity of the rigid/multibdoy at the contaft
	virtual Vec3 getSplitVa() const;

	virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal);

	virtual void setPenetrationScale(Scalar scale)
	{
		m_penetration *= scale;
	}

	Scalar solveSplitImpulse(const ContactSolverInfo& infoGlobal);

	virtual void applySplitImpulse(const Vec3& impulse) = 0;
};

//
// Constraint between rigid/multi body and deformable objects nodes
class DeformableNodeRigidContactConstraint : public DeformableRigidContactConstraint
{
public:
	// the deformable node in contact
	SoftBody::Node* m_node;

	DeformableNodeRigidContactConstraint(const SoftBody::DeformableNodeRigidContact& contact, const ContactSolverInfo& infoGlobal);
	DeformableNodeRigidContactConstraint(const DeformableNodeRigidContactConstraint& other);
	DeformableNodeRigidContactConstraint() {}
	virtual ~DeformableNodeRigidContactConstraint()
	{
	}

	// get the velocity of the deformable node in contact
	virtual Vec3 getVb() const;

	// get the split impulse velocity of the deformable face at the contact point
	virtual Vec3 getSplitVb() const;

	// get the velocity change of the input soft body node in the constraint
	virtual Vec3 getDv(const SoftBody::Node*) const;

	// cast the contact to the desired type
	const SoftBody::DeformableNodeRigidContact* getContact() const
	{
		return static_cast<const SoftBody::DeformableNodeRigidContact*>(m_contact);
	}

	virtual void applyImpulse(const Vec3& impulse);

	virtual void applySplitImpulse(const Vec3& impulse);
};

//
// Constraint between rigid/multi body and deformable objects faces
class DeformableFaceRigidContactConstraint : public DeformableRigidContactConstraint
{
public:
	SoftBody::Face* m_face;
	bool m_useStrainLimiting;
	DeformableFaceRigidContactConstraint(const SoftBody::DeformableFaceRigidContact& contact, const ContactSolverInfo& infoGlobal, bool useStrainLimiting);
	DeformableFaceRigidContactConstraint(const DeformableFaceRigidContactConstraint& other);
	DeformableFaceRigidContactConstraint() : m_useStrainLimiting(false) {}
	virtual ~DeformableFaceRigidContactConstraint()
	{
	}

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

	virtual void applyImpulse(const Vec3& impulse);

	virtual void applySplitImpulse(const Vec3& impulse);
};

//
// Constraint between  deformable objects faces and deformable objects nodes
class DeformableFaceNodeContactConstraint : public DeformableContactConstraint
{
public:
	SoftBody::Node* m_node;
	SoftBody::Face* m_face;
	const SoftBody::DeformableFaceNodeContact* m_contact;
	Vec3 m_total_normal_dv;
	Vec3 m_total_tangent_dv;

	DeformableFaceNodeContactConstraint(const SoftBody::DeformableFaceNodeContact& contact, const ContactSolverInfo& infoGlobal);
	DeformableFaceNodeContactConstraint() {}
	virtual ~DeformableFaceNodeContactConstraint() {}

	virtual Scalar solveConstraint(const ContactSolverInfo& infoGlobal);

	// get the velocity of the object A in the contact
	virtual Vec3 getVa() const;

	// get the velocity of the object B in the contact
	virtual Vec3 getVb() const;

	// get the velocity change of the input soft body node in the constraint
	virtual Vec3 getDv(const SoftBody::Node*) const;

	// cast the contact to the desired type
	const SoftBody::DeformableFaceNodeContact* getContact() const
	{
		return static_cast<const SoftBody::DeformableFaceNodeContact*>(m_contact);
	}

	virtual void applyImpulse(const Vec3& impulse);

	virtual void setPenetrationScale(Scalar scale) {}
};
#endif /* DRX3D_DEFORMABLE_CONTACT_CONSTRAINT_H */
