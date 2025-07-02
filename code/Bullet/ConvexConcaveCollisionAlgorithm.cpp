#include <drx3D/Physics/Collision/Dispatch/ConvexConcaveCollisionAlgorithm.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>
#include <drx3D/Physics/Collision/Shapes/SdfCollisionShape.h>

ConvexConcaveCollisionAlgorithm::ConvexConcaveCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_ConvexTriangleCallback(ci.m_dispatcher1, body0Wrap, body1Wrap, isSwapped),
	  m_isSwapped(isSwapped)
{
}

ConvexConcaveCollisionAlgorithm::~ConvexConcaveCollisionAlgorithm()
{
}

void ConvexConcaveCollisionAlgorithm::getAllContactManifolds(ManifoldArray& manifoldArray)
{
	if (m_ConvexTriangleCallback.m_manifoldPtr)
	{
		manifoldArray.push_back(m_ConvexTriangleCallback.m_manifoldPtr);
	}
}

ConvexTriangleCallback::ConvexTriangleCallback(Dispatcher* dispatcher, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped) : m_dispatcher(dispatcher),
																																													 m_dispatchInfoPtr(0)
{
	m_convexBodyWrap = isSwapped ? body1Wrap : body0Wrap;
	m_triBodyWrap = isSwapped ? body0Wrap : body1Wrap;

	//
	// create the manifold from the dispatcher 'manifold pool'
	//
	m_manifoldPtr = m_dispatcher->getNewManifold(m_convexBodyWrap->getCollisionObject(), m_triBodyWrap->getCollisionObject());

	clearCache();
}

ConvexTriangleCallback::~ConvexTriangleCallback()
{
	clearCache();
	m_dispatcher->releaseManifold(m_manifoldPtr);
}

void ConvexTriangleCallback::clearCache()
{
	m_dispatcher->clearManifold(m_manifoldPtr);
}

void ConvexTriangleCallback::processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
{
	DRX3D_PROFILE("ConvexTriangleCallback::processTriangle");

	if (!TestTriangleAgainstAabb2(triangle, m_aabbMin, m_aabbMax))
	{
		return;
	}

	//just for debugging purposes
	//printf("triangle %d",m_triangleCount++);

	CollisionAlgorithmConstructionInfo ci;
	ci.m_dispatcher1 = m_dispatcher;

#if 0	
	
	///debug drawing of the overlapping triangles
	if (m_dispatchInfoPtr && m_dispatchInfoPtr->m_debugDraw && (m_dispatchInfoPtr->m_debugDraw->getDebugMode() &IDebugDraw::DBG_DrawWireframe ))
	{
		const CollisionObject2* ob = const_cast<CollisionObject2*>(m_triBodyWrap->getCollisionObject());
		Vec3 color(1,1,0);
		Transform2& tr = ob->getWorldTransform();
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[0]),tr(triangle[1]),color);
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[1]),tr(triangle[2]),color);
		m_dispatchInfoPtr->m_debugDraw->drawLine(tr(triangle[2]),tr(triangle[0]),color);
	}
#endif

	if (m_convexBodyWrap->getCollisionShape()->isConvex())
	{
#ifdef DRX3D_ENABLE_CONVEX_CONCAVE_EARLY_OUT
    //todo: check this issue https://github.com/bulletphysics/bullet3/issues/4263
		//an early out optimisation if the object is separated from the triangle
		//projected on the triangle normal)
		{
			const Vec3 v0 = m_triBodyWrap->getWorldTransform()*triangle[0];
			const Vec3 v1 = m_triBodyWrap->getWorldTransform()*triangle[1];
			const Vec3 v2 = m_triBodyWrap->getWorldTransform()*triangle[2];

			Vec3 triangle_normal_world = ( v1 - v0).cross(v2 - v0);
			triangle_normal_world.normalize();

		    ConvexShape* convex = (ConvexShape*)m_convexBodyWrap->getCollisionShape();
			
			Vec3 localPt = convex->localGetSupportingVertex(m_convexBodyWrap->getWorldTransform().getBasis().inverse()*triangle_normal_world);
			Vec3 worldPt = m_convexBodyWrap->getWorldTransform()*localPt;
			//now check if this is fully on one side of the triangle
			Scalar proj_distPt = triangle_normal_world.dot(worldPt);
			Scalar proj_distTr = triangle_normal_world.dot(v0);
			Scalar contact_threshold = m_manifoldPtr->getContactBreakingThreshold()+ m_resultOut->m_closestPointDistanceThreshold;
			Scalar dist = proj_distTr - proj_distPt;
			if (dist > contact_threshold)
				return;

			//also check the other side of the triangle
			triangle_normal_world*=-1;

			localPt = convex->localGetSupportingVertex(m_convexBodyWrap->getWorldTransform().getBasis().inverse()*triangle_normal_world);
			worldPt = m_convexBodyWrap->getWorldTransform()*localPt;
			//now check if this is fully on one side of the triangle
			proj_distPt = triangle_normal_world.dot(worldPt);
			proj_distTr = triangle_normal_world.dot(v0);
			
			dist = proj_distTr - proj_distPt;
			if (dist > contact_threshold)
				return;
        }
#endif //DRX3D_ENABLE_CONVEX_CONCAVE_EARLY_OUT

		TriangleShape tm(triangle[0], triangle[1], triangle[2]);
		tm.setMargin(m_collisionMarginTriangle);

		CollisionObject2Wrapper triObWrap(m_triBodyWrap, &tm, m_triBodyWrap->getCollisionObject(), m_triBodyWrap->getWorldTransform(), partId, triangleIndex);  //correct transform?
		CollisionAlgorithm* colAlgo = 0;

		if (m_resultOut->m_closestPointDistanceThreshold > 0)
		{
			colAlgo = ci.m_dispatcher1->findAlgorithm(m_convexBodyWrap, &triObWrap, 0, DRX3D_CLOSEST_POINT_ALGORITHMS);
		}
		else
		{
			colAlgo = ci.m_dispatcher1->findAlgorithm(m_convexBodyWrap, &triObWrap, m_manifoldPtr, DRX3D_CONTACT_POINT_ALGORITHMS);
		}
		const CollisionObject2Wrapper* tmpWrap = 0;

		if (m_resultOut->getBody0Internal() == m_triBodyWrap->getCollisionObject())
		{
			tmpWrap = m_resultOut->getBody0Wrap();
			m_resultOut->setBody0Wrap(&triObWrap);
			m_resultOut->setShapeIdentifiersA(partId, triangleIndex);
		}
		else
		{
			tmpWrap = m_resultOut->getBody1Wrap();
			m_resultOut->setBody1Wrap(&triObWrap);
			m_resultOut->setShapeIdentifiersB(partId, triangleIndex);
		}

		{
			DRX3D_PROFILE("processCollision (GJK?)");
			colAlgo->processCollision(m_convexBodyWrap, &triObWrap, *m_dispatchInfoPtr, m_resultOut);
		}

		if (m_resultOut->getBody0Internal() == m_triBodyWrap->getCollisionObject())
		{
			m_resultOut->setBody0Wrap(tmpWrap);
		}
		else
		{
			m_resultOut->setBody1Wrap(tmpWrap);
		}

		colAlgo->~CollisionAlgorithm();
		ci.m_dispatcher1->freeCollisionAlgorithm(colAlgo);
	}
}

void ConvexTriangleCallback::setTimeStepAndCounters(Scalar collisionMarginTriangle, const DispatcherInfo& dispatchInfo, const CollisionObject2Wrapper* convexBodyWrap, const CollisionObject2Wrapper* triBodyWrap, ManifoldResult* resultOut)
{
	m_convexBodyWrap = convexBodyWrap;
	m_triBodyWrap = triBodyWrap;

	m_dispatchInfoPtr = &dispatchInfo;
	m_collisionMarginTriangle = collisionMarginTriangle;
	m_resultOut = resultOut;

	//recalc aabbs
	Transform2 convexInTriangleSpace;
	convexInTriangleSpace = m_triBodyWrap->getWorldTransform().inverse() * m_convexBodyWrap->getWorldTransform();
	const CollisionShape* convexShape = static_cast<const CollisionShape*>(m_convexBodyWrap->getCollisionShape());
	//CollisionShape* triangleShape = static_cast<CollisionShape*>(triBody->m_collisionShape);
	convexShape->getAabb(convexInTriangleSpace, m_aabbMin, m_aabbMax);
	Scalar extraMargin = collisionMarginTriangle + resultOut->m_closestPointDistanceThreshold;

	Vec3 extra(extraMargin, extraMargin, extraMargin);

	m_aabbMax += extra;
	m_aabbMin -= extra;
}

void ConvexConcaveCollisionAlgorithm::clearCache()
{
	m_ConvexTriangleCallback.clearCache();
}

void ConvexConcaveCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	DRX3D_PROFILE("ConvexConcaveCollisionAlgorithm::processCollision");

	const CollisionObject2Wrapper* convexBodyWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* triBodyWrap = m_isSwapped ? body0Wrap : body1Wrap;

	if (triBodyWrap->getCollisionShape()->isConcave())
	{
		if (triBodyWrap->getCollisionShape()->getShapeType() == SDF_SHAPE_PROXYTYPE)
		{
			SdfCollisionShape* sdfShape = (SdfCollisionShape*)triBodyWrap->getCollisionShape();
			if (convexBodyWrap->getCollisionShape()->isConvex())
			{
				ConvexShape* convex = (ConvexShape*)convexBodyWrap->getCollisionShape();
				AlignedObjectArray<Vec3> queryVertices;

				if (convex->isPolyhedral())
				{
					PolyhedralConvexShape* poly = (PolyhedralConvexShape*)convex;
					for (i32 v = 0; v < poly->getNumVertices(); v++)
					{
						Vec3 vtx;
						poly->getVertex(v, vtx);
						queryVertices.push_back(vtx);
					}
				}
				Scalar maxDist = SIMD_EPSILON;

				if (convex->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
				{
					queryVertices.push_back(Vec3(0, 0, 0));
					SphereShape* sphere = (SphereShape*)convex;
					maxDist = sphere->getRadius() + SIMD_EPSILON;
				}
				if (queryVertices.size())
				{
					resultOut->setPersistentManifold(m_ConvexTriangleCallback.m_manifoldPtr);
					//m_ConvexTriangleCallback.m_manifoldPtr->clearManifold();

					PolyhedralConvexShape* poly = (PolyhedralConvexShape*)convex;
					for (i32 v = 0; v < queryVertices.size(); v++)
					{
						const Vec3& vtx = queryVertices[v];
						Vec3 vtxWorldSpace = convexBodyWrap->getWorldTransform() * vtx;
						Vec3 vtxInSdf = triBodyWrap->getWorldTransform().invXform(vtxWorldSpace);

						Vec3 normalLocal;
						Scalar dist;
						if (sdfShape->queryPoint(vtxInSdf, dist, normalLocal))
						{
							if (dist <= maxDist)
							{
								normalLocal.safeNormalize();
								Vec3 normal = triBodyWrap->getWorldTransform().getBasis() * normalLocal;

								if (convex->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
								{
									SphereShape* sphere = (SphereShape*)convex;
									dist -= sphere->getRadius();
									vtxWorldSpace -= sphere->getRadius() * normal;
								}
								resultOut->addContactPoint(normal, vtxWorldSpace - normal * dist, dist);
							}
						}
					}
					resultOut->refreshContactPoints();
				}
			}
		}
		else
		{
			const ConcaveShape* concaveShape = static_cast<const ConcaveShape*>(triBodyWrap->getCollisionShape());

			if (convexBodyWrap->getCollisionShape()->isConvex())
			{
				Scalar collisionMarginTriangle = concaveShape->getMargin();

				resultOut->setPersistentManifold(m_ConvexTriangleCallback.m_manifoldPtr);
				m_ConvexTriangleCallback.setTimeStepAndCounters(collisionMarginTriangle, dispatchInfo, convexBodyWrap, triBodyWrap, resultOut);

				m_ConvexTriangleCallback.m_manifoldPtr->setBodies(convexBodyWrap->getCollisionObject(), triBodyWrap->getCollisionObject());

				concaveShape->processAllTriangles(&m_ConvexTriangleCallback, m_ConvexTriangleCallback.getAabbMin(), m_ConvexTriangleCallback.getAabbMax());

				resultOut->refreshContactPoints();

				m_ConvexTriangleCallback.clearWrapperData();
			}
		}
	}
}

Scalar ConvexConcaveCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
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
			DRX3D_PROFILE("processTriangle");
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
