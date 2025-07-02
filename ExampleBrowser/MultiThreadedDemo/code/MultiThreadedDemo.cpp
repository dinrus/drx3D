#include "../MultiThreadedDemo.h"
#include "../CommonRigidBodyMTBase.h"
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <stdio.h>  //printf debugging
#include <algorithm>
#include <cmath>

static Scalar gSliderStackRows = 1.0f;
static Scalar gSliderStackColumns = 1.0f;
static Scalar gSliderStackHeight = 15.0f;
static Scalar gSliderStackWidth = 8.0f;
static Scalar gSliderGroundHorizontalAmplitude = 0.0f;
static Scalar gSliderGroundVerticalAmplitude = 0.0f;
static Scalar gSliderGroundTilt = 0.0f;
static Scalar gSliderRollingFriction = 0.0f;
static bool gSpheresNotBoxes = false;

static void boolPtrButtonCallback(i32 buttonId, bool buttonState, uk userPointer)
{
	if (bool* val = static_cast<bool*>(userPointer))
	{
		*val = !*val;
	}
}

/// MultiThreadedDemo shows how to setup and use multithreading
class MultiThreadedDemo : public CommonRigidBodyMTBase
{
	static i32k kUpAxis = 1;

	RigidBody* localCreateRigidBody(Scalar mass, const Transform2& worldTransform, CollisionShape* colSape);

	Vec3 m_cameraTargetPos;
	float m_cameraPitch;
	float m_cameraYaw;
	float m_cameraDist;
	RigidBody* m_groundBody;
	Transform2 m_groundStartXf;
	float m_groundMovePhase;
	float m_prevRollingFriction;

	void createStack(const Transform2& trans, CollisionShape* boxShape, const Vec3& halfBoxSize, i32 size, i32 width);
	void createSceneObjects();
	void destroySceneObjects();

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultiThreadedDemo(struct GUIHelperInterface* helper);

	virtual ~MultiThreadedDemo() {}

	Quat getGroundRotation() const
	{
		Scalar tilt = gSliderGroundTilt * SIMD_2_PI / 360.0f;
		return Quat(Vec3(1.0f, 0.0f, 0.0f), tilt);
	}
	struct TestSumBody : public IParallelSumBody
	{
		virtual Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
		{
			Scalar sum = 0.0f;
			for (i32 i = iBegin; i < iEnd; ++i)
			{
				if (i > 0)
				{
					sum += 1.0f / Scalar(i);
				}
			}
			return sum;
		}
	};
	virtual void stepSimulation(float deltaTime) DRX3D_OVERRIDE
	{
		if (m_dynamicsWorld)
		{
			if (m_prevRollingFriction != gSliderRollingFriction)
			{
				m_prevRollingFriction = gSliderRollingFriction;
				CollisionObject2Array& objArray = m_dynamicsWorld->getCollisionObjectArray();
				for (i32 i = 0; i < objArray.size(); ++i)
				{
					CollisionObject2* obj = objArray[i];
					obj->setRollingFriction(gSliderRollingFriction);
				}
			}
			if (m_groundBody)
			{
				// update ground
				const float cyclesPerSecond = 1.0f;
				m_groundMovePhase += cyclesPerSecond * deltaTime;
				m_groundMovePhase -= std::floor(m_groundMovePhase);  // keep phase between 0 and 1
				Transform2 xf = m_groundStartXf;
				float gndHOffset = Sin(m_groundMovePhase * SIMD_2_PI) * gSliderGroundHorizontalAmplitude;
				float gndHVel = Cos(m_groundMovePhase * SIMD_2_PI) * gSliderGroundHorizontalAmplitude * cyclesPerSecond * SIMD_2_PI;  // d(gndHOffset)/dt
				float gndVOffset = Sin(m_groundMovePhase * SIMD_2_PI) * gSliderGroundVerticalAmplitude;
				float gndVVel = Cos(m_groundMovePhase * SIMD_2_PI) * gSliderGroundVerticalAmplitude * cyclesPerSecond * SIMD_2_PI;  // d(gndVOffset)/dt
				Vec3 offset(0, 0, 0);
				Vec3 vel(0, 0, 0);
				i32 horizAxis = 2;
				offset[horizAxis] = gndHOffset;
				vel[horizAxis] = gndHVel;
				offset[kUpAxis] = gndVOffset;
				vel[kUpAxis] = gndVVel;
				xf.setOrigin(xf.getOrigin() + offset);
				xf.setRotation(getGroundRotation());
				m_groundBody->setWorldTransform(xf);
				m_groundBody->setLinearVelocity(vel);
			}
			// always step by 1/60 for benchmarking
			m_dynamicsWorld->stepSimulation(1.0f / 60.0f, 0);
		}
#if 0
        {
            // test parallelSum
            TestSumBody testSumBody;
            float testSum = ParallelSum( 1, 10000000, 10000, testSumBody );
            printf( "sum = %f\n", testSum );
        }
#endif
	}

	virtual void initPhysics() DRX3D_OVERRIDE;
	virtual void resetCamera() DRX3D_OVERRIDE
	{
		m_guiHelper->resetCamera(m_cameraDist,
								 m_cameraYaw,
								 m_cameraPitch,
								 m_cameraTargetPos.x(),
								 m_cameraTargetPos.y(),
								 m_cameraTargetPos.z());
	}
};

MultiThreadedDemo::MultiThreadedDemo(struct GUIHelperInterface* helper)
	: CommonRigidBodyMTBase(helper)
{
	m_groundBody = NULL;
	m_groundMovePhase = 0.0f;
	m_cameraTargetPos = Vec3(0.0f, 0.0f, 0.0f);
	m_cameraPitch = -30.0f;
	m_cameraYaw = 90.0f;
	m_cameraDist = 48.0f;
	m_prevRollingFriction = -1.0f;
	helper->setUpAxis(kUpAxis);
}

void MultiThreadedDemo::initPhysics()
{
	createEmptyDynamicsWorld();

	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

	{
		SliderParams slider("Stack height", &gSliderStackHeight);
		slider.m_minVal = 1.0f;
		slider.m_maxVal = 30.0f;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Stack width", &gSliderStackWidth);
		slider.m_minVal = 1.0f;
		slider.m_maxVal = 30.0f;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Stack rows", &gSliderStackRows);
		slider.m_minVal = 1.0f;
		slider.m_maxVal = 20.0f;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Stack columns", &gSliderStackColumns);
		slider.m_minVal = 1.0f;
		slider.m_maxVal = 20.0f;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		// horizontal ground shake
		SliderParams slider("Ground horiz amp", &gSliderGroundHorizontalAmplitude);
		slider.m_minVal = 0.0f;
		slider.m_maxVal = 1.0f;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		// vertical ground shake
		SliderParams slider("Ground vert amp", &gSliderGroundVerticalAmplitude);
		slider.m_minVal = 0.0f;
		slider.m_maxVal = 1.0f;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		// ground tilt
		SliderParams slider("Ground tilt", &gSliderGroundTilt);
		slider.m_minVal = -45.0f;
		slider.m_maxVal = 45.0f;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		// rolling friction
		SliderParams slider("Rolling friction", &gSliderRollingFriction);
		slider.m_minVal = 0.0f;
		slider.m_maxVal = 1.0f;
		slider.m_clampToNotches = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		ButtonParams button("Spheres not boxes", 0, false);
		button.m_initialState = gSpheresNotBoxes;
		button.m_userPointer = &gSpheresNotBoxes;
		button.m_callback = boolPtrButtonCallback;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}

	createSceneObjects();

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
}

RigidBody* MultiThreadedDemo::localCreateRigidBody(Scalar mass, const Transform2& startTransform, CollisionShape* shape)
{
	RigidBody* body = createRigidBody(mass, startTransform, shape);
	if (mass > 0.0f)
	{
		// prevent bodies from sleeping to make profiling/benchmarking easier
		body->forceActivationState(DISABLE_DEACTIVATION);
	}
	return body;
}

void MultiThreadedDemo::createStack(const Transform2& parentTrans, CollisionShape* boxShape, const Vec3& halfBoxSize, i32 height, i32 width)
{
	Transform2 trans;
	trans.setIdentity();
	trans.setRotation(parentTrans.getRotation());
	float halfBoxHeight = halfBoxSize.y();
	float halfBoxWidth = halfBoxSize.x();

	Vec3 offset = Vec3(0, 0, -halfBoxSize.z() * (width - 1));
	for (i32 iZ = 0; iZ < width; iZ++)
	{
		offset += Vec3(0, 0, halfBoxSize.z() * 2.0f);
		for (i32 iY = 0; iY < height; iY++)
		{
			// This constructs a row, from left to right
			i32 rowSize = height - iY;
			for (i32 iX = 0; iX < rowSize; iX++)
			{
				Vec3 pos = offset + Vec3(halfBoxWidth * (1 + iX * 2 - rowSize),
												   halfBoxHeight * (1 + iY * 2),
												   0.0f);

				trans.setOrigin(parentTrans(pos));
				Scalar mass = 1.f;

				RigidBody* body = localCreateRigidBody(mass, trans, boxShape);
				body->setFriction(1.0f);
				body->setRollingFriction(gSliderRollingFriction);
			}
		}
	}
}

void MultiThreadedDemo::createSceneObjects()
{
	{
		// create ground box
		m_groundStartXf.setOrigin(Vec3(0.f, -3.f, 0.f));
		m_groundStartXf.setRotation(getGroundRotation());

		//either use heightfield or triangle mesh

		Vec3 groundExtents(400, 400, 400);
		groundExtents[kUpAxis] = 3;
		CollisionShape* groundShape = new BoxShape(groundExtents);
		m_collisionShapes.push_back(groundShape);

		//create ground object
		m_groundBody = createKinematicBody(m_groundStartXf, groundShape);
		m_groundBody->forceActivationState(DISABLE_DEACTIVATION);
		m_groundBody->setFriction(1.0f);
	}

	{
		// create walls of cubes
		const Vec3 halfExtents = Vec3(0.5f, 0.25f, 0.5f);
		i32 numStackRows = d3Max(1, i32(gSliderStackRows));
		i32 numStackCols = d3Max(1, i32(gSliderStackColumns));
		i32 stackHeight = i32(gSliderStackHeight);
		i32 stackWidth = i32(gSliderStackWidth);
		float stackZSpacing = 2.0f + stackWidth * halfExtents.x() * 2.0f;
		float stackXSpacing = 20.0f;

		BoxShape* boxShape = new BoxShape(halfExtents);
		m_collisionShapes.push_back(boxShape);

		SphereShape* sphereShape = new SphereShape(0.5f);
		m_collisionShapes.push_back(sphereShape);

		CollisionShape* shape = boxShape;
		if (gSpheresNotBoxes)
		{
			shape = sphereShape;
		}

		Transform2 groundTrans;
		groundTrans.setIdentity();
		groundTrans.setRotation(getGroundRotation());
		for (i32 iX = 0; iX < numStackCols; ++iX)
		{
			for (i32 iZ = 0; iZ < numStackRows; ++iZ)
			{
				Vec3 center = Vec3(iX * stackXSpacing, 0.0f, (iZ - numStackRows / 2) * stackZSpacing);
				Transform2 trans = groundTrans;
				trans.setOrigin(groundTrans(center));
				createStack(trans, shape, halfExtents, stackHeight, stackWidth);
			}
		}
	}
#if 0
    if ( false )
    {
        // destroyer ball
        Transform2 sphereTrans;
        sphereTrans.setIdentity();
        sphereTrans.setOrigin( Vec3( 0, 2, 40 ) );
        SphereShape* ball = new SphereShape( 2.f );
        m_collisionShapes.push_back( ball );
        RigidBody* ballBody = localCreateRigidBody( 10000.f, sphereTrans, ball );
        ballBody->setLinearVelocity( Vec3( 0, 0, -10 ) );
    }
#endif
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

CommonExampleInterface* MultiThreadedDemoCreateFunc(struct CommonExampleOptions& options)
{
	return new MultiThreadedDemo(options.m_guiHelper);
}
