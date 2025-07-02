
#include <drx3D/Physics/Collision/Shapes/TriangleBuffer.h>

void TriangleBuffer::processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
{
	Triangle tri;
	tri.m_vertex0 = triangle[0];
	tri.m_vertex1 = triangle[1];
	tri.m_vertex2 = triangle[2];
	tri.m_partId = partId;
	tri.m_triangleIndex = triangleIndex;

	m_triangleBuffer.push_back(tri);
}
