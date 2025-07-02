
#if defined(_WIN32) || defined(__i386__)
#define DRX3D_USE_SSE_IN_API
#endif

#include <drx3D/Physics/Collision/Shapes/ConvexHullShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>
#include <drx3D/Maths/Linear/ConvexHullComputer.h>

ConvexHullShape ::ConvexHullShape(const Scalar* points, i32 numPoints, i32 stride) : PolyhedralConvexAabbCachingShape()
{
	m_shapeType = CONVEX_HULL_SHAPE_PROXYTYPE;
	m_unscaledPoints.resize(numPoints);

	u8* pointsAddress = (u8*)points;

	for (i32 i = 0; i < numPoints; i++)
	{
		Scalar* point = (Scalar*)pointsAddress;
		m_unscaledPoints[i] = Vec3(point[0], point[1], point[2]);
		pointsAddress += stride;
	}

	recalcLocalAabb();
}

void ConvexHullShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling;
	recalcLocalAabb();
}

void ConvexHullShape::addPoint(const Vec3& point, bool recalculateLocalAabb)
{
	m_unscaledPoints.push_back(point);
	if (recalculateLocalAabb)
		recalcLocalAabb();
}

Vec3 ConvexHullShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	Vec3 supVec(Scalar(0.), Scalar(0.), Scalar(0.));
	Scalar maxDot = Scalar(-DRX3D_LARGE_FLOAT);

	// Here we take advantage of dot(a, b*c) = dot(a*b, c).  Note: This is true mathematically, but not numerically.
	if (0 < m_unscaledPoints.size())
	{
		Vec3 scaled = vec * m_localScaling;
		i32 index = (i32)scaled.maxDot(&m_unscaledPoints[0], m_unscaledPoints.size(), maxDot);  // FIXME: may violate encapsulation of m_unscaledPoints
		return m_unscaledPoints[index] * m_localScaling;
	}

	return supVec;
}

void ConvexHullShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	Scalar newDot;
	//use 'w' component of supportVerticesOut?
	{
		for (i32 i = 0; i < numVectors; i++)
		{
			supportVerticesOut[i][3] = Scalar(-DRX3D_LARGE_FLOAT);
		}
	}

	for (i32 j = 0; j < numVectors; j++)
	{
		Vec3 vec = vectors[j] * m_localScaling;  // dot(a*b,c) = dot(a,b*c)
		if (0 < m_unscaledPoints.size())
		{
			i32 i = (i32)vec.maxDot(&m_unscaledPoints[0], m_unscaledPoints.size(), newDot);
			supportVerticesOut[j] = getScaledPoint(i);
			supportVerticesOut[j][3] = newDot;
		}
		else
			supportVerticesOut[j][3] = -DRX3D_LARGE_FLOAT;
	}
}

Vec3 ConvexHullShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 supVertex = localGetSupportingVertexWithoutMargin(vec);

	if (getMargin() != Scalar(0.))
	{
		Vec3 vecnorm = vec;
		if (vecnorm.length2() < (SIMD_EPSILON * SIMD_EPSILON))
		{
			vecnorm.setVal(Scalar(-1.), Scalar(-1.), Scalar(-1.));
		}
		vecnorm.normalize();
		supVertex += getMargin() * vecnorm;
	}
	return supVertex;
}

void ConvexHullShape::optimizeConvexHull()
{
	ConvexHullComputer conv;
	conv.compute(&m_unscaledPoints[0].getX(), sizeof(Vec3), m_unscaledPoints.size(), 0.f, 0.f);
	i32 numVerts = conv.vertices.size();
	m_unscaledPoints.resize(0);
	for (i32 i = 0; i < numVerts; i++)
	{
		m_unscaledPoints.push_back(conv.vertices[i]);
	}
}

//currently just for debugging (drawing), perhaps future support for algebraic continuous collision detection
//Please note that you can debug-draw ConvexHullShape with the Raytracer Demo
i32 ConvexHullShape::getNumVertices() const
{
	return m_unscaledPoints.size();
}

i32 ConvexHullShape::getNumEdges() const
{
	return m_unscaledPoints.size();
}

void ConvexHullShape::getEdge(i32 i, Vec3& pa, Vec3& pb) const
{
	i32 index0 = i % m_unscaledPoints.size();
	i32 index1 = (i + 1) % m_unscaledPoints.size();
	pa = getScaledPoint(index0);
	pb = getScaledPoint(index1);
}

void ConvexHullShape::getVertex(i32 i, Vec3& vtx) const
{
	vtx = getScaledPoint(i);
}

i32 ConvexHullShape::getNumPlanes() const
{
	return 0;
}

void ConvexHullShape::getPlane(Vec3&, Vec3&, i32) const
{
	Assert(0);
}

//not yet
bool ConvexHullShape::isInside(const Vec3&, Scalar) const
{
	Assert(0);
	return false;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk ConvexHullShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	//i32 szc = sizeof(ConvexHullShapeData);
	ConvexHullShapeData* shapeData = (ConvexHullShapeData*)dataBuffer;
	ConvexInternalShape::serialize(&shapeData->m_convexInternalShapeData, serializer);

	i32 numElem = m_unscaledPoints.size();
	shapeData->m_numUnscaledPoints = numElem;
#ifdef DRX3D_USE_DOUBLE_PRECISION
	shapeData->m_unscaledPointsFloatPtr = 0;
	shapeData->m_unscaledPointsDoublePtr = numElem ? (Vec3Data*)serializer->getUniquePointer((uk )&m_unscaledPoints[0]) : 0;
#else
	shapeData->m_unscaledPointsFloatPtr = numElem ? (Vec3Data*)serializer->getUniquePointer((uk )&m_unscaledPoints[0]) : 0;
	shapeData->m_unscaledPointsDoublePtr = 0;
#endif

	if (numElem)
	{
		i32 sz = sizeof(Vec3Data);
		//	i32 sz2 = sizeof(Vec3DoubleData);
		//	i32 sz3 = sizeof(Vec3FloatData);
		Chunk* chunk = serializer->allocate(sz, numElem);
		Vec3Data* memPtr = (Vec3Data*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			m_unscaledPoints[i].serialize(*memPtr);
		}
		serializer->finalizeChunk(chunk, Vec3DataName, DRX3D_ARRAY_CODE, (uk )&m_unscaledPoints[0]);
	}

	// Fill padding with zeros to appease msan.
	memset(shapeData->m_padding3, 0, sizeof(shapeData->m_padding3));

	return "ConvexHullShapeData";
}

void ConvexHullShape::project(const Transform2& trans, const Vec3& dir, Scalar& minProj, Scalar& maxProj, Vec3& witnesPtMin, Vec3& witnesPtMax) const
{
#if 1
	minProj = FLT_MAX;
	maxProj = -FLT_MAX;

	i32 numVerts = m_unscaledPoints.size();
	for (i32 i = 0; i < numVerts; i++)
	{
		Vec3 vtx = m_unscaledPoints[i] * m_localScaling;
		Vec3 pt = trans * vtx;
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
#else
	Vec3 localAxis = dir * trans.getBasis();
	witnesPtMin = trans(localGetSupportingVertex(localAxis));
	witnesPtMax = trans(localGetSupportingVertex(-localAxis));

	minProj = witnesPtMin.dot(dir);
	maxProj = witnesPtMax.dot(dir);
#endif

	if (minProj > maxProj)
	{
		Swap(minProj, maxProj);
		Swap(witnesPtMin, witnesPtMax);
	}
}
