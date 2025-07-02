#ifndef DRX3D_OBB_BOX_2D_SHAPE_H
#define DRX3D_OBB_BOX_2D_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/MinMax.h>

///The Box2dShape is a box primitive around the origin, its sides axis aligned with length specified by half extents, in local shape coordinates. When used as part of a CollisionObject2 or RigidBody it will be an oriented box in world space.
ATTRIBUTE_ALIGNED16(class)
Box2dShape : public PolyhedralConvexShape
{
	//Vec3	m_boxHalfExtents1; //use m_implicitShapeDimensions instead

	Vec3 m_centroid;
	Vec3 m_vertices[4];
	Vec3 m_normals[4];

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Vec3 getHalfExtentsWithMargin() const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();
		Vec3 margin(getMargin(), getMargin(), getMargin());
		halfExtents += margin;
		return halfExtents;
	}

	const Vec3& getHalfExtentsWithoutMargin() const
	{
		return m_implicitShapeDimensions;  //changed in drx3D 2.63: assume the scaling and margin are included
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();
		Vec3 margin(getMargin(), getMargin(), getMargin());
		halfExtents += margin;

		return Vec3(Fsels(vec.x(), halfExtents.x(), -halfExtents.x()),
						 Fsels(vec.y(), halfExtents.y(), -halfExtents.y()),
						 Fsels(vec.z(), halfExtents.z(), -halfExtents.z()));
	}

	SIMD_FORCE_INLINE Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const
	{
		const Vec3& halfExtents = getHalfExtentsWithoutMargin();

		return Vec3(Fsels(vec.x(), halfExtents.x(), -halfExtents.x()),
						 Fsels(vec.y(), halfExtents.y(), -halfExtents.y()),
						 Fsels(vec.z(), halfExtents.z(), -halfExtents.z()));
	}

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
	{
		const Vec3& halfExtents = getHalfExtentsWithoutMargin();

		for (i32 i = 0; i < numVectors; i++)
		{
			const Vec3& vec = vectors[i];
			supportVerticesOut[i].setVal(Fsels(vec.x(), halfExtents.x(), -halfExtents.x()),
										   Fsels(vec.y(), halfExtents.y(), -halfExtents.y()),
										   Fsels(vec.z(), halfExtents.z(), -halfExtents.z()));
		}
	}

	///a Box2dShape is a flat 2D box in the X-Y plane (Z extents are zero)
	Box2dShape(const Vec3& boxHalfExtents)
		: PolyhedralConvexShape(),
		  m_centroid(0, 0, 0)
	{
		m_vertices[0].setVal(-boxHalfExtents.getX(), -boxHalfExtents.getY(), 0);
		m_vertices[1].setVal(boxHalfExtents.getX(), -boxHalfExtents.getY(), 0);
		m_vertices[2].setVal(boxHalfExtents.getX(), boxHalfExtents.getY(), 0);
		m_vertices[3].setVal(-boxHalfExtents.getX(), boxHalfExtents.getY(), 0);

		m_normals[0].setVal(0, -1, 0);
		m_normals[1].setVal(1, 0, 0);
		m_normals[2].setVal(0, 1, 0);
		m_normals[3].setVal(-1, 0, 0);

		Scalar minDimension = boxHalfExtents.getX();
		if (minDimension > boxHalfExtents.getY())
			minDimension = boxHalfExtents.getY();

		m_shapeType = BOX_2D_SHAPE_PROXYTYPE;
		Vec3 margin(getMargin(), getMargin(), getMargin());
		m_implicitShapeDimensions = (boxHalfExtents * m_localScaling) - margin;

		setSafeMargin(minDimension);
	};

	virtual void setMargin(Scalar collisionMargin)
	{
		//correct the m_implicitShapeDimensions for the margin
		Vec3 oldMargin(getMargin(), getMargin(), getMargin());
		Vec3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions + oldMargin;

		ConvexInternalShape::setMargin(collisionMargin);
		Vec3 newMargin(getMargin(), getMargin(), getMargin());
		m_implicitShapeDimensions = implicitShapeDimensionsWithMargin - newMargin;
	}
	virtual void setLocalScaling(const Vec3& scaling)
	{
		Vec3 oldMargin(getMargin(), getMargin(), getMargin());
		Vec3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions + oldMargin;
		Vec3 unScaledImplicitShapeDimensionsWithMargin = implicitShapeDimensionsWithMargin / m_localScaling;

		ConvexInternalShape::setLocalScaling(scaling);

		m_implicitShapeDimensions = (unScaledImplicitShapeDimensionsWithMargin * m_localScaling) - oldMargin;
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	i32 getVertexCount() const
	{
		return 4;
	}

	virtual i32 getNumVertices() const
	{
		return 4;
	}

	const Vec3* getVertices() const
	{
		return &m_vertices[0];
	}

	const Vec3* getNormals() const
	{
		return &m_normals[0];
	}

	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const
	{
		//this plane might not be aligned...
		Vec4 plane;
		getPlaneEquation(plane, i);
		planeNormal = Vec3(plane.getX(), plane.getY(), plane.getZ());
		planeSupport = localGetSupportingVertex(-planeNormal);
	}

	const Vec3& getCentroid() const
	{
		return m_centroid;
	}

	virtual i32 getNumPlanes() const
	{
		return 6;
	}

	virtual i32 getNumEdges() const
	{
		return 12;
	}

	virtual void getVertex(i32 i, Vec3& vtx) const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();

		vtx = Vec3(
			halfExtents.x() * (1 - (i & 1)) - halfExtents.x() * (i & 1),
			halfExtents.y() * (1 - ((i & 2) >> 1)) - halfExtents.y() * ((i & 2) >> 1),
			halfExtents.z() * (1 - ((i & 4) >> 2)) - halfExtents.z() * ((i & 4) >> 2));
	}

	virtual void getPlaneEquation(Vec4 & plane, i32 i) const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();

		switch (i)
		{
			case 0:
				plane.setVal(Scalar(1.), Scalar(0.), Scalar(0.), -halfExtents.x());
				break;
			case 1:
				plane.setVal(Scalar(-1.), Scalar(0.), Scalar(0.), -halfExtents.x());
				break;
			case 2:
				plane.setVal(Scalar(0.), Scalar(1.), Scalar(0.), -halfExtents.y());
				break;
			case 3:
				plane.setVal(Scalar(0.), Scalar(-1.), Scalar(0.), -halfExtents.y());
				break;
			case 4:
				plane.setVal(Scalar(0.), Scalar(0.), Scalar(1.), -halfExtents.z());
				break;
			case 5:
				plane.setVal(Scalar(0.), Scalar(0.), Scalar(-1.), -halfExtents.z());
				break;
			default:
				Assert(0);
		}
	}

	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const
	//virtual void getEdge(i32 i,Edge& edge) const
	{
		i32 edgeVert0 = 0;
		i32 edgeVert1 = 0;

		switch (i)
		{
			case 0:
				edgeVert0 = 0;
				edgeVert1 = 1;
				break;
			case 1:
				edgeVert0 = 0;
				edgeVert1 = 2;
				break;
			case 2:
				edgeVert0 = 1;
				edgeVert1 = 3;

				break;
			case 3:
				edgeVert0 = 2;
				edgeVert1 = 3;
				break;
			case 4:
				edgeVert0 = 0;
				edgeVert1 = 4;
				break;
			case 5:
				edgeVert0 = 1;
				edgeVert1 = 5;

				break;
			case 6:
				edgeVert0 = 2;
				edgeVert1 = 6;
				break;
			case 7:
				edgeVert0 = 3;
				edgeVert1 = 7;
				break;
			case 8:
				edgeVert0 = 4;
				edgeVert1 = 5;
				break;
			case 9:
				edgeVert0 = 4;
				edgeVert1 = 6;
				break;
			case 10:
				edgeVert0 = 5;
				edgeVert1 = 7;
				break;
			case 11:
				edgeVert0 = 6;
				edgeVert1 = 7;
				break;
			default:
				Assert(0);
		}

		getVertex(edgeVert0, pa);
		getVertex(edgeVert1, pb);
	}

	virtual bool isInside(const Vec3& pt, Scalar tolerance) const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();

		//Scalar minDist = 2*tolerance;

		bool result = (pt.x() <= (halfExtents.x() + tolerance)) &&
					  (pt.x() >= (-halfExtents.x() - tolerance)) &&
					  (pt.y() <= (halfExtents.y() + tolerance)) &&
					  (pt.y() >= (-halfExtents.y() - tolerance)) &&
					  (pt.z() <= (halfExtents.z() + tolerance)) &&
					  (pt.z() >= (-halfExtents.z() - tolerance));

		return result;
	}

	//debugging
	virtual tukk getName() const
	{
		return "Box2d";
	}

	virtual i32 getNumPreferredPenetrationDirections() const
	{
		return 6;
	}

	virtual void getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const
	{
		switch (index)
		{
			case 0:
				penetrationVector.setVal(Scalar(1.), Scalar(0.), Scalar(0.));
				break;
			case 1:
				penetrationVector.setVal(Scalar(-1.), Scalar(0.), Scalar(0.));
				break;
			case 2:
				penetrationVector.setVal(Scalar(0.), Scalar(1.), Scalar(0.));
				break;
			case 3:
				penetrationVector.setVal(Scalar(0.), Scalar(-1.), Scalar(0.));
				break;
			case 4:
				penetrationVector.setVal(Scalar(0.), Scalar(0.), Scalar(1.));
				break;
			case 5:
				penetrationVector.setVal(Scalar(0.), Scalar(0.), Scalar(-1.));
				break;
			default:
				Assert(0);
		}
	}
};

#endif  //DRX3D_OBB_BOX_2D_SHAPE_H
