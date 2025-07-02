
#ifndef _DRX3D_CONVEX_UTILITY_H
#define _DRX3D_CONVEX_UTILITY_H

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Transform.h>

struct b3MyFace
{
	b3AlignedObjectArray<i32> m_indices;
	b3Scalar m_plane[4];
};

D3_ATTRIBUTE_ALIGNED16(class)
b3ConvexUtility
{
public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3Vec3 m_localCenter;
	b3Vec3 m_extents;
	b3Vec3 mC;
	b3Vec3 mE;
	b3Scalar m_radius;

	b3AlignedObjectArray<b3Vec3> m_vertices;
	b3AlignedObjectArray<b3MyFace> m_faces;
	b3AlignedObjectArray<b3Vec3> m_uniqueEdges;

	b3ConvexUtility()
	{
	}
	virtual ~b3ConvexUtility();

	bool initializePolyhedralFeatures(const b3Vec3* orgVertices, i32 numVertices, bool mergeCoplanarTriangles = true);

	void initialize();
	bool testContainment() const;
};
#endif
