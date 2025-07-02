
#include <drx3D/Maths/Linear/Scalar.h>
#include  <drx3D/Physics/Collision/Dispatch/SphereTriangleDetector.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

SphereTriangleDetector::SphereTriangleDetector(SphereShape* sphere, TriangleShape* triangle, Scalar contactBreakingThreshold)
	: m_sphere(sphere),
	  m_triangle(triangle),
	  m_contactBreakingThreshold(contactBreakingThreshold)
{
}

void SphereTriangleDetector::getClosestPoints(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw, bool swapResults)
{
	(void)debugDraw;
	const Transform2& transformA = input.m_transformA;
	const Transform2& transformB = input.m_transformB;

	Vec3 point, normal;
	Scalar timeOfImpact = Scalar(1.);
	Scalar depth = Scalar(0.);
	//	output.m_distance = Scalar(DRX3D_LARGE_FLOAT);
	//move sphere into triangle space
	Transform2 sphereInTr = transformB.inverseTimes(transformA);

	if (collide(sphereInTr.getOrigin(), point, normal, depth, timeOfImpact, m_contactBreakingThreshold))
	{
		if (swapResults)
		{
			Vec3 normalOnB = transformB.getBasis() * normal;
			Vec3 normalOnA = -normalOnB;
			Vec3 pointOnA = transformB * point + normalOnB * depth;
			output.addContactPoint(normalOnA, pointOnA, depth);
		}
		else
		{
			output.addContactPoint(transformB.getBasis() * normal, transformB * point, depth);
		}
	}
}

// See also geometrictools.com
// Basic idea: D = |p - (lo + t0*lv)| where t0 = lv . (p - lo) / lv . lv
Scalar SegmentSqrDistance(const Vec3& from, const Vec3& to, const Vec3& p, Vec3& nearest);

Scalar SegmentSqrDistance(const Vec3& from, const Vec3& to, const Vec3& p, Vec3& nearest)
{
	Vec3 diff = p - from;
	Vec3 v = to - from;
	Scalar t = v.dot(diff);

	if (t > 0)
	{
		Scalar dotVV = v.dot(v);
		if (t < dotVV)
		{
			t /= dotVV;
			diff -= t * v;
		}
		else
		{
			t = 1;
			diff -= v;
		}
	}
	else
		t = 0;

	nearest = from + t * v;
	return diff.dot(diff);
}

bool SphereTriangleDetector::facecontains(const Vec3& p, const Vec3* vertices, Vec3& normal)
{
	Vec3 lp(p);
	Vec3 lnormal(normal);

	return pointInTriangle(vertices, lnormal, &lp);
}

bool SphereTriangleDetector::collide(const Vec3& sphereCenter, Vec3& point, Vec3& resultNormal, Scalar& depth, Scalar& timeOfImpact, Scalar contactBreakingThreshold)
{
	const Vec3* vertices = &m_triangle->getVertexPtr(0);

	Scalar radius = m_sphere->getRadius();
	Scalar radiusWithThreshold = radius + contactBreakingThreshold;

	Vec3 normal = (vertices[1] - vertices[0]).cross(vertices[2] - vertices[0]);

	Scalar l2 = normal.length2();
	bool hasContact = false;
	Vec3 contactPoint;

	if (l2 >= SIMD_EPSILON * SIMD_EPSILON)
	{
		normal /= Sqrt(l2);

		Vec3 p1ToCentre = sphereCenter - vertices[0];
		Scalar distanceFromPlane = p1ToCentre.dot(normal);

		if (distanceFromPlane < Scalar(0.))
		{
			//triangle facing the other way
			distanceFromPlane *= Scalar(-1.);
			normal *= Scalar(-1.);
		}

		bool isInsideContactPlane = distanceFromPlane < radiusWithThreshold;

		// Check for contact / intersection

		if (isInsideContactPlane)
		{
			if (facecontains(sphereCenter, vertices, normal))
			{
				// Inside the contact wedge - touches a point on the shell plane
				hasContact = true;
				contactPoint = sphereCenter - normal * distanceFromPlane;
			}
			else
			{
				// Could be inside one of the contact capsules
				Scalar contactCapsuleRadiusSqr = radiusWithThreshold * radiusWithThreshold;
				Scalar minDistSqr = contactCapsuleRadiusSqr;
				Vec3 nearestOnEdge;
				for (i32 i = 0; i < m_triangle->getNumEdges(); i++)
				{
					Vec3 pa;
					Vec3 pb;

					m_triangle->getEdge(i, pa, pb);

					Scalar distanceSqr = SegmentSqrDistance(pa, pb, sphereCenter, nearestOnEdge);
					if (distanceSqr < minDistSqr)
					{
						// Yep, we're inside a capsule, and record the capsule with smallest distance
						minDistSqr = distanceSqr;
						hasContact = true;
						contactPoint = nearestOnEdge;
					}
				}
			}
		}
	}

	if (hasContact)
	{
		Vec3 contactToCentre = sphereCenter - contactPoint;
		Scalar distanceSqr = contactToCentre.length2();

		if (distanceSqr < radiusWithThreshold * radiusWithThreshold)
		{
			if (distanceSqr > SIMD_EPSILON)
			{
				Scalar distance = Sqrt(distanceSqr);
				resultNormal = contactToCentre;
				resultNormal.normalize();
				point = contactPoint;
				depth = -(radius - distance);
			}
			else
			{
				resultNormal = normal;
				point = contactPoint;
				depth = -radius;
			}
			return true;
		}
	}

	return false;
}

bool SphereTriangleDetector::pointInTriangle(const Vec3 vertices[], const Vec3& normal, Vec3* p)
{
	const Vec3* p1 = &vertices[0];
	const Vec3* p2 = &vertices[1];
	const Vec3* p3 = &vertices[2];

	Vec3 edge1(*p2 - *p1);
	Vec3 edge2(*p3 - *p2);
	Vec3 edge3(*p1 - *p3);

	Vec3 p1_to_p(*p - *p1);
	Vec3 p2_to_p(*p - *p2);
	Vec3 p3_to_p(*p - *p3);

	Vec3 edge1_normal(edge1.cross(normal));
	Vec3 edge2_normal(edge2.cross(normal));
	Vec3 edge3_normal(edge3.cross(normal));

	Scalar r1, r2, r3;
	r1 = edge1_normal.dot(p1_to_p);
	r2 = edge2_normal.dot(p2_to_p);
	r3 = edge3_normal.dot(p3_to_p);
	if ((r1 > 0 && r2 > 0 && r3 > 0) ||
		(r1 <= 0 && r2 <= 0 && r3 <= 0))
		return true;
	return false;
}
