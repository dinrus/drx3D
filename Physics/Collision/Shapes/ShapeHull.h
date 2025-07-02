#ifndef DRX3D_SHAPE_HULL_H
#define DRX3D_SHAPE_HULL_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>

///The ShapeHull class takes a ConvexShape, builds a simplified convex hull using ConvexHull and provides triangle indices and vertices.
///It can be useful for to simplify a complex convex object and for visualization of a non-polyhedral convex object.
///It approximates the convex hull using the supporting vertex of 42 directions.
ATTRIBUTE_ALIGNED16(class)
ShapeHull
{
protected:
	AlignedObjectArray<Vec3> m_vertices;
	AlignedObjectArray<u32> m_indices;
	u32 m_numIndices;
	const ConvexShape* m_shape;

	static Vec3* getUnitSpherePoints(i32 highres = 0);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ShapeHull(const ConvexShape* shape);
	~ShapeHull();

	bool buildHull(Scalar margin, i32 highres = 0);

	i32 numTriangles() const;
	i32 numVertices() const;
	i32 numIndices() const;

	const Vec3* getVertexPointer() const
	{
		return &m_vertices[0];
	}
	u32k* getIndexPointer() const
	{
		return &m_indices[0];
	}
};

#endif  //DRX3D_SHAPE_HULL_H
