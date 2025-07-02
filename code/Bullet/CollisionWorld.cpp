#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>                 //for raycasting
#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>        //for raycasting
#include <drx3D/Physics/Collision/Shapes/ScaledBvhTriangleMeshShape.h>  //for raycasting
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>     //for raycasting
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/ContinuousConvexCollision.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

//#define DISABLE_DBVT_COMPOUNDSHAPE_RAYCAST_ACCELERATION

//#define USE_BRUTEFORCE_RAYBROADPHASE 1
//RECALCULATE_AABB is slower, but benefit is that you don't need to call 'stepSimulation'  or 'updateAabbs' before using a rayTest
//#define RECALCULATE_AABB_RAYCAST 1

//When the user doesn't provide dispatcher or broadphase, create basic versions (and delete them in destructor)
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionConfiguration.h>

///for debug drawing

//for debug rendering
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/ConeShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>
#include <drx3D/Physics/Collision/Shapes/TriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>

CollisionWorld::CollisionWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, CollisionConfiguration* collisionConfiguration)
	: m_dispatcher1(dispatcher),
	  m_broadphasePairCache(pairCache),
	  m_debugDrawer(0),
	  m_forceUpdateAllAabbs(true)
{
}

CollisionWorld::~CollisionWorld()
{
	//clean up remaining objects
	i32 i;
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* collisionObject = m_collisionObjects[i];

		BroadphaseProxy* bp = collisionObject->getBroadphaseHandle();
		if (bp)
		{
			//
			// only clear the cached algorithms
			//
			getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(bp, m_dispatcher1);
			getBroadphase()->destroyProxy(bp, m_dispatcher1);
			collisionObject->setBroadphaseHandle(0);
		}
	}
}

void CollisionWorld::refreshBroadphaseProxy(CollisionObject2* collisionObject)
{
	if (collisionObject->getBroadphaseHandle())
	{
		i32 collisionFilterGroup = collisionObject->getBroadphaseHandle()->m_collisionFilterGroup;
		i32 collisionFilterMask = collisionObject->getBroadphaseHandle()->m_collisionFilterMask;

		getBroadphase()->destroyProxy(collisionObject->getBroadphaseHandle(), getDispatcher());

		//calculate new AABB
		Transform2 trans = collisionObject->getWorldTransform();

		Vec3 minAabb;
		Vec3 maxAabb;
		collisionObject->getCollisionShape()->getAabb(trans, minAabb, maxAabb);

		i32 type = collisionObject->getCollisionShape()->getShapeType();
		collisionObject->setBroadphaseHandle(getBroadphase()->createProxy(
			minAabb,
			maxAabb,
			type,
			collisionObject,
			collisionFilterGroup,
			collisionFilterMask,
			m_dispatcher1));
	}
}

void CollisionWorld::addCollisionObject(CollisionObject2* collisionObject, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	Assert(collisionObject);

	//check that the object isn't already added
	Assert(m_collisionObjects.findLinearSearch(collisionObject) == m_collisionObjects.size());
	Assert(collisionObject->getWorldArrayIndex() == -1);  // do not add the same object to more than one collision world

	collisionObject->setWorldArrayIndex(m_collisionObjects.size());
	m_collisionObjects.push_back(collisionObject);

	//calculate new AABB
	Transform2 trans = collisionObject->getWorldTransform();

	Vec3 minAabb;
	Vec3 maxAabb;
	collisionObject->getCollisionShape()->getAabb(trans, minAabb, maxAabb);

	i32 type = collisionObject->getCollisionShape()->getShapeType();
	collisionObject->setBroadphaseHandle(getBroadphase()->createProxy(
		minAabb,
		maxAabb,
		type,
		collisionObject,
		collisionFilterGroup,
		collisionFilterMask,
		m_dispatcher1));
}

void CollisionWorld::updateSingleAabb(CollisionObject2* colObj)
{
	Vec3 minAabb, maxAabb;
	colObj->getCollisionShape()->getAabb(colObj->getWorldTransform(), minAabb, maxAabb);
	//need to increase the aabb for contact thresholds
	Vec3 contactThreshold(gContactBreakingThreshold, gContactBreakingThreshold, gContactBreakingThreshold);
	minAabb -= contactThreshold;
	maxAabb += contactThreshold;

	if (getDispatchInfo().m_useContinuous && colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY && !colObj->isStaticOrKinematicObject())
	{
		Vec3 minAabb2, maxAabb2;
		colObj->getCollisionShape()->getAabb(colObj->getInterpolationWorldTransform(), minAabb2, maxAabb2);
		minAabb2 -= contactThreshold;
		maxAabb2 += contactThreshold;
		minAabb.setMin(minAabb2);
		maxAabb.setMax(maxAabb2);
	}

	BroadphaseInterface* bp = (BroadphaseInterface*)m_broadphasePairCache;

	//moving objects should be moderately sized, probably something wrong if not
	if (colObj->isStaticObject() || ((maxAabb - minAabb).length2() < Scalar(1e12)))
	{
		bp->setAabb(colObj->getBroadphaseHandle(), minAabb, maxAabb, m_dispatcher1);
	}
	else
	{
		//something went wrong, investigate
		//this assert is unwanted in 3D modelers (danger of loosing work)
		colObj->setActivationState(DISABLE_SIMULATION);

		static bool reportMe = true;
		if (reportMe && m_debugDrawer)
		{
			reportMe = false;
			m_debugDrawer->reportErrorWarning("Overflow in AABB, object removed from simulation");
			m_debugDrawer->reportErrorWarning("If you can reproduce this, please email bugs@continuousphysics.com\n");
			m_debugDrawer->reportErrorWarning("Please include above information, your Platform, version of OS.\n");
			m_debugDrawer->reportErrorWarning("Thanks.\n");
		}
	}
}

void CollisionWorld::updateAabbs()
{
	DRX3D_PROFILE("updateAabbs");

	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		Assert(colObj->getWorldArrayIndex() == i);

		//only update aabb of active objects
		if (m_forceUpdateAllAabbs || colObj->isActive())
		{
			updateSingleAabb(colObj);
		}
	}
}

void CollisionWorld::computeOverlappingPairs()
{
	DRX3D_PROFILE("calculateOverlappingPairs");
	m_broadphasePairCache->calculateOverlappingPairs(m_dispatcher1);
}

void CollisionWorld::performDiscreteCollisionDetection()
{
	DRX3D_PROFILE("performDiscreteCollisionDetection");

	DispatcherInfo& dispatchInfo = getDispatchInfo();

	updateAabbs();

	computeOverlappingPairs();

	Dispatcher* dispatcher = getDispatcher();
	{
		DRX3D_PROFILE("dispatchAllCollisionPairs");
		if (dispatcher)
			dispatcher->dispatchAllCollisionPairs(m_broadphasePairCache->getOverlappingPairCache(), dispatchInfo, m_dispatcher1);
	}
}

void CollisionWorld::removeCollisionObject(CollisionObject2* collisionObject)
{
	//bool removeFromBroadphase = false;

	{
		BroadphaseProxy* bp = collisionObject->getBroadphaseHandle();
		if (bp)
		{
			//
			// only clear the cached algorithms
			//
			getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(bp, m_dispatcher1);
			getBroadphase()->destroyProxy(bp, m_dispatcher1);
			collisionObject->setBroadphaseHandle(0);
		}
	}

	i32 iObj = collisionObject->getWorldArrayIndex();
	//    Assert(iObj >= 0 && iObj < m_collisionObjects.size()); // trying to remove an object that was never added or already removed previously?
	if (iObj >= 0 && iObj < m_collisionObjects.size())
	{
		Assert(collisionObject == m_collisionObjects[iObj]);
		m_collisionObjects.swap(iObj, m_collisionObjects.size() - 1);
		m_collisionObjects.pop_back();
		if (iObj < m_collisionObjects.size())
		{
			m_collisionObjects[iObj]->setWorldArrayIndex(iObj);
		}
	}
	else
	{
		// slow linear search
		//swapremove
		m_collisionObjects.remove(collisionObject);
	}
	collisionObject->setWorldArrayIndex(-1);
}

void CollisionWorld::rayTestSingle(const Transform2& rayFromTrans, const Transform2& rayToTrans,
									 CollisionObject2* collisionObject,
									 const CollisionShape* collisionShape,
									 const Transform2& colObjWorldTransform,
									 RayResultCallback& resultCallback)
{
	CollisionObject2Wrapper colObWrap(0, collisionShape, collisionObject, colObjWorldTransform, -1, -1);
	CollisionWorld::rayTestSingleInternal(rayFromTrans, rayToTrans, &colObWrap, resultCallback);
}

void CollisionWorld::rayTestSingleInternal(const Transform2& rayFromTrans, const Transform2& rayToTrans,
											 const CollisionObject2Wrapper* collisionObjectWrap,
											 RayResultCallback& resultCallback)
{
	SphereShape pointShape(Scalar(0.0));
	pointShape.setMargin(0.f);
	const ConvexShape* castShape = &pointShape;
	const CollisionShape* collisionShape = collisionObjectWrap->getCollisionShape();
	const Transform2& colObjWorldTransform = collisionObjectWrap->getWorldTransform();

	if (collisionShape->isConvex())
	{
		//		DRX3D_PROFILE("rayTestConvex");
		ConvexCast::CastResult castResult;
		castResult.m_fraction = resultCallback.m_closestHitFraction;

		ConvexShape* convexShape = (ConvexShape*)collisionShape;
		VoronoiSimplexSolver simplexSolver;
		SubsimplexConvexCast subSimplexConvexCaster(castShape, convexShape, &simplexSolver);

		GjkConvexCast gjkConvexCaster(castShape, convexShape, &simplexSolver);

		//ContinuousConvexCollision convexCaster(castShape,convexShape,&simplexSolver,0);

		ConvexCast* convexCasterPtr = 0;
		//use kF_UseSubSimplexConvexCastRaytest by default
		if (resultCallback.m_flags & TriangleRaycastCallback::kF_UseGjkConvexCastRaytest)
			convexCasterPtr = &gjkConvexCaster;
		else
			convexCasterPtr = &subSimplexConvexCaster;

		ConvexCast& convexCaster = *convexCasterPtr;

		if (convexCaster.calcTimeOfImpact(rayFromTrans, rayToTrans, colObjWorldTransform, colObjWorldTransform, castResult))
		{
			//add hit
			if (castResult.m_normal.length2() > Scalar(0.0001))
			{
				if (castResult.m_fraction < resultCallback.m_closestHitFraction)
				{
					//todo: figure out what this is about. When is rayFromTest.getBasis() not identity?
#ifdef USE_SUBSIMPLEX_CONVEX_CAST
					//rotate normal into worldspace
					castResult.m_normal = rayFromTrans.getBasis() * castResult.m_normal;
#endif  //USE_SUBSIMPLEX_CONVEX_CAST

					castResult.m_normal.normalize();
					CollisionWorld::LocalRayResult localRayResult(
						collisionObjectWrap->getCollisionObject(),
						0,
						castResult.m_normal,
						castResult.m_fraction);

					bool normalInWorldSpace = true;
					resultCallback.addSingleResult(localRayResult, normalInWorldSpace);
				}
			}
		}
	}
	else
	{
		if (collisionShape->isConcave())
		{
			//ConvexCast::CastResult
			struct BridgeTriangleRaycastCallback : public TriangleRaycastCallback
			{
				CollisionWorld::RayResultCallback* m_resultCallback;
				const CollisionObject2* m_collisionObject;
				const ConcaveShape* m_triangleMesh;

				Transform2 m_colObjWorldTransform;

				BridgeTriangleRaycastCallback(const Vec3& from, const Vec3& to,
											  CollisionWorld::RayResultCallback* resultCallback, const CollisionObject2* collisionObject, const ConcaveShape* triangleMesh, const Transform2& colObjWorldTransform) :  //@BP Mod
																																																							TriangleRaycastCallback(from, to, resultCallback->m_flags),
																																																							m_resultCallback(resultCallback),
																																																							m_collisionObject(collisionObject),
																																																							m_triangleMesh(triangleMesh),
																																																							m_colObjWorldTransform(colObjWorldTransform)
				{
				}

				virtual Scalar reportHit(const Vec3& hitNormalLocal, Scalar hitFraction, i32 partId, i32 triangleIndex)
				{
					CollisionWorld::LocalShapeInfo shapeInfo;
					shapeInfo.m_shapePart = partId;
					shapeInfo.m_triangleIndex = triangleIndex;

					Vec3 hitNormalWorld = m_colObjWorldTransform.getBasis() * hitNormalLocal;

					CollisionWorld::LocalRayResult rayResult(m_collisionObject,
															   &shapeInfo,
															   hitNormalWorld,
															   hitFraction);

					bool normalInWorldSpace = true;
					return m_resultCallback->addSingleResult(rayResult, normalInWorldSpace);
				}
			};

			Transform2 worldTocollisionObject = colObjWorldTransform.inverse();
			Vec3 rayFromLocal = worldTocollisionObject * rayFromTrans.getOrigin();
			Vec3 rayToLocal = worldTocollisionObject * rayToTrans.getOrigin();

			//			DRX3D_PROFILE("rayTestConcave");
			if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				///optimized version for BvhTriangleMeshShape
				BvhTriangleMeshShape* triangleMesh = (BvhTriangleMeshShape*)collisionShape;

				BridgeTriangleRaycastCallback rcb(rayFromLocal, rayToLocal, &resultCallback, collisionObjectWrap->getCollisionObject(), triangleMesh, colObjWorldTransform);
				rcb.m_hitFraction = resultCallback.m_closestHitFraction;
				triangleMesh->performRaycast(&rcb, rayFromLocal, rayToLocal);
			}
			else if (collisionShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				///optimized version for ScaledBvhTriangleMeshShape
				ScaledBvhTriangleMeshShape* scaledTriangleMesh = (ScaledBvhTriangleMeshShape*)collisionShape;
				BvhTriangleMeshShape* triangleMesh = (BvhTriangleMeshShape*)scaledTriangleMesh->getChildShape();

				//scale the ray positions
				Vec3 scale = scaledTriangleMesh->getLocalScaling();
				Vec3 rayFromLocalScaled = rayFromLocal / scale;
				Vec3 rayToLocalScaled = rayToLocal / scale;

				//perform raycast in the underlying BvhTriangleMeshShape
				BridgeTriangleRaycastCallback rcb(rayFromLocalScaled, rayToLocalScaled, &resultCallback, collisionObjectWrap->getCollisionObject(), triangleMesh, colObjWorldTransform);
				rcb.m_hitFraction = resultCallback.m_closestHitFraction;
				triangleMesh->performRaycast(&rcb, rayFromLocalScaled, rayToLocalScaled);
			}
			else if (((resultCallback.m_flags&TriangleRaycastCallback::kF_DisableHeightfieldAccelerator)==0) 
				&& collisionShape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE 
				)
			{
				///optimized version for HeightfieldTerrainShape
				HeightfieldTerrainShape* heightField = (HeightfieldTerrainShape*)collisionShape;
				Transform2 worldTocollisionObject = colObjWorldTransform.inverse();
				Vec3 rayFromLocal = worldTocollisionObject * rayFromTrans.getOrigin();
				Vec3 rayToLocal = worldTocollisionObject * rayToTrans.getOrigin();

				BridgeTriangleRaycastCallback rcb(rayFromLocal, rayToLocal, &resultCallback, collisionObjectWrap->getCollisionObject(), heightField, colObjWorldTransform);
				rcb.m_hitFraction = resultCallback.m_closestHitFraction;
				heightField->performRaycast(&rcb, rayFromLocal, rayToLocal);
			}
			else
			{
				//generic (slower) case
				ConcaveShape* concaveShape = (ConcaveShape*)collisionShape;

				Transform2 worldTocollisionObject = colObjWorldTransform.inverse();

				Vec3 rayFromLocal = worldTocollisionObject * rayFromTrans.getOrigin();
				Vec3 rayToLocal = worldTocollisionObject * rayToTrans.getOrigin();

				//ConvexCast::CastResult

				struct BridgeTriangleRaycastCallback : public TriangleRaycastCallback
				{
					CollisionWorld::RayResultCallback* m_resultCallback;
					const CollisionObject2* m_collisionObject;
					ConcaveShape* m_triangleMesh;

					Transform2 m_colObjWorldTransform;

					BridgeTriangleRaycastCallback(const Vec3& from, const Vec3& to,
												  CollisionWorld::RayResultCallback* resultCallback, const CollisionObject2* collisionObject, ConcaveShape* triangleMesh, const Transform2& colObjWorldTransform) :  //@BP Mod
																																																						  TriangleRaycastCallback(from, to, resultCallback->m_flags),
																																																						  m_resultCallback(resultCallback),
																																																						  m_collisionObject(collisionObject),
																																																						  m_triangleMesh(triangleMesh),
																																																						  m_colObjWorldTransform(colObjWorldTransform)
					{
					}

					virtual Scalar reportHit(const Vec3& hitNormalLocal, Scalar hitFraction, i32 partId, i32 triangleIndex)
					{
						CollisionWorld::LocalShapeInfo shapeInfo;
						shapeInfo.m_shapePart = partId;
						shapeInfo.m_triangleIndex = triangleIndex;

						Vec3 hitNormalWorld = m_colObjWorldTransform.getBasis() * hitNormalLocal;

						CollisionWorld::LocalRayResult rayResult(m_collisionObject,
																   &shapeInfo,
																   hitNormalWorld,
																   hitFraction);

						bool normalInWorldSpace = true;
						return m_resultCallback->addSingleResult(rayResult, normalInWorldSpace);
					}
				};

				BridgeTriangleRaycastCallback rcb(rayFromLocal, rayToLocal, &resultCallback, collisionObjectWrap->getCollisionObject(), concaveShape, colObjWorldTransform);
				rcb.m_hitFraction = resultCallback.m_closestHitFraction;

				Vec3 rayAabbMinLocal = rayFromLocal;
				rayAabbMinLocal.setMin(rayToLocal);
				Vec3 rayAabbMaxLocal = rayFromLocal;
				rayAabbMaxLocal.setMax(rayToLocal);

				concaveShape->processAllTriangles(&rcb, rayAabbMinLocal, rayAabbMaxLocal);
			}
		}
		else
		{
			//			DRX3D_PROFILE("rayTestCompound");
			if (collisionShape->isCompound())
			{
				struct LocalInfoAdder2 : public RayResultCallback
				{
					RayResultCallback* m_userCallback;
					i32 m_i;

					LocalInfoAdder2(i32 i, RayResultCallback* user)
						: m_userCallback(user), m_i(i)
					{
						m_closestHitFraction = m_userCallback->m_closestHitFraction;
						m_flags = m_userCallback->m_flags;
					}
					virtual bool needsCollision(BroadphaseProxy* p) const
					{
						return m_userCallback->needsCollision(p);
					}

					virtual Scalar addSingleResult(CollisionWorld::LocalRayResult& r, bool b)
					{
						CollisionWorld::LocalShapeInfo shapeInfo;
						shapeInfo.m_shapePart = -1;
						shapeInfo.m_triangleIndex = m_i;
						if (r.m_localShapeInfo == NULL)
							r.m_localShapeInfo = &shapeInfo;

						const Scalar result = m_userCallback->addSingleResult(r, b);
						m_closestHitFraction = m_userCallback->m_closestHitFraction;
						return result;
					}
				};

				struct RayTester : Dbvt::ICollide
				{
					const CollisionObject2* m_collisionObject;
					const CompoundShape* m_compoundShape;
					const Transform2& m_colObjWorldTransform;
					const Transform2& m_rayFromTrans;
					const Transform2& m_rayToTrans;
					RayResultCallback& m_resultCallback;

					RayTester(const CollisionObject2* collisionObject,
							  const CompoundShape* compoundShape,
							  const Transform2& colObjWorldTransform,
							  const Transform2& rayFromTrans,
							  const Transform2& rayToTrans,
							  RayResultCallback& resultCallback) : m_collisionObject(collisionObject),
																   m_compoundShape(compoundShape),
																   m_colObjWorldTransform(colObjWorldTransform),
																   m_rayFromTrans(rayFromTrans),
																   m_rayToTrans(rayToTrans),
																   m_resultCallback(resultCallback)
					{
					}

					void ProcessLeaf(i32 i)
					{
						const CollisionShape* childCollisionShape = m_compoundShape->getChildShape(i);
						const Transform2& childTrans = m_compoundShape->getChildTransform(i);
						Transform2 childWorldTrans = m_colObjWorldTransform * childTrans;

						CollisionObject2Wrapper tmpOb(0, childCollisionShape, m_collisionObject, childWorldTrans, -1, i);
						// replace collision shape so that callback can determine the triangle

						LocalInfoAdder2 my_cb(i, &m_resultCallback);

						rayTestSingleInternal(
							m_rayFromTrans,
							m_rayToTrans,
							&tmpOb,
							my_cb);
					}

					void Process(const DbvtNode* leaf)
					{
						ProcessLeaf(leaf->dataAsInt);
					}
				};

				const CompoundShape* compoundShape = static_cast<const CompoundShape*>(collisionShape);
				const Dbvt* dbvt = compoundShape->getDynamicAabbTree();

				RayTester rayCB(
					collisionObjectWrap->getCollisionObject(),
					compoundShape,
					colObjWorldTransform,
					rayFromTrans,
					rayToTrans,
					resultCallback);
#ifndef DISABLE_DBVT_COMPOUNDSHAPE_RAYCAST_ACCELERATION
				if (dbvt)
				{
					Vec3 localRayFrom = colObjWorldTransform.inverseTimes(rayFromTrans).getOrigin();
					Vec3 localRayTo = colObjWorldTransform.inverseTimes(rayToTrans).getOrigin();
					Dbvt::rayTest(dbvt->m_root, localRayFrom, localRayTo, rayCB);
				}
				else
#endif  //DISABLE_DBVT_COMPOUNDSHAPE_RAYCAST_ACCELERATION
				{
					for (i32 i = 0, n = compoundShape->getNumChildShapes(); i < n; ++i)
					{
						rayCB.ProcessLeaf(i);
					}
				}
			}
		}
	}
}

void CollisionWorld::objectQuerySingle(const ConvexShape* castShape, const Transform2& convexFromTrans, const Transform2& convexToTrans,
										 CollisionObject2* collisionObject,
										 const CollisionShape* collisionShape,
										 const Transform2& colObjWorldTransform,
										 ConvexResultCallback& resultCallback, Scalar allowedPenetration)
{
	CollisionObject2Wrapper tmpOb(0, collisionShape, collisionObject, colObjWorldTransform, -1, -1);
	CollisionWorld::objectQuerySingleInternal(castShape, convexFromTrans, convexToTrans, &tmpOb, resultCallback, allowedPenetration);
}

void CollisionWorld::objectQuerySingleInternal(const ConvexShape* castShape, const Transform2& convexFromTrans, const Transform2& convexToTrans,
												 const CollisionObject2Wrapper* colObjWrap,
												 ConvexResultCallback& resultCallback, Scalar allowedPenetration)
{
	const CollisionShape* collisionShape = colObjWrap->getCollisionShape();
	const Transform2& colObjWorldTransform = colObjWrap->getWorldTransform();

	if (collisionShape->isConvex())
	{
		//DRX3D_PROFILE("convexSweepConvex");
		ConvexCast::CastResult castResult;
		castResult.m_allowedPenetration = allowedPenetration;
		castResult.m_fraction = resultCallback.m_closestHitFraction;  //Scalar(1.);//??

		ConvexShape* convexShape = (ConvexShape*)collisionShape;
		VoronoiSimplexSolver simplexSolver;
		GjkEpaPenetrationDepthSolver gjkEpaPenetrationSolver;

		ContinuousConvexCollision convexCaster1(castShape, convexShape, &simplexSolver, &gjkEpaPenetrationSolver);
		//GjkConvexCast convexCaster2(castShape,convexShape,&simplexSolver);
		//SubsimplexConvexCast convexCaster3(castShape,convexShape,&simplexSolver);

		ConvexCast* castPtr = &convexCaster1;

		if (castPtr->calcTimeOfImpact(convexFromTrans, convexToTrans, colObjWorldTransform, colObjWorldTransform, castResult))
		{
			//add hit
			if (castResult.m_normal.length2() > Scalar(0.0001))
			{
				if (castResult.m_fraction < resultCallback.m_closestHitFraction)
				{
					castResult.m_normal.normalize();
					CollisionWorld::LocalConvexResult localConvexResult(
						colObjWrap->getCollisionObject(),
						0,
						castResult.m_normal,
						castResult.m_hitPoint,
						castResult.m_fraction);

					bool normalInWorldSpace = true;
					resultCallback.addSingleResult(localConvexResult, normalInWorldSpace);
				}
			}
		}
	}
	else
	{
		if (collisionShape->isConcave())
		{
			if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				//DRX3D_PROFILE("convexSweepBvhTriangleMesh");
				BvhTriangleMeshShape* triangleMesh = (BvhTriangleMeshShape*)collisionShape;
				Transform2 worldTocollisionObject = colObjWorldTransform.inverse();
				Vec3 convexFromLocal = worldTocollisionObject * convexFromTrans.getOrigin();
				Vec3 convexToLocal = worldTocollisionObject * convexToTrans.getOrigin();
				// rotation of box in local mesh space = MeshRotation^-1 * ConvexToRotation
				Transform2 rotationXform = Transform2(worldTocollisionObject.getBasis() * convexToTrans.getBasis());

				//ConvexCast::CastResult
				struct BridgeTriangleConvexcastCallback : public TriangleConvexcastCallback
				{
					CollisionWorld::ConvexResultCallback* m_resultCallback;
					const CollisionObject2* m_collisionObject;
					TriangleMeshShape* m_triangleMesh;

					BridgeTriangleConvexcastCallback(const ConvexShape* castShape, const Transform2& from, const Transform2& to,
													 CollisionWorld::ConvexResultCallback* resultCallback, const CollisionObject2* collisionObject, TriangleMeshShape* triangleMesh, const Transform2& triangleToWorld) : TriangleConvexcastCallback(castShape, from, to, triangleToWorld, triangleMesh->getMargin()),
																																																								m_resultCallback(resultCallback),
																																																								m_collisionObject(collisionObject),
																																																								m_triangleMesh(triangleMesh)
					{
					}

					virtual Scalar reportHit(const Vec3& hitNormalLocal, const Vec3& hitPointLocal, Scalar hitFraction, i32 partId, i32 triangleIndex)
					{
						CollisionWorld::LocalShapeInfo shapeInfo;
						shapeInfo.m_shapePart = partId;
						shapeInfo.m_triangleIndex = triangleIndex;
						if (hitFraction <= m_resultCallback->m_closestHitFraction)
						{
							CollisionWorld::LocalConvexResult convexResult(m_collisionObject,
																			 &shapeInfo,
																			 hitNormalLocal,
																			 hitPointLocal,
																			 hitFraction);

							bool normalInWorldSpace = true;

							return m_resultCallback->addSingleResult(convexResult, normalInWorldSpace);
						}
						return hitFraction;
					}
				};

				BridgeTriangleConvexcastCallback tccb(castShape, convexFromTrans, convexToTrans, &resultCallback, colObjWrap->getCollisionObject(), triangleMesh, colObjWorldTransform);
				tccb.m_hitFraction = resultCallback.m_closestHitFraction;
				tccb.m_allowedPenetration = allowedPenetration;
				Vec3 boxMinLocal, boxMaxLocal;
				castShape->getAabb(rotationXform, boxMinLocal, boxMaxLocal);
				triangleMesh->performConvexcast(&tccb, convexFromLocal, convexToLocal, boxMinLocal, boxMaxLocal);
			}
			else
			{
				if (collisionShape->getShapeType() == STATIC_PLANE_PROXYTYPE)
				{
					ConvexCast::CastResult castResult;
					castResult.m_allowedPenetration = allowedPenetration;
					castResult.m_fraction = resultCallback.m_closestHitFraction;
					StaticPlaneShape* planeShape = (StaticPlaneShape*)collisionShape;
					ContinuousConvexCollision convexCaster1(castShape, planeShape);
					ConvexCast* castPtr = &convexCaster1;

					if (castPtr->calcTimeOfImpact(convexFromTrans, convexToTrans, colObjWorldTransform, colObjWorldTransform, castResult))
					{
						//add hit
						if (castResult.m_normal.length2() > Scalar(0.0001))
						{
							if (castResult.m_fraction < resultCallback.m_closestHitFraction)
							{
								castResult.m_normal.normalize();
								CollisionWorld::LocalConvexResult localConvexResult(
									colObjWrap->getCollisionObject(),
									0,
									castResult.m_normal,
									castResult.m_hitPoint,
									castResult.m_fraction);

								bool normalInWorldSpace = true;
								resultCallback.addSingleResult(localConvexResult, normalInWorldSpace);
							}
						}
					}
				}
				else
				{
					//DRX3D_PROFILE("convexSweepConcave");
					ConcaveShape* concaveShape = (ConcaveShape*)collisionShape;
					Transform2 worldTocollisionObject = colObjWorldTransform.inverse();
					Vec3 convexFromLocal = worldTocollisionObject * convexFromTrans.getOrigin();
					Vec3 convexToLocal = worldTocollisionObject * convexToTrans.getOrigin();
					// rotation of box in local mesh space = MeshRotation^-1 * ConvexToRotation
					Transform2 rotationXform = Transform2(worldTocollisionObject.getBasis() * convexToTrans.getBasis());

					//ConvexCast::CastResult
					struct BridgeTriangleConvexcastCallback : public TriangleConvexcastCallback
					{
						CollisionWorld::ConvexResultCallback* m_resultCallback;
						const CollisionObject2* m_collisionObject;
						ConcaveShape* m_triangleMesh;

						BridgeTriangleConvexcastCallback(const ConvexShape* castShape, const Transform2& from, const Transform2& to,
														 CollisionWorld::ConvexResultCallback* resultCallback, const CollisionObject2* collisionObject, ConcaveShape* triangleMesh, const Transform2& triangleToWorld) : TriangleConvexcastCallback(castShape, from, to, triangleToWorld, triangleMesh->getMargin()),
																																																							   m_resultCallback(resultCallback),
																																																							   m_collisionObject(collisionObject),
																																																							   m_triangleMesh(triangleMesh)
						{
						}

						virtual Scalar reportHit(const Vec3& hitNormalLocal, const Vec3& hitPointLocal, Scalar hitFraction, i32 partId, i32 triangleIndex)
						{
							CollisionWorld::LocalShapeInfo shapeInfo;
							shapeInfo.m_shapePart = partId;
							shapeInfo.m_triangleIndex = triangleIndex;
							if (hitFraction <= m_resultCallback->m_closestHitFraction)
							{
								CollisionWorld::LocalConvexResult convexResult(m_collisionObject,
																				 &shapeInfo,
																				 hitNormalLocal,
																				 hitPointLocal,
																				 hitFraction);

								bool normalInWorldSpace = true;

								return m_resultCallback->addSingleResult(convexResult, normalInWorldSpace);
							}
							return hitFraction;
						}
					};

					BridgeTriangleConvexcastCallback tccb(castShape, convexFromTrans, convexToTrans, &resultCallback, colObjWrap->getCollisionObject(), concaveShape, colObjWorldTransform);
					tccb.m_hitFraction = resultCallback.m_closestHitFraction;
					tccb.m_allowedPenetration = allowedPenetration;
					Vec3 boxMinLocal, boxMaxLocal;
					castShape->getAabb(rotationXform, boxMinLocal, boxMaxLocal);

					Vec3 rayAabbMinLocal = convexFromLocal;
					rayAabbMinLocal.setMin(convexToLocal);
					Vec3 rayAabbMaxLocal = convexFromLocal;
					rayAabbMaxLocal.setMax(convexToLocal);
					rayAabbMinLocal += boxMinLocal;
					rayAabbMaxLocal += boxMaxLocal;
					concaveShape->processAllTriangles(&tccb, rayAabbMinLocal, rayAabbMaxLocal);
				}
			}
		}
		else
		{
			if (collisionShape->isCompound())
			{
				struct CompoundLeafCallback : Dbvt::ICollide
				{
					CompoundLeafCallback(
						const CollisionObject2Wrapper* colObjWrap,
						const ConvexShape* castShape,
						const Transform2& convexFromTrans,
						const Transform2& convexToTrans,
						Scalar allowedPenetration,
						const CompoundShape* compoundShape,
						const Transform2& colObjWorldTransform,
						ConvexResultCallback& resultCallback)
						: m_colObjWrap(colObjWrap),
						  m_castShape(castShape),
						  m_convexFromTrans(convexFromTrans),
						  m_convexToTrans(convexToTrans),
						  m_allowedPenetration(allowedPenetration),
						  m_compoundShape(compoundShape),
						  m_colObjWorldTransform(colObjWorldTransform),
						  m_resultCallback(resultCallback)
					{
					}

					const CollisionObject2Wrapper* m_colObjWrap;
					const ConvexShape* m_castShape;
					const Transform2& m_convexFromTrans;
					const Transform2& m_convexToTrans;
					Scalar m_allowedPenetration;
					const CompoundShape* m_compoundShape;
					const Transform2& m_colObjWorldTransform;
					ConvexResultCallback& m_resultCallback;

				public:
					void ProcessChild(i32 index, const Transform2& childTrans, const CollisionShape* childCollisionShape)
					{
						Transform2 childWorldTrans = m_colObjWorldTransform * childTrans;

						struct LocalInfoAdder : public ConvexResultCallback
						{
							ConvexResultCallback* m_userCallback;
							i32 m_i;

							LocalInfoAdder(i32 i, ConvexResultCallback* user)
								: m_userCallback(user), m_i(i)
							{
								m_closestHitFraction = m_userCallback->m_closestHitFraction;
							}
							virtual bool needsCollision(BroadphaseProxy* p) const
							{
								return m_userCallback->needsCollision(p);
							}
							virtual Scalar addSingleResult(CollisionWorld::LocalConvexResult& r, bool b)
							{
								CollisionWorld::LocalShapeInfo shapeInfo;
								shapeInfo.m_shapePart = -1;
								shapeInfo.m_triangleIndex = m_i;
								if (r.m_localShapeInfo == NULL)
									r.m_localShapeInfo = &shapeInfo;
								const Scalar result = m_userCallback->addSingleResult(r, b);
								m_closestHitFraction = m_userCallback->m_closestHitFraction;
								return result;
							}
						};

						LocalInfoAdder my_cb(index, &m_resultCallback);

						CollisionObject2Wrapper tmpObj(m_colObjWrap, childCollisionShape, m_colObjWrap->getCollisionObject(), childWorldTrans, -1, index);

						objectQuerySingleInternal(m_castShape, m_convexFromTrans, m_convexToTrans, &tmpObj, my_cb, m_allowedPenetration);
					}

					void Process(const DbvtNode* leaf)
					{
						// Processing leaf node
						i32 index = leaf->dataAsInt;

						Transform2 childTrans = m_compoundShape->getChildTransform(index);
						const CollisionShape* childCollisionShape = m_compoundShape->getChildShape(index);

						ProcessChild(index, childTrans, childCollisionShape);
					}
				};

				DRX3D_PROFILE("convexSweepCompound");
				const CompoundShape* compoundShape = static_cast<const CompoundShape*>(collisionShape);

				Vec3 fromLocalAabbMin, fromLocalAabbMax;
				Vec3 toLocalAabbMin, toLocalAabbMax;

				castShape->getAabb(colObjWorldTransform.inverse() * convexFromTrans, fromLocalAabbMin, fromLocalAabbMax);
				castShape->getAabb(colObjWorldTransform.inverse() * convexToTrans, toLocalAabbMin, toLocalAabbMax);

				fromLocalAabbMin.setMin(toLocalAabbMin);
				fromLocalAabbMax.setMax(toLocalAabbMax);

				CompoundLeafCallback callback(colObjWrap, castShape, convexFromTrans, convexToTrans,
												allowedPenetration, compoundShape, colObjWorldTransform, resultCallback);

				const Dbvt* tree = compoundShape->getDynamicAabbTree();
				if (tree)
				{
					const ATTRIBUTE_ALIGNED16(DbvtVolume) bounds = DbvtVolume::FromMM(fromLocalAabbMin, fromLocalAabbMax);
					tree->collideTV(tree->m_root, bounds, callback);
				}
				else
				{
					i32 i;
					for (i = 0; i < compoundShape->getNumChildShapes(); i++)
					{
						const CollisionShape* childCollisionShape = compoundShape->getChildShape(i);
						Transform2 childTrans = compoundShape->getChildTransform(i);
						callback.ProcessChild(i, childTrans, childCollisionShape);
					}
				}
			}
		}
	}
}

struct SingleRayCallback : public BroadphaseRayCallback
{
	Vec3 m_rayFromWorld;
	Vec3 m_rayToWorld;
	Transform2 m_rayFromTrans;
	Transform2 m_rayToTrans;
	Vec3 m_hitNormal;

	const CollisionWorld* m_world;
	CollisionWorld::RayResultCallback& m_resultCallback;

	SingleRayCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld, const CollisionWorld* world, CollisionWorld::RayResultCallback& resultCallback)
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
		///what about division by zero? --> just set rayDirection[i] to INF/DRX3D_LARGE_FLOAT
		m_rayDirectionInverse[0] = rayDir[0] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[0];
		m_rayDirectionInverse[1] = rayDir[1] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[1];
		m_rayDirectionInverse[2] = rayDir[2] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[2];
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

void CollisionWorld::rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, RayResultCallback& resultCallback) const
{
	//DRX3D_PROFILE("rayTest");
	/// use the broadphase to accelerate the search for objects, based on their aabb
	/// and for each object with ray-aabb overlap, perform an exact ray test
	SingleRayCallback rayCB(rayFromWorld, rayToWorld, this, resultCallback);

#ifndef USE_BRUTEFORCE_RAYBROADPHASE
	m_broadphasePairCache->rayTest(rayFromWorld, rayToWorld, rayCB);
#else
	for (i32 i = 0; i < this->getNumCollisionObjects(); i++)
	{
		rayCB.process(m_collisionObjects[i]->getBroadphaseHandle());
	}
#endif  //USE_BRUTEFORCE_RAYBROADPHASE
}

struct SingleSweepCallback : public BroadphaseRayCallback
{
	Transform2 m_convexFromTrans;
	Transform2 m_convexToTrans;
	Vec3 m_hitNormal;
	const CollisionWorld* m_world;
	CollisionWorld::ConvexResultCallback& m_resultCallback;
	Scalar m_allowedCcdPenetration;
	const ConvexShape* m_castShape;

	SingleSweepCallback(const ConvexShape* castShape, const Transform2& convexFromTrans, const Transform2& convexToTrans, const CollisionWorld* world, CollisionWorld::ConvexResultCallback& resultCallback, Scalar allowedPenetration)
		: m_convexFromTrans(convexFromTrans),
		  m_convexToTrans(convexToTrans),
		  m_world(world),
		  m_resultCallback(resultCallback),
		  m_allowedCcdPenetration(allowedPenetration),
		  m_castShape(castShape)
	{
		Vec3 unnormalizedRayDir = (m_convexToTrans.getOrigin() - m_convexFromTrans.getOrigin());
		Vec3 rayDir = unnormalizedRayDir.fuzzyZero() ? Vec3(Scalar(0.0), Scalar(0.0), Scalar(0.0)) : unnormalizedRayDir.normalized();
		///what about division by zero? --> just set rayDirection[i] to INF/DRX3D_LARGE_FLOAT
		m_rayDirectionInverse[0] = rayDir[0] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[0];
		m_rayDirectionInverse[1] = rayDir[1] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[1];
		m_rayDirectionInverse[2] = rayDir[2] == Scalar(0.0) ? Scalar(DRX3D_LARGE_FLOAT) : Scalar(1.0) / rayDir[2];
		m_signs[0] = m_rayDirectionInverse[0] < 0.0;
		m_signs[1] = m_rayDirectionInverse[1] < 0.0;
		m_signs[2] = m_rayDirectionInverse[2] < 0.0;

		m_lambda_max = rayDir.dot(unnormalizedRayDir);
	}

	virtual bool process(const BroadphaseProxy* proxy)
	{
		///terminate further convex sweep tests, once the closestHitFraction reached zero
		if (m_resultCallback.m_closestHitFraction == Scalar(0.f))
			return false;

		CollisionObject2* collisionObject = (CollisionObject2*)proxy->m_clientObject;

		//only perform raycast if filterMask matches
		if (m_resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			//RigidcollisionObject* collisionObject = ctrl->GetRigidcollisionObject();
			m_world->objectQuerySingle(m_castShape, m_convexFromTrans, m_convexToTrans,
									   collisionObject,
									   collisionObject->getCollisionShape(),
									   collisionObject->getWorldTransform(),
									   m_resultCallback,
									   m_allowedCcdPenetration);
		}

		return true;
	}
};

void CollisionWorld::convexSweepTest(const ConvexShape* castShape, const Transform2& convexFromWorld, const Transform2& convexToWorld, ConvexResultCallback& resultCallback, Scalar allowedCcdPenetration) const
{
	DRX3D_PROFILE("convexSweepTest");
	/// use the broadphase to accelerate the search for objects, based on their aabb
	/// and for each object with ray-aabb overlap, perform an exact ray test
	/// unfortunately the implementation for rayTest and convexSweepTest duplicated, albeit practically identical

	Transform2 convexFromTrans, convexToTrans;
	convexFromTrans = convexFromWorld;
	convexToTrans = convexToWorld;
	Vec3 castShapeAabbMin, castShapeAabbMax;
	/* Compute AABB that encompasses angular movement */
	{
		Vec3 linVel, angVel;
		Transform2Util::calculateVelocity(convexFromTrans, convexToTrans, 1.0f, linVel, angVel);
		Vec3 zeroLinVel;
		zeroLinVel.setVal(0, 0, 0);
		Transform2 R;
		R.setIdentity();
		R.setRotation(convexFromTrans.getRotation());
		castShape->calculateTemporalAabb(R, zeroLinVel, angVel, 1.0f, castShapeAabbMin, castShapeAabbMax);
	}

#ifndef USE_BRUTEFORCE_RAYBROADPHASE

	SingleSweepCallback convexCB(castShape, convexFromWorld, convexToWorld, this, resultCallback, allowedCcdPenetration);

	m_broadphasePairCache->rayTest(convexFromTrans.getOrigin(), convexToTrans.getOrigin(), convexCB, castShapeAabbMin, castShapeAabbMax);

#else
	/// go over all objects, and if the ray intersects their aabb + cast shape aabb,
	// do a ray-shape query using convexCaster (CCD)
	i32 i;
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* collisionObject = m_collisionObjects[i];
		//only perform raycast if filterMask matches
		if (resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			//RigidcollisionObject* collisionObject = ctrl->GetRigidcollisionObject();
			Vec3 collisionObjectAabbMin, collisionObjectAabbMax;
			collisionObject->getCollisionShape()->getAabb(collisionObject->getWorldTransform(), collisionObjectAabbMin, collisionObjectAabbMax);
			AabbExpand(collisionObjectAabbMin, collisionObjectAabbMax, castShapeAabbMin, castShapeAabbMax);
			Scalar hitLambda = Scalar(1.);  //could use resultCallback.m_closestHitFraction, but needs testing
			Vec3 hitNormal;
			if (RayAabb(convexFromWorld.getOrigin(), convexToWorld.getOrigin(), collisionObjectAabbMin, collisionObjectAabbMax, hitLambda, hitNormal))
			{
				objectQuerySingle(castShape, convexFromTrans, convexToTrans,
								  collisionObject,
								  collisionObject->getCollisionShape(),
								  collisionObject->getWorldTransform(),
								  resultCallback,
								  allowedCcdPenetration);
			}
		}
	}
#endif  //USE_BRUTEFORCE_RAYBROADPHASE
}

struct BridgedManifoldResult : public ManifoldResult
{
	CollisionWorld::ContactResultCallback& m_resultCallback;

	BridgedManifoldResult(const CollisionObject2Wrapper* obj0Wrap, const CollisionObject2Wrapper* obj1Wrap, CollisionWorld::ContactResultCallback& resultCallback)
		: ManifoldResult(obj0Wrap, obj1Wrap),
		  m_resultCallback(resultCallback)
	{
	}

	virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
	{
		bool isSwapped = m_manifoldPtr->getBody0() != m_body0Wrap->getCollisionObject();
		Vec3 pointA = pointInWorld + normalOnBInWorld * depth;
		Vec3 localA;
		Vec3 localB;
		if (isSwapped)
		{
			localA = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
			localB = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
		}
		else
		{
			localA = m_body0Wrap->getCollisionObject()->getWorldTransform().invXform(pointA);
			localB = m_body1Wrap->getCollisionObject()->getWorldTransform().invXform(pointInWorld);
		}

		ManifoldPoint newPt(localA, localB, normalOnBInWorld, depth);
		newPt.m_positionWorldOnA = pointA;
		newPt.m_positionWorldOnB = pointInWorld;

		//BP mod, store contact triangles.
		if (isSwapped)
		{
			newPt.m_partId0 = m_partId1;
			newPt.m_partId1 = m_partId0;
			newPt.m_index0 = m_index1;
			newPt.m_index1 = m_index0;
		}
		else
		{
			newPt.m_partId0 = m_partId0;
			newPt.m_partId1 = m_partId1;
			newPt.m_index0 = m_index0;
			newPt.m_index1 = m_index1;
		}

		//experimental feature info, for per-triangle material etc.
		const CollisionObject2Wrapper* obj0Wrap = isSwapped ? m_body1Wrap : m_body0Wrap;
		const CollisionObject2Wrapper* obj1Wrap = isSwapped ? m_body0Wrap : m_body1Wrap;
		m_resultCallback.addSingleResult(newPt, obj0Wrap, newPt.m_partId0, newPt.m_index0, obj1Wrap, newPt.m_partId1, newPt.m_index1);
	}
};

struct SingleContactCallback : public BroadphaseAabbCallback
{
	CollisionObject2* m_collisionObject;
	CollisionWorld* m_world;
	CollisionWorld::ContactResultCallback& m_resultCallback;

	SingleContactCallback(CollisionObject2* collisionObject, CollisionWorld* world, CollisionWorld::ContactResultCallback& resultCallback)
		: m_collisionObject(collisionObject),
		  m_world(world),
		  m_resultCallback(resultCallback)
	{
	}

	virtual bool process(const BroadphaseProxy* proxy)
	{
		CollisionObject2* collisionObject = (CollisionObject2*)proxy->m_clientObject;
		if (collisionObject == m_collisionObject)
			return true;

		//only perform raycast if filterMask matches
		if (m_resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
		{
			CollisionObject2Wrapper ob0(0, m_collisionObject->getCollisionShape(), m_collisionObject, m_collisionObject->getWorldTransform(), -1, -1);
			CollisionObject2Wrapper ob1(0, collisionObject->getCollisionShape(), collisionObject, collisionObject->getWorldTransform(), -1, -1);

			CollisionAlgorithm* algorithm = m_world->getDispatcher()->findAlgorithm(&ob0, &ob1, 0, DRX3D_CLOSEST_POINT_ALGORITHMS);
			if (algorithm)
			{
				BridgedManifoldResult contactPointResult(&ob0, &ob1, m_resultCallback);
				//discrete collision detection query

				algorithm->processCollision(&ob0, &ob1, m_world->getDispatchInfo(), &contactPointResult);

				algorithm->~CollisionAlgorithm();
				m_world->getDispatcher()->freeCollisionAlgorithm(algorithm);
			}
		}
		return true;
	}
};

///contactTest performs a discrete collision test against all objects in the CollisionWorld, and calls the resultCallback.
///it reports one or more contact points for every overlapping object (including the one with deepest penetration)
void CollisionWorld::contactTest(CollisionObject2* colObj, ContactResultCallback& resultCallback)
{
	Vec3 aabbMin, aabbMax;
	colObj->getCollisionShape()->getAabb(colObj->getWorldTransform(), aabbMin, aabbMax);
	SingleContactCallback contactCB(colObj, this, resultCallback);

	m_broadphasePairCache->aabbTest(aabbMin, aabbMax, contactCB);
}

///contactTest performs a discrete collision test between two collision objects and calls the resultCallback if overlap if detected.
///it reports one or more contact points (including the one with deepest penetration)
void CollisionWorld::contactPairTest(CollisionObject2* colObjA, CollisionObject2* colObjB, ContactResultCallback& resultCallback)
{
	CollisionObject2Wrapper obA(0, colObjA->getCollisionShape(), colObjA, colObjA->getWorldTransform(), -1, -1);
	CollisionObject2Wrapper obB(0, colObjB->getCollisionShape(), colObjB, colObjB->getWorldTransform(), -1, -1);

	CollisionAlgorithm* algorithm = getDispatcher()->findAlgorithm(&obA, &obB, 0, DRX3D_CLOSEST_POINT_ALGORITHMS);
	if (algorithm)
	{
		BridgedManifoldResult contactPointResult(&obA, &obB, resultCallback);
		contactPointResult.m_closestPointDistanceThreshold = resultCallback.m_closestDistanceThreshold;
		//discrete collision detection query
		algorithm->processCollision(&obA, &obB, getDispatchInfo(), &contactPointResult);

		algorithm->~CollisionAlgorithm();
		getDispatcher()->freeCollisionAlgorithm(algorithm);
	}
}

class DebugDrawcallback : public TriangleCallback, public InternalTriangleIndexCallback
{
	IDebugDraw* m_debugDrawer;
	Vec3 m_color;
	Transform2 m_worldTrans;

public:
	DebugDrawcallback(IDebugDraw* debugDrawer, const Transform2& worldTrans, const Vec3& color) : m_debugDrawer(debugDrawer),
																										  m_color(color),
																										  m_worldTrans(worldTrans)
	{
	}

	virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		processTriangle(triangle, partId, triangleIndex);
	}

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		(void)partId;
		(void)triangleIndex;

		Vec3 wv0, wv1, wv2;
		wv0 = m_worldTrans * triangle[0];
		wv1 = m_worldTrans * triangle[1];
		wv2 = m_worldTrans * triangle[2];
		Vec3 center = (wv0 + wv1 + wv2) * Scalar(1. / 3.);

		if (m_debugDrawer->getDebugMode() & IDebugDraw::DBG_DrawNormals)
		{
			Vec3 normal = (wv1 - wv0).cross(wv2 - wv0);
			normal.normalize();
			Vec3 normalColor(1, 1, 0);
			m_debugDrawer->drawLine(center, center + normal, normalColor);
		}
		m_debugDrawer->drawTriangle(wv0, wv1, wv2, m_color, 1.0);
	}
};

void CollisionWorld::debugDrawObject(const Transform2& worldTransform2, const CollisionShape* shape, const Vec3& color)
{
	// Draw a small simplex at the center of the object
	if (getDebugDrawer() && getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawFrames)
	{
		getDebugDrawer()->drawTransform2(worldTransform2, .1);
	}

	if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		const CompoundShape* compoundShape = static_cast<const CompoundShape*>(shape);
		for (i32 i = compoundShape->getNumChildShapes() - 1; i >= 0; i--)
		{
			Transform2 childTrans = compoundShape->getChildTransform(i);
			const CollisionShape* colShape = compoundShape->getChildShape(i);
			debugDrawObject(worldTransform2 * childTrans, colShape, color);
		}
	}
	else
	{
		switch (shape->getShapeType())
		{
			case BOX_SHAPE_PROXYTYPE:
			{
				const BoxShape* boxShape = static_cast<const BoxShape*>(shape);
				Vec3 halfExtents = boxShape->getHalfExtentsWithMargin();
				getDebugDrawer()->drawBox(-halfExtents, halfExtents, worldTransform2, color);
				break;
			}

			case SPHERE_SHAPE_PROXYTYPE:
			{
				const SphereShape* sphereShape = static_cast<const SphereShape*>(shape);
				Scalar radius = sphereShape->getMargin();  //radius doesn't include the margin, so draw with margin

				getDebugDrawer()->drawSphere(radius, worldTransform2, color);
				break;
			}
			case MULTI_SPHERE_SHAPE_PROXYTYPE:
			{
				const MultiSphereShape* multiSphereShape = static_cast<const MultiSphereShape*>(shape);

				Transform2 childTransform2;
				childTransform2.setIdentity();

				for (i32 i = multiSphereShape->getSphereCount() - 1; i >= 0; i--)
				{
					childTransform2.setOrigin(multiSphereShape->getSpherePosition(i));
					getDebugDrawer()->drawSphere(multiSphereShape->getSphereRadius(i), worldTransform2 * childTransform2, color);
				}

				break;
			}
			case CAPSULE_SHAPE_PROXYTYPE:
			{
				const CapsuleShape* capsuleShape = static_cast<const CapsuleShape*>(shape);

				Scalar radius = capsuleShape->getRadius();
				Scalar halfHeight = capsuleShape->getHalfHeight();

				i32 upAxis = capsuleShape->getUpAxis();
				getDebugDrawer()->drawCapsule(radius, halfHeight, upAxis, worldTransform2, color);
				break;
			}
			case CONE_SHAPE_PROXYTYPE:
			{
				const ConeShape* coneShape = static_cast<const ConeShape*>(shape);
				Scalar radius = coneShape->getRadius();  //+coneShape->getMargin();
				Scalar height = coneShape->getHeight();  //+coneShape->getMargin();

				i32 upAxis = coneShape->getConeUpIndex();
				getDebugDrawer()->drawCone(radius, height, upAxis, worldTransform2, color);
				break;
			}
			case CYLINDER_SHAPE_PROXYTYPE:
			{
				const CylinderShape* cylinder = static_cast<const CylinderShape*>(shape);
				i32 upAxis = cylinder->getUpAxis();
				Scalar radius = cylinder->getRadius();
				Scalar halfHeight = cylinder->getHalfExtentsWithMargin()[upAxis];
				getDebugDrawer()->drawCylinder(radius, halfHeight, upAxis, worldTransform2, color);
				break;
			}

			case STATIC_PLANE_PROXYTYPE:
			{
				const StaticPlaneShape* staticPlaneShape = static_cast<const StaticPlaneShape*>(shape);
				Scalar planeConst = staticPlaneShape->getPlaneConstant();
				const Vec3& planeNormal = staticPlaneShape->getPlaneNormal();
				getDebugDrawer()->drawPlane(planeNormal, planeConst, worldTransform2, color);
				break;
			}
			default:
			{
				/// for polyhedral shapes
				if (shape->isPolyhedral())
				{
					PolyhedralConvexShape* polyshape = (PolyhedralConvexShape*)shape;

					i32 i;
					if (polyshape->getConvexPolyhedron())
					{
						const ConvexPolyhedron* poly = polyshape->getConvexPolyhedron();
						for (i = 0; i < poly->m_faces.size(); i++)
						{
							Vec3 centroid(0, 0, 0);
							i32 numVerts = poly->m_faces[i].m_indices.size();
							if (numVerts)
							{
								i32 lastV = poly->m_faces[i].m_indices[numVerts - 1];
								for (i32 v = 0; v < poly->m_faces[i].m_indices.size(); v++)
								{
									i32 curVert = poly->m_faces[i].m_indices[v];
									centroid += poly->m_vertices[curVert];
									getDebugDrawer()->drawLine(worldTransform2 * poly->m_vertices[lastV], worldTransform2 * poly->m_vertices[curVert], color);
									lastV = curVert;
								}
							}
							centroid *= Scalar(1.f) / Scalar(numVerts);
							if (getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawNormals)
							{
								Vec3 normalColor(1, 1, 0);
								Vec3 faceNormal(poly->m_faces[i].m_plane[0], poly->m_faces[i].m_plane[1], poly->m_faces[i].m_plane[2]);
								getDebugDrawer()->drawLine(worldTransform2 * centroid, worldTransform2 * (centroid + faceNormal), normalColor);
							}
						}
					}
					else
					{
						for (i = 0; i < polyshape->getNumEdges(); i++)
						{
							Vec3 a, b;
							polyshape->getEdge(i, a, b);
							Vec3 wa = worldTransform2 * a;
							Vec3 wb = worldTransform2 * b;
							getDebugDrawer()->drawLine(wa, wb, color);
						}
					}
				}

				if (shape->isConcave())
				{
					ConcaveShape* concaveMesh = (ConcaveShape*)shape;

					///@todo pass camera, for some culling? no -> we are not a graphics lib
					Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
					Vec3 aabbMin(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));

					DebugDrawcallback drawCallback(getDebugDrawer(), worldTransform2, color);
					concaveMesh->processAllTriangles(&drawCallback, aabbMin, aabbMax);
				}

				if (shape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE)
				{
					ConvexTriangleMeshShape* convexMesh = (ConvexTriangleMeshShape*)shape;
					//todo: pass camera for some culling
					Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
					Vec3 aabbMin(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
					//DebugDrawcallback drawCallback;
					DebugDrawcallback drawCallback(getDebugDrawer(), worldTransform2, color);
					convexMesh->getMeshInterface()->InternalProcessAllTriangles(&drawCallback, aabbMin, aabbMax);
				}
			}
		}
	}
}

void CollisionWorld::debugDrawWorld()
{
	if (getDebugDrawer())
	{
		getDebugDrawer()->clearLines();

		IDebugDraw::DefaultColors defaultColors = getDebugDrawer()->getDefaultColors();

		if (getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawContactPoints)
		{
			if (getDispatcher())
			{
				i32 numManifolds = getDispatcher()->getNumManifolds();

				for (i32 i = 0; i < numManifolds; i++)
				{
					PersistentManifold* contactManifold = getDispatcher()->getManifoldByIndexInternal(i);
					//CollisionObject2* obA = static_cast<CollisionObject2*>(contactManifold->getBody0());
					//CollisionObject2* obB = static_cast<CollisionObject2*>(contactManifold->getBody1());

					i32 numContacts = contactManifold->getNumContacts();
					for (i32 j = 0; j < numContacts; j++)
					{
						ManifoldPoint& cp = contactManifold->getContactPoint(j);
						getDebugDrawer()->drawContactPoint(cp.m_positionWorldOnB, cp.m_normalWorldOnB, cp.getDistance(), cp.getLifeTime(), defaultColors.m_contactPoint);
					}
				}
			}
		}

		if ((getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe | IDebugDraw::DBG_DrawAabb)))
		{
			i32 i;

			for (i = 0; i < m_collisionObjects.size(); i++)
			{
				CollisionObject2* colObj = m_collisionObjects[i];
				if ((colObj->getCollisionFlags() & CollisionObject2::CF_DISABLE_VISUALIZE_OBJECT) == 0)
				{
					if (getDebugDrawer() && (getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawWireframe))
					{
						Vec3 color(Scalar(0.4), Scalar(0.4), Scalar(0.4));

						switch (colObj->getActivationState())
						{
							case ACTIVE_TAG:
								color = defaultColors.m_activeObject;
								break;
							case ISLAND_SLEEPING:
								color = defaultColors.m_deactivatedObject;
								break;
							case WANTS_DEACTIVATION:
								color = defaultColors.m_wantsDeactivationObject;
								break;
							case DISABLE_DEACTIVATION:
								color = defaultColors.m_disabledDeactivationObject;
								break;
							case DISABLE_SIMULATION:
								color = defaultColors.m_disabledSimulationObject;
								break;
							default:
							{
								color = Vec3(Scalar(.3), Scalar(0.3), Scalar(0.3));
							}
						};

						colObj->getCustomDebugColor(color);

						debugDrawObject(colObj->getWorldTransform(), colObj->getCollisionShape(), color);
					}
					if (m_debugDrawer && (m_debugDrawer->getDebugMode() & IDebugDraw::DBG_DrawAabb))
					{
						Vec3 minAabb, maxAabb;
						Vec3 colorvec = defaultColors.m_aabb;
						colObj->getCollisionShape()->getAabb(colObj->getWorldTransform(), minAabb, maxAabb);
						Vec3 contactThreshold(gContactBreakingThreshold, gContactBreakingThreshold, gContactBreakingThreshold);
						minAabb -= contactThreshold;
						maxAabb += contactThreshold;

						Vec3 minAabb2, maxAabb2;

						if (getDispatchInfo().m_useContinuous && colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY && !colObj->isStaticOrKinematicObject())
						{
							colObj->getCollisionShape()->getAabb(colObj->getInterpolationWorldTransform(), minAabb2, maxAabb2);
							minAabb2 -= contactThreshold;
							maxAabb2 += contactThreshold;
							minAabb.setMin(minAabb2);
							maxAabb.setMax(maxAabb2);
						}

						m_debugDrawer->drawAabb(minAabb, maxAabb, colorvec);
					}
				}
			}
		}
	}
}

void CollisionWorld::serializeCollisionObjects(Serializer* serializer)
{
	i32 i;

	///keep track of shapes already serialized
	HashMap<HashPtr, CollisionShape*> serializedShapes;

	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		CollisionShape* shape = colObj->getCollisionShape();

		if (!serializedShapes.find(shape))
		{
			serializedShapes.insert(shape, shape);
			shape->serializeSingleShape(serializer);
		}
	}

	//serialize all collision objects
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		if (colObj->getInternalType() == CollisionObject2::CO_COLLISION_OBJECT)
		{
			colObj->serializeSingleObject(serializer);
		}
	}
}

void CollisionWorld::serializeContactManifolds(Serializer* serializer)
{
	if (serializer->getSerializationFlags() & DRX3D_SERIALIZE_CONTACT_MANIFOLDS)
	{
		i32 numManifolds = getDispatcher()->getNumManifolds();
		for (i32 i = 0; i < numManifolds; i++)
		{
			const PersistentManifold* manifold = getDispatcher()->getInternalManifoldPointer()[i];
			//don't serialize empty manifolds, they just take space
			//(may have to do it anyway if it destroys determinism)
			if (manifold->getNumContacts() == 0)
				continue;

			Chunk* chunk = serializer->allocate(manifold->calculateSerializeBufferSize(), 1);
			tukk structType = manifold->serialize(manifold, chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_CONTACTMANIFOLD_CODE, (uk )manifold);
		}
	}
}

void CollisionWorld::serialize(Serializer* serializer)
{
	serializer->startSerialization();

	serializeCollisionObjects(serializer);

	serializeContactManifolds(serializer);

	serializer->finishSerialization();
}
