#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/ContinuousConvexCollision.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/RaycastCallback.h>

TriangleRaycastCallback::TriangleRaycastCallback(const Vec3& from, const Vec3& to, u32 flags)
	: m_from(from),
	  m_to(to),
	  //@BP Mod
	  m_flags(flags),
	  m_hitFraction(Scalar(1.))
{
}

void TriangleRaycastCallback::processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
{
	const Vec3& vert0 = triangle[0];
	const Vec3& vert1 = triangle[1];
	const Vec3& vert2 = triangle[2];

	Vec3 v10;
	v10 = vert1 - vert0;
	Vec3 v20;
	v20 = vert2 - vert0;

	Vec3 triangleNormal;
	triangleNormal = v10.cross(v20);

	const Scalar dist = vert0.dot(triangleNormal);
	Scalar dist_a = triangleNormal.dot(m_from);
	dist_a -= dist;
	Scalar dist_b = triangleNormal.dot(m_to);
	dist_b -= dist;

	if (dist_a * dist_b >= Scalar(0.0))
	{
		return;  // same sign
	}

	if (((m_flags & kF_FilterBackfaces) != 0) && (dist_a <= Scalar(0.0)))
	{
		// Backface, skip check
		return;
	}

	const Scalar proj_length = dist_a - dist_b;
	const Scalar distance = (dist_a) / (proj_length);
	// Now we have the intersection point on the plane, we'll see if it's inside the triangle
	// Add an epsilon as a tolerance for the raycast,
	// in case the ray hits exacly on the edge of the triangle.
	// It must be scaled for the triangle size.

	if (distance < m_hitFraction)
	{
		Scalar edge_tolerance = triangleNormal.length2();
		edge_tolerance *= Scalar(-0.0001);
		Vec3 point;
		point.setInterpolate3(m_from, m_to, distance);
		{
			Vec3 v0p;
			v0p = vert0 - point;
			Vec3 v1p;
			v1p = vert1 - point;
			Vec3 cp0;
			cp0 = v0p.cross(v1p);

			if ((Scalar)(cp0.dot(triangleNormal)) >= edge_tolerance)
			{
				Vec3 v2p;
				v2p = vert2 - point;
				Vec3 cp1;
				cp1 = v1p.cross(v2p);
				if ((Scalar)(cp1.dot(triangleNormal)) >= edge_tolerance)
				{
					Vec3 cp2;
					cp2 = v2p.cross(v0p);

					if ((Scalar)(cp2.dot(triangleNormal)) >= edge_tolerance)
					{
						//@BP Mod
						// Triangle normal isn't normalized
						triangleNormal.normalize();

						//@BP Mod - Allow for unflipped normal when raycasting against backfaces
						if (((m_flags & kF_KeepUnflippedNormal) == 0) && (dist_a <= Scalar(0.0)))
						{
							m_hitFraction = reportHit(-triangleNormal, distance, partId, triangleIndex);
						}
						else
						{
							m_hitFraction = reportHit(triangleNormal, distance, partId, triangleIndex);
						}
					}
				}
			}
		}
	}
}

TriangleConvexcastCallback::TriangleConvexcastCallback(const ConvexShape* convexShape, const Transform2& convexShapeFrom, const Transform2& convexShapeTo, const Transform2& triangleToWorld, const Scalar triangleCollisionMargin)
{
	m_convexShape = convexShape;
	m_convexShapeFrom = convexShapeFrom;
	m_convexShapeTo = convexShapeTo;
	m_triangleToWorld = triangleToWorld;
	m_hitFraction = 1.0f;
	m_triangleCollisionMargin = triangleCollisionMargin;
	m_allowedPenetration = 0.f;
}

void TriangleConvexcastCallback::processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
{
	TriangleShape triangleShape(triangle[0], triangle[1], triangle[2]);
	triangleShape.setMargin(m_triangleCollisionMargin);

	VoronoiSimplexSolver simplexSolver;
	GjkEpaPenetrationDepthSolver gjkEpaPenetrationSolver;

//#define  USE_SUBSIMPLEX_CONVEX_CAST 1
//if you reenable USE_SUBSIMPLEX_CONVEX_CAST see commented out code below
#ifdef USE_SUBSIMPLEX_CONVEX_CAST
	SubsimplexConvexCast convexCaster(m_convexShape, &triangleShape, &simplexSolver);
#else
	//GjkConvexCast	convexCaster(m_convexShape,&triangleShape,&simplexSolver);
	ContinuousConvexCollision convexCaster(m_convexShape, &triangleShape, &simplexSolver, &gjkEpaPenetrationSolver);
#endif  //#USE_SUBSIMPLEX_CONVEX_CAST

	ConvexCast::CastResult castResult;
	castResult.m_fraction = Scalar(1.);
	castResult.m_allowedPenetration = m_allowedPenetration;
	if (convexCaster.calcTimeOfImpact(m_convexShapeFrom, m_convexShapeTo, m_triangleToWorld, m_triangleToWorld, castResult))
	{
		//add hit
		if (castResult.m_normal.length2() > Scalar(0.0001))
		{
			if (castResult.m_fraction < m_hitFraction)
			{
				/* ContinuousConvexCast's normal is already in world space */
				/*
#ifdef USE_SUBSIMPLEX_CONVEX_CAST
				//rotate normal into worldspace
				castResult.m_normal = m_convexShapeFrom.getBasis() * castResult.m_normal;
#endif //USE_SUBSIMPLEX_CONVEX_CAST
*/
				castResult.m_normal.normalize();

				reportHit(castResult.m_normal,
						  castResult.m_hitPoint,
						  castResult.m_fraction,
						  partId,
						  triangleIndex);
			}
		}
	}
}
