///Specialized capsule-capsule collision algorithm has been added for drx3D 2.75 release to increase ragdoll performance
///If you experience problems with capsule-capsule collision, try to define DRX3D_DISABLE_CAPSULE_CAPSULE_COLLIDER and report it in the drx3D forums
///with reproduction case
//#define DRX3D_DISABLE_CAPSULE_CAPSULE_COLLIDER 1
//#define ZERO_MARGIN

#include <drx3D/Physics/Collision/Dispatch/ConvexConvexAlgorithm.h>

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>

#include <drx3D/Physics/Collision/NarrowPhase/ConvexPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/ContinuousConvexCollision.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkConvexCast.h>

#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

#include <drx3D/Physics/Collision/NarrowPhase/MinkowskiPenetrationDepthSolver.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/PolyhedralContactClipping.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

///////////

static SIMD_FORCE_INLINE void segmentsClosestPoints(
	Vec3& ptsVector,
	Vec3& offsetA,
	Vec3& offsetB,
	Scalar& tA, Scalar& tB,
	const Vec3& translation,
	const Vec3& dirA, Scalar hlenA,
	const Vec3& dirB, Scalar hlenB)
{
	// compute the parameters of the closest points on each line segment

	Scalar dirA_dot_dirB = Dot(dirA, dirB);
	Scalar dirA_dot_trans = Dot(dirA, translation);
	Scalar dirB_dot_trans = Dot(dirB, translation);

	Scalar denom = 1.0f - dirA_dot_dirB * dirA_dot_dirB;

	if (denom == 0.0f)
	{
		tA = 0.0f;
	}
	else
	{
		tA = (dirA_dot_trans - dirB_dot_trans * dirA_dot_dirB) / denom;
		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}

	tB = tA * dirA_dot_dirB - dirB_dot_trans;

	if (tB < -hlenB)
	{
		tB = -hlenB;
		tA = tB * dirA_dot_dirB + dirA_dot_trans;

		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}
	else if (tB > hlenB)
	{
		tB = hlenB;
		tA = tB * dirA_dot_dirB + dirA_dot_trans;

		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}

	// compute the closest points relative to segment centers.

	offsetA = dirA * tA;
	offsetB = dirB * tB;

	ptsVector = translation - offsetA + offsetB;
}

static SIMD_FORCE_INLINE Scalar capsuleCapsuleDistance(
	Vec3& normalOnB,
	Vec3& pointOnB,
	Scalar capsuleLengthA,
	Scalar capsuleRadiusA,
	Scalar capsuleLengthB,
	Scalar capsuleRadiusB,
	i32 capsuleAxisA,
	i32 capsuleAxisB,
	const Transform2& transformA,
	const Transform2& transformB,
	Scalar distanceThreshold)
{
	Vec3 directionA = transformA.getBasis().getColumn(capsuleAxisA);
	Vec3 translationA = transformA.getOrigin();
	Vec3 directionB = transformB.getBasis().getColumn(capsuleAxisB);
	Vec3 translationB = transformB.getOrigin();

	// translation between centers

	Vec3 translation = translationB - translationA;

	// compute the closest points of the capsule line segments

	Vec3 ptsVector;  // the vector between the closest points

	Vec3 offsetA, offsetB;  // offsets from segment centers to their closest points
	Scalar tA, tB;             // parameters on line segment

	segmentsClosestPoints(ptsVector, offsetA, offsetB, tA, tB, translation,
						  directionA, capsuleLengthA, directionB, capsuleLengthB);

	Scalar distance = ptsVector.length() - capsuleRadiusA - capsuleRadiusB;

	if (distance > distanceThreshold)
		return distance;

	Scalar lenSqr = ptsVector.length2();
	if (lenSqr <= (SIMD_EPSILON * SIMD_EPSILON))
	{
		//degenerate case where 2 capsules are likely at the same location: take a vector tangential to 'directionA'
		Vec3 q;
		PlaneSpace1(directionA, normalOnB, q);
	}
	else
	{
		// compute the contact normal
		normalOnB = ptsVector * -RecipSqrt(lenSqr);
	}
	pointOnB = transformB.getOrigin() + offsetB + normalOnB * capsuleRadiusB;

	return distance;
}

//////////

ConvexConvexAlgorithm::CreateFunc::CreateFunc(ConvexPenetrationDepthSolver* pdSolver)
{
	m_numPerturbationIterations = 0;
	m_minimumPointsPerturbationThreshold = 3;
	m_pdSolver = pdSolver;
}

ConvexConvexAlgorithm::CreateFunc::~CreateFunc()
{
}

ConvexConvexAlgorithm::ConvexConvexAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, ConvexPenetrationDepthSolver* pdSolver, i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_pdSolver(pdSolver),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_lowLevelOfDetail(false),
#ifdef USE_SEPDISTANCE_UTIL2
	  m_sepDistance((static_cast<ConvexShape*>(body0->getCollisionShape()))->getAngularMotionDisc(),
					(static_cast<ConvexShape*>(body1->getCollisionShape()))->getAngularMotionDisc()),
#endif
	  m_numPerturbationIterations(numPerturbationIterations),
	  m_minimumPointsPerturbationThreshold(minimumPointsPerturbationThreshold)
{
	(void)body0Wrap;
	(void)body1Wrap;
}

ConvexConvexAlgorithm::~ConvexConvexAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void ConvexConvexAlgorithm ::setLowLevelOfDetail(bool useLowLevel)
{
	m_lowLevelOfDetail = useLowLevel;
}

struct PerturbedContactResult : public ManifoldResult
{
	ManifoldResult* m_originalManifoldResult;
	Transform2 m_transformA;
	Transform2 m_transformB;
	Transform2 m_unPerturbedTransform2;
	bool m_perturbA;
	IDebugDraw* m_debugDrawer;

	PerturbedContactResult(ManifoldResult* originalResult, const Transform2& transformA, const Transform2& transformB, const Transform2& unPerturbedTransform2, bool perturbA, IDebugDraw* debugDrawer)
		: m_originalManifoldResult(originalResult),
		  m_transformA(transformA),
		  m_transformB(transformB),
		  m_unPerturbedTransform2(unPerturbedTransform2),
		  m_perturbA(perturbA),
		  m_debugDrawer(debugDrawer)
	{
	}
	virtual ~PerturbedContactResult()
	{
	}

	virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar orgDepth)
	{
		Vec3 endPt, startPt;
		Scalar newDepth;
		Vec3 newNormal;

		if (m_perturbA)
		{
			Vec3 endPtOrg = pointInWorld + normalOnBInWorld * orgDepth;
			endPt = (m_unPerturbedTransform2 * m_transformA.inverse())(endPtOrg);
			newDepth = (endPt - pointInWorld).dot(normalOnBInWorld);
			startPt = endPt - normalOnBInWorld * newDepth;
		}
		else
		{
			endPt = pointInWorld + normalOnBInWorld * orgDepth;
			startPt = (m_unPerturbedTransform2 * m_transformB.inverse())(pointInWorld);
			newDepth = (endPt - startPt).dot(normalOnBInWorld);
		}

//#define DEBUG_CONTACTS 1
#ifdef DEBUG_CONTACTS
		m_debugDrawer->drawLine(startPt, endPt, Vec3(1, 0, 0));
		m_debugDrawer->drawSphere(startPt, 0.05, Vec3(0, 1, 0));
		m_debugDrawer->drawSphere(endPt, 0.05, Vec3(0, 0, 1));
#endif  //DEBUG_CONTACTS

		m_originalManifoldResult->addContactPoint(normalOnBInWorld, startPt, newDepth);
	}
};

extern Scalar gContactBreakingThreshold;

//
// Convex-Convex collision algorithm
//
void ConvexConvexAlgorithm ::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
	{
		//swapped?
		m_manifoldPtr = m_dispatcher->getNewManifold(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
	resultOut->setPersistentManifold(m_manifoldPtr);

	//comment-out next line to test multi-contact generation
	//resultOut->getPersistentManifold()->clearManifold();

	const ConvexShape* min0 = static_cast<const ConvexShape*>(body0Wrap->getCollisionShape());
	const ConvexShape* min1 = static_cast<const ConvexShape*>(body1Wrap->getCollisionShape());

	Vec3 normalOnB;
	Vec3 pointOnBWorld;
#ifndef DRX3D_DISABLE_CAPSULE_CAPSULE_COLLIDER
	if ((min0->getShapeType() == CAPSULE_SHAPE_PROXYTYPE) && (min1->getShapeType() == CAPSULE_SHAPE_PROXYTYPE))
	{
		//m_manifoldPtr->clearManifold();

		CapsuleShape* capsuleA = (CapsuleShape*)min0;
		CapsuleShape* capsuleB = (CapsuleShape*)min1;

		Scalar threshold = m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;

		Scalar dist = capsuleCapsuleDistance(normalOnB, pointOnBWorld, capsuleA->getHalfHeight(), capsuleA->getRadius(),
											   capsuleB->getHalfHeight(), capsuleB->getRadius(), capsuleA->getUpAxis(), capsuleB->getUpAxis(),
											   body0Wrap->getWorldTransform(), body1Wrap->getWorldTransform(), threshold);

		if (dist < threshold)
		{
			Assert(normalOnB.length2() >= (SIMD_EPSILON * SIMD_EPSILON));
			resultOut->addContactPoint(normalOnB, pointOnBWorld, dist);
		}
		resultOut->refreshContactPoints();
		return;
	}

	if ((min0->getShapeType() == CAPSULE_SHAPE_PROXYTYPE) && (min1->getShapeType() == SPHERE_SHAPE_PROXYTYPE))
	{
		//m_manifoldPtr->clearManifold();

		CapsuleShape* capsuleA = (CapsuleShape*)min0;
		SphereShape* capsuleB = (SphereShape*)min1;

		Scalar threshold = m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;

		Scalar dist = capsuleCapsuleDistance(normalOnB, pointOnBWorld, capsuleA->getHalfHeight(), capsuleA->getRadius(),
											   0., capsuleB->getRadius(), capsuleA->getUpAxis(), 1,
											   body0Wrap->getWorldTransform(), body1Wrap->getWorldTransform(), threshold);

		if (dist < threshold)
		{
			Assert(normalOnB.length2() >= (SIMD_EPSILON * SIMD_EPSILON));
			resultOut->addContactPoint(normalOnB, pointOnBWorld, dist);
		}
		resultOut->refreshContactPoints();
		return;
	}

	if ((min0->getShapeType() == SPHERE_SHAPE_PROXYTYPE) && (min1->getShapeType() == CAPSULE_SHAPE_PROXYTYPE))
	{
		//m_manifoldPtr->clearManifold();

		SphereShape* capsuleA = (SphereShape*)min0;
		CapsuleShape* capsuleB = (CapsuleShape*)min1;

		Scalar threshold = m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;

		Scalar dist = capsuleCapsuleDistance(normalOnB, pointOnBWorld, 0., capsuleA->getRadius(),
											   capsuleB->getHalfHeight(), capsuleB->getRadius(), 1, capsuleB->getUpAxis(),
											   body0Wrap->getWorldTransform(), body1Wrap->getWorldTransform(), threshold);

		if (dist < threshold)
		{
			Assert(normalOnB.length2() >= (SIMD_EPSILON * SIMD_EPSILON));
			resultOut->addContactPoint(normalOnB, pointOnBWorld, dist);
		}
		resultOut->refreshContactPoints();
		return;
	}
#endif  //DRX3D_DISABLE_CAPSULE_CAPSULE_COLLIDER

#ifdef USE_SEPDISTANCE_UTIL2
	if (dispatchInfo.m_useConvexConservativeDistanceUtil)
	{
		m_sepDistance.updateSeparatingDistance(body0->getWorldTransform(), body1->getWorldTransform());
	}

	if (!dispatchInfo.m_useConvexConservativeDistanceUtil || m_sepDistance.getConservativeSeparatingDistance() <= 0.f)
#endif  //USE_SEPDISTANCE_UTIL2

	{
		GjkPairDetector::ClosestPointInput input;
		VoronoiSimplexSolver simplexSolver;
		GjkPairDetector gjkPairDetector(min0, min1, &simplexSolver, m_pdSolver);
		//TODO: if (dispatchInfo.m_useContinuous)
		gjkPairDetector.setMinkowskiA(min0);
		gjkPairDetector.setMinkowskiB(min1);

#ifdef USE_SEPDISTANCE_UTIL2
		if (dispatchInfo.m_useConvexConservativeDistanceUtil)
		{
			input.m_maximumDistanceSquared = DRX3D_LARGE_FLOAT;
		}
		else
#endif  //USE_SEPDISTANCE_UTIL2
		{
			//if (dispatchInfo.m_convexMaxDistanceUseCPT)
			//{
			//	input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactProcessingThreshold();
			//} else
			//{
			input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactBreakingThreshold() + resultOut->m_closestPointDistanceThreshold;
			//		}

			input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;
		}

		input.m_transformA = body0Wrap->getWorldTransform();
		input.m_transformB = body1Wrap->getWorldTransform();

#ifdef USE_SEPDISTANCE_UTIL2
		Scalar sepDist = 0.f;
		if (dispatchInfo.m_useConvexConservativeDistanceUtil)
		{
			sepDist = gjkPairDetector.getCachedSeparatingDistance();
			if (sepDist > SIMD_EPSILON)
			{
				sepDist += dispatchInfo.m_convexConservativeDistanceThreshold;
				//now perturbe directions to get multiple contact points
			}
		}
#endif  //USE_SEPDISTANCE_UTIL2

		if (min0->isPolyhedral() && min1->isPolyhedral())
		{
			struct DummyResult : public DiscreteCollisionDetectorInterface::Result
			{
				Vec3 m_normalOnBInWorld;
				Vec3 m_pointInWorld;
				Scalar m_depth;
				bool m_hasContact;

				DummyResult()
					: m_hasContact(false)
				{
				}

				virtual void setShapeIdentifiersA(i32 partId0, i32 index0) {}
				virtual void setShapeIdentifiersB(i32 partId1, i32 index1) {}
				virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
				{
					m_hasContact = true;
					m_normalOnBInWorld = normalOnBInWorld;
					m_pointInWorld = pointInWorld;
					m_depth = depth;
				}
			};

			struct WithoutMarginResult : public DiscreteCollisionDetectorInterface::Result
			{
				DiscreteCollisionDetectorInterface::Result* m_originalResult;
				Vec3 m_reportedNormalOnWorld;
				Scalar m_marginOnA;
				Scalar m_marginOnB;
				Scalar m_reportedDistance;

				bool m_foundResult;
				WithoutMarginResult(DiscreteCollisionDetectorInterface::Result* result, Scalar marginOnA, Scalar marginOnB)
					: m_originalResult(result),
					  m_marginOnA(marginOnA),
					  m_marginOnB(marginOnB),
					  m_foundResult(false)
				{
				}

				virtual void setShapeIdentifiersA(i32 partId0, i32 index0) {}
				virtual void setShapeIdentifiersB(i32 partId1, i32 index1) {}
				virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorldOrg, Scalar depthOrg)
				{
					m_reportedDistance = depthOrg;
					m_reportedNormalOnWorld = normalOnBInWorld;

					Vec3 adjustedPointB = pointInWorldOrg - normalOnBInWorld * m_marginOnB;
					m_reportedDistance = depthOrg + (m_marginOnA + m_marginOnB);
					if (m_reportedDistance < 0.f)
					{
						m_foundResult = true;
					}
					m_originalResult->addContactPoint(normalOnBInWorld, adjustedPointB, m_reportedDistance);
				}
			};

			DummyResult dummy;

			///BoxShape is an exception: its vertices are created WITH margin so don't subtract it

			Scalar min0Margin = min0->getShapeType() == BOX_SHAPE_PROXYTYPE ? 0.f : min0->getMargin();
			Scalar min1Margin = min1->getShapeType() == BOX_SHAPE_PROXYTYPE ? 0.f : min1->getMargin();

			WithoutMarginResult withoutMargin(resultOut, min0Margin, min1Margin);

			PolyhedralConvexShape* polyhedronA = (PolyhedralConvexShape*)min0;
			PolyhedralConvexShape* polyhedronB = (PolyhedralConvexShape*)min1;
			if (polyhedronA->getConvexPolyhedron() && polyhedronB->getConvexPolyhedron())
			{
				Scalar threshold = m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;

				Scalar minDist = -1e30f;
				Vec3 sepNormalWorldSpace;
				bool foundSepAxis = true;

				if (dispatchInfo.m_enableSatConvex)
				{
					foundSepAxis = PolyhedralContactClipping::findSeparatingAxis(
						*polyhedronA->getConvexPolyhedron(), *polyhedronB->getConvexPolyhedron(),
						body0Wrap->getWorldTransform(),
						body1Wrap->getWorldTransform(),
						sepNormalWorldSpace, *resultOut);
				}
				else
				{
#ifdef ZERO_MARGIN
					gjkPairDetector.setIgnoreMargin(true);
					gjkPairDetector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);
#else

					gjkPairDetector.getClosestPoints(input, withoutMargin, dispatchInfo.m_debugDraw);
					//gjkPairDetector.getClosestPoints(input,dummy,dispatchInfo.m_debugDraw);
#endif  //ZERO_MARGIN
					//Scalar l2 = gjkPairDetector.getCachedSeparatingAxis().length2();
					//if (l2>SIMD_EPSILON)
					{
						sepNormalWorldSpace = withoutMargin.m_reportedNormalOnWorld;  //gjkPairDetector.getCachedSeparatingAxis()*(1.f/l2);
						//minDist = -1e30f;//gjkPairDetector.getCachedSeparatingDistance();
						minDist = withoutMargin.m_reportedDistance;  //gjkPairDetector.getCachedSeparatingDistance()+min0->getMargin()+min1->getMargin();

#ifdef ZERO_MARGIN
						foundSepAxis = true;  //gjkPairDetector.getCachedSeparatingDistance()<0.f;
#else
						foundSepAxis = withoutMargin.m_foundResult && minDist < 0;  //-(min0->getMargin()+min1->getMargin());
#endif
					}
				}
				if (foundSepAxis)
				{
					//				printf("sepNormalWorldSpace=%f,%f,%f\n",sepNormalWorldSpace.getX(),sepNormalWorldSpace.getY(),sepNormalWorldSpace.getZ());

					worldVertsB1.resize(0);
					PolyhedralContactClipping::clipHullAgainstHull(sepNormalWorldSpace, *polyhedronA->getConvexPolyhedron(), *polyhedronB->getConvexPolyhedron(),
																	 body0Wrap->getWorldTransform(),
																	 body1Wrap->getWorldTransform(), minDist - threshold, threshold, worldVertsB1, worldVertsB2,
																	 *resultOut);
				}
				if (m_ownManifold)
				{
					resultOut->refreshContactPoints();
				}
				return;
			}
			else
			{
				//we can also deal with convex versus triangle (without connectivity data)
				if (dispatchInfo.m_enableSatConvex && polyhedronA->getConvexPolyhedron() && polyhedronB->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE)
				{
					VertexArray worldSpaceVertices;
					TriangleShape* tri = (TriangleShape*)polyhedronB;
					worldSpaceVertices.push_back(body1Wrap->getWorldTransform() * tri->m_vertices1[0]);
					worldSpaceVertices.push_back(body1Wrap->getWorldTransform() * tri->m_vertices1[1]);
					worldSpaceVertices.push_back(body1Wrap->getWorldTransform() * tri->m_vertices1[2]);

					//tri->initializePolyhedralFeatures();

					Scalar threshold = m_manifoldPtr->getContactBreakingThreshold()+ resultOut->m_closestPointDistanceThreshold;

					Vec3 sepNormalWorldSpace;
					Scalar minDist = -1e30f;
					Scalar maxDist = threshold;

					bool foundSepAxis = false;
					bool useSatSepNormal = true;

					if (useSatSepNormal)
					{
#if 0
					if (0)
					{
						//initializePolyhedralFeatures performs a convex hull computation, not needed for a single triangle
						polyhedronB->initializePolyhedralFeatures();
					} else
#endif
						{
							Vec3 uniqueEdges[3] = {tri->m_vertices1[1] - tri->m_vertices1[0],
														tri->m_vertices1[2] - tri->m_vertices1[1],
														tri->m_vertices1[0] - tri->m_vertices1[2]};

							uniqueEdges[0].normalize();
							uniqueEdges[1].normalize();
							uniqueEdges[2].normalize();

							ConvexPolyhedron polyhedron;
							polyhedron.m_vertices.push_back(tri->m_vertices1[2]);
							polyhedron.m_vertices.push_back(tri->m_vertices1[0]);
							polyhedron.m_vertices.push_back(tri->m_vertices1[1]);

							{
								Face combinedFaceA;
								combinedFaceA.m_indices.push_back(0);
								combinedFaceA.m_indices.push_back(1);
								combinedFaceA.m_indices.push_back(2);
								Vec3 faceNormal = uniqueEdges[0].cross(uniqueEdges[1]);
								faceNormal.normalize();
								Scalar planeEq = 1e30f;
								for (i32 v = 0; v < combinedFaceA.m_indices.size(); v++)
								{
									Scalar eq = tri->m_vertices1[combinedFaceA.m_indices[v]].dot(faceNormal);
									if (planeEq > eq)
									{
										planeEq = eq;
									}
								}
								combinedFaceA.m_plane[0] = faceNormal[0];
								combinedFaceA.m_plane[1] = faceNormal[1];
								combinedFaceA.m_plane[2] = faceNormal[2];
								combinedFaceA.m_plane[3] = -planeEq;
								polyhedron.m_faces.push_back(combinedFaceA);
							}
							{
								Face combinedFaceB;
								combinedFaceB.m_indices.push_back(0);
								combinedFaceB.m_indices.push_back(2);
								combinedFaceB.m_indices.push_back(1);
								Vec3 faceNormal = -uniqueEdges[0].cross(uniqueEdges[1]);
								faceNormal.normalize();
								Scalar planeEq = 1e30f;
								for (i32 v = 0; v < combinedFaceB.m_indices.size(); v++)
								{
									Scalar eq = tri->m_vertices1[combinedFaceB.m_indices[v]].dot(faceNormal);
									if (planeEq > eq)
									{
										planeEq = eq;
									}
								}

								combinedFaceB.m_plane[0] = faceNormal[0];
								combinedFaceB.m_plane[1] = faceNormal[1];
								combinedFaceB.m_plane[2] = faceNormal[2];
								combinedFaceB.m_plane[3] = -planeEq;
								polyhedron.m_faces.push_back(combinedFaceB);
							}

							polyhedron.m_uniqueEdges.push_back(uniqueEdges[0]);
							polyhedron.m_uniqueEdges.push_back(uniqueEdges[1]);
							polyhedron.m_uniqueEdges.push_back(uniqueEdges[2]);
							polyhedron.initialize2();

							polyhedronB->setPolyhedralFeatures(polyhedron);
						}

						foundSepAxis = PolyhedralContactClipping::findSeparatingAxis(
							*polyhedronA->getConvexPolyhedron(), *polyhedronB->getConvexPolyhedron(),
							body0Wrap->getWorldTransform(),
							body1Wrap->getWorldTransform(),
							sepNormalWorldSpace, *resultOut);
						//	 printf("sepNormalWorldSpace=%f,%f,%f\n",sepNormalWorldSpace.getX(),sepNormalWorldSpace.getY(),sepNormalWorldSpace.getZ());
					}
					else
					{
#ifdef ZERO_MARGIN
						gjkPairDetector.setIgnoreMargin(true);
						gjkPairDetector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);
#else
						gjkPairDetector.getClosestPoints(input, dummy, dispatchInfo.m_debugDraw);
#endif  //ZERO_MARGIN

						if (dummy.m_hasContact && dummy.m_depth < 0)
						{
							if (foundSepAxis)
							{
								if (dummy.m_normalOnBInWorld.dot(sepNormalWorldSpace) < 0.99)
								{
									printf("?\n");
								}
							}
							else
							{
								printf("!\n");
							}
							sepNormalWorldSpace.setVal(0, 0, 1);  // = dummy.m_normalOnBInWorld;
							//minDist = dummy.m_depth;
							foundSepAxis = true;
						}
#if 0
					Scalar l2 = gjkPairDetector.getCachedSeparatingAxis().length2();
					if (l2>SIMD_EPSILON)
					{
						sepNormalWorldSpace = gjkPairDetector.getCachedSeparatingAxis()*(1.f/l2);
						//minDist = gjkPairDetector.getCachedSeparatingDistance();
						//maxDist = threshold;
						minDist = gjkPairDetector.getCachedSeparatingDistance()-min0->getMargin()-min1->getMargin();
						foundSepAxis = true;
					}
#endif
					}

					if (foundSepAxis)
					{
						worldVertsB2.resize(0);
						PolyhedralContactClipping::clipFaceAgainstHull(sepNormalWorldSpace, *polyhedronA->getConvexPolyhedron(),
																		 body0Wrap->getWorldTransform(), worldSpaceVertices, worldVertsB2, minDist - threshold, maxDist, *resultOut);
					}

					if (m_ownManifold)
					{
						resultOut->refreshContactPoints();
					}

					return;
				}
			}
		}

		gjkPairDetector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);

		//now perform 'm_numPerturbationIterations' collision queries with the perturbated collision objects

		//perform perturbation when more then 'm_minimumPointsPerturbationThreshold' points
		if (m_numPerturbationIterations && resultOut->getPersistentManifold()->getNumContacts() < m_minimumPointsPerturbationThreshold)
		{
			i32 i;
			Vec3 v0, v1;
			Vec3 sepNormalWorldSpace;
			Scalar l2 = gjkPairDetector.getCachedSeparatingAxis().length2();

			if (l2 > SIMD_EPSILON)
			{
				sepNormalWorldSpace = gjkPairDetector.getCachedSeparatingAxis() * (1.f / l2);

				PlaneSpace1(sepNormalWorldSpace, v0, v1);

				bool perturbeA = true;
				const Scalar angleLimit = 0.125f * SIMD_PI;
				Scalar perturbeAngle;
				Scalar radiusA = min0->getAngularMotionDisc();
				Scalar radiusB = min1->getAngularMotionDisc();
				if (radiusA < radiusB)
				{
					perturbeAngle = gContactBreakingThreshold / radiusA;
					perturbeA = true;
				}
				else
				{
					perturbeAngle = gContactBreakingThreshold / radiusB;
					perturbeA = false;
				}
				if (perturbeAngle > angleLimit)
					perturbeAngle = angleLimit;

				Transform2 unPerturbedTransform2;
				if (perturbeA)
				{
					unPerturbedTransform2 = input.m_transformA;
				}
				else
				{
					unPerturbedTransform2 = input.m_transformB;
				}

				for (i = 0; i < m_numPerturbationIterations; i++)
				{
					if (v0.length2() > SIMD_EPSILON)
					{
						Quat perturbeRot(v0, perturbeAngle);
						Scalar iterationAngle = i * (SIMD_2_PI / Scalar(m_numPerturbationIterations));
						Quat rotq(sepNormalWorldSpace, iterationAngle);

						if (perturbeA)
						{
							input.m_transformA.setBasis(Matrix3x3(rotq.inverse() * perturbeRot * rotq) * body0Wrap->getWorldTransform().getBasis());
							input.m_transformB = body1Wrap->getWorldTransform();
#ifdef DEBUG_CONTACTS
							dispatchInfo.m_debugDraw->drawTransform2(input.m_transformA, 10.0);
#endif  //DEBUG_CONTACTS
						}
						else
						{
							input.m_transformA = body0Wrap->getWorldTransform();
							input.m_transformB.setBasis(Matrix3x3(rotq.inverse() * perturbeRot * rotq) * body1Wrap->getWorldTransform().getBasis());
#ifdef DEBUG_CONTACTS
							dispatchInfo.m_debugDraw->drawTransform2(input.m_transformB, 10.0);
#endif
						}

						PerturbedContactResult perturbedResultOut(resultOut, input.m_transformA, input.m_transformB, unPerturbedTransform2, perturbeA, dispatchInfo.m_debugDraw);
						gjkPairDetector.getClosestPoints(input, perturbedResultOut, dispatchInfo.m_debugDraw);
					}
				}
			}
		}

#ifdef USE_SEPDISTANCE_UTIL2
		if (dispatchInfo.m_useConvexConservativeDistanceUtil && (sepDist > SIMD_EPSILON))
		{
			m_sepDistance.initSeparatingDistance(gjkPairDetector.getCachedSeparatingAxis(), sepDist, body0->getWorldTransform(), body1->getWorldTransform());
		}
#endif  //USE_SEPDISTANCE_UTIL2
	}

	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
}

bool disableCcd = false;
Scalar ConvexConvexAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	///Rather then checking ALL pairs, only calculate TOI when motion exceeds threshold

	///Linear motion for one of objects needs to exceed m_ccdSquareMotionThreshold
	///col0->m_worldTransform,
	Scalar resultFraction = Scalar(1.);

	Scalar squareMot0 = (col0->getInterpolationWorldTransform().getOrigin() - col0->getWorldTransform().getOrigin()).length2();
	Scalar squareMot1 = (col1->getInterpolationWorldTransform().getOrigin() - col1->getWorldTransform().getOrigin()).length2();

	if (squareMot0 < col0->getCcdSquareMotionThreshold() &&
		squareMot1 < col1->getCcdSquareMotionThreshold())
		return resultFraction;

	if (disableCcd)
		return Scalar(1.);

	//An adhoc way of testing the Continuous Collision Detection algorithms
	//One object is approximated as a sphere, to simplify things
	//Starting in penetration should report no time of impact
	//For proper CCD, better accuracy and handling of 'allowed' penetration should be added
	//also the mainloop of the physics should have a kind of toi queue (something like Brian Mirtich's application of Timewarp for Rigidbodies)

	/// Convex0 against sphere for Convex1
	{
		ConvexShape* convex0 = static_cast<ConvexShape*>(col0->getCollisionShape());

		SphereShape sphere1(col1->getCcdSweptSphereRadius());  //todo: allow non-zero sphere sizes, for better approximation
		ConvexCast::CastResult result;
		VoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		GjkConvexCast ccd1(convex0, &sphere1, &voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(), col0->getInterpolationWorldTransform(),
								  col1->getWorldTransform(), col1->getInterpolationWorldTransform(), result))
		{
			//store result.m_fraction in both bodies

			if (col0->getHitFraction() > result.m_fraction)
				col0->setHitFraction(result.m_fraction);

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction(result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;
		}
	}

	/// Sphere (for convex0) against Convex1
	{
		ConvexShape* convex1 = static_cast<ConvexShape*>(col1->getCollisionShape());

		SphereShape sphere0(col0->getCcdSweptSphereRadius());  //todo: allow non-zero sphere sizes, for better approximation
		ConvexCast::CastResult result;
		VoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		GjkConvexCast ccd1(&sphere0, convex1, &voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(), col0->getInterpolationWorldTransform(),
								  col1->getWorldTransform(), col1->getInterpolationWorldTransform(), result))
		{
			//store result.m_fraction in both bodies

			if (col0->getHitFraction() > result.m_fraction)
				col0->setHitFraction(result.m_fraction);

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction(result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;
		}
	}

	return resultFraction;
}
