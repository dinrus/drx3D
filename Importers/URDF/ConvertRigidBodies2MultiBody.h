#ifndef CONVERT_RIGIDBODIES_2_MULTIBODY_H
#define CONVERT_RIGIDBODIES_2_MULTIBODY_H

struct ConvertRigidBodies2MultiBody
{
	AlignedObjectArray<RigidBody*> m_bodies;
	AlignedObjectArray<TypedConstraint*> m_constraints;

	virtual void addRigidBody(RigidBody* body);
	virtual void addConstraint(TypedConstraint* constraint, bool disableCollisionsBetweenLinkedBodies = false);
	virtual MultiBody* convertToMultiBody();
};
#endif  //CONVERT_RIGIDBODIES_2_MULTIBODY_H
