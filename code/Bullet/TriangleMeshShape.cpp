#include <drx3D/Physics/Collision/Shapes/TriangleMeshShape.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Physics/Collision/Shapes/StridingMeshInterface.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

TriangleMeshShape::TriangleMeshShape(StridingMeshInterface* meshInterface)
	: ConcaveShape(), m_meshInterface(meshInterface)
{
	m_shapeType = TRIANGLE_MESH_SHAPE_PROXYTYPE;
	if (meshInterface->hasPremadeAabb())
	{
		meshInterface->getPremadeAabb(&m_localAabbMin, &m_localAabbMax);
	}
	else
	{
		recalcLocalAabb();
	}
}

TriangleMeshShape::~TriangleMeshShape()
{
}

void TriangleMeshShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	Vec3 localHalfExtents = Scalar(0.5) * (m_localAabbMax - m_localAabbMin);
	localHalfExtents += Vec3(getMargin(), getMargin(), getMargin());
	Vec3 localCenter = Scalar(0.5) * (m_localAabbMax + m_localAabbMin);

	Matrix3x3 abs_b = trans.getBasis().absolute();

	Vec3 center = trans(localCenter);

	Vec3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMin = center - extent;
	aabbMax = center + extent;
}

void TriangleMeshShape::recalcLocalAabb()
{
	for (i32 i = 0; i < 3; i++)
	{
		Vec3 vec(Scalar(0.), Scalar(0.), Scalar(0.));
		vec[i] = Scalar(1.);
		Vec3 tmp = localGetSupportingVertex(vec);
		m_localAabbMax[i] = tmp[i] + m_collisionMargin;
		vec[i] = Scalar(-1.);
		tmp = localGetSupportingVertex(vec);
		m_localAabbMin[i] = tmp[i] - m_collisionMargin;
	}
}

class SupportVertexCallback : public TriangleCallback
{
	Vec3 m_supportVertexLocal;

public:
	Transform2 m_worldTrans;
	Scalar m_maxDot;
	Vec3 m_supportVecLocal;

	SupportVertexCallback(const Vec3& supportVecWorld, const Transform2& trans)
		: m_supportVertexLocal(Scalar(0.), Scalar(0.), Scalar(0.)), m_worldTrans(trans), m_maxDot(Scalar(-DRX3D_LARGE_FLOAT))

	{
		m_supportVecLocal = supportVecWorld * m_worldTrans.getBasis();
	}

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		(void)partId;
		(void)triangleIndex;
		for (i32 i = 0; i < 3; i++)
		{
			Scalar dot = m_supportVecLocal.dot(triangle[i]);
			if (dot > m_maxDot)
			{
				m_maxDot = dot;
				m_supportVertexLocal = triangle[i];
			}
		}
	}

	Vec3 GetSupportVertexWorldSpace()
	{
		return m_worldTrans(m_supportVertexLocal);
	}

	Vec3 GetSupportVertexLocal()
	{
		return m_supportVertexLocal;
	}
};

void TriangleMeshShape::setLocalScaling(const Vec3& scaling)
{
	m_meshInterface->setScaling(scaling);
	recalcLocalAabb();
}

const Vec3& TriangleMeshShape::getLocalScaling() const
{
	return m_meshInterface->getScaling();
}

//#define DEBUG_TRIANGLE_MESH

void TriangleMeshShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	struct FilteredCallback : public InternalTriangleIndexCallback
	{
		TriangleCallback* m_callback;
		Vec3 m_aabbMin;
		Vec3 m_aabbMax;

		FilteredCallback(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax)
			: m_callback(callback),
			  m_aabbMin(aabbMin),
			  m_aabbMax(aabbMax)
		{
		}

		virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			if (TestTriangleAgainstAabb2(&triangle[0], m_aabbMin, m_aabbMax))
			{
				//check aabb in triangle-space, before doing this
				m_callback->processTriangle(triangle, partId, triangleIndex);
			}
		}
	};

	FilteredCallback filterCallback(callback, aabbMin, aabbMax);

	m_meshInterface->InternalProcessAllTriangles(&filterCallback, aabbMin, aabbMax);
}

void TriangleMeshShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	(void)mass;
	//moving concave objects not supported
	Assert(0);
	inertia.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
}

Vec3 TriangleMeshShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 supportVertex;

	Transform2 ident;
	ident.setIdentity();

	SupportVertexCallback supportCallback(vec, ident);

	Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));

	processAllTriangles(&supportCallback, -aabbMax, aabbMax);

	supportVertex = supportCallback.GetSupportVertexLocal();

	return supportVertex;
}
