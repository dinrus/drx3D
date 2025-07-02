#ifndef D3_STRIDING_MESHINTERFACE_H
#define D3_STRIDING_MESHINTERFACE_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3TriangleCallback.h>
//#include <drx3D/Physics/Collision/NarrowPhase/b3ConcaveShape.h>

enum PHY_ScalarType
{
	PHY_FLOAT,
	PHY_DOUBLE,
	PHY_INTEGER,
	PHY_SHORT,
	PHY_FIXEDPOINT88,
	PHY_UCHAR
};

///	The b3StridingMeshInterface is the interface class for high performance generic access to triangle meshes, used in combination with b3BvhTriangleMeshShape and some other collision shapes.
/// Using index striding of 3*sizeof(integer) it can use triangle arrays, using index striding of 1*sizeof(integer) it can handle triangle strips.
/// It allows for sharing graphics and collision meshes. Also it provides locking/unlocking of graphics meshes that are in gpu memory.
D3_ATTRIBUTE_ALIGNED16(class)
b3StridingMeshInterface
{
protected:
	b3Vec3 m_scaling;

public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3StridingMeshInterface() : m_scaling(b3MakeVector3(b3Scalar(1.), b3Scalar(1.), b3Scalar(1.)))
	{
	}

	virtual ~b3StridingMeshInterface();

	virtual void InternalProcessAllTriangles(b3InternalTriangleIndexCallback * callback, const b3Vec3& aabbMin, const b3Vec3& aabbMax) const;

	///brute force method to calculate aabb
	void calculateAabbBruteForce(b3Vec3 & aabbMin, b3Vec3 & aabbMax);

	/// get read and write access to a subpart of a triangle mesh
	/// this subpart has a continuous array of vertices and indices
	/// in this way the mesh can be handled as chunks of memory with striding
	/// very similar to OpenGL vertexarray support
	/// make a call to unLockVertexBase when the read and write access is finished
	virtual void getLockedVertexIndexBase(u8** vertexbase, i32& numverts, PHY_ScalarType& type, i32& stride, u8** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart = 0) = 0;

	virtual void getLockedReadOnlyVertexIndexBase(u8k** vertexbase, i32& numverts, PHY_ScalarType& type, i32& stride, u8k** indexbase, i32& indexstride, i32& numfaces, PHY_ScalarType& indicestype, i32 subpart = 0) const = 0;

	/// unLockVertexBase finishes the access to a subpart of the triangle mesh
	/// make a call to unLockVertexBase when the read and write access (using getLockedVertexIndexBase) is finished
	virtual void unLockVertexBase(i32 subpart) = 0;

	virtual void unLockReadOnlyVertexBase(i32 subpart) const = 0;

	/// getNumSubParts returns the number of separate subparts
	/// each subpart has a continuous array of vertices and indices
	virtual i32 getNumSubParts() const = 0;

	virtual void preallocateVertices(i32 numverts) = 0;
	virtual void preallocateIndices(i32 numindices) = 0;

	virtual bool hasPremadeAabb() const { return false; }
	virtual void setPremadeAabb(const b3Vec3& aabbMin, const b3Vec3& aabbMax) const
	{
		(void)aabbMin;
		(void)aabbMax;
	}
	virtual void getPremadeAabb(b3Vec3 * aabbMin, b3Vec3 * aabbMax) const
	{
		(void)aabbMin;
		(void)aabbMax;
	}

	const b3Vec3& getScaling() const
	{
		return m_scaling;
	}
	void setScaling(const b3Vec3& scaling)
	{
		m_scaling = scaling;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	//virtual	tukk 	serialize(uk dataBuffer, b3Serializer* serializer) const;
};

struct b3IntIndexData
{
	i32 m_value;
};

struct b3ShortIntIndexData
{
	short m_value;
	char m_pad[2];
};

struct b3ShortIntIndexTripletData
{
	short m_values[3];
	char m_pad[2];
};

struct b3CharIndexTripletData
{
	u8 m_values[3];
	char m_pad;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3MeshPartData
{
	b3Vec3FloatData* m_vertices3f;
	b3Vec3DoubleData* m_vertices3d;

	b3IntIndexData* m_indices32;
	b3ShortIntIndexTripletData* m_3indices16;
	b3CharIndexTripletData* m_3indices8;

	b3ShortIntIndexData* m_indices16;  //backwards compatibility

	i32 m_numTriangles;  //length of m_indices = m_numTriangles
	i32 m_numVertices;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3StridingMeshInterfaceData
{
	b3MeshPartData* m_meshPartsPtr;
	b3Vec3FloatData m_scaling;
	i32 m_numMeshParts;
	char m_padding[4];
};

D3_FORCE_INLINE i32 b3StridingMeshInterface::calculateSerializeBufferSize() const
{
	return sizeof(b3StridingMeshInterfaceData);
}

#endif  //D3_STRIDING_MESHINTERFACE_H
