
#include <drx3D/Physics/SoftBody/SoftBodyConcaveCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/TetrahedronShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexHullShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>

#define DRX3D_SOFTBODY_TRIANGLE_EXTRUSION Scalar(0.06)  //make this configurable

SoftBodyConcaveCollisionAlgorithm::SoftBodyConcaveCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped)
	: CollisionAlgorithm(ci),
	  m_isSwapped(isSwapped),
	  m_SoftBodyTriangleCallback(ci.m_dispatcher1, body0Wrap, body1Wrap, isSwapped)
{
}

SoftBodyConcaveCollisionAlgorithm::~SoftBodyConcaveCollisionAlgorithm()
{
}

SoftBodyTriangleCallback::SoftBodyTriangleCallback(Dispatcher* dispatcher, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped) : m_dispatcher(dispatcher),
																																														 m_dispatchInfoPtr(0)
{
	m_softBody = (isSwapped ? (SoftBody*)body1Wrap->getCollisionObject() : (SoftBody*)body0Wrap->getCollisionObject());
	m_triBody = isSwapped ? body0Wrap->getCollisionObject() : body1Wrap->getCollisionObject();

	//
	// create the manifold from the dispatcher 'manifold pool'
	//
	//	  m_manifoldPtr = m_dispatcher->getNewManifold(m_convexBody,m_triBody);

	clearCache();
}

SoftBodyTriangleCallback::~SoftBodyTriangleCallback()
{
	clearCache();
	//	m_dispatcher->releaseManifold( m_manifoldPtr );
}

void SoftBodyTriangleCallback::clearCache()
{
	for (i32 i = 0; i < m_shapeCache.size(); i++)
	{
		TriIndex* tmp = m_shapeCache.getAtIndex(i);
		Assert(tmp);
		Assert(tmp->m_childShape);
		m_softBody->getWorldInfo()->m_sparsesdf.RemoveReferences(tmp->m_childShape);  //necessary?
		delete tmp->m_childShape;
	}
	m_shapeCache.clear();
}

void SoftBodyTriangleCallback::processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
{
	//just for debugging purposes
	//printf("triangle %d",m_triangleCount++);

	CollisionAlgorithmConstructionInfo ci;
	ci.m_dispatcher1 = m_dispatcher;

	///debug drawing of the overlapping triangles
	if (m_dispatchInfoPtr && m_dispatchInfoPtr->m_debugDraw && (m_dispatchInfoPtr->m_debugDraw->getDebugMode() & IDebugDraw::DBG_DrawWireframe))
	{
		Vec3 color(1, 1, 0);
		const Transform2& tr = m_triBody->getWorldTransform();
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[0]), tr(triangle[1]), color);
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[1]), tr(triangle[2]), color);
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[2]), tr(triangle[0]), color);
	}

	TriIndex triIndex(partId, triangleIndex, 0);
	HashKey<TriIndex> triKey(triIndex.getUid());

	TriIndex* shapeIndex = m_shapeCache[triKey];
	if (shapeIndex)
	{
		CollisionShape* tm = shapeIndex->m_childShape;
		Assert(tm);

		//copy over user pointers to temporary shape
		tm->setUserPointer(m_triBody->getCollisionShape()->getUserPointer());

		CollisionObject2Wrapper softBody(0, m_softBody->getCollisionShape(), m_softBody, m_softBody->getWorldTransform(), -1, -1);
		//CollisionObject2Wrapper triBody(0,tm, ob, Transform2::getIdentity());//ob->getWorldTransform());//??
		CollisionObject2Wrapper triBody(0, tm, m_triBody, m_triBody->getWorldTransform(), partId, triangleIndex);
		eDispatcherQueryType algoType = m_resultOut->m_closestPointDistanceThreshold > 0 ? DRX3D_CLOSEST_POINT_ALGORITHMS : DRX3D_CONTACT_POINT_ALGORITHMS;
		CollisionAlgorithm* colAlgo = ci.m_dispatcher1->findAlgorithm(&softBody, &triBody, 0, algoType);  //m_manifoldPtr);

		colAlgo->processCollision(&softBody, &triBody, *m_dispatchInfoPtr, m_resultOut);
		colAlgo->~CollisionAlgorithm();
		ci.m_dispatcher1->freeCollisionAlgorithm(colAlgo);

		return;
	}

	//aabb filter is already applied!

	//CollisionObject2* colObj = static_cast<CollisionObject2*>(m_convexProxy->m_clientObject);

	//	if (m_softBody->getCollisionShape()->getShapeType()==
	{
		//		Vec3 other;
		Vec3 normal = (triangle[1] - triangle[0]).cross(triangle[2] - triangle[0]);
		normal.normalize();
		normal *= DRX3D_SOFTBODY_TRIANGLE_EXTRUSION;
		//		other=(triangle[0]+triangle[1]+triangle[2])*0.333333f;
		//		other+=normal*22.f;
		Vec3 pts[6] = {triangle[0] + normal,
							triangle[1] + normal,
							triangle[2] + normal,
							triangle[0] - normal,
							triangle[1] - normal,
							triangle[2] - normal};

		ConvexHullShape* tm = new ConvexHullShape(&pts[0].getX(), 6);

		//		BU_Simplex1to4 tm(triangle[0],triangle[1],triangle[2],other);

		//TriangleShape tm(triangle[0],triangle[1],triangle[2]);
		//	tm.setMargin(m_collisionMarginTriangle);

		//copy over user pointers to temporary shape
		tm->setUserPointer(m_triBody->getCollisionShape()->getUserPointer());

		CollisionObject2Wrapper softBody(0, m_softBody->getCollisionShape(), m_softBody, m_softBody->getWorldTransform(), -1, -1);
		CollisionObject2Wrapper triBody(0, tm, m_triBody, m_triBody->getWorldTransform(), partId, triangleIndex);  //Transform2::getIdentity());//??

		eDispatcherQueryType algoType = m_resultOut->m_closestPointDistanceThreshold > 0 ? DRX3D_CLOSEST_POINT_ALGORITHMS : DRX3D_CONTACT_POINT_ALGORITHMS;
		CollisionAlgorithm* colAlgo = ci.m_dispatcher1->findAlgorithm(&softBody, &triBody, 0, algoType);  //m_manifoldPtr);

		colAlgo->processCollision(&softBody, &triBody, *m_dispatchInfoPtr, m_resultOut);
		colAlgo->~CollisionAlgorithm();
		ci.m_dispatcher1->freeCollisionAlgorithm(colAlgo);

		triIndex.m_childShape = tm;
		m_shapeCache.insert(triKey, triIndex);
	}
}

void SoftBodyTriangleCallback::setTimeStepAndCounters(Scalar collisionMarginTriangle, const CollisionObject2Wrapper* triBodyWrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	m_dispatchInfoPtr = &dispatchInfo;
	m_collisionMarginTriangle = collisionMarginTriangle + Scalar(DRX3D_SOFTBODY_TRIANGLE_EXTRUSION);
	m_resultOut = resultOut;

	Vec3 aabbWorldSpaceMin, aabbWorldSpaceMax;
	m_softBody->getAabb(aabbWorldSpaceMin, aabbWorldSpaceMax);
	Vec3 halfExtents = (aabbWorldSpaceMax - aabbWorldSpaceMin) * Scalar(0.5);
	Vec3 softBodyCenter = (aabbWorldSpaceMax + aabbWorldSpaceMin) * Scalar(0.5);

	Transform2 softTransform2;
	softTransform2.setIdentity();
	softTransform2.setOrigin(softBodyCenter);

	Transform2 convexInTriangleSpace;
	convexInTriangleSpace = triBodyWrap->getWorldTransform().inverse() * softTransform2;
	Transform2Aabb(halfExtents, m_collisionMarginTriangle, convexInTriangleSpace, m_aabbMin, m_aabbMax);
}

void SoftBodyConcaveCollisionAlgorithm::clearCache()
{
	m_SoftBodyTriangleCallback.clearCache();
}

void SoftBodyConcaveCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	//CollisionObject2* convexBody = m_isSwapped ? body1 : body0;
	const CollisionObject2Wrapper* triBody = m_isSwapped ? body0Wrap : body1Wrap;

	if (triBody->getCollisionShape()->isConcave())
	{
		const ConcaveShape* concaveShape = static_cast<const ConcaveShape*>(triBody->getCollisionShape());

		//	if (convexBody->getCollisionShape()->isConvex())
		{
			Scalar collisionMarginTriangle = concaveShape->getMargin();

			//			resultOut->setPersistentManifold(m_SoftBodyTriangleCallback.m_manifoldPtr);
			m_SoftBodyTriangleCallback.setTimeStepAndCounters(collisionMarginTriangle, triBody, dispatchInfo, resultOut);

			concaveShape->processAllTriangles(&m_SoftBodyTriangleCallback, m_SoftBodyTriangleCallback.getAabbMin(), m_SoftBodyTriangleCallback.getAabbMax());

			//	resultOut->refreshContactPoints();
		}
	}
}

Scalar SoftBodyConcaveCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	CollisionObject2* convexbody = m_isSwapped ? body1 : body0;
	CollisionObject2* triBody = m_isSwapped ? body0 : body1;

	//quick approximation using raycast, todo: hook up to the continuous collision detection (one of the ConvexCast)

	//only perform CCD above a certain threshold, this prevents blocking on the long run
	//because object in a blocked ccd state (hitfraction<1) get their linear velocity halved each frame...
	Scalar squareMot0 = (convexbody->getInterpolationWorldTransform().getOrigin() - convexbody->getWorldTransform().getOrigin()).length2();
	if (squareMot0 < convexbody->getCcdSquareMotionThreshold())
	{
		return Scalar(1.);
	}

	//const Vec3& from = convexbody->m_worldTransform.getOrigin();
	//Vec3 to = convexbody->m_interpolationWorldTransform.getOrigin();
	//todo: only do if the motion exceeds the 'radius'

	Transform2 triInv = triBody->getWorldTransform().inverse();
	Transform2 convexFromLocal = triInv * convexbody->getWorldTransform();
	Transform2 convexToLocal = triInv * convexbody->getInterpolationWorldTransform();

	struct LocalTriangleSphereCastCallback : public TriangleCallback
	{
		Transform2 m_ccdSphereFromTrans;
		Transform2 m_ccdSphereToTrans;
		Transform2 m_meshTransform2;

		Scalar m_ccdSphereRadius;
		Scalar m_hitFraction;

		LocalTriangleSphereCastCallback(const Transform2& from, const Transform2& to, Scalar ccdSphereRadius, Scalar hitFraction)
			: m_ccdSphereFromTrans(from),
			  m_ccdSphereToTrans(to),
			  m_ccdSphereRadius(ccdSphereRadius),
			  m_hitFraction(hitFraction)
		{
		}

		virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			(void)partId;
			(void)triangleIndex;
			//do a swept sphere for now
			Transform2 ident;
			ident.setIdentity();
			ConvexCast::CastResult castResult;
			castResult.m_fraction = m_hitFraction;
			SphereShape pointShape(m_ccdSphereRadius);
			TriangleShape triShape(triangle[0], triangle[1], triangle[2]);
			VoronoiSimplexSolver simplexSolver;
			SubsimplexConvexCast convexCaster(&pointShape, &triShape, &simplexSolver);
			//GjkConvexCast	convexCaster(&pointShape,convexShape,&simplexSolver);
			//ContinuousConvexCollision convexCaster(&pointShape,convexShape,&simplexSolver,0);
			//local space?

			if (convexCaster.calcTimeOfImpact(m_ccdSphereFromTrans, m_ccdSphereToTrans,
											  ident, ident, castResult))
			{
				if (m_hitFraction > castResult.m_fraction)
					m_hitFraction = castResult.m_fraction;
			}
		}
	};

	if (triBody->getCollisionShape()->isConcave())
	{
		Vec3 rayAabbMin = convexFromLocal.getOrigin();
		rayAabbMin.setMin(convexToLocal.getOrigin());
		Vec3 rayAabbMax = convexFromLocal.getOrigin();
		rayAabbMax.setMax(convexToLocal.getOrigin());
		Scalar ccdRadius0 = convexbody->getCcdSweptSphereRadius();
		rayAabbMin -= Vec3(ccdRadius0, ccdRadius0, ccdRadius0);
		rayAabbMax += Vec3(ccdRadius0, ccdRadius0, ccdRadius0);

		Scalar curHitFraction = Scalar(1.);  //is this available?
		LocalTriangleSphereCastCallback raycastCallback(convexFromLocal, convexToLocal,
														convexbody->getCcdSweptSphereRadius(), curHitFraction);

		raycastCallback.m_hitFraction = convexbody->getHitFraction();

		CollisionObject2* concavebody = triBody;

		ConcaveShape* triangleMesh = (ConcaveShape*)concavebody->getCollisionShape();

		if (triangleMesh)
		{
			triangleMesh->processAllTriangles(&raycastCallback, rayAabbMin, rayAabbMax);
		}

		if (raycastCallback.m_hitFraction < convexbody->getHitFraction())
		{
			convexbody->setHitFraction(raycastCallback.m_hitFraction);
			return raycastCallback.m_hitFraction;
		}
	}

	return Scalar(1.);
}
