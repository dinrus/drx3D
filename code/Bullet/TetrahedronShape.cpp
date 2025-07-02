#include <drx3D/Physics/Collision/Shapes/TetrahedronShape.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>

BU_Simplex1to4::BU_Simplex1to4() : PolyhedralConvexAabbCachingShape(),
									   m_numVertices(0)
{
	m_shapeType = TETRAHEDRAL_SHAPE_PROXYTYPE;
}

BU_Simplex1to4::BU_Simplex1to4(const Vec3& pt0) : PolyhedralConvexAabbCachingShape(),
														   m_numVertices(0)
{
	m_shapeType = TETRAHEDRAL_SHAPE_PROXYTYPE;
	addVertex(pt0);
}

BU_Simplex1to4::BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1) : PolyhedralConvexAabbCachingShape(),
																				 m_numVertices(0)
{
	m_shapeType = TETRAHEDRAL_SHAPE_PROXYTYPE;
	addVertex(pt0);
	addVertex(pt1);
}

BU_Simplex1to4::BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1, const Vec3& pt2) : PolyhedralConvexAabbCachingShape(),
																									   m_numVertices(0)
{
	m_shapeType = TETRAHEDRAL_SHAPE_PROXYTYPE;
	addVertex(pt0);
	addVertex(pt1);
	addVertex(pt2);
}

BU_Simplex1to4::BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1, const Vec3& pt2, const Vec3& pt3) : PolyhedralConvexAabbCachingShape(),
																															 m_numVertices(0)
{
	m_shapeType = TETRAHEDRAL_SHAPE_PROXYTYPE;
	addVertex(pt0);
	addVertex(pt1);
	addVertex(pt2);
	addVertex(pt3);
}

void BU_Simplex1to4::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
#if 1
	PolyhedralConvexAabbCachingShape::getAabb(t, aabbMin, aabbMax);
#else
	aabbMin.setVal(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
	aabbMax.setVal(-DRX3D_LARGE_FLOAT, -DRX3D_LARGE_FLOAT, -DRX3D_LARGE_FLOAT);

	//just transform the vertices in worldspace, and take their AABB
	for (i32 i = 0; i < m_numVertices; i++)
	{
		Vec3 worldVertex = t(m_vertices[i]);
		aabbMin.setMin(worldVertex);
		aabbMax.setMax(worldVertex);
	}
#endif
}

void BU_Simplex1to4::addVertex(const Vec3& pt)
{
	m_vertices[m_numVertices++] = pt;
	recalcLocalAabb();
}

i32 BU_Simplex1to4::getNumVertices() const
{
	return m_numVertices;
}

i32 BU_Simplex1to4::getNumEdges() const
{
	//euler formula, F-E+V = 2, so E = F+V-2

	switch (m_numVertices)
	{
		case 0:
			return 0;
		case 1:
			return 0;
		case 2:
			return 1;
		case 3:
			return 3;
		case 4:
			return 6;
	}

	return 0;
}

void BU_Simplex1to4::getEdge(i32 i, Vec3& pa, Vec3& pb) const
{
	switch (m_numVertices)
	{
		case 2:
			pa = m_vertices[0];
			pb = m_vertices[1];
			break;
		case 3:
			switch (i)
			{
				case 0:
					pa = m_vertices[0];
					pb = m_vertices[1];
					break;
				case 1:
					pa = m_vertices[1];
					pb = m_vertices[2];
					break;
				case 2:
					pa = m_vertices[2];
					pb = m_vertices[0];
					break;
			}
			break;
		case 4:
			switch (i)
			{
				case 0:
					pa = m_vertices[0];
					pb = m_vertices[1];
					break;
				case 1:
					pa = m_vertices[1];
					pb = m_vertices[2];
					break;
				case 2:
					pa = m_vertices[2];
					pb = m_vertices[0];
					break;
				case 3:
					pa = m_vertices[0];
					pb = m_vertices[3];
					break;
				case 4:
					pa = m_vertices[1];
					pb = m_vertices[3];
					break;
				case 5:
					pa = m_vertices[2];
					pb = m_vertices[3];
					break;
			}
	}
}

void BU_Simplex1to4::getVertex(i32 i, Vec3& vtx) const
{
	vtx = m_vertices[i];
}

i32 BU_Simplex1to4::getNumPlanes() const
{
	switch (m_numVertices)
	{
		case 0:
			return 0;
		case 1:
			return 0;
		case 2:
			return 0;
		case 3:
			return 2;
		case 4:
			return 4;
		default:
		{
		}
	}
	return 0;
}

void BU_Simplex1to4::getPlane(Vec3&, Vec3&, i32) const
{
}

i32 BU_Simplex1to4::getIndex(i32) const
{
	return 0;
}

bool BU_Simplex1to4::isInside(const Vec3&, Scalar) const
{
	return false;
}
