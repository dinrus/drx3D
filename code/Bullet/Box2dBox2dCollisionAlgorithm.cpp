
#include <drx3D/Physics/Collision/Dispatch/Box2dBox2dCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/BoxBoxDetector.h>
#include <drx3D/Physics/Collision/Shapes/Box2dShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

#define USE_PERSISTENT_CONTACTS 1

Box2dBox2dCollisionAlgorithm::Box2dBox2dCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* obj0Wrap, const CollisionObject2Wrapper* obj1Wrap)
	: ActivatingCollisionAlgorithm(ci, obj0Wrap, obj1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf)
{
	if (!m_manifoldPtr && m_dispatcher->needsCollision(obj0Wrap->getCollisionObject(), obj1Wrap->getCollisionObject()))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(obj0Wrap->getCollisionObject(), obj1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
}

Box2dBox2dCollisionAlgorithm::~Box2dBox2dCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void b2CollidePolygons(ManifoldResult* manifold, const Box2dShape* polyA, const Transform2& xfA, const Box2dShape* polyB, const Transform2& xfB);

//#include <stdio.h>
void Box2dBox2dCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
		return;

	const Box2dShape* box0 = (const Box2dShape*)body0Wrap->getCollisionShape();
	const Box2dShape* box1 = (const Box2dShape*)body1Wrap->getCollisionShape();

	resultOut->setPersistentManifold(m_manifoldPtr);

	b2CollidePolygons(resultOut, box0, body0Wrap->getWorldTransform(), box1, body1Wrap->getWorldTransform());

	//  refreshContactPoints is only necessary when using persistent contact points. otherwise all points are newly added
	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
}

Scalar Box2dBox2dCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* /*body0*/, CollisionObject2* /*body1*/, const DispatcherInfo& /*dispatchInfo*/, ManifoldResult* /*resultOut*/)
{
	//not yet
	return 1.f;
}

struct ClipVertex
{
	Vec3 v;
	i32 id;
	//b2ContactID id;
	//b2ContactID id;
};

#define b2Dot(a, b) (a).dot(b)
#define b2Mul(a, b) (a) * (b)
#define b2MulT(a, b) (a).transpose() * (b)
#define b2Cross(a, b) (a).cross(b)
#define CrossS(a, s) Vec3(s* a.getY(), -s* a.getX(), 0.f)

i32 b2_maxManifoldPoints = 2;

static i32 ClipSegmentToLine(ClipVertex vOut[2], ClipVertex vIn[2],
							 const Vec3& normal, Scalar offset)
{
	// Start with no output points
	i32 numOut = 0;

	// Calculate the distance of end points to the line
	Scalar distance0 = b2Dot(normal, vIn[0].v) - offset;
	Scalar distance1 = b2Dot(normal, vIn[1].v) - offset;

	// If the points are behind the plane
	if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
	if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

	// If the points are on different sides of the plane
	if (distance0 * distance1 < 0.0f)
	{
		// Find intersection point of edge and plane
		Scalar interp = distance0 / (distance0 - distance1);
		vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v);
		if (distance0 > 0.0f)
		{
			vOut[numOut].id = vIn[0].id;
		}
		else
		{
			vOut[numOut].id = vIn[1].id;
		}
		++numOut;
	}

	return numOut;
}

// Find the separation between poly1 and poly2 for a give edge normal on poly1.
static Scalar EdgeSeparation(const Box2dShape* poly1, const Transform2& xf1, i32 edge1,
							   const Box2dShape* poly2, const Transform2& xf2)
{
	const Vec3* vertices1 = poly1->getVertices();
	const Vec3* normals1 = poly1->getNormals();

	i32 count2 = poly2->getVertexCount();
	const Vec3* vertices2 = poly2->getVertices();

	Assert(0 <= edge1 && edge1 < poly1->getVertexCount());

	// Convert normal from poly1's frame into poly2's frame.
	Vec3 normal1World = b2Mul(xf1.getBasis(), normals1[edge1]);
	Vec3 normal1 = b2MulT(xf2.getBasis(), normal1World);

	// Find support vertex on poly2 for -normal.
	i32 index = 0;
	Scalar minDot = DRX3D_LARGE_FLOAT;

	if (count2 > 0)
		index = (i32)normal1.minDot(vertices2, count2, minDot);

	Vec3 v1 = b2Mul(xf1, vertices1[edge1]);
	Vec3 v2 = b2Mul(xf2, vertices2[index]);
	Scalar separation = b2Dot(v2 - v1, normal1World);
	return separation;
}

// Find the max separation between poly1 and poly2 using edge normals from poly1.
static Scalar FindMaxSeparation(i32* edgeIndex,
								  const Box2dShape* poly1, const Transform2& xf1,
								  const Box2dShape* poly2, const Transform2& xf2)
{
	i32 count1 = poly1->getVertexCount();
	const Vec3* normals1 = poly1->getNormals();

	// Vector pointing from the centroid of poly1 to the centroid of poly2.
	Vec3 d = b2Mul(xf2, poly2->getCentroid()) - b2Mul(xf1, poly1->getCentroid());
	Vec3 dLocal1 = b2MulT(xf1.getBasis(), d);

	// Find edge normal on poly1 that has the largest projection onto d.
	i32 edge = 0;
	Scalar maxDot;
	if (count1 > 0)
		edge = (i32)dLocal1.maxDot(normals1, count1, maxDot);

	// Get the separation for the edge normal.
	Scalar s = EdgeSeparation(poly1, xf1, edge, poly2, xf2);
	if (s > 0.0f)
	{
		return s;
	}

	// Check the separation for the previous edge normal.
	i32 prevEdge = edge - 1 >= 0 ? edge - 1 : count1 - 1;
	Scalar sPrev = EdgeSeparation(poly1, xf1, prevEdge, poly2, xf2);
	if (sPrev > 0.0f)
	{
		return sPrev;
	}

	// Check the separation for the next edge normal.
	i32 nextEdge = edge + 1 < count1 ? edge + 1 : 0;
	Scalar sNext = EdgeSeparation(poly1, xf1, nextEdge, poly2, xf2);
	if (sNext > 0.0f)
	{
		return sNext;
	}

	// Find the best edge and the search direction.
	i32 bestEdge;
	Scalar bestSeparation;
	i32 increment;
	if (sPrev > s && sPrev > sNext)
	{
		increment = -1;
		bestEdge = prevEdge;
		bestSeparation = sPrev;
	}
	else if (sNext > s)
	{
		increment = 1;
		bestEdge = nextEdge;
		bestSeparation = sNext;
	}
	else
	{
		*edgeIndex = edge;
		return s;
	}

	// Perform a local search for the best edge normal.
	for (;;)
	{
		if (increment == -1)
			edge = bestEdge - 1 >= 0 ? bestEdge - 1 : count1 - 1;
		else
			edge = bestEdge + 1 < count1 ? bestEdge + 1 : 0;

		s = EdgeSeparation(poly1, xf1, edge, poly2, xf2);
		if (s > 0.0f)
		{
			return s;
		}

		if (s > bestSeparation)
		{
			bestEdge = edge;
			bestSeparation = s;
		}
		else
		{
			break;
		}
	}

	*edgeIndex = bestEdge;
	return bestSeparation;
}

static void FindIncidentEdge(ClipVertex c[2],
							 const Box2dShape* poly1, const Transform2& xf1, i32 edge1,
							 const Box2dShape* poly2, const Transform2& xf2)
{
	const Vec3* normals1 = poly1->getNormals();

	i32 count2 = poly2->getVertexCount();
	const Vec3* vertices2 = poly2->getVertices();
	const Vec3* normals2 = poly2->getNormals();

	Assert(0 <= edge1 && edge1 < poly1->getVertexCount());

	// Get the normal of the reference edge in poly2's frame.
	Vec3 normal1 = b2MulT(xf2.getBasis(), b2Mul(xf1.getBasis(), normals1[edge1]));

	// Find the incident edge on poly2.
	i32 index = 0;
	Scalar minDot = DRX3D_LARGE_FLOAT;
	for (i32 i = 0; i < count2; ++i)
	{
		Scalar dot = b2Dot(normal1, normals2[i]);
		if (dot < minDot)
		{
			minDot = dot;
			index = i;
		}
	}

	// Build the clip vertices for the incident edge.
	i32 i1 = index;
	i32 i2 = i1 + 1 < count2 ? i1 + 1 : 0;

	c[0].v = b2Mul(xf2, vertices2[i1]);
	//	c[0].id.features.referenceEdge = (u8)edge1;
	//	c[0].id.features.incidentEdge = (u8)i1;
	//	c[0].id.features.incidentVertex = 0;

	c[1].v = b2Mul(xf2, vertices2[i2]);
	//	c[1].id.features.referenceEdge = (u8)edge1;
	//	c[1].id.features.incidentEdge = (u8)i2;
	//	c[1].id.features.incidentVertex = 1;
}

// Find edge normal of max separation on A - return if separating axis is found
// Find edge normal of max separation on B - return if separation axis is found
// Choose reference edge as min(minA, minB)
// Find incident edge
// Clip

// The normal points from 1 to 2
void b2CollidePolygons(ManifoldResult* manifold,
					   const Box2dShape* polyA, const Transform2& xfA,
					   const Box2dShape* polyB, const Transform2& xfB)
{
	i32 edgeA = 0;
	Scalar separationA = FindMaxSeparation(&edgeA, polyA, xfA, polyB, xfB);
	if (separationA > 0.0f)
		return;

	i32 edgeB = 0;
	Scalar separationB = FindMaxSeparation(&edgeB, polyB, xfB, polyA, xfA);
	if (separationB > 0.0f)
		return;

	const Box2dShape* poly1;  // reference poly
	const Box2dShape* poly2;  // incident poly
	Transform2 xf1, xf2;
	i32 edge1;  // reference edge
	u8 flip;
	const Scalar k_relativeTol = 0.98f;
	const Scalar k_absoluteTol = 0.001f;

	// TODO_ERIN use "radius" of poly for absolute tolerance.
	if (separationB > k_relativeTol * separationA + k_absoluteTol)
	{
		poly1 = polyB;
		poly2 = polyA;
		xf1 = xfB;
		xf2 = xfA;
		edge1 = edgeB;
		flip = 1;
	}
	else
	{
		poly1 = polyA;
		poly2 = polyB;
		xf1 = xfA;
		xf2 = xfB;
		edge1 = edgeA;
		flip = 0;
	}

	ClipVertex incidentEdge[2];
	FindIncidentEdge(incidentEdge, poly1, xf1, edge1, poly2, xf2);

	i32 count1 = poly1->getVertexCount();
	const Vec3* vertices1 = poly1->getVertices();

	Vec3 v11 = vertices1[edge1];
	Vec3 v12 = edge1 + 1 < count1 ? vertices1[edge1 + 1] : vertices1[0];

	//Vec3 dv = v12 - v11;
	Vec3 sideNormal = b2Mul(xf1.getBasis(), v12 - v11);
	sideNormal.normalize();
	Vec3 frontNormal = CrossS(sideNormal, 1.0f);

	v11 = b2Mul(xf1, v11);
	v12 = b2Mul(xf1, v12);

	Scalar frontOffset = b2Dot(frontNormal, v11);
	Scalar sideOffset1 = -b2Dot(sideNormal, v11);
	Scalar sideOffset2 = b2Dot(sideNormal, v12);

	// Clip incident edge against extruded edge1 side edges.
	ClipVertex clipPoints1[2];
	clipPoints1[0].v.setVal(0, 0, 0);
	clipPoints1[1].v.setVal(0, 0, 0);

	ClipVertex clipPoints2[2];
	clipPoints2[0].v.setVal(0, 0, 0);
	clipPoints2[1].v.setVal(0, 0, 0);

	i32 np;

	// Clip to box side 1
	np = ClipSegmentToLine(clipPoints1, incidentEdge, -sideNormal, sideOffset1);

	if (np < 2)
		return;

	// Clip to negative box side 1
	np = ClipSegmentToLine(clipPoints2, clipPoints1, sideNormal, sideOffset2);

	if (np < 2)
	{
		return;
	}

	// Now clipPoints2 contains the clipped points.
	Vec3 manifoldNormal = flip ? -frontNormal : frontNormal;

	i32 pointCount = 0;
	for (i32 i = 0; i < b2_maxManifoldPoints; ++i)
	{
		Scalar separation = b2Dot(frontNormal, clipPoints2[i].v) - frontOffset;

		if (separation <= 0.0f)
		{
			//b2ManifoldPoint* cp = manifold->points + pointCount;
			//Scalar separation = separation;
			//cp->localPoint1 = b2MulT(xfA, clipPoints2[i].v);
			//cp->localPoint2 = b2MulT(xfB, clipPoints2[i].v);

			manifold->addContactPoint(-manifoldNormal, clipPoints2[i].v, separation);

			//			cp->id = clipPoints2[i].id;
			//			cp->id.features.flip = flip;
			++pointCount;
		}
	}

	//	manifold->pointCount = pointCount;}
}
