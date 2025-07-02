#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>
#include <drx3D/Maths/Linear/HashMap.h>

ConvexPolyhedron::ConvexPolyhedron()
{
}
ConvexPolyhedron::~ConvexPolyhedron()
{
}

inline bool IsAlmostZero1(const Vec3& v)
{
	if (Fabs(v.x()) > 1e-6 || Fabs(v.y()) > 1e-6 || Fabs(v.z()) > 1e-6) return false;
	return true;
}

struct InternalVertexPair
{
	InternalVertexPair(i16 v0, i16 v1)
		: m_v0(v0),
		  m_v1(v1)
	{
		if (m_v1 > m_v0)
			Swap(m_v0, m_v1);
	}
	i16 m_v0;
	i16 m_v1;
	i32 getHash() const
	{
		return m_v0 + (m_v1 << 16);
	}
	bool equals(const InternalVertexPair& other) const
	{
		return m_v0 == other.m_v0 && m_v1 == other.m_v1;
	}
};

struct InternalEdge
{
	InternalEdge()
		: m_face0(-1),
		  m_face1(-1)
	{
	}
	i16 m_face0;
	i16 m_face1;
};

//

#ifdef TEST_INTERNAL_OBJECTS
bool ConvexPolyhedron::testContainment() const
{
	for (i32 p = 0; p < 8; p++)
	{
		Vec3 LocalPt;
		if (p == 0)
			LocalPt = m_localCenter + Vec3(m_extents[0], m_extents[1], m_extents[2]);
		else if (p == 1)
			LocalPt = m_localCenter + Vec3(m_extents[0], m_extents[1], -m_extents[2]);
		else if (p == 2)
			LocalPt = m_localCenter + Vec3(m_extents[0], -m_extents[1], m_extents[2]);
		else if (p == 3)
			LocalPt = m_localCenter + Vec3(m_extents[0], -m_extents[1], -m_extents[2]);
		else if (p == 4)
			LocalPt = m_localCenter + Vec3(-m_extents[0], m_extents[1], m_extents[2]);
		else if (p == 5)
			LocalPt = m_localCenter + Vec3(-m_extents[0], m_extents[1], -m_extents[2]);
		else if (p == 6)
			LocalPt = m_localCenter + Vec3(-m_extents[0], -m_extents[1], m_extents[2]);
		else if (p == 7)
			LocalPt = m_localCenter + Vec3(-m_extents[0], -m_extents[1], -m_extents[2]);

		for (i32 i = 0; i < m_faces.size(); i++)
		{
			const Vec3 Normal(m_faces[i].m_plane[0], m_faces[i].m_plane[1], m_faces[i].m_plane[2]);
			const Scalar d = LocalPt.dot(Normal) + m_faces[i].m_plane[3];
			if (d > 0.0f)
				return false;
		}
	}
	return true;
}
#endif

void ConvexPolyhedron::initialize()
{
	HashMap<InternalVertexPair, InternalEdge> edges;

	for (i32 i = 0; i < m_faces.size(); i++)
	{
		i32 numVertices = m_faces[i].m_indices.size();
		i32 NbTris = numVertices;
		for (i32 j = 0; j < NbTris; j++)
		{
			i32 k = (j + 1) % numVertices;
			InternalVertexPair vp(m_faces[i].m_indices[j], m_faces[i].m_indices[k]);
			InternalEdge* edptr = edges.find(vp);
			Vec3 edge = m_vertices[vp.m_v1] - m_vertices[vp.m_v0];
			edge.normalize();

			bool found = false;

			for (i32 p = 0; p < m_uniqueEdges.size(); p++)
			{
				if (IsAlmostZero1(m_uniqueEdges[p] - edge) ||
					IsAlmostZero1(m_uniqueEdges[p] + edge))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_uniqueEdges.push_back(edge);
			}

			if (edptr)
			{
				Assert(edptr->m_face0 >= 0);
				Assert(edptr->m_face1 < 0);
				edptr->m_face1 = i;
			}
			else
			{
				InternalEdge ed;
				ed.m_face0 = i;
				edges.insert(vp, ed);
			}
		}
	}

#ifdef USE_CONNECTED_FACES
	for (i32 i = 0; i < m_faces.size(); i++)
	{
		i32 numVertices = m_faces[i].m_indices.size();
		m_faces[i].m_connectedFaces.resize(numVertices);

		for (i32 j = 0; j < numVertices; j++)
		{
			i32 k = (j + 1) % numVertices;
			InternalVertexPair vp(m_faces[i].m_indices[j], m_faces[i].m_indices[k]);
			InternalEdge* edptr = edges.find(vp);
			Assert(edptr);
			Assert(edptr->m_face0 >= 0);
			Assert(edptr->m_face1 >= 0);

			i32 connectedFace = (edptr->m_face0 == i) ? edptr->m_face1 : edptr->m_face0;
			m_faces[i].m_connectedFaces[j] = connectedFace;
		}
	}
#endif  //USE_CONNECTED_FACES

	initialize2();
}

void ConvexPolyhedron::initialize2()
{
	m_localCenter.setVal(0, 0, 0);
	Scalar TotalArea = 0.0f;
	for (i32 i = 0; i < m_faces.size(); i++)
	{
		i32 numVertices = m_faces[i].m_indices.size();
		i32 NbTris = numVertices - 2;

		const Vec3& p0 = m_vertices[m_faces[i].m_indices[0]];
		for (i32 j = 1; j <= NbTris; j++)
		{
			i32 k = (j + 1) % numVertices;
			const Vec3& p1 = m_vertices[m_faces[i].m_indices[j]];
			const Vec3& p2 = m_vertices[m_faces[i].m_indices[k]];
			Scalar Area = ((p0 - p1).cross(p0 - p2)).length() * 0.5f;
			Vec3 Center = (p0 + p1 + p2) / 3.0f;
			m_localCenter += Area * Center;
			TotalArea += Area;
		}
	}
	m_localCenter /= TotalArea;

#ifdef TEST_INTERNAL_OBJECTS
	if (1)
	{
		m_radius = FLT_MAX;
		for (i32 i = 0; i < m_faces.size(); i++)
		{
			const Vec3 Normal(m_faces[i].m_plane[0], m_faces[i].m_plane[1], m_faces[i].m_plane[2]);
			const Scalar dist = Fabs(m_localCenter.dot(Normal) + m_faces[i].m_plane[3]);
			if (dist < m_radius)
				m_radius = dist;
		}

		Scalar MinX = FLT_MAX;
		Scalar MinY = FLT_MAX;
		Scalar MinZ = FLT_MAX;
		Scalar MaxX = -FLT_MAX;
		Scalar MaxY = -FLT_MAX;
		Scalar MaxZ = -FLT_MAX;
		for (i32 i = 0; i < m_vertices.size(); i++)
		{
			const Vec3& pt = m_vertices[i];
			if (pt.x() < MinX) MinX = pt.x();
			if (pt.x() > MaxX) MaxX = pt.x();
			if (pt.y() < MinY) MinY = pt.y();
			if (pt.y() > MaxY) MaxY = pt.y();
			if (pt.z() < MinZ) MinZ = pt.z();
			if (pt.z() > MaxZ) MaxZ = pt.z();
		}
		mC.setVal(MaxX + MinX, MaxY + MinY, MaxZ + MinZ);
		mE.setVal(MaxX - MinX, MaxY - MinY, MaxZ - MinZ);

		//		const Scalar r = m_radius / sqrtf(2.0f);
		const Scalar r = m_radius / sqrtf(3.0f);
		i32k LargestExtent = mE.maxAxis();
		const Scalar Step = (mE[LargestExtent] * 0.5f - r) / 1024.0f;
		m_extents[0] = m_extents[1] = m_extents[2] = r;
		m_extents[LargestExtent] = mE[LargestExtent] * 0.5f;
		bool FoundBox = false;
		for (i32 j = 0; j < 1024; j++)
		{
			if (testContainment())
			{
				FoundBox = true;
				break;
			}

			m_extents[LargestExtent] -= Step;
		}
		if (!FoundBox)
		{
			m_extents[0] = m_extents[1] = m_extents[2] = r;
		}
		else
		{
			// Refine the box
			const Scalar Step = (m_radius - r) / 1024.0f;
			i32k e0 = (1 << LargestExtent) & 3;
			i32k e1 = (1 << e0) & 3;

			for (i32 j = 0; j < 1024; j++)
			{
				const Scalar Saved0 = m_extents[e0];
				const Scalar Saved1 = m_extents[e1];
				m_extents[e0] += Step;
				m_extents[e1] += Step;

				if (!testContainment())
				{
					m_extents[e0] = Saved0;
					m_extents[e1] = Saved1;
					break;
				}
			}
		}
	}
#endif
}
void ConvexPolyhedron::project(const Transform2& trans, const Vec3& dir, Scalar& minProj, Scalar& maxProj, Vec3& witnesPtMin, Vec3& witnesPtMax) const
{
	minProj = FLT_MAX;
	maxProj = -FLT_MAX;
	i32 numVerts = m_vertices.size();
	for (i32 i = 0; i < numVerts; i++)
	{
		Vec3 pt = trans * m_vertices[i];
		Scalar dp = pt.dot(dir);
		if (dp < minProj)
		{
			minProj = dp;
			witnesPtMin = pt;
		}
		if (dp > maxProj)
		{
			maxProj = dp;
			witnesPtMax = pt;
		}
	}
	if (minProj > maxProj)
	{
		Swap(minProj, maxProj);
		Swap(witnesPtMin, witnesPtMax);
	}
}
