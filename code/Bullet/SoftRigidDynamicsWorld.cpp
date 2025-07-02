#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>
#include <drx3D/Maths/Linear/Quickprof.h>

//softbody & helpers
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/DefaultSoftBodySolver.h>
#include <drx3D/Maths/Linear/Serializer.h>

SoftRigidDynamicsWorld::SoftRigidDynamicsWorld(
	Dispatcher* dispatcher,
	BroadphaseInterface* pairCache,
	ConstraintSolver* constraintSolver,
	CollisionConfiguration* collisionConfiguration,
	SoftBodySolver* softBodySolver) : DiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration),
										m_softBodySolver(softBodySolver),
										m_ownsSolver(false)
{
	if (!m_softBodySolver)
	{
		uk ptr = AlignedAlloc(sizeof(DefaultSoftBodySolver), 16);
		m_softBodySolver = new (ptr) DefaultSoftBodySolver();
		m_ownsSolver = true;
	}

	m_drawFlags = fDrawFlags::Std;
	m_drawNodeTree = true;
	m_drawFaceTree = false;
	m_drawClusterTree = false;
	m_sbi.m_broadphase = pairCache;
	m_sbi.m_dispatcher = dispatcher;
	m_sbi.m_sparsesdf.Initialize();
	m_sbi.m_sparsesdf.Reset();

	m_sbi.air_density = (Scalar)1.2;
	m_sbi.water_density = 0;
	m_sbi.water_offset = 0;
	m_sbi.water_normal = Vec3(0, 0, 0);
	m_sbi.m_gravity.setVal(0, -10, 0);

	m_sbi.m_sparsesdf.Initialize();
}

SoftRigidDynamicsWorld::~SoftRigidDynamicsWorld()
{
	if (m_ownsSolver)
	{
		m_softBodySolver->~SoftBodySolver();
		AlignedFree(m_softBodySolver);
	}
}

void SoftRigidDynamicsWorld::predictUnconstraintMotion(Scalar timeStep)
{
	DiscreteDynamicsWorld::predictUnconstraintMotion(timeStep);
	{
		DRX3D_PROFILE("predictUnconstraintMotionSoftBody");
		m_softBodySolver->predictMotion(float(timeStep));
	}
}

void SoftRigidDynamicsWorld::internalSingleStepSimulation(Scalar timeStep)
{
	// Let the solver grab the soft bodies and if necessary optimize for it
	m_softBodySolver->optimize(getSoftBodyArray());

	if (!m_softBodySolver->checkInitialized())
	{
		Assert("Solver initialization failed\n");
	}

	DiscreteDynamicsWorld::internalSingleStepSimulation(timeStep);

	///solve soft bodies constraints
	solveSoftBodiesConstraints(timeStep);

	//self collisions
	for (i32 i = 0; i < m_softBodies.size(); i++)
	{
		SoftBody* psb = (SoftBody*)m_softBodies[i];
		psb->defaultCollisionHandler(psb);
	}

	///update soft bodies
	m_softBodySolver->updateSoftBodies();

	// End solver-wise simulation step
	// ///////////////////////////////
}

void SoftRigidDynamicsWorld::solveSoftBodiesConstraints(Scalar timeStep)
{
	DRX3D_PROFILE("solveSoftConstraints");

	if (m_softBodies.size())
	{
		SoftBody::solveClusters(m_softBodies);
	}

	// Solve constraints solver-wise
	m_softBodySolver->solveConstraints(timeStep * m_softBodySolver->getTimeScale());
}

void SoftRigidDynamicsWorld::addSoftBody(SoftBody* body, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	m_softBodies.push_back(body);

	// Set the soft body solver that will deal with this body
	// to be the world's solver
	body->setSoftBodySolver(m_softBodySolver);

	CollisionWorld::addCollisionObject(body,
										 collisionFilterGroup,
										 collisionFilterMask);
}

void SoftRigidDynamicsWorld::removeSoftBody(SoftBody* body)
{
	m_softBodies.remove(body);

	CollisionWorld::removeCollisionObject(body);
}

void SoftRigidDynamicsWorld::removeCollisionObject(CollisionObject2* collisionObject)
{
	SoftBody* body = SoftBody::upcast(collisionObject);
	if (body)
		removeSoftBody(body);
	else
		DiscreteDynamicsWorld::removeCollisionObject(collisionObject);
}

void SoftRigidDynamicsWorld::debugDrawWorld()
{
	DiscreteDynamicsWorld::debugDrawWorld();

	if (getDebugDrawer())
	{
		i32 i;
		for (i = 0; i < this->m_softBodies.size(); i++)
		{
			SoftBody* psb = (SoftBody*)this->m_softBodies[i];
			if (getDebugDrawer() && (getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
			{
				SoftBodyHelpers::DrawFrame(psb, m_debugDrawer);
				SoftBodyHelpers::Draw(psb, m_debugDrawer, m_drawFlags);
			}

			if (m_debugDrawer && (m_debugDrawer->getDebugMode() & IDebugDraw::DBG_DrawAabb))
			{
				if (m_drawNodeTree) SoftBodyHelpers::DrawNodeTree(psb, m_debugDrawer);
				if (m_drawFaceTree) SoftBodyHelpers::DrawFaceTree(psb, m_debugDrawer);
				if (m_drawClusterTree) SoftBodyHelpers::DrawClusterTree(psb, m_debugDrawer);
			}
		}
	}
}

struct SoftSingleRayCallback : public BroadphaseRayCallback
{
	Vec3 m_rayFromWorld;
	Vec3 m_rayToWorld;
	Transform2 m_rayFromTrans;
	Transform2 m_rayToTrans;
	Vec3 m_hitNormal;

	const SoftRigidDynamicsWorld* m_world;
	CollisionWorld::RayResultCallback& m_resultCallback;

	SoftSingleRayCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld, const SoftRigidDynamicsWorld* world, CollisionWorld::RayResultCallback& resultCallback)
		: m_rayFromWorld(rayFromWorld),
		  m_rayToWorld(rayToWorld),
		  m_world(world),
		  m_resultCallback(resultCallback)
	{
		m_rayFromTrans.setIdentity();
		m_rayFromTrans.setOrigin(m_rayFromWorld);
		m_rayToTrans.setIdentity();
		m_rayToTrans.setOrigin(m_rayToWorld);

		Vec3 rayDir = (rayToWorld - rayFromWorld);

		rayDir.normalize();
		///what about division by zero? --> just set rayDirection[i] to INF/1e30
		m_rayDirectionInverse[0] = rayDir[0] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[0];
		m_rayDirectionInverse[1] = rayDir[1] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[1];
		m_rayDirectionInverse[2] = rayDir[2] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[2];
		m_signs[0] = m_rayDirectionInverse[0] < 0.0;
		m_signs[1] = m_rayDirectionInverse[1] < 0.0;
		m_signs[2] = m_rayDirectionInverse[2] < 0.0;

		m_lambda_max = rayDir.dot(m_rayToWorld - m_rayFromWorld);
	}

	virtual bool process(const BroadphaseProxy* proxy)
	{
		///terminate further ray tests, once the closestHitFraction reached zero
		if (m_resultCallback.m_closestHitFraction == Scalar(0.f))
			return false;

		CollisionObject2* collisionObject = (CollisionObject2*)proxy->m_clientObject;

		//only perform raycast if filterMask matches
		if (m_resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			//RigidcollisionObject* collisionObject = ctrl->GetRigidcollisionObject();
			//Vec3 collisionObjectAabbMin,collisionObjectAabbMax;
#if 0
#ifdef RECALCULATE_AABB
			Vec3 collisionObjectAabbMin,collisionObjectAabbMax;
			collisionObject->getCollisionShape()->getAabb(collisionObject->getWorldTransform(),collisionObjectAabbMin,collisionObjectAabbMax);
#else
			//getBroadphase()->getAabb(collisionObject->getBroadphaseHandle(),collisionObjectAabbMin,collisionObjectAabbMax);
			const Vec3& collisionObjectAabbMin = collisionObject->getBroadphaseHandle()->m_aabbMin;
			const Vec3& collisionObjectAabbMax = collisionObject->getBroadphaseHandle()->m_aabbMax;
#endif
#endif
			//Scalar hitLambda = m_resultCallback.m_closestHitFraction;
			//culling already done by broadphase
			//if (RayAabb(m_rayFromWorld,m_rayToWorld,collisionObjectAabbMin,collisionObjectAabbMax,hitLambda,m_hitNormal))
			{
				m_world->rayTestSingle(m_rayFromTrans, m_rayToTrans,
									   collisionObject,
									   collisionObject->getCollisionShape(),
									   collisionObject->getWorldTransform(),
									   m_resultCallback);
			}
		}
		return true;
	}
};

void SoftRigidDynamicsWorld::rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, RayResultCallback& resultCallback) const
{
	DRX3D_PROFILE("rayTest");
	/// use the broadphase to accelerate the search for objects, based on their aabb
	/// and for each object with ray-aabb overlap, perform an exact ray test
	SoftSingleRayCallback rayCB(rayFromWorld, rayToWorld, this, resultCallback);

#ifndef USE_BRUTEFORCE_RAYBROADPHASE
	m_broadphasePairCache->rayTest(rayFromWorld, rayToWorld, rayCB);
#else
	for (i32 i = 0; i < this->getNumCollisionObjects(); i++)
	{
		rayCB.process(m_collisionObjects[i]->getBroadphaseHandle());
	}
#endif  //USE_BRUTEFORCE_RAYBROADPHASE
}

void SoftRigidDynamicsWorld::rayTestSingle(const Transform2& rayFromTrans, const Transform2& rayToTrans,
											 CollisionObject2* collisionObject,
											 const CollisionShape* collisionShape,
											 const Transform2& colObjWorldTransform,
											 RayResultCallback& resultCallback)
{
	if (collisionShape->isSoftBody())
	{
		SoftBody* softBody = SoftBody::upcast(collisionObject);
		if (softBody)
		{
			SoftBody::sRayCast softResult;
			if (softBody->rayTest(rayFromTrans.getOrigin(), rayToTrans.getOrigin(), softResult))
			{
				if (softResult.fraction <= resultCallback.m_closestHitFraction)
				{
					CollisionWorld::LocalShapeInfo shapeInfo;
					shapeInfo.m_shapePart = 0;
					shapeInfo.m_triangleIndex = softResult.index;
					// get the normal
					Vec3 rayDir = rayToTrans.getOrigin() - rayFromTrans.getOrigin();
					Vec3 normal = -rayDir;
					normal.normalize();

					if (softResult.feature == SoftBody::eFeature::Face)
					{
						normal = softBody->m_faces[softResult.index].m_normal;
						if (normal.dot(rayDir) > 0)
						{
							// normal always point toward origin of the ray
							normal = -normal;
						}
					}

					CollisionWorld::LocalRayResult rayResult(collisionObject,
															   &shapeInfo,
															   normal,
															   softResult.fraction);
					bool normalInWorldSpace = true;
					resultCallback.addSingleResult(rayResult, normalInWorldSpace);
				}
			}
		}
	}
	else
	{
		CollisionWorld::rayTestSingle(rayFromTrans, rayToTrans, collisionObject, collisionShape, colObjWorldTransform, resultCallback);
	}
}

void SoftRigidDynamicsWorld::serializeSoftBodies(Serializer* serializer)
{
	i32 i;
	//serialize all collision objects
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		if (colObj->getInternalType() & CollisionObject2::CO_SOFT_BODY)
		{
			i32 len = colObj->calculateSerializeBufferSize();
			Chunk* chunk = serializer->allocate(len, 1);
			tukk structType = colObj->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_SOFTBODY_CODE, colObj);
		}
	}
}

void SoftRigidDynamicsWorld::serialize(Serializer* serializer)
{
	serializer->startSerialization();

	serializeDynamicsWorldInfo(serializer);

	serializeSoftBodies(serializer);

	serializeRigidBodies(serializer);

	serializeCollisionObjects(serializer);

	serializer->finishSerialization();
}
