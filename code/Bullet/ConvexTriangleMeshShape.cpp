#include <drx3D/Physics/Collision/Shapes/ConvexTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Physics/Collision/Shapes/StridingMeshInterface.h>

ConvexTriangleMeshShape ::ConvexTriangleMeshShape(StridingMeshInterface* meshInterface, bool calcAabb)
	: PolyhedralConvexAabbCachingShape(), m_stridingMesh(meshInterface)
{
	m_shapeType = CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE;
	if (calcAabb)
		recalcLocalAabb();
}

///It's not nice to have all this virtual function overhead, so perhaps we can also gather the points once
///but then we are duplicating
class LocalSupportVertexCallback : public InternalTriangleIndexCallback
{
	Vec3 m_supportVertexLocal;

public:
	Scalar m_maxDot;
	Vec3 m_supportVecLocal;

	LocalSupportVertexCallback(const Vec3& supportVecLocal)
		: m_supportVertexLocal(Scalar(0.), Scalar(0.), Scalar(0.)),
		  m_maxDot(Scalar(-DRX3D_LARGE_FLOAT)),
		  m_supportVecLocal(supportVecLocal)
	{
	}

	virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		(void)triangleIndex;
		(void)partId;

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

	Vec3 GetSupportVertexLocal()
	{
		return m_supportVertexLocal;
	}
};

Vec3 ConvexTriangleMeshShape::localGetSupportingVertexWithoutMargin(const Vec3& vec0) const
{
	Vec3 supVec(Scalar(0.), Scalar(0.), Scalar(0.));

	Vec3 vec = vec0;
	Scalar lenSqr = vec.length2();
	if (lenSqr < Scalar(0.0001))
	{
		vec.setVal(1, 0, 0);
	}
	else
	{
		Scalar rlen = Scalar(1.) / Sqrt(lenSqr);
		vec *= rlen;
	}

	LocalSupportVertexCallback supportCallback(vec);
	Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	m_stridingMesh->InternalProcessAllTriangles(&supportCallback, -aabbMax, aabbMax);
	supVec = supportCallback.GetSupportVertexLocal();

	return supVec;
}

void ConvexTriangleMeshShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	//use 'w' component of supportVerticesOut?
	{
		for (i32 i = 0; i < numVectors; i++)
		{
			supportVerticesOut[i][3] = Scalar(-DRX3D_LARGE_FLOAT);
		}
	}

	///@todo: could do the batch inside the callback!

	for (i32 j = 0; j < numVectors; j++)
	{
		const Vec3& vec = vectors[j];
		LocalSupportVertexCallback supportCallback(vec);
		Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
		m_stridingMesh->InternalProcessAllTriangles(&supportCallback, -aabbMax, aabbMax);
		supportVerticesOut[j] = supportCallback.GetSupportVertexLocal();
	}
}

Vec3 ConvexTriangleMeshShape::localGetSupportingVertex(const Vec3& vec) const
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

//currently just for debugging (drawing), perhaps future support for algebraic continuous collision detection
//Please note that you can debug-draw ConvexTriangleMeshShape with the Raytracer Demo
i32 ConvexTriangleMeshShape::getNumVertices() const
{
	//cache this?
	return 0;
}

i32 ConvexTriangleMeshShape::getNumEdges() const
{
	return 0;
}

void ConvexTriangleMeshShape::getEdge(i32, Vec3&, Vec3&) const
{
	Assert(0);
}

void ConvexTriangleMeshShape::getVertex(i32, Vec3&) const
{
	Assert(0);
}

i32 ConvexTriangleMeshShape::getNumPlanes() const
{
	return 0;
}

void ConvexTriangleMeshShape::getPlane(Vec3&, Vec3&, i32) const
{
	Assert(0);
}

//not yet
bool ConvexTriangleMeshShape::isInside(const Vec3&, Scalar) const
{
	Assert(0);
	return false;
}

void ConvexTriangleMeshShape::setLocalScaling(const Vec3& scaling)
{
	m_stridingMesh->setScaling(scaling);

	recalcLocalAabb();
}

const Vec3& ConvexTriangleMeshShape::getLocalScaling() const
{
	return m_stridingMesh->getScaling();
}

void ConvexTriangleMeshShape::calculatePrincipalAxisTransform(Transform2& principal, Vec3& inertia, Scalar& volume) const
{
	class CenterCallback : public InternalTriangleIndexCallback
	{
		bool first;
		Vec3 ref;
		Vec3 sum;
		Scalar volume;

	public:
		CenterCallback() : first(true), ref(0, 0, 0), sum(0, 0, 0), volume(0)
		{
		}

		virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			(void)triangleIndex;
			(void)partId;
			if (first)
			{
				ref = triangle[0];
				first = false;
			}
			else
			{
				Scalar vol = Fabs((triangle[0] - ref).triple(triangle[1] - ref, triangle[2] - ref));
				sum += (Scalar(0.25) * vol) * ((triangle[0] + triangle[1] + triangle[2] + ref));
				volume += vol;
			}
		}

		Vec3 getCenter()
		{
			return (volume > 0) ? sum / volume : ref;
		}

		Scalar getVolume()
		{
			return volume * Scalar(1. / 6);
		}
	};

	class InertiaCallback : public InternalTriangleIndexCallback
	{
		Matrix3x3 sum;
		Vec3 center;

	public:
		InertiaCallback(Vec3& center) : sum(0, 0, 0, 0, 0, 0, 0, 0, 0), center(center)
		{
		}

		virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			(void)triangleIndex;
			(void)partId;
			Matrix3x3 i;
			Vec3 a = triangle[0] - center;
			Vec3 b = triangle[1] - center;
			Vec3 c = triangle[2] - center;
			Scalar volNeg = -Fabs(a.triple(b, c)) * Scalar(1. / 6);
			for (i32 j = 0; j < 3; j++)
			{
				for (i32 k = 0; k <= j; k++)
				{
					i[j][k] = i[k][j] = volNeg * (Scalar(0.1) * (a[j] * a[k] + b[j] * b[k] + c[j] * c[k]) + Scalar(0.05) * (a[j] * b[k] + a[k] * b[j] + a[j] * c[k] + a[k] * c[j] + b[j] * c[k] + b[k] * c[j]));
				}
			}
			Scalar i00 = -i[0][0];
			Scalar i11 = -i[1][1];
			Scalar i22 = -i[2][2];
			i[0][0] = i11 + i22;
			i[1][1] = i22 + i00;
			i[2][2] = i00 + i11;
			sum[0] += i[0];
			sum[1] += i[1];
			sum[2] += i[2];
		}

		Matrix3x3& getInertia()
		{
			return sum;
		}
	};

	CenterCallback centerCallback;
	Vec3 aabbMax(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	m_stridingMesh->InternalProcessAllTriangles(&centerCallback, -aabbMax, aabbMax);
	Vec3 center = centerCallback.getCenter();
	principal.setOrigin(center);
	volume = centerCallback.getVolume();

	InertiaCallback inertiaCallback(center);
	m_stridingMesh->InternalProcessAllTriangles(&inertiaCallback, -aabbMax, aabbMax);

	Matrix3x3& i = inertiaCallback.getInertia();
	i.diagonalize(principal.getBasis(), Scalar(0.00001), 20);
	inertia.setVal(i[0][0], i[1][1], i[2][2]);
	inertia /= volume;
}
