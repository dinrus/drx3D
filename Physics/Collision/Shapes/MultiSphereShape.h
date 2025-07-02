#ifndef DRX3D_MULTI_SPHERE_MINKOWSKI_H
#define DRX3D_MULTI_SPHERE_MINKOWSKI_H

#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

///The MultiSphereShape represents the convex hull of a collection of spheres. You can create special capsules or other smooth volumes.
///It is possible to animate the spheres for deformation, but call 'recalcLocalAabb' after changing any sphere position/radius
ATTRIBUTE_ALIGNED16(class)
MultiSphereShape : public ConvexInternalAabbCachingShape
{
	AlignedObjectArray<Vec3> m_localPositionArray;
	AlignedObjectArray<Scalar> m_radiArray;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres);

	///CollisionShape Interface
	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	/// ConvexShape Interface
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	i32 getSphereCount() const
	{
		return m_localPositionArray.size();
	}

	const Vec3& getSpherePosition(i32 index) const
	{
		return m_localPositionArray[index];
	}

	Scalar getSphereRadius(i32 index) const
	{
		return m_radiArray[index];
	}

	virtual tukk getName() const
	{
		return "MultiSphere";
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

struct PositionAndRadius
{
	Vec3FloatData m_pos;
	float m_radius;
};

// clang-format off

struct	MultiSphereShapeData
{
	ConvexInternalShapeData	m_convexInternalShapeData;

	PositionAndRadius	*m_localPositionArrayPtr;
	i32				m_localPositionArraySize;
	char	m_padding[4];
};

// clang-format on

SIMD_FORCE_INLINE i32 MultiSphereShape::calculateSerializeBufferSize() const
{
	return sizeof(MultiSphereShapeData);
}

#endif  //DRX3D_MULTI_SPHERE_MINKOWSKI_H
