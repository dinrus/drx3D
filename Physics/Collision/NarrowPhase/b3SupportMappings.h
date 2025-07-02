#ifndef D3_SUPPORT_MAPPINGS_H
#define D3_SUPPORT_MAPPINGS_H

#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3VecFloat4.h>

struct b3GjkPairDetector;

inline b3Vec3 localGetSupportVertexWithMargin(const float4& supportVec, const struct b3ConvexPolyhedronData* hull,
												 const b3AlignedObjectArray<b3Vec3>& verticesA, b3Scalar margin)
{
	b3Vec3 supVec = b3MakeVector3(b3Scalar(0.), b3Scalar(0.), b3Scalar(0.));
	b3Scalar maxDot = b3Scalar(-D3_LARGE_FLOAT);

	// Here we take advantage of dot(a, b*c) = dot(a*b, c).  Note: This is true mathematically, but not numerically.
	if (0 < hull->m_numVertices)
	{
		const b3Vec3 scaled = supportVec;
		i32 index = (i32)scaled.maxDot(&verticesA[hull->m_vertexOffset], hull->m_numVertices, maxDot);
		return verticesA[hull->m_vertexOffset + index];
	}

	return supVec;
}

inline b3Vec3 localGetSupportVertexWithoutMargin(const float4& supportVec, const struct b3ConvexPolyhedronData* hull,
													const b3AlignedObjectArray<b3Vec3>& verticesA)
{
	return localGetSupportVertexWithMargin(supportVec, hull, verticesA, 0.f);
}

#endif  //D3_SUPPORT_MAPPINGS_H
