#ifndef _DRX3D_POLYHEDRAL_FEATURES_H
#define _DRX3D_POLYHEDRAL_FEATURES_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#define TEST_INTERNAL_OBJECTS 1

struct Face
{
	AlignedObjectArray<i32> m_indices;
	//	AlignedObjectArray<i32>	m_connectedFaces;
	Scalar m_plane[4];
};

ATTRIBUTE_ALIGNED16(class)
ConvexPolyhedron
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConvexPolyhedron();
	virtual ~ConvexPolyhedron();

	AlignedObjectArray<Vec3> m_vertices;
	AlignedObjectArray<Face> m_faces;
	AlignedObjectArray<Vec3> m_uniqueEdges;

	Vec3 m_localCenter;
	Vec3 m_extents;
	Scalar m_radius;
	Vec3 mC;
	Vec3 mE;

	void initialize();
	void initialize2();
	bool testContainment() const;

	void project(const Transform2& trans, const Vec3& dir, Scalar& minProj, Scalar& maxProj, Vec3& witnesPtMin, Vec3& witnesPtMax) const;
};

#endif  //_DRX3D_POLYHEDRAL_FEATURES_H
