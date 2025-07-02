#ifndef DRX3D_TRIANGLE_BUFFER_H
#define DRX3D_TRIANGLE_BUFFER_H

#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct Triangle
{
	Vec3 m_vertex0;
	Vec3 m_vertex1;
	Vec3 m_vertex2;
	i32 m_partId;
	i32 m_triangleIndex;
};

///The TriangleBuffer callback can be useful to collect and store overlapping triangles between AABB and concave objects that support 'processAllTriangles'
///Example usage of this class:
///			TriangleBuffer	triBuf;
///			concaveShape->processAllTriangles(&triBuf,aabbMin, aabbMax);
///			for (i32 i=0;i<triBuf.getNumTriangles();i++)
///			{
///				const Triangle& tri = triBuf.getTriangle(i);
///				//do something useful here with the triangle
///			}
class TriangleBuffer : public TriangleCallback
{
	AlignedObjectArray<Triangle> m_triangleBuffer;

public:
	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex);

	i32 getNumTriangles() const
	{
		return i32(m_triangleBuffer.size());
	}

	const Triangle& getTriangle(i32 index) const
	{
		return m_triangleBuffer[index];
	}

	void clearBuffer()
	{
		m_triangleBuffer.clear();
	}
};

#endif  //DRX3D_TRIANGLE_BUFFER_H
