// Collision Radius
#define COLLISION_RADIUS 0.0f

#include "../BenchmarkDemo.h"

///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <stdio.h>  //printf debugging
#include "../TaruData.h"
#include "../HaltonData.h"
#include "../landscapeData.h"
#include <drx3D/Physics/Collision/BroadPhase/DbvtBroadphase.h>

#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>

class DynamicsWorld;

#define NUMRAYS 500
#define USE_PARALLEL_RAYCASTS 1

class RigidBody;
class BroadphaseInterface;
class CollisionShape;
class OverlappingPairCache;
class CollisionDispatcher;
class ConstraintSolver;
struct CollisionAlgorithmCreateFunc;
class DefaultCollisionConfiguration;

#include "../../MultiThreadedDemo/CommonRigidBodyMTBase.h"

class BenchmarkDemo : public CommonRigidBodyMTBase
{
	//keep the collision shapes, for deletion/cleanup

	AlignedObjectArray<class RagDoll*> m_ragdolls;

	i32 m_benchmark;

	void myinit()
	{
		//??
	}

	void setCameraDistance(Scalar dist)
	{
	}
	void createTest1();
	void createTest2();
	void createTest3();
	void createTest4();
	void createTest5();
	void createTest6();
	void createTest7();
	void createTest8();

	void createWall(const Vec3& offsetPosition, i32 stackSize, const Vec3& boxSize);
	void createPyramid(const Vec3& offsetPosition, i32 stackSize, const Vec3& boxSize);
	void createTowerCircle(const Vec3& offsetPosition, i32 stackSize, i32 rotSize, const Vec3& boxSize);
	void createLargeMeshBody();

	class SpuBatchRaycaster* m_batchRaycaster;
	class ThreadSupportInterface* m_batchRaycasterThreadSupport;

	void castRays();
	void initRays();

public:
	BenchmarkDemo(struct GUIHelperInterface* helper, i32 benchmark)
		: CommonRigidBodyMTBase(helper),
		  m_benchmark(benchmark)
	{
	}
	virtual ~BenchmarkDemo()
	{
		exitPhysics();
	}
	void initPhysics();

	void exitPhysics();

	void stepSimulation(float deltaTime);

	void resetCamera()
	{
		float dist = 120;
		float pitch = -35;
		float yaw = 52;
		float targetPos[3] = {0, 10.46, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

class RaycastBar2
{
public:
	Vec3 source[NUMRAYS];
	Vec3 dest[NUMRAYS];
	Vec3 direction[NUMRAYS];
	Vec3 hit[NUMRAYS];
	Vec3 normal[NUMRAYS];
	struct GUIHelperInterface* m_guiHelper;

	i32 frame_counter;
	i32 ms;
	i32 sum_ms;
	i32 sum_ms_samples;
	i32 min_ms;
	i32 max_ms;

#ifdef USE_DRX3D_CLOCK
	Clock frame_timer;
#endif  //USE_DRX3D_CLOCK

	Scalar dx;
	Scalar min_x;
	Scalar max_x;
	Scalar max_y;
	Scalar sign;

	RaycastBar2()
	{
		m_guiHelper = 0;
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
	}

	RaycastBar2(Scalar ray_length, Scalar z, Scalar max_y, struct GUIHelperInterface* guiHelper)
	{
		m_guiHelper = guiHelper;
		frame_counter = 0;
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
		dx = 10.0;
		min_x = 0;
		max_x = 0;
		this->max_y = max_y;
		sign = 1.0;
		Scalar dalpha = 2 * SIMD_2_PI / NUMRAYS;
		for (i32 i = 0; i < NUMRAYS; i++)
		{
			Scalar alpha = dalpha * i;
			// rotate around by alpha degrees y
			Quat q(Vec3(0.0, 1.0, 0.0), alpha);
			direction[i] = Vec3(1.0, 0.0, 0.0);
			direction[i] = quatRotate(q, direction[i]);
			direction[i] = direction[i] * ray_length;

			source[i] = Vec3(min_x, max_y, z);
			dest[i] = source[i] + direction[i];
			dest[i][1] = -1000;
			normal[i] = Vec3(1.0, 0.0, 0.0);
		}
	}

	void move(Scalar dt)
	{
		if (dt > Scalar(1.0 / 60.0))
			dt = Scalar(1.0 / 60.0);
		for (i32 i = 0; i < NUMRAYS; i++)
		{
			source[i][0] += dx * dt * sign;
			dest[i][0] += dx * dt * sign;
		}
		if (source[0][0] < min_x)
			sign = 1.0;
		else if (source[0][0] > max_x)
			sign = -1.0;
	}

	void castRays(CollisionWorld* cw, i32 iBegin, i32 iEnd)
	{
		for (i32 i = iBegin; i < iEnd; ++i)
		{
			CollisionWorld::ClosestRayResultCallback cb(source[i], dest[i]);

			{
				DRX3D_PROFILE("cw->rayTest");
				cw->rayTest(source[i], dest[i], cb);
			}
			if (cb.hasHit())
			{
				hit[i] = cb.m_hitPointWorld;
				normal[i] = cb.m_hitNormalWorld;
				normal[i].normalize();
			}
			else
			{
				hit[i] = dest[i];
				normal[i] = Vec3(1.0, 0.0, 0.0);
			}
		}
	}

	struct CastRaysLoopBody : public IParallelForBody
	{
		CollisionWorld* mWorld;
		RaycastBar2* mRaycasts;

		CastRaysLoopBody(CollisionWorld* cw, RaycastBar2* rb) : mWorld(cw), mRaycasts(rb) {}

		void forLoop(i32 iBegin, i32 iEnd) const
		{
			mRaycasts->castRays(mWorld, iBegin, iEnd);
		}
	};

	void cast(CollisionWorld* cw, bool multiThreading = false)
	{
		DRX3D_PROFILE("cast");

#ifdef USE_DRX3D_CLOCK
		frame_timer.reset();
#endif  //USE_DRX3D_CLOCK

#ifdef BATCH_RAYCASTER
		if (!gBatchRaycaster)
			return;

		gBatchRaycaster->clearRays();
		for (i32 i = 0; i < NUMRAYS; i++)
		{
			gBatchRaycaster->addRay(source[i], dest[i]);
		}
		gBatchRaycaster->performBatchRaycast();
		for (i32 i = 0; i < gBatchRaycaster->getNumRays(); i++)
		{
			const SpuRaycastTaskWorkUnitOut& out = (*gBatchRaycaster)[i];
			hit[i].setInterpolate3(source[i], dest[i], out.hitFraction);
			normal[i] = out.hitNormal;
			normal[i].normalize();
		}
#else
#if USE_PARALLEL_RAYCASTS
		if (multiThreading)
		{
			CastRaysLoopBody rayLooper(cw, this);
			i32 grainSize = 20;  // number of raycasts per task
			ParallelFor(0, NUMRAYS, grainSize, rayLooper);
		}
		else
#endif  // USE_PARALLEL_RAYCASTS
		{
			// single threaded
			castRays(cw, 0, NUMRAYS);
		}
#ifdef USE_DRX3D_CLOCK
		ms += frame_timer.getTimeMilliseconds();
#endif  //USE_DRX3D_CLOCK
		frame_counter++;
		if (frame_counter > 50)
		{
			min_ms = ms < min_ms ? ms : min_ms;
			max_ms = ms > max_ms ? ms : max_ms;
			sum_ms += ms;
			sum_ms_samples++;
			Scalar mean_ms = (Scalar)sum_ms / (Scalar)sum_ms_samples;
			printf("%d лучей за %d мс %d %d %f\n", NUMRAYS * frame_counter, ms, min_ms, max_ms, mean_ms);
			ms = 0;
			frame_counter = 0;
		}
#endif
	}

	void draw()
	{
		if (m_guiHelper)
		{
			AlignedObjectArray<u32> indices;
			AlignedObjectArray<Vec3FloatData> points;

			float lineColor[4] = {1, 0.4, .4, 1};

			for (i32 i = 0; i < NUMRAYS; i++)
			{
				Vec3FloatData s, h;
				for (i32 w = 0; w < 4; w++)
				{
					s.m_floats[w] = source[i][w];
					h.m_floats[w] = hit[i][w];
				}

				points.push_back(s);
				points.push_back(h);
				indices.push_back(indices.size());
				indices.push_back(indices.size());
			}

			m_guiHelper->getRenderInterface()->drawLines(&points[0].m_floats[0], lineColor, points.size(), sizeof(Vec3FloatData), &indices[0], indices.size(), 1);
		}

	}
};

static RaycastBar2 raycastBar;

void BenchmarkDemo::stepSimulation(float deltaTime)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(deltaTime);
	}

	if (m_benchmark == 7)
	{
		castRays();

		raycastBar.draw();
	}
}

void BenchmarkDemo::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	setCameraDistance(Scalar(100.));

	createEmptyDynamicsWorld();
	/////collision configuration contains default setup for memory, collision setup
	//DefaultCollisionConstructionInfo cci;
	//cci.m_defaultMaxPersistentManifoldPoolSize = 32768;
	//m_collisionConfiguration = new btDefaultCollisionConfiguration(cci);

	/////use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	//m_dispatcher = new	CollisionDispatcher(m_collisionConfiguration);
	//
	//m_dispatcher->setDispatcherFlags(CollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);

	/////the maximum size of the collision world. Make sure objects stay within these boundaries
	/////Don't make the world AABB size too large, it will harm simulation quality and performance
	//Vec3 worldAabbMin(-1000,-1000,-1000);
	//Vec3 worldAabbMax(1000,1000,1000);
	//
	//HashedOverlappingPairCache* pairCache = new btHashedOverlappingPairCache();
	//m_broadphase = new AxisSweep3(worldAabbMin,worldAabbMax,3500,pairCache);
	//	m_broadphase = new SimpleBroadphase();
	//	m_broadphase = new DbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	//SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;

	//m_solver = sol;

	//DiscreteDynamicsWorld* dynamicsWorld;
	//m_dynamicsWorld = dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

	///the following 3 lines increase the performance dramatically, with a little bit of loss of quality
	m_dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;  //don't recalculate friction values each frame
	m_dynamicsWorld->getSolverInfo().m_numIterations = 5;                                       //few solver iterations
	//m_defaultContactProcessingThreshold = 0.f;//used when creating bodies: body->setContactProcessingThreshold(...);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

	if (m_benchmark < 5)
	{
		///create a few basic rigid bodies
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(250.), Scalar(50.), Scalar(250.)));
		//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),0);

		m_collisionShapes.push_back(groundShape);

		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, -50, 0));

		//We can also use DemoApplication::createRigidBody, but for clarity it is provided here:
		{
			Scalar mass(0.);

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			Vec3 localInertia(0, 0, 0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass, localInertia);

			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
			RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
			RigidBody* body = new RigidBody(rbInfo);

			//add the body to the dynamics world
			m_dynamicsWorld->addRigidBody(body);
		}
	}

	switch (m_benchmark)
	{
		case 1:
		{
			createTest1();
			break;
		}
		case 2:
		{
			createTest2();
			break;
		}
		case 3:
		{
			createTest3();
			break;
		}
		case 4:
		{
			createTest4();
			break;
		}
		case 5:
		{
			createTest5();
			break;
		}
		case 6:
		{
			createTest6();
			break;
		}
		case 7:
		{
			createTest7();
			break;
		}
		case 8:
		{
			createTest8();
			break;
		}

		default:
		{
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void BenchmarkDemo::createTest1()
{
	// 3000
	i32 size = 8;
	const float cubeSize = 1.0f;
	float spacing = cubeSize;
	Vec3 pos(0.0f, cubeSize * 2, 0.f);
	float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

	BoxShape* blockShape = new BoxShape(Vec3(cubeSize - COLLISION_RADIUS, cubeSize - COLLISION_RADIUS, cubeSize - COLLISION_RADIUS));
	Vec3 localInertia(0, 0, 0);
	float mass = 2.f;
	blockShape->calculateLocalInertia(mass, localInertia);

	Transform2 trans;
	trans.setIdentity();

	for (i32 k = 0; k < 47; k++)
	{
		for (i32 j = 0; j < size; j++)
		{
			pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
			for (i32 i = 0; i < size; i++)
			{
				pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);

				trans.setOrigin(pos);
				RigidBody* cmbody;
				cmbody = createRigidBody(mass, trans, blockShape);
			}
		}
		offset -= 0.05f * spacing * (size - 1);
		//		spacing *= 1.01f;
		pos[1] += (cubeSize * 2.0f + spacing);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Pyramid 3

void BenchmarkDemo::createWall(const Vec3& offsetPosition, i32 stackSize, const Vec3& boxSize)
{
	BoxShape* blockShape = new BoxShape(Vec3(boxSize[0] - COLLISION_RADIUS, boxSize[1] - COLLISION_RADIUS, boxSize[2] - COLLISION_RADIUS));

	float mass = 1.f;
	Vec3 localInertia(0, 0, 0);
	blockShape->calculateLocalInertia(mass, localInertia);

	//	Scalar  diffX = boxSize[0] * 1.0f;
	Scalar diffY = boxSize[1] * 1.0f;
	Scalar diffZ = boxSize[2] * 1.0f;

	Scalar offset = -stackSize * (diffZ * 2.0f) * 0.5f;
	Vec3 pos(0.0f, diffY, 0.0f);

	Transform2 trans;
	trans.setIdentity();

	while (stackSize)
	{
		for (i32 i = 0; i < stackSize; i++)
		{
			pos[2] = offset + (float)i * (diffZ * 2.0f);

			trans.setOrigin(offsetPosition + pos);
			createRigidBody(mass, trans, blockShape);
		}
		offset += diffZ;
		pos[1] += (diffY * 2.0f);
		stackSize--;
	}
}

void BenchmarkDemo::createPyramid(const Vec3& offsetPosition, i32 stackSize, const Vec3& boxSize)
{
	Scalar space = 0.0001f;

	Vec3 pos(0.0f, boxSize[1], 0.0f);

	BoxShape* blockShape = new BoxShape(Vec3(boxSize[0] - COLLISION_RADIUS, boxSize[1] - COLLISION_RADIUS, boxSize[2] - COLLISION_RADIUS));
	Transform2 trans;
	trans.setIdentity();

	Scalar mass = 1.f;
	Vec3 localInertia(0, 0, 0);
	blockShape->calculateLocalInertia(mass, localInertia);

	Scalar diffX = boxSize[0] * 1.02f;
	Scalar diffY = boxSize[1] * 1.02f;
	Scalar diffZ = boxSize[2] * 1.02f;

	Scalar offsetX = -stackSize * (diffX * 2.0f + space) * 0.5f;
	Scalar offsetZ = -stackSize * (diffZ * 2.0f + space) * 0.5f;
	while (stackSize)
	{
		for (i32 j = 0; j < stackSize; j++)
		{
			pos[2] = offsetZ + (float)j * (diffZ * 2.0f + space);
			for (i32 i = 0; i < stackSize; i++)
			{
				pos[0] = offsetX + (float)i * (diffX * 2.0f + space);
				trans.setOrigin(offsetPosition + pos);
				this->createRigidBody(mass, trans, blockShape);
			}
		}
		offsetX += diffX;
		offsetZ += diffZ;
		pos[1] += (diffY * 2.0f + space);
		stackSize--;
	}
}

const Vec3 rotate(const Quat& quat, const Vec3& vec)
{
	float tmpX, tmpY, tmpZ, tmpW;
	tmpX = (((quat.getW() * vec.getX()) + (quat.getY() * vec.getZ())) - (quat.getZ() * vec.getY()));
	tmpY = (((quat.getW() * vec.getY()) + (quat.getZ() * vec.getX())) - (quat.getX() * vec.getZ()));
	tmpZ = (((quat.getW() * vec.getZ()) + (quat.getX() * vec.getY())) - (quat.getY() * vec.getX()));
	tmpW = (((quat.getX() * vec.getX()) + (quat.getY() * vec.getY())) + (quat.getZ() * vec.getZ()));
	return Vec3(
		((((tmpW * quat.getX()) + (tmpX * quat.getW())) - (tmpY * quat.getZ())) + (tmpZ * quat.getY())),
		((((tmpW * quat.getY()) + (tmpY * quat.getW())) - (tmpZ * quat.getX())) + (tmpX * quat.getZ())),
		((((tmpW * quat.getZ()) + (tmpZ * quat.getW())) - (tmpX * quat.getY())) + (tmpY * quat.getX())));
}

void BenchmarkDemo::createTowerCircle(const Vec3& offsetPosition, i32 stackSize, i32 rotSize, const Vec3& boxSize)
{
	BoxShape* blockShape = new BoxShape(Vec3(boxSize[0] - COLLISION_RADIUS, boxSize[1] - COLLISION_RADIUS, boxSize[2] - COLLISION_RADIUS));

	Transform2 trans;
	trans.setIdentity();

	float mass = 1.f;
	Vec3 localInertia(0, 0, 0);
	blockShape->calculateLocalInertia(mass, localInertia);

	float radius = 1.3f * rotSize * boxSize[0] / SIMD_PI;

	// create active boxes
	Quat rotY(0, 1, 0, 0);
	float posY = boxSize[1];

	for (i32 i = 0; i < stackSize; i++)
	{
		for (i32 j = 0; j < rotSize; j++)
		{
			trans.setOrigin(offsetPosition + rotate(rotY, Vec3(0.0f, posY, radius)));
			trans.setRotation(rotY);
			createRigidBody(mass, trans, blockShape);

			rotY *= Quat(Vec3(0, 1, 0), SIMD_PI / (rotSize * Scalar(0.5)));
		}

		posY += boxSize[1] * 2.0f;
		rotY *= Quat(Vec3(0, 1, 0), SIMD_PI / (float)rotSize);
	}
}

void BenchmarkDemo::createTest2()
{
	setCameraDistance(Scalar(50.));
	const float cubeSize = 1.0f;

	createPyramid(Vec3(-20.0f, 0.0f, 0.0f), 12, Vec3(cubeSize, cubeSize, cubeSize));
	createWall(Vec3(-2.0f, 0.0f, 0.0f), 12, Vec3(cubeSize, cubeSize, cubeSize));
	createWall(Vec3(4.0f, 0.0f, 0.0f), 12, Vec3(cubeSize, cubeSize, cubeSize));
	createWall(Vec3(10.0f, 0.0f, 0.0f), 12, Vec3(cubeSize, cubeSize, cubeSize));
	createTowerCircle(Vec3(25.0f, 0.0f, 0.0f), 8, 24, Vec3(cubeSize, cubeSize, cubeSize));
}

// Enrico: Shouldn't these three variables be real constants and not defines?

#ifndef M_PI
#define M_PI Scalar(3.14159265358979323846)
#endif

#ifndef M_PI_2
#define M_PI_2 Scalar(1.57079632679489661923)
#endif

#ifndef M_PI_4
#define M_PI_4 Scalar(0.785398163397448309616)
#endif

class RagDoll
{
	enum
	{
		BODYPART_PELVIS = 0,
		BODYPART_SPINE,
		BODYPART_HEAD,

		BODYPART_LEFT_UPPER_LEG,
		BODYPART_LEFT_LOWER_LEG,

		BODYPART_RIGHT_UPPER_LEG,
		BODYPART_RIGHT_LOWER_LEG,

		BODYPART_LEFT_UPPER_ARM,
		BODYPART_LEFT_LOWER_ARM,

		BODYPART_RIGHT_UPPER_ARM,
		BODYPART_RIGHT_LOWER_ARM,

		BODYPART_COUNT
	};

	enum
	{
		JOINT_PELVIS_SPINE = 0,
		JOINT_SPINE_HEAD,

		JOINT_LEFT_HIP,
		JOINT_LEFT_KNEE,

		JOINT_RIGHT_HIP,
		JOINT_RIGHT_KNEE,

		JOINT_LEFT_SHOULDER,
		JOINT_LEFT_ELBOW,

		JOINT_RIGHT_SHOULDER,
		JOINT_RIGHT_ELBOW,

		JOINT_COUNT
	};

	DynamicsWorld* m_ownerWorld;
	CollisionShape* m_shapes[BODYPART_COUNT];
	RigidBody* m_bodies[BODYPART_COUNT];
	TypedConstraint* m_joints[JOINT_COUNT];

	RigidBody* createRigidBody(Scalar mass, const Transform2& startTransform, CollisionShape* shape)
	{
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			shape->calculateLocalInertia(mass, localInertia);

		DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}

public:
	RagDoll(DynamicsWorld* ownerWorld, const Vec3& positionOffset, Scalar scale)
		: m_ownerWorld(ownerWorld)
	{
		// Setup the geometry
		m_shapes[BODYPART_PELVIS] = new CapsuleShape(Scalar(0.15) * scale, Scalar(0.20) * scale);
		m_shapes[BODYPART_SPINE] = new CapsuleShape(Scalar(0.15) * scale, Scalar(0.28) * scale);
		m_shapes[BODYPART_HEAD] = new CapsuleShape(Scalar(0.10) * scale, Scalar(0.05) * scale);
		m_shapes[BODYPART_LEFT_UPPER_LEG] = new CapsuleShape(Scalar(0.07) * scale, Scalar(0.45) * scale);
		m_shapes[BODYPART_LEFT_LOWER_LEG] = new CapsuleShape(Scalar(0.05) * scale, Scalar(0.37) * scale);
		m_shapes[BODYPART_RIGHT_UPPER_LEG] = new CapsuleShape(Scalar(0.07) * scale, Scalar(0.45) * scale);
		m_shapes[BODYPART_RIGHT_LOWER_LEG] = new CapsuleShape(Scalar(0.05) * scale, Scalar(0.37) * scale);
		m_shapes[BODYPART_LEFT_UPPER_ARM] = new CapsuleShape(Scalar(0.05) * scale, Scalar(0.33) * scale);
		m_shapes[BODYPART_LEFT_LOWER_ARM] = new CapsuleShape(Scalar(0.04) * scale, Scalar(0.25) * scale);
		m_shapes[BODYPART_RIGHT_UPPER_ARM] = new CapsuleShape(Scalar(0.05) * scale, Scalar(0.33) * scale);
		m_shapes[BODYPART_RIGHT_LOWER_ARM] = new CapsuleShape(Scalar(0.04) * scale, Scalar(0.25) * scale);

		// Setup all the rigid bodies
		Transform2 offset;
		offset.setIdentity();
		offset.setOrigin(positionOffset);

		Transform2 transform;
		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.), Scalar(1.), Scalar(0.)));
		m_bodies[BODYPART_PELVIS] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_PELVIS]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.), Scalar(1.2), Scalar(0.)));
		m_bodies[BODYPART_SPINE] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_SPINE]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.), Scalar(1.6), Scalar(0.)));
		m_bodies[BODYPART_HEAD] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_HEAD]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(-0.18), Scalar(0.65), Scalar(0.)));
		m_bodies[BODYPART_LEFT_UPPER_LEG] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_LEFT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(-0.18), Scalar(0.2), Scalar(0.)));
		m_bodies[BODYPART_LEFT_LOWER_LEG] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_LEFT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.18), Scalar(0.65), Scalar(0.)));
		m_bodies[BODYPART_RIGHT_UPPER_LEG] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.18), Scalar(0.2), Scalar(0.)));
		m_bodies[BODYPART_RIGHT_LOWER_LEG] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(-0.35), Scalar(1.45), Scalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, M_PI_2);
		m_bodies[BODYPART_LEFT_UPPER_ARM] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_LEFT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(-0.7), Scalar(1.45), Scalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, M_PI_2);
		m_bodies[BODYPART_LEFT_LOWER_ARM] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_LEFT_LOWER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.35), Scalar(1.45), Scalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, -M_PI_2);
		m_bodies[BODYPART_RIGHT_UPPER_ARM] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale * Vec3(Scalar(0.7), Scalar(1.45), Scalar(0.)));
		transform.getBasis().setEulerZYX(0, 0, -M_PI_2);
		m_bodies[BODYPART_RIGHT_LOWER_ARM] = createRigidBody(Scalar(1.), offset * transform, m_shapes[BODYPART_RIGHT_LOWER_ARM]);

		// Setup some damping on the m_bodies
		for (i32 i = 0; i < BODYPART_COUNT; ++i)
		{
			m_bodies[i]->setDamping(Scalar(0.05), Scalar(0.85));
			m_bodies[i]->setDeactivationTime(Scalar(0.8));
			m_bodies[i]->setSleepingThresholds(Scalar(1.6), Scalar(2.5));
		}

		// Now setup the constraints
		HingeConstraint* hingeC;
		ConeTwistConstraint* coneC;

		Transform2 localA, localB;

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.15), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.15), Scalar(0.)));
		hingeC = new HingeConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_SPINE], localA, localB);
		hingeC->setLimit(Scalar(-M_PI_4), Scalar(M_PI_2));
		m_joints[JOINT_PELVIS_SPINE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_PELVIS_SPINE], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, M_PI_2);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.30), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, M_PI_2);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.14), Scalar(0.)));
		coneC = new ConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_HEAD], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, M_PI_2);
		m_joints[JOINT_SPINE_HEAD] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_SPINE_HEAD], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, -M_PI_4 * 5);
		localA.setOrigin(scale * Vec3(Scalar(-0.18), Scalar(-0.10), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, -M_PI_4 * 5);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.225), Scalar(0.)));
		coneC = new ConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_LEFT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_LEFT_HIP] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_HIP], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.225), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.185), Scalar(0.)));
		hingeC = new HingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_LEG], *m_bodies[BODYPART_LEFT_LOWER_LEG], localA, localB);
		hingeC->setLimit(Scalar(0), Scalar(M_PI_2));
		m_joints[JOINT_LEFT_KNEE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_KNEE], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, M_PI_4);
		localA.setOrigin(scale * Vec3(Scalar(0.18), Scalar(-0.10), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, M_PI_4);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.225), Scalar(0.)));
		coneC = new ConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_RIGHT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_RIGHT_HIP] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_HIP], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.225), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.185), Scalar(0.)));
		hingeC = new HingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_LEG], *m_bodies[BODYPART_RIGHT_LOWER_LEG], localA, localB);
		hingeC->setLimit(Scalar(0), Scalar(M_PI_2));
		m_joints[JOINT_RIGHT_KNEE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_KNEE], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, M_PI);
		localA.setOrigin(scale * Vec3(Scalar(-0.2), Scalar(0.15), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, M_PI_2);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.18), Scalar(0.)));
		coneC = new ConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_LEFT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		m_joints[JOINT_LEFT_SHOULDER] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_SHOULDER], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.18), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.14), Scalar(0.)));
		hingeC = new HingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_ARM], *m_bodies[BODYPART_LEFT_LOWER_ARM], localA, localB);
		hingeC->setLimit(Scalar(-M_PI_2), Scalar(0));
		m_joints[JOINT_LEFT_ELBOW] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_ELBOW], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, 0, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.2), Scalar(0.15), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, 0, M_PI_2);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.18), Scalar(0.)));
		coneC = new ConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_RIGHT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		m_joints[JOINT_RIGHT_SHOULDER] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_SHOULDER], true);

		localA.setIdentity();
		localB.setIdentity();
		localA.getBasis().setEulerZYX(0, M_PI_2, 0);
		localA.setOrigin(scale * Vec3(Scalar(0.), Scalar(0.18), Scalar(0.)));
		localB.getBasis().setEulerZYX(0, M_PI_2, 0);
		localB.setOrigin(scale * Vec3(Scalar(0.), Scalar(-0.14), Scalar(0.)));
		hingeC = new HingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_ARM], *m_bodies[BODYPART_RIGHT_LOWER_ARM], localA, localB);
		hingeC->setLimit(Scalar(-M_PI_2), Scalar(0));
		m_joints[JOINT_RIGHT_ELBOW] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_ELBOW], true);
	}

	virtual ~RagDoll()
	{
		i32 i;

		// Remove all constraints
		for (i = 0; i < JOINT_COUNT; ++i)
		{
			m_ownerWorld->removeConstraint(m_joints[i]);
			delete m_joints[i];
			m_joints[i] = 0;
		}

		// Remove all bodies and shapes
		for (i = 0; i < BODYPART_COUNT; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);

			delete m_bodies[i]->getMotionState();

			delete m_bodies[i];
			m_bodies[i] = 0;
			delete m_shapes[i];
			m_shapes[i] = 0;
		}
	}
};

void BenchmarkDemo::createTest3()
{
	setCameraDistance(Scalar(50.));

	i32 size = 16;

	float sizeX = 1.f;
	float sizeY = 1.f;

	//i32 rc=0;

	Scalar scale(3.5);
	Vec3 pos(0.0f, sizeY, 0.0f);
	while (size)
	{
		float offset = -size * (sizeX * 6.0f) * 0.5f;
		for (i32 i = 0; i < size; i++)
		{
			pos[0] = offset + (float)i * (sizeX * 6.0f);

			RagDoll* ragDoll = new RagDoll(m_dynamicsWorld, pos, scale);
			m_ragdolls.push_back(ragDoll);
		}

		offset += sizeX;
		pos[1] += (sizeY * 7.0f);
		pos[2] -= sizeX * 2.0f;
		size--;
	}
}
void BenchmarkDemo::createTest4()
{
	setCameraDistance(Scalar(50.));

	i32 size = 8;
	const float cubeSize = 1.5f;
	float spacing = cubeSize;
	Vec3 pos(0.0f, cubeSize * 2, 0.0f);
	float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

	ConvexHullShape* convexHullShape = new ConvexHullShape();

	Scalar scaling(1);

	convexHullShape->setLocalScaling(Vec3(scaling, scaling, scaling));

	for (i32 i = 0; i < TaruVtxCount; i++)
	{
		Vec3 vtx(TaruVtx[i * 3], TaruVtx[i * 3 + 1], TaruVtx[i * 3 + 2]);
		convexHullShape->addPoint(vtx * Scalar(1. / scaling));
	}

	//this will enable polyhedral contact clipping, better quality, slightly slower
	convexHullShape->initializePolyhedralFeatures();

	Transform2 trans;
	trans.setIdentity();

	float mass = 1.f;
	Vec3 localInertia(0, 0, 0);
	convexHullShape->calculateLocalInertia(mass, localInertia);

	for (i32 k = 0; k < 15; k++)
	{
		for (i32 j = 0; j < size; j++)
		{
			pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
			for (i32 i = 0; i < size; i++)
			{
				pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
				trans.setOrigin(pos);
				createRigidBody(mass, trans, convexHullShape);
			}
		}
		offset -= 0.05f * spacing * (size - 1);
		spacing *= 1.01f;
		pos[1] += (cubeSize * 2.0f + spacing);
	}
}

///////////////////////////////////////////////////////////////////////////////
// LargeMesh

i32 LandscapeVtxCount[] = {
	Landscape01VtxCount,
	Landscape02VtxCount,
	Landscape03VtxCount,
	Landscape04VtxCount,
	Landscape05VtxCount,
	Landscape06VtxCount,
	Landscape07VtxCount,
	Landscape08VtxCount,
};

i32 LandscapeIdxCount[] = {
	Landscape01IdxCount,
	Landscape02IdxCount,
	Landscape03IdxCount,
	Landscape04IdxCount,
	Landscape05IdxCount,
	Landscape06IdxCount,
	Landscape07IdxCount,
	Landscape08IdxCount,
};

Scalar* LandscapeVtx[] = {
	Landscape01Vtx,
	Landscape02Vtx,
	Landscape03Vtx,
	Landscape04Vtx,
	Landscape05Vtx,
	Landscape06Vtx,
	Landscape07Vtx,
	Landscape08Vtx,
};

Scalar* LandscapeNml[] = {
	Landscape01Nml,
	Landscape02Nml,
	Landscape03Nml,
	Landscape04Nml,
	Landscape05Nml,
	Landscape06Nml,
	Landscape07Nml,
	Landscape08Nml,
};

Scalar* LandscapeTex[] = {
	Landscape01Tex,
	Landscape02Tex,
	Landscape03Tex,
	Landscape04Tex,
	Landscape05Tex,
	Landscape06Tex,
	Landscape07Tex,
	Landscape08Tex,
};

unsigned short* LandscapeIdx[] = {
	Landscape01Idx,
	Landscape02Idx,
	Landscape03Idx,
	Landscape04Idx,
	Landscape05Idx,
	Landscape06Idx,
	Landscape07Idx,
	Landscape08Idx,
};

void BenchmarkDemo::createLargeMeshBody()
{
	Transform2 trans;
	trans.setIdentity();

	for (i32 i = 0; i < 8; i++)
	{
		TriangleIndexVertexArray* meshInterface = new TriangleIndexVertexArray();
		IndexedMesh part;

		part.m_vertexBase = (u8k*)LandscapeVtx[i];
		part.m_vertexStride = sizeof(Scalar) * 3;
		part.m_numVertices = LandscapeVtxCount[i];
		part.m_triangleIndexBase = (u8k*)LandscapeIdx[i];
		part.m_triangleIndexStride = sizeof(short) * 3;
		part.m_numTriangles = LandscapeIdxCount[i] / 3;
		part.m_indexType = PHY_SHORT;

		meshInterface->addIndexedMesh(part, PHY_SHORT);

		bool useQuantizedAabbCompression = true;
		BvhTriangleMeshShape* trimeshShape = new BvhTriangleMeshShape(meshInterface, useQuantizedAabbCompression);
		Vec3 localInertia(0, 0, 0);
		trans.setOrigin(Vec3(0, -25, 0));

		RigidBody* body = createRigidBody(0, trans, trimeshShape);
		body->setFriction(Scalar(0.9));
	}
}

void BenchmarkDemo::createTest5()
{
	setCameraDistance(Scalar(250.));
	Vec3 boxSize(1.5f, 1.5f, 1.5f);
	float boxMass = 1.0f;
	float sphereRadius = 1.5f;
	float sphereMass = 1.0f;
	float capsuleHalf = 2.0f;
	float capsuleRadius = 1.0f;
	float capsuleMass = 1.0f;

	{
		i32 size = 10;
		i32 height = 10;

		const float cubeSize = boxSize[0];
		float spacing = 2.0f;
		Vec3 pos(0.0f, 20.0f, 0.0f);
		float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

		i32 numBodies = 0;

		for (i32 k = 0; k < height; k++)
		{
			for (i32 j = 0; j < size; j++)
			{
				pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
				for (i32 i = 0; i < size; i++)
				{
					pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
					Vec3 bpos = Vec3(0, 25, 0) + Vec3(5.0f, 1.0f, 5.0f) * pos;
					i32 idx = rand() % 9;
					Transform2 trans;
					trans.setIdentity();
					trans.setOrigin(bpos);

					switch (idx)
					{
						case 0:
						case 1:
						case 2:
						{
							float r = 0.5f * (idx + 1);
							BoxShape* boxShape = new BoxShape(boxSize * r);
							createRigidBody(boxMass * r, trans, boxShape);
						}
						break;

						case 3:
						case 4:
						case 5:
						{
							float r = 0.5f * (idx - 3 + 1);
							SphereShape* sphereShape = new SphereShape(sphereRadius * r);
							createRigidBody(sphereMass * r, trans, sphereShape);
						}
						break;

						case 6:
						case 7:
						case 8:
						{
							float r = 0.5f * (idx - 6 + 1);
							CapsuleShape* capsuleShape = new CapsuleShape(capsuleRadius * r, capsuleHalf * r);
							createRigidBody(capsuleMass * r, trans, capsuleShape);
						}
						break;
					}

					numBodies++;
				}
			}
			offset -= 0.05f * spacing * (size - 1);
			spacing *= 1.1f;
			pos[1] += (cubeSize * 2.0f + spacing);
		}
	}

	createLargeMeshBody();
}
void BenchmarkDemo::createTest6()
{
	setCameraDistance(Scalar(250.));

	Vec3 boxSize(1.5f, 1.5f, 1.5f);

	ConvexHullShape* convexHullShape = new ConvexHullShape();

	for (i32 i = 0; i < TaruVtxCount; i++)
	{
		Vec3 vtx(TaruVtx[i * 3], TaruVtx[i * 3 + 1], TaruVtx[i * 3 + 2]);
		convexHullShape->addPoint(vtx);
	}

	Transform2 trans;
	trans.setIdentity();

	float mass = 1.f;
	Vec3 localInertia(0, 0, 0);
	convexHullShape->calculateLocalInertia(mass, localInertia);

	{
		i32 size = 10;
		i32 height = 10;

		const float cubeSize = boxSize[0];
		float spacing = 2.0f;
		Vec3 pos(0.0f, 20.0f, 0.0f);
		float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

		for (i32 k = 0; k < height; k++)
		{
			for (i32 j = 0; j < size; j++)
			{
				pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
				for (i32 i = 0; i < size; i++)
				{
					pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
					Vec3 bpos = Vec3(0, 25, 0) + Vec3(5.0f, 1.0f, 5.0f) * pos;
					trans.setOrigin(bpos);

					createRigidBody(mass, trans, convexHullShape);
				}
			}
			offset -= 0.05f * spacing * (size - 1);
			spacing *= 1.1f;
			pos[1] += (cubeSize * 2.0f + spacing);
		}
	}

	createLargeMeshBody();
}

void BenchmarkDemo::initRays()
{
	raycastBar = RaycastBar2(2500.0, 0, 50.0, m_guiHelper);
}

void BenchmarkDemo::castRays()
{
	raycastBar.cast(m_dynamicsWorld, m_multithreadedWorld);
}

void BenchmarkDemo::createTest7()
{
	createTest6();
	setCameraDistance(Scalar(150.));
	initRays();
}

void BenchmarkDemo::createTest8()
{
	float dist = 8;
	float pitch = -15;
	float yaw = 20;
	float targetPos[3] = {0, 1, 0};
	m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	// Create a shape and rigid body for each Voronoi cell.
	const float fallHeight = 3.5f;
	for (i32 i=0; i<halton_numc; ++i)
	{
		ConvexHullShape* shp = new ConvexHullShape();
		const float* verts  = halton_verts[i];
		const float* origin = halton_pos[i];
		for (i32 v=0; v<halton_numv[i]; ++v)
		{
			Vec3 vtx(verts[0],verts[1],verts[2]);
			shp->addPoint(vtx);
			verts += 3;
		}
		shp->initializePolyhedralFeatures();
		shp->setMargin(0.04f);
		Transform2 transform;
		transform.setIdentity();
		transform.setOrigin(Vec3(origin[0],origin[1]+fallHeight,origin[2]));
		const float mass = halton_volu[i];
		Vec3 inertia(0,0,0);
		shp->calculateLocalInertia(mass, inertia);
		RigidBody::RigidBodyConstructionInfo ci(mass, 0, shp, inertia);
		ci.m_startWorldTransform = transform;
		RigidBody* body = new RigidBody(ci);
		body->setFriction(0.6f);
		m_dynamicsWorld->addRigidBody(body);
	}

        ContactSolverInfo& si = m_dynamicsWorld->getSolverInfo();
	si.m_numIterations = 20;
	si.m_erp = 0.8f;
	si.m_erp2 = si.m_erp / 2;
	si.m_globalCfm = 0.015f;
	// Create a ground plane
	CollisionShape* groundplane = new StaticPlaneShape(Vec3(0,1,0),0);
	groundplane->setMargin(0.04f);
	RigidBody::RigidBodyConstructionInfo rc(0.0f, 0, groundplane, Vec3(0,0,0));
	RigidBody* groundbody = new RigidBody(rc);
	m_dynamicsWorld->addRigidBody(groundbody);
#if 0
	// Use SAT for slower, but better contact generation.
	btDispatcherInfo& di = m_dynamicsWorld->getDispatchInfo();
	di.m_enableSatConvex = true;
#endif
}

void BenchmarkDemo::exitPhysics()
{
	i32 i;

	for (i = 0; i < m_ragdolls.size(); i++)
	{
		RagDoll* doll = m_ragdolls[i];
		delete doll;
	}
	m_ragdolls.clear();

	CommonRigidBodyMTBase::exitPhysics();
}

CommonExampleInterface* BenchmarkCreateFunc(struct CommonExampleOptions& options)
{
	return new BenchmarkDemo(options.m_guiHelper, options.m_option);
}
