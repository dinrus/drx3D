#include <drx3D/Physics/Collision/Shapes/ConeShape.h>

ConeShape::ConeShape(Scalar radius, Scalar height) : ConvexInternalShape(),
															 m_radius(radius),
															 m_height(height)
{
	m_shapeType = CONE_SHAPE_PROXYTYPE;
	setConeUpIndex(1);
	Vec3 halfExtents;
	m_sinAngle = (m_radius / Sqrt(m_radius * m_radius + m_height * m_height));
}

ConeShapeZ::ConeShapeZ(Scalar radius, Scalar height) : ConeShape(radius, height)
{
	setConeUpIndex(2);
}

ConeShapeX::ConeShapeX(Scalar radius, Scalar height) : ConeShape(radius, height)
{
	setConeUpIndex(0);
}

///choose upAxis index
void ConeShape::setConeUpIndex(i32 upIndex)
{
	switch (upIndex)
	{
		case 0:
			m_coneIndices[0] = 1;
			m_coneIndices[1] = 0;
			m_coneIndices[2] = 2;
			break;
		case 1:
			m_coneIndices[0] = 0;
			m_coneIndices[1] = 1;
			m_coneIndices[2] = 2;
			break;
		case 2:
			m_coneIndices[0] = 0;
			m_coneIndices[1] = 2;
			m_coneIndices[2] = 1;
			break;
		default:
			Assert(0);
	};

	m_implicitShapeDimensions[m_coneIndices[0]] = m_radius;
	m_implicitShapeDimensions[m_coneIndices[1]] = m_height;
	m_implicitShapeDimensions[m_coneIndices[2]] = m_radius;
}

Vec3 ConeShape::coneLocalSupport(const Vec3& v) const
{
	Scalar halfHeight = m_height * Scalar(0.5);

	if (v[m_coneIndices[1]] > v.length() * m_sinAngle)
	{
		Vec3 tmp;

		tmp[m_coneIndices[0]] = Scalar(0.);
		tmp[m_coneIndices[1]] = halfHeight;
		tmp[m_coneIndices[2]] = Scalar(0.);
		return tmp;
	}
	else
	{
		Scalar s = Sqrt(v[m_coneIndices[0]] * v[m_coneIndices[0]] + v[m_coneIndices[2]] * v[m_coneIndices[2]]);
		if (s > SIMD_EPSILON)
		{
			Scalar d = m_radius / s;
			Vec3 tmp;
			tmp[m_coneIndices[0]] = v[m_coneIndices[0]] * d;
			tmp[m_coneIndices[1]] = -halfHeight;
			tmp[m_coneIndices[2]] = v[m_coneIndices[2]] * d;
			return tmp;
		}
		else
		{
			Vec3 tmp;
			tmp[m_coneIndices[0]] = Scalar(0.);
			tmp[m_coneIndices[1]] = -halfHeight;
			tmp[m_coneIndices[2]] = Scalar(0.);
			return tmp;
		}
	}
}

Vec3 ConeShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	return coneLocalSupport(vec);
}

void ConeShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 i = 0; i < numVectors; i++)
	{
		const Vec3& vec = vectors[i];
		supportVerticesOut[i] = coneLocalSupport(vec);
	}
}

Vec3 ConeShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 supVertex = coneLocalSupport(vec);
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

void ConeShape::setLocalScaling(const Vec3& scaling)
{
	i32 axis = m_coneIndices[1];
	i32 r1 = m_coneIndices[0];
	i32 r2 = m_coneIndices[2];
	m_height *= scaling[axis] / m_localScaling[axis];
	m_radius *= (scaling[r1] / m_localScaling[r1] + scaling[r2] / m_localScaling[r2]) / 2;
	m_sinAngle = (m_radius / Sqrt(m_radius * m_radius + m_height * m_height));
	ConvexInternalShape::setLocalScaling(scaling);
}