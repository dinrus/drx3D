
#include <stdio.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointLimitConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySphericalJointLimit.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Importers/URDF/URDFImporterInterface.h>
#include <drx3D/Importers/URDF/MultiBodyCreationInterface.h>
#include <string>
#include <drx3D/Common/b3Logging.h>

//static i32 bodyCollisionFilterGroup=BroadphaseProxy::CharacterFilter;
//static i32 bodyCollisionFilterMask=BroadphaseProxy::AllFilter&(~BroadphaseProxy::CharacterFilter);
static bool enableConstraints = true;

static Vec4 gGoogleyColors[4] =
{
		Vec4(60. / 256., 186. / 256., 84. / 256., 1),
		Vec4(244. / 256., 194. / 256., 13. / 256., 1),
		Vec4(219. / 256., 50. / 256., 54. / 256., 1),
		Vec4(72. / 256., 133. / 256., 237. / 256., 1),
};

static Vec4 selectColor2()
{
#ifdef DRX3D_THREADSAFE
	static SpinMutex sMutex;
	sMutex.lock();
#endif
	static i32 curColor = 0;
	Vec4 color = gGoogleyColors[curColor];
	curColor++;
	curColor &= 3;
#ifdef DRX3D_THREADSAFE
	sMutex.unlock();
#endif
	return color;
}

struct URDF2BulletCachedData
{
	URDF2BulletCachedData()
		: m_currentMultiBodyLinkIndex(-1),
		  m_bulletMultiBody(0),
		  m_totalNumJoints1(0)
	{
	}
	//these arrays will be initialized in the 'InitURDF2BulletCache'

	AlignedObjectArray<i32> m_urdfLinkParentIndices;
	AlignedObjectArray<i32> m_urdfLinkIndices2BulletLinkIndices;
	AlignedObjectArray<class RigidBody*> m_urdfLink2rigidBodies;
	AlignedObjectArray<Transform2> m_urdfLinkLocalInertialFrames;

	i32 m_currentMultiBodyLinkIndex;

	class MultiBody* m_bulletMultiBody;

	//this will be initialized in the constructor
	i32 m_totalNumJoints1;
	i32 getParentUrdfIndex(i32 linkIndex) const
	{
		return m_urdfLinkParentIndices[linkIndex];
	}
	i32 getMbIndexFromUrdfIndex(i32 urdfIndex) const
	{
		if (urdfIndex == -2)
			return -2;
		return m_urdfLinkIndices2BulletLinkIndices[urdfIndex];
	}

	void registerMultiBody(i32 urdfLinkIndex, class MultiBody* body, const Transform2& worldTransform, Scalar mass, const Vec3& localInertiaDiagonal, const class CollisionShape* compound, const Transform2& localInertialFrame)
	{
		m_urdfLinkLocalInertialFrames[urdfLinkIndex] = localInertialFrame;
	}

	class RigidBody* getRigidBodyFromLink(i32 urdfLinkIndex)
	{
		return m_urdfLink2rigidBodies[urdfLinkIndex];
	}

	void registerRigidBody(i32 urdfLinkIndex, class RigidBody* body, const Transform2& worldTransform, Scalar mass, const Vec3& localInertiaDiagonal, const class CollisionShape* compound, const Transform2& localInertialFrame)
	{
		Assert(m_urdfLink2rigidBodies[urdfLinkIndex] == 0);

		m_urdfLink2rigidBodies[urdfLinkIndex] = body;
		m_urdfLinkLocalInertialFrames[urdfLinkIndex] = localInertialFrame;
	}
};

void ComputeTotalNumberOfJoints(const URDFImporterInterface& u2b, URDF2BulletCachedData& cache, i32 linkIndex)
{
	AlignedObjectArray<i32> childIndices;
	u2b.getLinkChildIndices(linkIndex, childIndices);
	//drx3DPrintf("link %s has %d children\n", u2b.getLinkName(linkIndex).c_str(),childIndices.size());
	//for (i32 i=0;i<childIndices.size();i++)
	//{
	//    drx3DPrintf("child %d has childIndex%d=%s\n",i,childIndices[i],u2b.getLinkName(childIndices[i]).c_str());
	//}
	cache.m_totalNumJoints1 += childIndices.size();
	for (i32 i = 0; i < childIndices.size(); i++)
	{
		i32 childIndex = childIndices[i];
		ComputeTotalNumberOfJoints(u2b, cache, childIndex);
	}
}


void ComputeParentIndices(const URDFImporterInterface& u2b, URDF2BulletCachedData& cache, i32 urdfLinkIndex, i32 urdfParentIndex)
{
	cache.m_urdfLinkParentIndices[urdfLinkIndex] = urdfParentIndex;
	cache.m_urdfLinkIndices2BulletLinkIndices[urdfLinkIndex] = cache.m_currentMultiBodyLinkIndex++;

	AlignedObjectArray<i32> childIndices;
	u2b.getLinkChildIndices(urdfLinkIndex, childIndices);
	for (i32 i = 0; i < childIndices.size(); i++)
	{
		ComputeParentIndices(u2b, cache, childIndices[i], urdfLinkIndex);
	}
}

void InitURDF2BulletCache(const URDFImporterInterface& u2b, URDF2BulletCachedData& cache, i32 flags)
{
	//compute the number of links, and compute parent indices array (and possibly other cached data?)
	cache.m_totalNumJoints1 = 0;

	i32 rootLinkIndex = u2b.getRootLinkIndex();
	if (rootLinkIndex >= 0)
	{
		ComputeTotalNumberOfJoints(u2b, cache, rootLinkIndex);
		i32 numTotalLinksIncludingBase = 1 + cache.m_totalNumJoints1;

		cache.m_urdfLinkParentIndices.resize(numTotalLinksIncludingBase);
		cache.m_urdfLinkIndices2BulletLinkIndices.resize(numTotalLinksIncludingBase);
		cache.m_urdfLink2rigidBodies.resize(numTotalLinksIncludingBase);
		cache.m_urdfLinkLocalInertialFrames.resize(numTotalLinksIncludingBase);

		cache.m_currentMultiBodyLinkIndex = -1;  //multi body base has 'link' index -1

		bool maintainLinkOrder  = (flags & CUF_MAINTAIN_LINK_ORDER)!=0;
		if (maintainLinkOrder)
		{
			URDF2BulletCachedData cache2 = cache;

			ComputeParentIndices(u2b, cache2, rootLinkIndex, -2);

			for (i32 j=0;j<numTotalLinksIncludingBase;j++)
			{
				cache.m_urdfLinkParentIndices[j] = cache2.m_urdfLinkParentIndices[j];
				cache.m_urdfLinkIndices2BulletLinkIndices[j] = j - 1;
			}
		}else
		{
			ComputeParentIndices(u2b, cache, rootLinkIndex, -2);
		}

	}
}

void processContactParameters(const URDFLinkContactInfo& contactInfo, CollisionObject2* col)
{
	if ((contactInfo.m_flags & URDF_CONTACT_HAS_LATERAL_FRICTION) != 0)
	{
		col->setFriction(contactInfo.m_lateralFriction);
	}
	if ((contactInfo.m_flags & URDF_CONTACT_HAS_RESTITUTION) != 0)
	{
		col->setRestitution(contactInfo.m_restitution);
	}

	if ((contactInfo.m_flags & URDF_CONTACT_HAS_ROLLING_FRICTION) != 0)
	{
		col->setRollingFriction(contactInfo.m_rollingFriction);
	}
	if ((contactInfo.m_flags & URDF_CONTACT_HAS_SPINNING_FRICTION) != 0)
	{
		col->setSpinningFriction(contactInfo.m_spinningFriction);
	}
	if ((contactInfo.m_flags & URDF_CONTACT_HAS_STIFFNESS_DAMPING) != 0)
	{
		col->setContactStiffnessAndDamping(contactInfo.m_contactStiffness, contactInfo.m_contactDamping);
	}
	if ((contactInfo.m_flags & URDF_CONTACT_HAS_FRICTION_ANCHOR) != 0)
	{
		col->setCollisionFlags(col->getCollisionFlags() | CollisionObject2::CF_HAS_FRICTION_ANCHOR);
	}
}

Scalar tmpUrdfScaling = 2;

Transform2 ConvertURDF2BulletInternal(
	const URDFImporterInterface& u2b, MultiBodyCreationInterface& creation,
	URDF2BulletCachedData& cache, i32 urdfLinkIndex,
	const Transform2& parentTransformInWorldSpace,

#ifdef USE_DISCRETE_DYNAMICS_WORLD
	DiscreteDynamicsWorld* world1,
#else
	MultiBodyDynamicsWorld* world1,
#endif


	bool createMultiBody, tukk pathPrefix,
	i32 flags, UrdfVisualShapeCache* cachedLinkGraphicsShapesIn, UrdfVisualShapeCache* cachedLinkGraphicsShapesOut, bool recursive)
{
	D3_PROFILE("ConvertURDF2BulletInternal2");
	//drx3DPrintf("start converting/extracting data from URDF interface\n");

	Transform2 linkTransformInWorldSpace;
	linkTransformInWorldSpace.setIdentity();

	i32 mbLinkIndex = cache.getMbIndexFromUrdfIndex(urdfLinkIndex);

	i32 urdfParentIndex = cache.getParentUrdfIndex(urdfLinkIndex);
	i32 mbParentIndex = cache.getMbIndexFromUrdfIndex(urdfParentIndex);
	RigidBody* parentRigidBody = 0;

	//drx3DPrintf("mb link index = %d\n",mbLinkIndex);

	Transform2 parentLocalInertialFrame;
	parentLocalInertialFrame.setIdentity();
	Scalar parentMass(1);
	Vec3 parentLocalInertiaDiagonal(1, 1, 1);

	if (urdfParentIndex == -2)
	{
		//drx3DPrintf("root link has no parent\n");
	}
	else
	{
		//drx3DPrintf("urdf parent index = %d\n",urdfParentIndex);
		//drx3DPrintf("mb parent index = %d\n",mbParentIndex);
		parentRigidBody = cache.getRigidBodyFromLink(urdfParentIndex);
		u2b.getMassAndInertia2(urdfParentIndex, parentMass, parentLocalInertiaDiagonal, parentLocalInertialFrame, flags);
	}

	Scalar mass = 0;
	Transform2 localInertialFrame;
	localInertialFrame.setIdentity();
	Vec3 localInertiaDiagonal(0, 0, 0);
	u2b.getMassAndInertia2(urdfLinkIndex, mass, localInertiaDiagonal, localInertialFrame, flags);

	Transform2 parent2joint;
	parent2joint.setIdentity();

	i32 jointType;
	Vec3 jointAxisInJointSpace;
	Scalar jointLowerLimit;
	Scalar jointUpperLimit;
	Scalar jointDamping;
	Scalar jointFriction;
	Scalar jointMaxForce;
	Scalar jointMaxVelocity;
	Scalar twistLimit;
	bool hasParentJoint = u2b.getJointInfo3(urdfLinkIndex, parent2joint, linkTransformInWorldSpace, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction, jointMaxForce, jointMaxVelocity, twistLimit);
	STxt linkName = u2b.getLinkName(urdfLinkIndex);

	if (flags & CUF_USE_SDF)
	{
		parent2joint = parentTransformInWorldSpace.inverse() * linkTransformInWorldSpace;
	}
	else
	{
		if (flags & CUF_USE_MJCF)
		{
			linkTransformInWorldSpace = parentTransformInWorldSpace * linkTransformInWorldSpace;
		}
		else
		{
			linkTransformInWorldSpace = parentTransformInWorldSpace * parent2joint;
		}
	}

	CompoundShape* tmpShape = u2b.convertLinkCollisionShapes(urdfLinkIndex, pathPrefix, localInertialFrame);
	CollisionShape* compoundShape = tmpShape;
	if (tmpShape->getNumChildShapes() == 1 && tmpShape->getChildTransform(0) == Transform2::getIdentity())
	{
		compoundShape = tmpShape->getChildShape(0);
	}

	i32 graphicsIndex;
	{
		D3_PROFILE("convertLinkVisualShapes");
		if (cachedLinkGraphicsShapesIn && cachedLinkGraphicsShapesIn->m_cachedUrdfLinkVisualShapeIndices.size() > (mbLinkIndex + 1))
		{
			graphicsIndex = cachedLinkGraphicsShapesIn->m_cachedUrdfLinkVisualShapeIndices[mbLinkIndex + 1];
			UrdfMaterialColor matColor = cachedLinkGraphicsShapesIn->m_cachedUrdfLinkColors[mbLinkIndex + 1];
			u2b.setLinkColor2(urdfLinkIndex, matColor);
		}
		else
		{
			graphicsIndex = u2b.convertLinkVisualShapes(urdfLinkIndex, pathPrefix, localInertialFrame);
			if (cachedLinkGraphicsShapesOut)
			{
				cachedLinkGraphicsShapesOut->m_cachedUrdfLinkVisualShapeIndices.push_back(graphicsIndex);
				UrdfMaterialColor matColor;
				u2b.getLinkColor2(urdfLinkIndex, matColor);
				cachedLinkGraphicsShapesOut->m_cachedUrdfLinkColors.push_back(matColor);
			}
		}
	}

	if (compoundShape)
	{
		UrdfMaterialColor matColor;

		Vec4 color2 = (flags & CUF_GOOGLEY_UNDEFINED_COLORS) ? selectColor2() : Vec4(1, 1, 1, 1);
		Vec3 specular(0.5, 0.5, 0.5);
		if (u2b.getLinkColor2(urdfLinkIndex, matColor))
		{
			color2 = matColor.m_rgbaColor;
			specular = matColor.m_specularColor;
		}

		/*
         if (visual->material.get())
         {
            color.setVal(visual->material->color.r,visual->material->color.g,visual->material->color.b);//,visual->material->color.a);
         }
         */
		if (mass)
		{
			if (!(flags & CUF_USE_URDF_INERTIA))
			{
				compoundShape->calculateLocalInertia(mass, localInertiaDiagonal);
				Assert(localInertiaDiagonal[0] < 1e10);
				Assert(localInertiaDiagonal[1] < 1e10);
				Assert(localInertiaDiagonal[2] < 1e10);
			}
			URDFLinkContactInfo contactInfo;
			u2b.getLinkContactInfo(urdfLinkIndex, contactInfo);
			//temporary inertia scaling until we load inertia from URDF
			if (contactInfo.m_flags & URDF_CONTACT_HAS_INERTIA_SCALING)
			{
				localInertiaDiagonal *= contactInfo.m_inertiaScaling;
			}
		}

		RigidBody* linkRigidBody = 0;
		Transform2 inertialFrameInWorldSpace = linkTransformInWorldSpace * localInertialFrame;
		bool canSleep = (flags & CUF_ENABLE_SLEEPING) != 0;

		if (!createMultiBody)
		{
			RigidBody* body = creation.allocateRigidBody(urdfLinkIndex, mass, localInertiaDiagonal, inertialFrameInWorldSpace, compoundShape);

			if (!canSleep)
			{
				body->forceActivationState(DISABLE_DEACTIVATION);
			}

			linkRigidBody = body;

			world1->addRigidBody(body);

			compoundShape->setUserIndex(graphicsIndex);

			URDFLinkContactInfo contactInfo;
			u2b.getLinkContactInfo(urdfLinkIndex, contactInfo);

			processContactParameters(contactInfo, body);
			creation.createRigidBodyGraphicsInstance2(urdfLinkIndex, body, color2, specular, graphicsIndex);
			cache.registerRigidBody(urdfLinkIndex, body, inertialFrameInWorldSpace, mass, localInertiaDiagonal, compoundShape, localInertialFrame);

			//untested: u2b.convertLinkVisualShapes2(linkIndex,urdfLinkIndex,pathPrefix,localInertialFrame,body);
		}
		else
		{
			if (cache.m_bulletMultiBody == 0)
			{
				bool isFixedBase = (mass == 0);  //todo: figure out when base is fixed
				i32 totalNumJoints = cache.m_totalNumJoints1;
				cache.m_bulletMultiBody = creation.allocateMultiBody(urdfLinkIndex, totalNumJoints, mass, localInertiaDiagonal, isFixedBase, canSleep);
				if (flags & CUF_GLOBAL_VELOCITIES_MB)
				{
					cache.m_bulletMultiBody->useGlobalVelocities(true);
				}
				if (flags & CUF_USE_MJCF)
				{
					cache.m_bulletMultiBody->setBaseWorldTransform(linkTransformInWorldSpace);
				}

				cache.registerMultiBody(urdfLinkIndex, cache.m_bulletMultiBody, inertialFrameInWorldSpace, mass, localInertiaDiagonal, compoundShape, localInertialFrame);
			}
		}

		//create a joint if necessary
		if (hasParentJoint)
		{
			Transform2 offsetInA, offsetInB;
			offsetInA = parentLocalInertialFrame.inverse() * parent2joint;
			offsetInB = localInertialFrame.inverse();
			Quat parentRotToThis = offsetInB.getRotation() * offsetInA.inverse().getRotation();

			bool disableParentCollision = true;

			if (createMultiBody && cache.m_bulletMultiBody)
			{
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointDamping = jointDamping;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointFriction = jointFriction;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointLowerLimit = jointLowerLimit;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointUpperLimit = jointUpperLimit;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointMaxForce = jointMaxForce;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointMaxVelocity = jointMaxVelocity;
			}

			switch (jointType)
			{
				case URDFSphericalJoint:
				{
					if (createMultiBody)
					{
						creation.addLinkMapping(urdfLinkIndex, mbLinkIndex);
						cache.m_bulletMultiBody->setupSpherical(mbLinkIndex, mass, localInertiaDiagonal, mbParentIndex,
							parentRotToThis, offsetInA.getOrigin(), -offsetInB.getOrigin(),
							disableParentCollision);


						//create a spherical joint limit, swing_x,. swing_y and twist
						//jointLowerLimit <= jointUpperLimit)
						if (jointUpperLimit > 0 && jointLowerLimit> 0 && twistLimit > 0 && jointMaxForce>0)
						{
							MultiBodySphericalJointLimit* con = new MultiBodySphericalJointLimit(cache.m_bulletMultiBody, mbLinkIndex,
								jointLowerLimit,
								jointUpperLimit,
								twistLimit,
								jointMaxForce);
							world1->addMultiBodyConstraint(con);
						}

					}
					else
					{
						Assert(0);
					}
					break;
				}
				case URDFPlanarJoint:
				{

					if (createMultiBody)
					{
#if 0
						void setupPlanar(i32 i,  // 0 to num_links-1
							Scalar mass,
							const Vec3 &inertia,
							i32 parent,
							const Quat &rotParentToThis,  // rotate points in parent frame to this frame, when q = 0
							const Vec3 &rotationAxis,
							const Vec3 &parentComToThisComOffset,  // vector from parent COM to this COM, in PARENT frame
							bool disableParentCollision = false);
#endif
						creation.addLinkMapping(urdfLinkIndex, mbLinkIndex);
						cache.m_bulletMultiBody->setupPlanar(mbLinkIndex, mass, localInertiaDiagonal, mbParentIndex,
							parentRotToThis, quatRotate(offsetInB.getRotation(), jointAxisInJointSpace), offsetInA.getOrigin(),
							disableParentCollision);
					}
					else
					{
#if 0
						//drx3DPrintf("Fixed joint\n");

						btGeneric6DofSpring2Constraint* dof6 = 0;

						//backward compatibility
						if (flags & CUF_RESERVED)
						{
							dof6 = creation.createFixedJoint(urdfLinkIndex, *parentRigidBody, *linkRigidBody, offsetInA, offsetInB);
						}
						else
						{
							dof6 = creation.createFixedJoint(urdfLinkIndex, *linkRigidBody, *parentRigidBody, offsetInB, offsetInA);
						}
						if (enableConstraints)
							world1->addConstraint(dof6, true);
#endif
					}
					break;
				}
				case URDFFloatingJoint:

				case URDFFixedJoint:
				{
					if ((jointType == URDFFloatingJoint) || (jointType == URDFPlanarJoint))
					{
						printf("Warning: joint unsupported, creating a fixed joint instead.");
					}
					creation.addLinkMapping(urdfLinkIndex, mbLinkIndex);

					if (createMultiBody)
					{
						//todo: adjust the center of mass transform and pivot axis properly
						cache.m_bulletMultiBody->setupFixed(mbLinkIndex, mass, localInertiaDiagonal, mbParentIndex,
															parentRotToThis, offsetInA.getOrigin(), -offsetInB.getOrigin());
					}
					else
					{
						//drx3DPrintf("Fixed joint\n");

						Generic6DofSpring2Constraint* dof6 = 0;

						//backward compatibility
						if (flags & CUF_RESERVED)
						{
							dof6 = creation.createFixedJoint(urdfLinkIndex, *parentRigidBody, *linkRigidBody, offsetInA, offsetInB);
						}
						else
						{
							dof6 = creation.createFixedJoint(urdfLinkIndex, *linkRigidBody, *parentRigidBody, offsetInB, offsetInA);
						}
						if (enableConstraints)
							world1->addConstraint(dof6, true);
					}
					break;
				}
				case URDFContinuousJoint:
				case URDFRevoluteJoint:
				{
					creation.addLinkMapping(urdfLinkIndex, mbLinkIndex);
					if (createMultiBody)
					{
#ifndef USE_DISCRETE_DYNAMICS_WORLD
						cache.m_bulletMultiBody->setupRevolute(mbLinkIndex, mass, localInertiaDiagonal, mbParentIndex,
															   parentRotToThis, quatRotate(offsetInB.getRotation(),
																   jointAxisInJointSpace),
																offsetInA.getOrigin(),
															   -offsetInB.getOrigin(),
															   disableParentCollision);

						if (jointType == URDFRevoluteJoint && jointLowerLimit <= jointUpperLimit)
						{
							//STxt name = u2b.getLinkName(urdfLinkIndex);
							//printf("create btMultiBodyJointLimitConstraint for revolute link name=%s urdf link index=%d (low=%f, up=%f)\n", name.c_str(), urdfLinkIndex, jointLowerLimit, jointUpperLimit);
							MultiBodyConstraint* con = new MultiBodyJointLimitConstraint(cache.m_bulletMultiBody, mbLinkIndex, jointLowerLimit, jointUpperLimit);
							world1->addMultiBodyConstraint(con);
						}
#endif
					}
					else

					{
						Generic6DofSpring2Constraint* dof6 = 0;
						if (jointType == URDFRevoluteJoint && jointLowerLimit <= jointUpperLimit)
						{
							//backwards compatibility
							if (flags & CUF_RESERVED)
							{
								dof6 = creation.createRevoluteJoint(urdfLinkIndex, *parentRigidBody, *linkRigidBody, offsetInA, offsetInB, jointAxisInJointSpace, jointLowerLimit, jointUpperLimit);
							}
							else
							{
								dof6 = creation.createRevoluteJoint(urdfLinkIndex, *linkRigidBody, *parentRigidBody, offsetInB, offsetInA, jointAxisInJointSpace, jointLowerLimit, jointUpperLimit);
							}
						}
						else
						{
							//disable joint limits
							if (flags & CUF_RESERVED)
							{
								dof6 = creation.createRevoluteJoint(urdfLinkIndex, *parentRigidBody, *linkRigidBody, offsetInA, offsetInB, jointAxisInJointSpace, 1, -1);
							}
							else
							{
								dof6 = creation.createRevoluteJoint(urdfLinkIndex, *linkRigidBody, *parentRigidBody, offsetInB, offsetInA, jointAxisInJointSpace, 1, -1);
							}
						}

						if (enableConstraints)
							world1->addConstraint(dof6, true);
						//drx3DPrintf("Revolute/Continuous joint\n");
					}
					break;
				}
				case URDFPrismaticJoint:
				{
					creation.addLinkMapping(urdfLinkIndex, mbLinkIndex);

					if (createMultiBody)
					{
#ifndef USE_DISCRETE_DYNAMICS_WORLD
						cache.m_bulletMultiBody->setupPrismatic(mbLinkIndex, mass, localInertiaDiagonal, mbParentIndex,
																parentRotToThis, quatRotate(offsetInB.getRotation(), jointAxisInJointSpace), offsetInA.getOrigin(),  //parent2joint.getOrigin(),
																-offsetInB.getOrigin(),
																disableParentCollision);

						if (jointLowerLimit <= jointUpperLimit)
						{
							//STxt name = u2b.getLinkName(urdfLinkIndex);
							//printf("create btMultiBodyJointLimitConstraint for prismatic link name=%s urdf link index=%d (low=%f, up=%f)\n", name.c_str(), urdfLinkIndex, jointLowerLimit,jointUpperLimit);

							MultiBodyConstraint* con = new MultiBodyJointLimitConstraint(cache.m_bulletMultiBody, mbLinkIndex, jointLowerLimit, jointUpperLimit);
							world1->addMultiBodyConstraint(con);
						}
						//printf("joint lower limit=%d, upper limit = %f\n", jointLowerLimit, jointUpperLimit);
#endif
					}
					else
					{
						Generic6DofSpring2Constraint* dof6 = creation.createPrismaticJoint(urdfLinkIndex, *parentRigidBody, *linkRigidBody, offsetInA, offsetInB, jointAxisInJointSpace, jointLowerLimit, jointUpperLimit);

						if (enableConstraints)
							world1->addConstraint(dof6, true);

						//drx3DPrintf("Prismatic\n");
					}
					break;
				}
				default:
				{
					//drx3DPrintf("Ошибка: unsupported joint type in URDF (%d)\n", jointType);
					Assert(0);
				}
			}
		}

		if (createMultiBody)
		{
			//if (compoundShape->getNumChildShapes()>0)
			{
				MultiBodyLinkCollider* col = creation.allocateMultiBodyLinkCollider(urdfLinkIndex, mbLinkIndex, cache.m_bulletMultiBody);

				compoundShape->setUserIndex(graphicsIndex);

				col->setCollisionShape(compoundShape);

				if (compoundShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
				{
					BvhTriangleMeshShape* trimeshShape = (BvhTriangleMeshShape*)compoundShape;
					if (trimeshShape->getTriangleInfoMap())
					{
						col->setCollisionFlags(col->getCollisionFlags() | CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK);
					}
				}

				if (compoundShape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
				{
					col->setCollisionFlags(col->getCollisionFlags() | CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK);
				}

				Transform2 tr;
				tr.setIdentity();
				tr = linkTransformInWorldSpace;
				//if we don't set the initial pose of the CollisionObject2, the simulator will do this
				//when syncing the btMultiBody link transforms to the btMultiBodyLinkCollider

				col->setWorldTransform(tr);

				//base and fixed? -> static, otherwise flag as dynamic
				bool isDynamic = (mbLinkIndex < 0 && cache.m_bulletMultiBody->hasFixedBase()) ? false : true;
				i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
				i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

				i32 colGroup = 0, colMask = 0;
				i32 collisionFlags = u2b.getCollisionGroupAndMask(urdfLinkIndex, colGroup, colMask);
				if (collisionFlags & URDF_HAS_COLLISION_GROUP)
				{
					collisionFilterGroup = colGroup;
				}
				if (collisionFlags & URDF_HAS_COLLISION_MASK)
				{
					collisionFilterMask = colMask;
				}
				world1->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);

				Vec4 color2 = (flags & CUF_GOOGLEY_UNDEFINED_COLORS) ? selectColor2() : Vec4(1, 1, 1, 1);
				Vec3 specularColor(1, 1, 1);
				UrdfMaterialColor matCol;
				if (u2b.getLinkColor2(urdfLinkIndex, matCol))
				{
					color2 = matCol.m_rgbaColor;
					specularColor = matCol.m_specularColor;
				}
				{
					D3_PROFILE("createCollisionObjectGraphicsInstance2");
					creation.createCollisionObjectGraphicsInstance2(urdfLinkIndex, col, color2, specularColor);
				}
				{
					D3_PROFILE("convertLinkVisualShapes2");
					u2b.convertLinkVisualShapes2(mbLinkIndex, urdfLinkIndex, pathPrefix, localInertialFrame, col, u2b.getBodyUniqueId());
				}
				URDFLinkContactInfo contactInfo;
				u2b.getLinkContactInfo(urdfLinkIndex, contactInfo);

				processContactParameters(contactInfo, col);

				if (mbLinkIndex >= 0)  //???? double-check +/- 1
				{
					//if the base is static and all joints in the chain between this link and the base are fixed,
					//then this link is static too (doesn't merge islands)
					if (cache.m_bulletMultiBody->getBaseMass() == 0)
					{
						bool allJointsFixed = true;
						i32 testLinkIndex = mbLinkIndex;
						do
						{
							if (cache.m_bulletMultiBody->getLink(testLinkIndex).m_jointType != MultibodyLink::eFixed)
							{
								allJointsFixed = false;
								break;
							}
							testLinkIndex = cache.m_bulletMultiBody->getLink(testLinkIndex).m_parent;
						} while (testLinkIndex> 0);
						if (allJointsFixed)
						{
							col->setCollisionFlags(col->getCollisionFlags() | CollisionObject2::CF_STATIC_OBJECT);
						}

					}
					cache.m_bulletMultiBody->getLink(mbLinkIndex).m_collider = col;
					if (flags & CUF_USE_SELF_COLLISION_INCLUDE_PARENT)
					{
						cache.m_bulletMultiBody->getLink(mbLinkIndex).m_flags &= ~DRX3D_MULTIBODYLINKFLAGS_DISABLE_PARENT_COLLISION;
					}
					if (flags & CUF_USE_SELF_COLLISION_EXCLUDE_ALL_PARENTS)
					{
						cache.m_bulletMultiBody->getLink(mbLinkIndex).m_flags |= DRX3D_MULTIBODYLINKFLAGS_DISABLE_ALL_PARENT_COLLISION;
					}
				}
				else
				{
					//					if (canSleep)
					{
						if (cache.m_bulletMultiBody->getBaseMass() == 0)
						//&& cache.m_bulletMultiBody->getNumDofs()==0)
						{
							//col->setCollisionFlags(CollisionObject2::CF_KINEMATIC_OBJECT);
							col->setCollisionFlags(col->getCollisionFlags() | CollisionObject2::CF_STATIC_OBJECT);
						}
					}

					cache.m_bulletMultiBody->setBaseCollider(col);
				}
			}
		}
		else
		{
			i32 mbLinkIndex = cache.getMbIndexFromUrdfIndex(urdfLinkIndex);
			//u2b.convertLinkVisualShapes2(mbLinkIndex, urdfLinkIndex, pathPrefix, localInertialFrame, col, u2b.getBodyUniqueId());
			u2b.convertLinkVisualShapes2(-1, urdfLinkIndex, pathPrefix, localInertialFrame, linkRigidBody, u2b.getBodyUniqueId());
		}
	}

	AlignedObjectArray<i32> urdfChildIndices;
	u2b.getLinkChildIndices(urdfLinkIndex, urdfChildIndices);

	i32 numChildren = urdfChildIndices.size();

	if (recursive)
	{
		for (i32 i = 0; i < numChildren; i++)
		{
			i32 urdfChildLinkIndex = urdfChildIndices[i];

			ConvertURDF2BulletInternal(u2b, creation, cache, urdfChildLinkIndex, linkTransformInWorldSpace, world1, createMultiBody, pathPrefix, flags, cachedLinkGraphicsShapesIn, cachedLinkGraphicsShapesOut, recursive);
		}
	}
	return linkTransformInWorldSpace;
}

struct childParentIndex
{
	i32 m_index;
	i32 m_mbIndex;
	i32 m_parentIndex;
	i32 m_parentMBIndex;

};

void GetAllIndices(const URDFImporterInterface& u2b, URDF2BulletCachedData& cache, i32 urdfLinkIndex, i32 parentIndex, AlignedObjectArray<childParentIndex>& allIndices)
{
	childParentIndex cp;
	cp.m_index = urdfLinkIndex;
	i32 mbIndex = cache.getMbIndexFromUrdfIndex(urdfLinkIndex);
	cp.m_mbIndex = mbIndex;
	cp.m_parentIndex = parentIndex;
	i32 parentMbIndex = parentIndex>=0? cache.getMbIndexFromUrdfIndex(parentIndex) : -1;
	cp.m_parentMBIndex = parentMbIndex;

	allIndices.push_back(cp);
	AlignedObjectArray<i32> urdfChildIndices;
	u2b.getLinkChildIndices(urdfLinkIndex, urdfChildIndices);
	i32 numChildren = urdfChildIndices.size();
	for (i32 i = 0; i < numChildren; i++)
	{
		i32 urdfChildLinkIndex = urdfChildIndices[i];
		GetAllIndices(u2b, cache, urdfChildLinkIndex, urdfLinkIndex, allIndices);
	}
}


bool MyIntCompareFunc(childParentIndex a, childParentIndex b)
{
	return (a.m_index < b.m_index);
}





void ConvertURDF2Bullet(
	const URDFImporterInterface& u2b, MultiBodyCreationInterface& creation,
	const Transform2& rootTransformInWorldSpace,

#ifdef USE_DISCRETE_DYNAMICS_WORLD
	DiscreteDynamicsWorld* world1,
#else
	MultiBodyDynamicsWorld* world1,
#endif
	bool createMultiBody, tukk pathPrefix, i32 flags, UrdfVisualShapeCache* cachedLinkGraphicsShapes)
{
	URDF2BulletCachedData cache;
	InitURDF2BulletCache(u2b, cache, flags);
	i32 urdfLinkIndex = u2b.getRootLinkIndex();
	i32 rootIndex = u2b.getRootLinkIndex();
	D3_PROFILE("ConvertURDF2Bullet");

	UrdfVisualShapeCache cachedLinkGraphicsShapesOut;


	bool recursive = (flags & CUF_MAINTAIN_LINK_ORDER)==0;
	if (recursive)
	{
		ConvertURDF2BulletInternal(u2b, creation, cache, urdfLinkIndex, rootTransformInWorldSpace, world1, createMultiBody, pathPrefix, flags, cachedLinkGraphicsShapes, &cachedLinkGraphicsShapesOut, recursive);
	}
	else
	{

		AlignedObjectArray<Transform2> parentTransforms;
		if (urdfLinkIndex >= parentTransforms.size())
		{
			parentTransforms.resize(urdfLinkIndex + 1);
		}
		parentTransforms[urdfLinkIndex] = rootTransformInWorldSpace;
		AlignedObjectArray<childParentIndex> allIndices;

		GetAllIndices(u2b, cache, urdfLinkIndex, -1, allIndices);
		allIndices.quickSort(MyIntCompareFunc);

		for (i32 i = 0; i < allIndices.size(); i++)
		{
			i32 urdfLinkIndex = allIndices[i].m_index;
			i32 parentIndex = allIndices[i].m_parentIndex;
			Transform2 parentTr = parentIndex >= 0 ? parentTransforms[parentIndex] : rootTransformInWorldSpace;
			Transform2 tr = ConvertURDF2BulletInternal(u2b, creation, cache, urdfLinkIndex, parentTr , world1, createMultiBody, pathPrefix, flags, cachedLinkGraphicsShapes, &cachedLinkGraphicsShapesOut, recursive);
			if ((urdfLinkIndex+1) >= parentTransforms.size())
			{
				parentTransforms.resize(urdfLinkIndex + 1);
			}
			parentTransforms[urdfLinkIndex] = tr;
		}



	}
	if (cachedLinkGraphicsShapes && cachedLinkGraphicsShapesOut.m_cachedUrdfLinkVisualShapeIndices.size() > cachedLinkGraphicsShapes->m_cachedUrdfLinkVisualShapeIndices.size())
	{
		*cachedLinkGraphicsShapes = cachedLinkGraphicsShapesOut;
	}
#ifndef USE_DISCRETE_DYNAMICS_WORLD
	if (world1 && cache.m_bulletMultiBody)
	{
		D3_PROFILE("Post process");
		MultiBody* mb = cache.m_bulletMultiBody;

		mb->setHasSelfCollision((flags & CUF_USE_SELF_COLLISION) != 0);

		mb->finalizeMultiDof();

		Transform2 localInertialFrameRoot = cache.m_urdfLinkLocalInertialFrames[urdfLinkIndex];

		if (flags & CUF_USE_MJCF)
		{
		}
		else
		{
			mb->setBaseWorldTransform(rootTransformInWorldSpace * localInertialFrameRoot);
		}
		AlignedObjectArray<Quat> scratch_q;
		AlignedObjectArray<Vec3> scratch_m;
		mb->forwardKinematics(scratch_q, scratch_m);
		mb->updateCollisionObjectWorldTransforms(scratch_q, scratch_m);

		world1->addMultiBody(mb);
	}
	#endif
}
