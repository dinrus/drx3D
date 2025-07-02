
#ifndef MY_MULTIBODY_CREATOR
#define MY_MULTIBODY_CREATOR

#include "MultiBodyCreationInterface.h"
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>

struct GUIHelperInterface;
class MultiBody;

struct GenericConstraintUserInfo
{
	i32 m_urdfIndex;
	i32 m_urdfJointType;
	Vec3 m_jointAxisInJointSpace;
	i32 m_jointAxisIndex;
	Scalar m_lowerJointLimit;
	Scalar m_upperJointLimit;
};

class MyMultiBodyCreator : public MultiBodyCreationInterface
{
protected:
	MultiBody* m_bulletMultiBody;
	RigidBody* m_rigidBody;

	struct GUIHelperInterface* m_guiHelper;

	AlignedObjectArray<Generic6DofSpring2Constraint*> m_6DofConstraints;

public:
	AlignedObjectArray<i32> m_mb2urdfLink;

	MyMultiBodyCreator(GUIHelperInterface* guiHelper);

	virtual ~MyMultiBodyCreator() {}

	virtual void createRigidBodyGraphicsInstance(i32 linkIndex, class RigidBody* body, const Vec3& colorRgba, i32 graphicsIndex);
	virtual void createRigidBodyGraphicsInstance2(i32 linkIndex, class RigidBody* body, const Vec3& colorRgba, const Vec3& specularColor, i32 graphicsIndex);

	///optionally create some graphical representation from a collision object, usually for visual debugging purposes.
	virtual void createCollisionObjectGraphicsInstance(i32 linkIndex, class CollisionObject2* col, const Vec3& colorRgba);
	virtual void createCollisionObjectGraphicsInstance2(i32 linkIndex, class CollisionObject2* col, const Vec4& colorRgba, const Vec3& specularColor);

	virtual class MultiBody* allocateMultiBody(i32 urdfLinkIndex, i32 totalNumJoints, Scalar mass, const Vec3& localInertiaDiagonal, bool isFixedBase, bool canSleep);

	virtual class RigidBody* allocateRigidBody(i32 urdfLinkIndex, Scalar mass, const Vec3& localInertiaDiagonal, const Transform2& initialWorldTrans, class CollisionShape* colShape);

	virtual class Generic6DofSpring2Constraint* allocateGeneric6DofSpring2Constraint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB, i32 rotateOrder = 0);

	virtual class Generic6DofSpring2Constraint* createPrismaticJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																	   const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit);
	virtual class Generic6DofSpring2Constraint* createRevoluteJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB,
																	  const Vec3& jointAxisInJointSpace, Scalar jointLowerLimit, Scalar jointUpperLimit);

	virtual class Generic6DofSpring2Constraint* createFixedJoint(i32 urdfLinkIndex, RigidBody& rbA /*parent*/, RigidBody& rbB, const Transform2& offsetInA, const Transform2& offsetInB);

	virtual class MultiBodyLinkCollider* allocateMultiBodyLinkCollider(i32 urdfLinkIndex, i32 mbLinkIndex, MultiBody* body);

	virtual void addLinkMapping(i32 urdfLinkIndex, i32 mbLinkIndex);

	MultiBody* getBulletMultiBody();
	RigidBody* getRigidBody()
	{
		return m_rigidBody;
	}

	i32 getNum6DofConstraints() const
	{
		return m_6DofConstraints.size();
	}

	Generic6DofSpring2Constraint* get6DofConstraint(i32 index)
	{
		return m_6DofConstraints[index];
	}
};

#endif  //MY_MULTIBODY_CREATOR
