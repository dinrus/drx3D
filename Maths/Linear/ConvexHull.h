#ifndef DRX3D_CD_HULL_H
#define DRX3D_CD_HULL_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

typedef AlignedObjectArray<u32> TUIntArray;

class HullResult
{
public:
	HullResult(void)
	{
		mPolygons = true;
		mNumOutputVertices = 0;
		mNumFaces = 0;
		mNumIndices = 0;
	}
	bool mPolygons;                                    // true if indices represents polygons, false indices are triangles
	u32 mNumOutputVertices;                   // number of vertices in the output hull
	AlignedObjectArray<Vec3> m_OutputVertices;  // array of vertices
	u32 mNumFaces;                            // the number of faces produced
	u32 mNumIndices;                          // the total number of indices
	AlignedObjectArray<u32> m_Indices;      // pointer to indices.

	// If triangles, then indices are array indexes into the vertex list.
	// If polygons, indices are in the form (number of points in face) (p1, p2, p3, ..) etc..
};

enum HullFlag
{
	QF_TRIANGLES = (1 << 0),      // report results as triangles, not polygons.
	QF_REVERSE_ORDER = (1 << 1),  // reverse order of the triangle indices.
	QF_DEFAULT = QF_TRIANGLES
};

class HullDesc
{
public:
	HullDesc(void)
	{
		mFlags = QF_DEFAULT;
		mVcount = 0;
		mVertices = 0;
		mVertexStride = sizeof(Vec3);
		mNormalEpsilon = 0.001f;
		mMaxVertices = 4096;  // maximum number of points to be considered for a convex hull.
		mMaxFaces = 4096;
	};

	HullDesc(HullFlag flag,
			 u32 vcount,
			 const Vec3* vertices,
			 u32 stride = sizeof(Vec3))
	{
		mFlags = flag;
		mVcount = vcount;
		mVertices = vertices;
		mVertexStride = stride;
		mNormalEpsilon = Scalar(0.001);
		mMaxVertices = 4096;
	}

	bool HasHullFlag(HullFlag flag) const
	{
		if (mFlags & flag) return true;
		return false;
	}

	void SetHullFlag(HullFlag flag)
	{
		mFlags |= flag;
	}

	void ClearHullFlag(HullFlag flag)
	{
		mFlags &= ~flag;
	}

	u32 mFlags;         // flags to use when generating the convex hull.
	u32 mVcount;        // number of vertices in the input point cloud
	const Vec3* mVertices;  // the array of vertices.
	u32 mVertexStride;  // the stride of each vertex, in bytes.
	Scalar mNormalEpsilon;     // the epsilon for removing duplicates.  This is a normalized value, if normalized bit is on.
	u32 mMaxVertices;   // maximum number of vertices to be considered for the hull!
	u32 mMaxFaces;
};

enum HullError
{
	QE_OK,   // success!
	QE_FAIL  // failed.
};

class Plane
{
public:
	Vec3 normal;
	Scalar dist;  // distance below origin - the D from plane equasion Ax+By+Cz+D=0
	Plane(const Vec3& n, Scalar d) : normal(n), dist(d) {}
	Plane() : normal(), dist(0) {}
};

class ConvexH
{
public:
	class HalfEdge
	{
	public:
		short ea;         // the other half of the edge (index into edges list)
		u8 v;  // the vertex at the start of this edge (index into vertices list)
		u8 p;  // the facet on which this edge lies (index into facets list)
		HalfEdge() {}
		HalfEdge(short _ea, u8 _v, u8 _p) : ea(_ea), v(_v), p(_p) {}
	};
	ConvexH()
	{
	}
	~ConvexH()
	{
	}
	AlignedObjectArray<Vec3> vertices;
	AlignedObjectArray<HalfEdge> edges;
	AlignedObjectArray<Plane> facets;
	ConvexH(i32 vertices_size, i32 edges_size, i32 facets_size);
};

class int4
{
public:
	i32 x, y, z, w;
	int4(){};
	int4(i32 _x, i32 _y, i32 _z, i32 _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	i32k& operator[](i32 i) const { return (&x)[i]; }
	i32& operator[](i32 i) { return (&x)[i]; }
};

class PHullResult
{
public:
	PHullResult(void)
	{
		mVcount = 0;
		mIndexCount = 0;
		mFaceCount = 0;
		mVertices = 0;
	}

	u32 mVcount;
	u32 mIndexCount;
	u32 mFaceCount;
	Vec3* mVertices;
	TUIntArray m_Indices;
};

///The HullLibrary class can create a convex hull from a collection of vertices, using the ComputeHull method.
///The ShapeHull class uses this HullLibrary to create a approximate convex mesh given a general (non-polyhedral) convex shape.
class HullLibrary
{
	AlignedObjectArray<class HullTriangle*> m_tris;

public:
	AlignedObjectArray<i32> m_vertexIndexMapping;

	HullError CreateConvexHull(const HullDesc& desc,  // describes the input request
							   HullResult& result);   // contains the resulst
	HullError ReleaseResult(HullResult& result);      // release memory allocated for this result, we are done with it.

private:
	bool ComputeHull(u32 vcount, const Vec3* vertices, PHullResult& result, u32 vlimit);

	class HullTriangle* allocateTriangle(i32 a, i32 b, i32 c);
	void deAllocateTriangle(HullTriangle*);
	void b2bfix(HullTriangle* s, HullTriangle* t);

	void removeb2b(HullTriangle* s, HullTriangle* t);

	void checkit(HullTriangle* t);

	HullTriangle* extrudable(Scalar epsilon);

	i32 calchull(Vec3* verts, i32 verts_count, TUIntArray& tris_out, i32& tris_count, i32 vlimit);

	i32 calchullgen(Vec3* verts, i32 verts_count, i32 vlimit);

	int4 FindSimplex(Vec3* verts, i32 verts_count, AlignedObjectArray<i32>& allow);

	class ConvexH* ConvexHCrop(ConvexH& convex, const Plane& slice);

	void extrude(class HullTriangle* t0, i32 v);

	ConvexH* test_cube();

	//BringOutYourDead (John Ratcliff): When you create a convex hull you hand it a large input set of vertices forming a 'point cloud'.
	//After the hull is generated it give you back a set of polygon faces which index the *original* point cloud.
	//The thing is, often times, there are many 'dead vertices' in the point cloud that are on longer referenced by the hull.
	//The routine 'BringOutYourDead' find only the referenced vertices, copies them to an new buffer, and re-indexes the hull so that it is a minimal representation.
	void BringOutYourDead(const Vec3* verts, u32 vcount, Vec3* overts, u32& ocount, u32* indices, unsigned indexcount);

	bool CleanupVertices(u32 svcount,
						 const Vec3* svertices,
						 u32 stride,
						 u32& vcount,  // output number of vertices
						 Vec3* vertices,   // location to store the results.
						 Scalar normalepsilon,
						 Vec3& scale);
};

#endif  //DRX3D_CD_HULL_H
