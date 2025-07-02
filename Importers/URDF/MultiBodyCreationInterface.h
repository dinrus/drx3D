#ifndef MULTIBODY_CREATION_INTERFACE_H
#define MULTIBODY_CREATION_INTERFACE_H

#include <drx3D/Maths/Linear/Transform2.h>

class MultiBodyCreationInterface
{
public:
	virtual ~MultiBodyCreationInterface() {}

	virtual void createRigidBodyGraphicsInstance(i32 linkIndex, class RigidBody* body, const Vec3& colorRgba, i32 graphicsIndex) = 0;
	virtual void createRigidBodyGraphicsInstance2(i32 linkIndex, class RigidBody* body, const Vec3& colorRgba, const Vec3& specularColor, i32 graphicsIndex)
	{
		createRigidBodyGraphicsInstance(linkIndex, body, colorRgba, graphicsIndex);
	}

	///optionally create some graphical representation from a collision object, usually for visual debugging purposes.
	virtual void createCollisionObjectGraphicsInstance(i32 linkIndex, class CollisionObject2* col, const Vec3& colorRgba) = 0;
	virtual void createCollisionObjectGraphicsInstance2(i32 linkIndex, class CollisionObject2* col, const Vec4& colorRgba, const Vec3& specularColor)
	{
		createCollisionObjectGraphicsInstance(linkIndex, col, colorRgba);
	}

	virtual class MultiBody* allocateMultiBody(i32 urdfLinkIndex, i32 totalNumJoints, Scalar mass, const Vec3& localInertiaDiagonal, bool isFixedBase, bool canSleep) = 0;

	virtual class RigidBody* allocateRigidBody(i32 urdfLinkIndex, Scalar mass, const Vec3& localInertiaDiagonal, const Transform2& initialWorldTrans, class CollisionShape* colShape) = 0;

	virtual class Generic6DofSpring2Constraint* allocateGeneric6DofSpring2Constraint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB, i32 rotateOrder = 0) = 0;

	virtual class Generic6DofSpring2Constraint* createPrismaticJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																	   const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit) = 0;
	virtual class Generic6DofSpring2Constraint* createRevoluteJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																	  const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit) = 0;

	virtual class Generic6DofSpring2Constraint* createFixedJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB) = 0;

	virtual class MultiBodyLinkCollider* allocateMultiBodyLinkCollider(i32 urdfLinkIndex, i32 mbLinkIndex, MultiBody* body) = 0;

	virtual void addLinkMapping(i32 urdfLinkIndex, i32 mbLinkIndex) = 0;
};

#endif  //MULTIBODY_CREATION_INTERFACE_H