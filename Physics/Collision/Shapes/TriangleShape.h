#ifndef DRX3D_OBB_TRIANGLE_MINKOWSKI_H
#define DRX3D_OBB_TRIANGLE_MINKOWSKI_H

#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>

ATTRIBUTE_ALIGNED16(class)
TriangleShape : public PolyhedralConvexShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Vec3 m_vertices1[3];

	virtual i32 getNumVertices() const
	{
		return 3;
	}

	Vec3& getVertexPtr(i32 index)
	{
		return m_vertices1[index];
	}

	const Vec3& getVertexPtr(i32 index) const
	{
		return m_vertices1[index];
	}
	virtual void getVertex(i32 index, Vec3& vert) const
	{
		vert = m_vertices1[index];
	}

	virtual i32 getNumEdges() const
	{
		return 3;
	}

	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const
	{
		getVertex(i, pa);
		getVertex((i + 1) % 3, pb);
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		//		Assert(0);
		getAabbSlow(t, aabbMin, aabbMax);
	}

	Vec3 localGetSupportingVertexWithoutMargin(const Vec3& dir) const
	{
		Vec3 dots = dir.dot3(m_vertices1[0], m_vertices1[1], m_vertices1[2]);
		return m_vertices1[dots.maxAxis()];
	}

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
	{
		for (i32 i = 0; i < numVectors; i++)
		{
			const Vec3& dir = vectors[i];
			Vec3 dots = dir.dot3(m_vertices1[0], m_vertices1[1], m_vertices1[2]);
			supportVerticesOut[i] = m_vertices1[dots.maxAxis()];
		}
	}

	TriangleShape() : PolyhedralConvexShape()
	{
		m_shapeType = TRIANGLE_SHAPE_PROXYTYPE;
	}

	TriangleShape(const Vec3& p0, const Vec3& p1, const Vec3& p2) : PolyhedralConvexShape()
	{
		m_shapeType = TRIANGLE_SHAPE_PROXYTYPE;
		m_vertices1[0] = p0;
		m_vertices1[1] = p1;
		m_vertices1[2] = p2;
	}

	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const
	{
		getPlaneEquation(i, planeNormal, planeSupport);
	}

	virtual i32 getNumPlanes() const
	{
		return 1;
	}

	void calcNormal(Vec3 & normal) const
	{
		normal = (m_vertices1[1] - m_vertices1[0]).cross(m_vertices1[2] - m_vertices1[0]);
		normal.normalize();
	}

	virtual void getPlaneEquation(i32 i, Vec3& planeNormal, Vec3& planeSupport) const
	{
		(void)i;
		calcNormal(planeNormal);
		planeSupport = m_vertices1[0];
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const
	{
		(void)mass;
		Assert(0);
		inertia.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
	}

	virtual bool isInside(const Vec3& pt, Scalar tolerance) const
	{
		Vec3 normal;
		calcNormal(normal);
		//distance to plane
		Scalar dist = pt.dot(normal);
		Scalar planeconst = m_vertices1[0].dot(normal);
		dist -= planeconst;
		if (dist >= -tolerance && dist <= tolerance)
		{
			//inside check on edge-planes
			i32 i;
			for (i = 0; i < 3; i++)
			{
				Vec3 pa, pb;
				getEdge(i, pa, pb);
				Vec3 edge = pb - pa;
				Vec3 edgeNormal = edge.cross(normal);
				edgeNormal.normalize();
				Scalar dist = pt.dot(edgeNormal);
				Scalar edgeConst = pa.dot(edgeNormal);
				dist -= edgeConst;
				if (dist < -tolerance)
					return false;
			}

			return true;
		}

		return false;
	}
	//debugging
	virtual tukk getName() const
	{
		return "Triangle";
	}

	virtual i32 getNumPreferredPenetrationDirections() const
	{
		return 2;
	}

	virtual void getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const
	{
		calcNormal(penetrationVector);
		if (index)
			penetrationVector *= Scalar(-1.);
	}
};

#endif  //DRX3D_OBB_TRIANGLE_MINKOWSKI_H
