#include <drx3D/ExampleBrowser/Collision/Internal/Bullet2CollisionSdk.h>
#include <drx3D/CollisionCommon.h>

struct Bullet2CollisionSdkInternalData
{
	CollisionConfiguration* m_collisionConfig;
	CollisionDispatcher* m_dispatcher;
	BroadphaseInterface* m_aabbBroadphase;
	CollisionWorld* m_collisionWorld;

	Bullet2CollisionSdkInternalData()
		: m_collisionConfig(0),
		  m_dispatcher(0),
		  m_aabbBroadphase(0),
		  m_collisionWorld(0)
	{
	}
};

Bullet2CollisionSdk::Bullet2CollisionSdk()
{
	m_internalData = new Bullet2CollisionSdkInternalData;
}

Bullet2CollisionSdk::~Bullet2CollisionSdk()
{
	delete m_internalData;
	m_internalData = 0;
}

plCollisionWorldHandle Bullet2CollisionSdk::createCollisionWorld(i32 /*maxNumObjsCapacity*/, i32 /*maxNumShapesCapacity*/, i32 /*maxNumPairsCapacity*/)
{
	m_internalData->m_collisionConfig = new DefaultCollisionConfiguration;

	m_internalData->m_dispatcher = new CollisionDispatcher(m_internalData->m_collisionConfig);
	m_internalData->m_aabbBroadphase = new DbvtBroadphase();
	m_internalData->m_collisionWorld = new CollisionWorld(m_internalData->m_dispatcher,
															m_internalData->m_aabbBroadphase,
															m_internalData->m_collisionConfig);
	return (plCollisionWorldHandle)m_internalData->m_collisionWorld;
}

void Bullet2CollisionSdk::deleteCollisionWorld(plCollisionWorldHandle worldHandle)
{
	CollisionWorld* world = (CollisionWorld*)worldHandle;
	Assert(m_internalData->m_collisionWorld == world);

	if (m_internalData->m_collisionWorld == world)
	{
		delete m_internalData->m_collisionWorld;
		m_internalData->m_collisionWorld = 0;
		delete m_internalData->m_aabbBroadphase;
		m_internalData->m_aabbBroadphase = 0;
		delete m_internalData->m_dispatcher;
		m_internalData->m_dispatcher = 0;
		delete m_internalData->m_collisionConfig;
		m_internalData->m_collisionConfig = 0;
	}
}

plCollisionShapeHandle Bullet2CollisionSdk::createSphereShape(plCollisionWorldHandle /*worldHandle*/, plReal radius)
{
	SphereShape* sphereShape = new SphereShape(radius);
	return (plCollisionShapeHandle)sphereShape;
}

plCollisionShapeHandle Bullet2CollisionSdk::createPlaneShape(plCollisionWorldHandle worldHandle,
															 plReal planeNormalX,
															 plReal planeNormalY,
															 plReal planeNormalZ,
															 plReal planeConstant)
{
	StaticPlaneShape* planeShape = new StaticPlaneShape(Vec3(planeNormalX, planeNormalY, planeNormalZ), planeConstant);
	return (plCollisionShapeHandle)planeShape;
}

plCollisionShapeHandle Bullet2CollisionSdk::createCapsuleShape(plCollisionWorldHandle worldHandle,
															   plReal radius,
															   plReal height,
															   i32 capsuleAxis)
{
	CapsuleShape* capsule = 0;

	switch (capsuleAxis)
	{
		case 0:
		{
			capsule = new CapsuleShapeX(radius, height);
			break;
		}
		case 1:
		{
			capsule = new CapsuleShape(radius, height);
			break;
		}
		case 2:
		{
			capsule = new CapsuleShapeZ(radius, height);
			break;
		}
		default:
		{
			Assert(0);
		}
	}
	return (plCollisionShapeHandle)capsule;
}

plCollisionShapeHandle Bullet2CollisionSdk::createCompoundShape(plCollisionWorldHandle worldHandle)
{
	return (plCollisionShapeHandle) new CompoundShape();
}
void Bullet2CollisionSdk::addChildShape(plCollisionWorldHandle worldHandle, plCollisionShapeHandle compoundShapeHandle, plCollisionShapeHandle childShapeHandle, plVector3 childPos, plQuat childOrn)
{
	CompoundShape* compound = (CompoundShape*)compoundShapeHandle;
	CollisionShape* childShape = (CollisionShape*)childShapeHandle;
	Transform2 localTrans;
	localTrans.setOrigin(Vec3(childPos[0], childPos[1], childPos[2]));
	localTrans.setRotation(Quat(childOrn[0], childOrn[1], childOrn[2], childOrn[3]));
	compound->addChildShape(localTrans, childShape);
}

void Bullet2CollisionSdk::deleteShape(plCollisionWorldHandle /*worldHandle*/, plCollisionShapeHandle shapeHandle)
{
	CollisionShape* shape = (CollisionShape*)shapeHandle;
	delete shape;
}

void Bullet2CollisionSdk::addCollisionObject(plCollisionWorldHandle worldHandle, plCollisionObjectHandle objectHandle)
{
	CollisionWorld* world = (CollisionWorld*)worldHandle;
	CollisionObject2* colObj = (CollisionObject2*)objectHandle;
	Assert(world && colObj);
	if (world == m_internalData->m_collisionWorld && colObj)
	{
		world->addCollisionObject(colObj);
	}
}
void Bullet2CollisionSdk::removeCollisionObject(plCollisionWorldHandle worldHandle, plCollisionObjectHandle objectHandle)
{
	CollisionWorld* world = (CollisionWorld*)worldHandle;
	CollisionObject2* colObj = (CollisionObject2*)objectHandle;
	Assert(world && colObj);
	if (world == m_internalData->m_collisionWorld && colObj)
	{
		world->removeCollisionObject(colObj);
	}
}

plCollisionObjectHandle Bullet2CollisionSdk::createCollisionObject(plCollisionWorldHandle worldHandle, uk userPointer, i32 userIndex, plCollisionShapeHandle shapeHandle,
																   plVector3 startPosition, plQuat startOrientation)

{
	CollisionShape* colShape = (CollisionShape*)shapeHandle;
	Assert(colShape);
	if (colShape)
	{
		CollisionObject2* colObj = new CollisionObject2;
		colObj->setUserIndex(userIndex);
		colObj->setUserPointer(userPointer);
		colObj->setCollisionShape(colShape);
		Transform2 tr;
		tr.setOrigin(Vec3(startPosition[0], startPosition[1], startPosition[2]));
		tr.setRotation(Quat(startOrientation[0], startOrientation[1], startOrientation[2], startOrientation[3]));
		colObj->setWorldTransform(tr);
		return (plCollisionObjectHandle)colObj;
	}
	return 0;
}

void Bullet2CollisionSdk::deleteCollisionObject(plCollisionObjectHandle bodyHandle)
{
	CollisionObject2* colObj = (CollisionObject2*)bodyHandle;
	delete colObj;
}
void Bullet2CollisionSdk::setCollisionObjectTransform(plCollisionWorldHandle /*worldHandle*/, plCollisionObjectHandle bodyHandle,
													  plVector3 position, plQuat orientation)
{
	CollisionObject2* colObj = (CollisionObject2*)bodyHandle;
	Transform2 tr;
	tr.setOrigin(Vec3(position[0], position[1], position[2]));
	tr.setRotation(Quat(orientation[0], orientation[1], orientation[2], orientation[3]));
	colObj->setWorldTransform(tr);
}

struct Bullet2ContactResultCallback : public CollisionWorld::ContactResultCallback
{
	i32 m_numContacts;
	lwContactPoint* m_pointsOut;
	i32 m_pointCapacity;

	Bullet2ContactResultCallback(lwContactPoint* pointsOut, i32 pointCapacity) : m_numContacts(0),
																				 m_pointsOut(pointsOut),
																				 m_pointCapacity(pointCapacity)
	{
	}
	virtual Scalar addSingleResult(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1)
	{
		if (m_numContacts < m_pointCapacity)
		{
			lwContactPoint& ptOut = m_pointsOut[m_numContacts];
			ptOut.m_distance = cp.m_distance1;
			ptOut.m_normalOnB[0] = cp.m_normalWorldOnB.getX();
			ptOut.m_normalOnB[1] = cp.m_normalWorldOnB.getY();
			ptOut.m_normalOnB[2] = cp.m_normalWorldOnB.getZ();
			ptOut.m_ptOnAWorld[0] = cp.m_positionWorldOnA[0];
			ptOut.m_ptOnAWorld[1] = cp.m_positionWorldOnA[1];
			ptOut.m_ptOnAWorld[2] = cp.m_positionWorldOnA[2];
			ptOut.m_ptOnBWorld[0] = cp.m_positionWorldOnB[0];
			ptOut.m_ptOnBWorld[1] = cp.m_positionWorldOnB[1];
			ptOut.m_ptOnBWorld[2] = cp.m_positionWorldOnB[2];
			m_numContacts++;
		}

		return 1.f;
	}
};

i32 Bullet2CollisionSdk::collide(plCollisionWorldHandle worldHandle, plCollisionObjectHandle colA, plCollisionObjectHandle colB,
								 lwContactPoint* pointsOut, i32 pointCapacity)
{
	CollisionWorld* world = (CollisionWorld*)worldHandle;
	CollisionObject2* colObjA = (CollisionObject2*)colA;
	CollisionObject2* colObjB = (CollisionObject2*)colB;
	Assert(world && colObjA && colObjB);
	if (world == m_internalData->m_collisionWorld && colObjA && colObjB)
	{
		Bullet2ContactResultCallback cb(pointsOut, pointCapacity);
		world->contactPairTest(colObjA, colObjB, cb);
		return cb.m_numContacts;
	}
	return 0;
}

static plNearCallback gTmpFilter;
static i32 gNearCallbackCount = 0;
static plCollisionSdkHandle gCollisionSdk = 0;
static plCollisionWorldHandle gCollisionWorldHandle = 0;

static uk gUserData = 0;

void Bullet2NearCallback(BroadphasePair& collisionPair, CollisionDispatcher& dispatcher, const DispatcherInfo& dispatchInfo)
{
	CollisionObject2* colObj0 = (CollisionObject2*)collisionPair.m_pProxy0->m_clientObject;
	CollisionObject2* colObj1 = (CollisionObject2*)collisionPair.m_pProxy1->m_clientObject;
	plCollisionObjectHandle obA = (plCollisionObjectHandle)colObj0;
	plCollisionObjectHandle obB = (plCollisionObjectHandle)colObj1;
	if (gTmpFilter)
	{
		gTmpFilter(gCollisionSdk, gCollisionWorldHandle, gUserData, obA, obB);
		gNearCallbackCount++;
	}
}

void Bullet2CollisionSdk::collideWorld(plCollisionWorldHandle worldHandle,
									   plNearCallback filter, uk userData)
{
	CollisionWorld* world = (CollisionWorld*)worldHandle;
	//chain the near-callback
	gTmpFilter = filter;
	gNearCallbackCount = 0;
	gUserData = userData;
	gCollisionSdk = (plCollisionSdkHandle)this;
	gCollisionWorldHandle = worldHandle;
	m_internalData->m_dispatcher->setNearCallback(Bullet2NearCallback);
	world->performDiscreteCollisionDetection();
	gTmpFilter = 0;
}

plCollisionSdkHandle Bullet2CollisionSdk::createBullet2SdkHandle()
{
	return (plCollisionSdkHandle) new Bullet2CollisionSdk;
}
