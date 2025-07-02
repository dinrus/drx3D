#include "../MultiBodyConstraintFeedback.h"
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointFeedback.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>

static Scalar radius(0.2);

struct MultiBodyConstraintFeedbackSetup : public CommonMultiBodyBase
{
	MultiBody* m_multiBody;
	AlignedObjectArray<MultiBodyJointFeedback*> m_jointFeedbacks;
	MultiBodyJointMotor* m_motor;
	bool m_once;

public:
	MultiBodyConstraintFeedbackSetup(struct GUIHelperInterface* helper);
	virtual ~MultiBodyConstraintFeedbackSetup();

	virtual void initPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void resetCamera()
	{
		float dist = 5;
		float pitch = -21;
		float yaw = 270;
		float targetPos[3] = {-1.34, 3.4, -0.44};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

MultiBodyConstraintFeedbackSetup::MultiBodyConstraintFeedbackSetup(struct GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper),
	  m_motor(0),
	  m_once(true)
{
}

MultiBodyConstraintFeedbackSetup::~MultiBodyConstraintFeedbackSetup()
{
}

void MultiBodyConstraintFeedbackSetup::initPhysics()
{
	i32 upAxis = 2;
	m_guiHelper->setUpAxis(upAxis);

	Vec4 colors[4] =
		{
			Vec4(1, 0, 0, 1),
			Vec4(0, 1, 0, 1),
			Vec4(0, 1, 1, 1),
			Vec4(1, 1, 0, 1),
		};
	i32 curColor = 0;

	this->createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(
		//IDebugDraw::DBG_DrawConstraints
		+IDebugDraw::DBG_DrawWireframe + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawAabb);  //+btIDebugDraw::DBG_DrawConstraintLimits);


	m_dynamicsWorld->getSolverInfo().m_jointFeedbackInWorldSpace = true;
	m_dynamicsWorld->getSolverInfo().m_jointFeedbackInJointFrame = true;

	//create a static ground object
	if (1)
	{
		Vec3 groundHalfExtents(10, 10, 0.2);
		BoxShape* box = new BoxShape(groundHalfExtents);
		box->initializePolyhedralFeatures();

		m_guiHelper->createCollisionShapeGraphicsObject(box);
		Transform2 start;
		start.setIdentity();
		Vec3 groundOrigin(-0.4f, 3.f, 0.f);
		//Vec3 basePosition = Vec3(-0.4f, 3.f, 0.f);
		groundOrigin[upAxis] -= .5;
		groundOrigin[2] -= 0.6;
		start.setOrigin(groundOrigin);
		Quat groundOrn(Vec3(0, 1, 0), 0.25 * SIMD_PI);

		//	start.setRotation(groundOrn);
		RigidBody* body = createRigidBody(0, start, box);
		body->setFriction(0);
		Vec4 color = colors[curColor];
		curColor++;
		curColor &= 3;
		m_guiHelper->createRigidBodyGraphicsObject(body, color);
	}

	{
		bool floating = false;
		bool damping = false;
		bool gyro = false;
		i32 numLinks = 2;
		bool spherical = false;  //set it ot false -to use 1DoF hinges instead of 3DoF sphericals
		bool canSleep = false;
		bool selfCollide = false;
		Vec3 linkHalfExtents(0.05, 0.5, 0.1);
		Vec3 baseHalfExtents(0.05, 0.5, 0.1);

		Vec3 basePosition = Vec3(-0.4f, 3.f, 0.f);
		//mbC->forceMultiDof();							//if !spherical, you can comment this line to check the 1DoF algorithm
		//init the base
		Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
		float baseMass = 0.01f;

		if (baseMass)
		{
			//CollisionShape *shape = new SphereShape(baseHalfExtents[0]);// BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
			CollisionShape* shape = new BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
			shape->calculateLocalInertia(baseMass, baseInertiaDiag);
			delete shape;
		}

		MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);

		m_multiBody = pMultiBody;
		Quat baseOriQuat(0.f, 0.f, 0.f, 1.f);
		//	baseOriQuat.setEulerZYX(-.25*SIMD_PI,0,-1.75*SIMD_PI);
		pMultiBody->setBasePos(basePosition);
		pMultiBody->setWorldToBaseRot(baseOriQuat);
		Vec3 vel(0, 0, 0);
		//	pMultiBody->setBaseVel(vel);

		//init the links
		Vec3 hingeJointAxis(1, 0, 0);

		//y-axis assumed up
		Vec3 parentComToCurrentCom(0, -linkHalfExtents[1] * 2.f, 0);                      //par body's COM to cur body's COM offset
		Vec3 currentPivotToCurrentCom(0, -linkHalfExtents[1], 0);                         //cur body's COM to cur body's PIV offset
		Vec3 parentComToCurrentPivot = parentComToCurrentCom - currentPivotToCurrentCom;  //par body's COM to cur body's PIV offset

		//////
		Scalar q0 = 0.f * SIMD_PI / 180.f;
		Quat quat0(Vec3(0, 1, 0).normalized(), q0);
		quat0.normalize();
		/////

		for (i32 i = 0; i < numLinks; ++i)
		{
			float linkMass = i == 0 ? 0.0001 : 1.f;
			//if (i==3 || i==2)
			//	linkMass= 1000;
			Vec3 linkInertiaDiag(0.f, 0.f, 0.f);

			CollisionShape* shape = 0;
			if (i == 0)
			{
				shape = new BoxShape(Vec3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));  //
			}
			else
			{
				shape = new SphereShape(radius);
			}
			shape->calculateLocalInertia(linkMass, linkInertiaDiag);
			delete shape;

			if (!spherical)
			{
				//pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), hingeJointAxis, parentComToCurrentPivot, currentPivotToCurrentCom, false);

				if (i == 0)
				{
					pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, i - 1,
											  Quat(0.f, 0.f, 0.f, 1.f),
											  hingeJointAxis,
											  parentComToCurrentPivot,
											  currentPivotToCurrentCom, false);
				}
				else
				{
					Vec3 parentComToCurrentCom(0, -linkHalfExtents[1], 0);  //par body's COM to cur body's COM offset
					Vec3 currentPivotToCurrentCom(0, 0, 0);                 //cur body's COM to cur body's PIV offset
					//Vec3 parentComToCurrentPivot = parentComToCurrentCom - currentPivotToCurrentCom;	//par body's COM to cur body's PIV offset

					pMultiBody->setupFixed(i, linkMass, linkInertiaDiag, i - 1,
										   Quat(0.f, 0.f, 0.f, 1.f),
										   parentComToCurrentPivot,
										   currentPivotToCurrentCom);
				}

				//pMultiBody->setupFixed(i,linkMass,linkInertiaDiag,i-1,Quat(0,0,0,1),parentComToCurrentPivot,currentPivotToCurrentCom,false);
			}
			else
			{
				//pMultiBody->setupPlanar(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f)/*quat0*/, Vec3(1, 0, 0), parentComToCurrentPivot*2, false);
				pMultiBody->setupSpherical(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), parentComToCurrentPivot, currentPivotToCurrentCom, false);
			}
		}

		pMultiBody->finalizeMultiDof();

		//for (i32 i=pMultiBody->getNumLinks()-1;i>=0;i--)//
		for (i32 i = 0; i < pMultiBody->getNumLinks(); i++)
		{
			MultiBodyJointFeedback* fb = new MultiBodyJointFeedback();
			pMultiBody->getLink(i).m_jointFeedback = fb;
			m_jointFeedbacks.push_back(fb);
			//break;
		}
		MultiBodyDynamicsWorld* world = m_dynamicsWorld;

		///
		world->addMultiBody(pMultiBody);
		MultiBody* mbC = pMultiBody;
		mbC->setCanSleep(canSleep);
		mbC->setHasSelfCollision(selfCollide);
		mbC->setUseGyroTerm(gyro);
		//
		if (!damping)
		{
			mbC->setLinearDamping(0.f);
			mbC->setAngularDamping(0.f);
		}
		else
		{
			mbC->setLinearDamping(0.1f);
			mbC->setAngularDamping(0.9f);
		}
		//
		m_dynamicsWorld->setGravity(Vec3(0, 0, -10));

		//////////////////////////////////////////////
		if (/* DISABLES CODE */ (0))  //numLinks > 0)
		{
			Scalar q0 = 45.f * SIMD_PI / 180.f;
			if (!spherical)
			{
				mbC->setJointPosMultiDof(0, &q0);
			}
			else
			{
				Quat quat0(Vec3(1, 1, 0).normalized(), q0);
				quat0.normalize();
				mbC->setJointPosMultiDof(0, quat0);
			}
		}
		///

		AlignedObjectArray<Quat> world_to_local;
		world_to_local.resize(pMultiBody->getNumLinks() + 1);

		AlignedObjectArray<Vec3> local_origin;
		local_origin.resize(pMultiBody->getNumLinks() + 1);
		world_to_local[0] = pMultiBody->getWorldToBaseRot();
		local_origin[0] = pMultiBody->getBasePos();
		//  double friction = 1;
		{
			//	float pos[4]={local_origin[0].x(),local_origin[0].y(),local_origin[0].z(),1};
			// float quat[4]={-world_to_local[0].x(),-world_to_local[0].y(),-world_to_local[0].z(),world_to_local[0].w()};

			if (1)
			{
				CollisionShape* shape = new BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));  //new SphereShape(baseHalfExtents[0]);
				m_guiHelper->createCollisionShapeGraphicsObject(shape);

				MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, -1);
				col->setCollisionShape(shape);

				Transform2 tr;
				tr.setIdentity();
				//if we don't set the initial pose of the CollisionObject2, the simulator will do this
				//when syncing the btMultiBody link transforms to the btMultiBodyLinkCollider

				tr.setOrigin(local_origin[0]);
				Quat orn(Vec3(0, 0, 1), 0.25 * 3.1415926538);

				tr.setRotation(orn);
				col->setWorldTransform(tr);

				bool isDynamic = (baseMass > 0 && floating);
				i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
				i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

				world->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);  //, 2,1+2);

				Vec3 color(0.0, 0.0, 0.5);
				m_guiHelper->createCollisionObjectGraphicsObject(col, color);

				//                col->setFriction(friction);
				pMultiBody->setBaseCollider(col);
			}
		}

		for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
		{
			i32k parent = pMultiBody->getParent(i);
			world_to_local[i + 1] = pMultiBody->getParentToLocalRot(i) * world_to_local[parent + 1];
			local_origin[i + 1] = local_origin[parent + 1] + (quatRotate(world_to_local[i + 1].inverse(), pMultiBody->getRVector(i)));
		}

		for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
		{
			Vec3 posr = local_origin[i + 1];
			//	float pos[4]={posr.x(),posr.y(),posr.z(),1};

			const Scalar quat[4] = {-world_to_local[i + 1].x(), -world_to_local[i + 1].y(), -world_to_local[i + 1].z(), world_to_local[i + 1].w()};
			CollisionShape* shape = 0;

			if (i == 0)
			{
				shape = new BoxShape(Vec3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));  //SphereShape(linkHalfExtents[0]);
			}
			else
			{
				shape = new SphereShape(radius);
			}

			m_guiHelper->createCollisionShapeGraphicsObject(shape);
			MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, i);

			col->setCollisionShape(shape);
			Transform2 tr;
			tr.setIdentity();
			tr.setOrigin(posr);
			tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
			col->setWorldTransform(tr);
			//       col->setFriction(friction);
			bool isDynamic = 1;  //(linkMass > 0);
			i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
			i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

			//if (i==0||i>numLinks-2)
			{
				world->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);  //,2,1+2);
				Vec4 color = colors[curColor];
				curColor++;
				curColor &= 3;
				m_guiHelper->createCollisionObjectGraphicsObject(col, color);

				pMultiBody->getLink(i).m_collider = col;
			}
		}
		i32 link = 0;
		i32 targetVelocity = 0.f;
		Scalar maxForce = 100000;
		m_motor = new MultiBodyJointMotor(pMultiBody, link, targetVelocity, maxForce);
		m_dynamicsWorld->addMultiBodyConstraint(m_motor);
	}
}

void MultiBodyConstraintFeedbackSetup::stepSimulation(float deltaTime)
{
	//m_multiBody->addLinkForce(0,Vec3(100,100,100));
	if (/* DISABLES CODE */ (0))  //m_once)
	{
		m_once = false;
		m_multiBody->addJointTorque(0, 10.0);

		Scalar torque = m_multiBody->getJointTorque(0);
		drx3DPrintf("t = %f,%f,%f\n", torque, torque, torque);  //[0],torque[1],torque[2]);
	}
	Scalar timeStep = 1. / 240.f;

	m_dynamicsWorld->stepSimulation(timeStep, 0);

	static i32 count = 0;
	if ((count & 0x0f) == 0)
	{
		if (m_motor)
		{
			float force = m_motor->getAppliedImpulse(0) / timeStep;
			drx3DPrintf("motor applied force = %f\n", force);
		}

		for (i32 i = 0; i < m_jointFeedbacks.size(); i++)
		{
			drx3DPrintf("F_reaction[%i] linear:%f,%f,%f, angular:%f,%f,%f",
					 i,
					 m_jointFeedbacks[i]->m_reactionForces.m_topVec[0],
					 m_jointFeedbacks[i]->m_reactionForces.m_topVec[1],
					 m_jointFeedbacks[i]->m_reactionForces.m_topVec[2],

					 m_jointFeedbacks[i]->m_reactionForces.m_bottomVec[0],
					 m_jointFeedbacks[i]->m_reactionForces.m_bottomVec[1],
					 m_jointFeedbacks[i]->m_reactionForces.m_bottomVec[2]

			);
		}
	}
	count++;

	/*
    drx3DPrintf("base angvel = %f,%f,%f",m_multiBody->getBaseOmega()[0],
             m_multiBody->getBaseOmega()[1],
             m_multiBody->getBaseOmega()[2]
             );
    */
	// Scalar jointVel =m_multiBody->getJointVel(0);

	//    drx3DPrintf("child angvel = %f",jointVel);
}

class CommonExampleInterface* MultiBodyConstraintFeedbackCreateFunc(struct CommonExampleOptions& options)
{
	return new MultiBodyConstraintFeedbackSetup(options.m_guiHelper);
}
