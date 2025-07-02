
#ifndef COMMON_RIGID_BODY_BASE_H
#define COMMON_RIGID_BODY_BASE_H

#include <drx3D/DynamicsCommon.h>
#include "CommonExampleInterface.h"
#include "CommonGUIHelperInterface.h"
#include "CommonRenderInterface.h"
#include "CommonCameraInterface.h"
#include "CommonGraphicsAppInterface.h"
#include "CommonWindowInterface.h"
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>

struct CommonRigidBodyBase : public CommonExampleInterface
{
	//keep the collision shapes, for deletion/cleanup
	AlignedObjectArray<CollisionShape*> m_collisionShapes;
	BroadphaseInterface* m_broadphase;
	CollisionDispatcher* m_dispatcher;
	ConstraintSolver* m_solver;
	DefaultCollisionConfiguration* m_collisionConfiguration;
	DiscreteDynamicsWorld* m_dynamicsWorld;

	//data for picking objects
	class RigidBody* m_pickedBody;
	class TypedConstraint* m_pickedConstraint;
	i32 m_savedState;
	Vec3 m_oldPickingPos;
	Vec3 m_hitPos;
	Scalar m_oldPickingDist;
	struct GUIHelperInterface* m_guiHelper;

	CommonRigidBodyBase(struct GUIHelperInterface* helper)
		: m_broadphase(0),
		  m_dispatcher(0),
		  m_solver(0),
		  m_collisionConfiguration(0),
		  m_dynamicsWorld(0),
		  m_pickedBody(0),
		  m_pickedConstraint(0),
		  m_guiHelper(helper)
	{
	}
	virtual ~CommonRigidBodyBase()
	{
	}

	DiscreteDynamicsWorld* getDynamicsWorld()
	{
		return m_dynamicsWorld;
	}

	virtual void createEmptyDynamicsWorld()
	{
		///collision configuration contains default setup for memory, collision setup
		m_collisionConfiguration = new DefaultCollisionConfiguration();
		//m_collisionConfiguration->setConvexConvexMultipointIterations();

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

		m_broadphase = new DbvtBroadphase();

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;
		m_solver = sol;

		m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

		m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	}

	virtual void stepSimulation(float deltaTime)
	{
		if (m_dynamicsWorld)
		{
			m_dynamicsWorld->stepSimulation(deltaTime);
		}
	}

	virtual void physicsDebugDraw(i32 debugFlags)
	{
		if (m_dynamicsWorld && m_dynamicsWorld->getDebugDrawer())
		{
			m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugFlags);
			m_dynamicsWorld->debugDrawWorld();
		}
	}

	virtual void exitPhysics()
	{
		removePickingConstraint();
		//cleanup in the reverse order of creation/initialization

		//remove the rigidbodies from the dynamics world and delete them

		if (m_dynamicsWorld)
		{
			i32 i;
			for (i = m_dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
			{
				m_dynamicsWorld->removeConstraint(m_dynamicsWorld->getConstraint(i));
			}
			for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
			{
				CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
				RigidBody* body = RigidBody::upcast(obj);
				if (body && body->getMotionState())
				{
					delete body->getMotionState();
				}
				m_dynamicsWorld->removeCollisionObject(obj);
				delete obj;
			}
		}
		//delete collision shapes
		for (i32 j = 0; j < m_collisionShapes.size(); j++)
		{
			CollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}
		m_collisionShapes.clear();

		delete m_dynamicsWorld;
		m_dynamicsWorld = 0;

		delete m_solver;
		m_solver = 0;

		delete m_broadphase;
		m_broadphase = 0;

		delete m_dispatcher;
		m_dispatcher = 0;

		delete m_collisionConfiguration;
		m_collisionConfiguration = 0;
	}

	virtual void debugDraw(i32 debugDrawFlags)
	{
		if (m_dynamicsWorld)
		{
			if (m_dynamicsWorld->getDebugDrawer())
			{
				m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugDrawFlags);
			}
			m_dynamicsWorld->debugDrawWorld();
		}
	}

	virtual bool keyboardCallback(i32 key, i32 state)
	{
		if ((key == B3G_F3) && state && m_dynamicsWorld)
		{
			DefaultSerializer* serializer = new DefaultSerializer();
			m_dynamicsWorld->serialize(serializer);

			FILE* file = fopen("testFile.bullet", "wb");
			fwrite(serializer->getBufferPointer(), serializer->getCurrentBufferSize(), 1, file);
			fclose(file);
			//drx3DPrintf("DefaultSerializer wrote testFile.bullet");
			delete serializer;
			return true;
		}
		return false;  //don't handle this key
	}

	Vec3 getRayTo(i32 x, i32 y)
	{
		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();

		if (!renderer)
		{
			Assert(0);
			return Vec3(0, 0, 0);
		}

		float top = 1.f;
		float bottom = -1.f;
		float nearPlane = 1.f;
		float tanFov = (top - bottom) * 0.5f / nearPlane;
		float fov = Scalar(2.0) * Atan(tanFov);

		Vec3 camPos, camTarget;

		renderer->getActiveCamera()->getCameraPosition(camPos);
		renderer->getActiveCamera()->getCameraTargetPosition(camTarget);

		Vec3 rayFrom = camPos;
		Vec3 rayForward = (camTarget - camPos);
		rayForward.normalize();
		float farPlane = 10000.f;
		rayForward *= farPlane;

		Vec3 rightOffset;
		Vec3 cameraUp = Vec3(0, 0, 0);
		cameraUp[m_guiHelper->getAppInterface()->getUpAxis()] = 1;

		Vec3 vertical = cameraUp;

		Vec3 hor;
		hor = rayForward.cross(vertical);
		hor.safeNormalize();
		vertical = hor.cross(rayForward);
		vertical.safeNormalize();

		float tanfov = tanf(0.5f * fov);

		hor *= 2.f * farPlane * tanfov;
		vertical *= 2.f * farPlane * tanfov;

		Scalar aspect;
		float width = float(renderer->getScreenWidth());
		float height = float(renderer->getScreenHeight());

		aspect = width / height;

		hor *= aspect;

		Vec3 rayToCenter = rayFrom + rayForward;
		Vec3 dHor = hor * 1.f / width;
		Vec3 dVert = vertical * 1.f / height;

		Vec3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
		rayTo += Scalar(x) * dHor;
		rayTo -= Scalar(y) * dVert;
		return rayTo;
	}

	virtual bool mouseMoveCallback(float x, float y)
	{
		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();

		if (!renderer)
		{
			Assert(0);
			return false;
		}

		Vec3 rayTo = getRayTo(i32(x), i32(y));
		Vec3 rayFrom;
		renderer->getActiveCamera()->getCameraPosition(rayFrom);
		movePickedBody(rayFrom, rayTo);

		return false;
	}

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();

		if (!renderer)
		{
			Assert(0);
			return false;
		}

		CommonWindowInterface* window = m_guiHelper->getAppInterface()->m_window;

#if 0
		if (window->isModifierKeyPressed(B3G_ALT))
		{
			printf("ALT pressed\n");
		} else
		{
			printf("NO ALT pressed\n");
		}
		
		if (window->isModifierKeyPressed(B3G_SHIFT))
		{
			printf("SHIFT pressed\n");
		} else
		{
			printf("NO SHIFT pressed\n");
		}
		
		if (window->isModifierKeyPressed(B3G_CONTROL))
		{
			printf("CONTROL pressed\n");
		} else
		{
			printf("NO CONTROL pressed\n");
		}
#endif

		if (state == 1)
		{
			if (button == 0 && (!window->isModifierKeyPressed(B3G_ALT) && !window->isModifierKeyPressed(B3G_CONTROL)))
			{
				Vec3 camPos;
				renderer->getActiveCamera()->getCameraPosition(camPos);

				Vec3 rayFrom = camPos;
				Vec3 rayTo = getRayTo(i32(x), i32(y));

				pickBody(rayFrom, rayTo);
			}
		}
		else
		{
			if (button == 0)
			{
				removePickingConstraint();
				//remove p2p
			}
		}

		//printf("button=%d, state=%d\n",button,state);
		return false;
	}

	virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
	{
		if (m_dynamicsWorld == 0)
			return false;

		CollisionWorld::ClosestRayResultCallback rayCallback(rayFromWorld, rayToWorld);

		rayCallback.m_flags |= TriangleRaycastCallback::kF_UseGjkConvexCastRaytest;
		m_dynamicsWorld->rayTest(rayFromWorld, rayToWorld, rayCallback);
		if (rayCallback.hasHit())
		{
			Vec3 pickPos = rayCallback.m_hitPointWorld;
			RigidBody* body = (RigidBody*)RigidBody::upcast(rayCallback.m_collisionObject);
			if (body)
			{
				//other exclusions?
				if (!(body->isStaticObject() || body->isKinematicObject()))
				{
					m_pickedBody = body;
					m_savedState = m_pickedBody->getActivationState();
					m_pickedBody->setActivationState(DISABLE_DEACTIVATION);
					//printf("pickPos=%f,%f,%f\n",pickPos.getX(),pickPos.getY(),pickPos.getZ());
					Vec3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
					Point2PointConstraint* p2p = new Point2PointConstraint(*body, localPivot);
					m_dynamicsWorld->addConstraint(p2p, true);
					m_pickedConstraint = p2p;
					Scalar mousePickClamping = 30.f;
					p2p->m_setting.m_impulseClamp = mousePickClamping;
					//very weak constraint for picking
					p2p->m_setting.m_tau = 0.001f;
				}
			}

			//					pickObject(pickPos, rayCallback.m_collisionObject);
			m_oldPickingPos = rayToWorld;
			m_hitPos = pickPos;
			m_oldPickingDist = (pickPos - rayFromWorld).length();
			//					printf("hit !\n");
			//add p2p
		}
		return false;
	}
	virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
	{
		if (m_pickedBody && m_pickedConstraint)
		{
			Point2PointConstraint* pickCon = static_cast<Point2PointConstraint*>(m_pickedConstraint);
			if (pickCon)
			{
				//keep it at the same picking distance

				Vec3 newPivotB;

				Vec3 dir = rayToWorld - rayFromWorld;
				dir.normalize();
				dir *= m_oldPickingDist;

				newPivotB = rayFromWorld + dir;
				pickCon->setPivotB(newPivotB);
				return true;
			}
		}
		return false;
	}
	virtual void removePickingConstraint()
	{
		if (m_pickedConstraint)
		{
			m_pickedBody->forceActivationState(m_savedState);
			m_pickedBody->activate();
			m_dynamicsWorld->removeConstraint(m_pickedConstraint);
			delete m_pickedConstraint;
			m_pickedConstraint = 0;
			m_pickedBody = 0;
		}
	}

	BoxShape* createBoxShape(const Vec3& halfExtents)
	{
		BoxShape* box = new BoxShape(halfExtents);
		return box;
	}

	void deleteRigidBody(RigidBody* body)
	{
		i32 graphicsUid = body->getUserIndex();
		m_guiHelper->removeGraphicsInstance(graphicsUid);

		m_dynamicsWorld->removeRigidBody(body);
		MotionState* ms = body->getMotionState();
		delete body;
		delete ms;
	}

	RigidBody* createRigidBody(float mass, const Transform2& startTransform, CollisionShape* shape, const Vec4& color = Vec4(1, 0, 0, 1))
	{
		Assert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			shape->calculateLocalInertia(mass, localInertia);

			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
		DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

		RigidBody::RigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

		RigidBody* body = new RigidBody(cInfo);
		//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
		RigidBody* body = new RigidBody(mass, 0, shape, localInertia);
		body->setWorldTransform(startTransform);
#endif  //

		body->setUserIndex(-1);
		m_dynamicsWorld->addRigidBody(body);
		return body;
	}

	virtual void renderScene()
	{
		if (m_dynamicsWorld)
		{
			{
				m_guiHelper->syncPhysicsToGraphics(m_dynamicsWorld);
			}

			{
				m_guiHelper->render(m_dynamicsWorld);
			}
		}
	}
};

#endif  //COMMON_RIGID_BODY_SETUP_H
