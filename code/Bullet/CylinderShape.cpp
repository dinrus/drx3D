
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>

CylinderShape::CylinderShape(const Vec3& halfExtents)
	: ConvexInternalShape(),
	  m_upAxis(1)
{
	Vec3 margin(getMargin(), getMargin(), getMargin());
	m_implicitShapeDimensions = (halfExtents * m_localScaling) - margin;

	setSafeMargin(halfExtents);

	m_shapeType = CYLINDER_SHAPE_PROXYTYPE;
}

CylinderShapeX::CylinderShapeX(const Vec3& halfExtents)
	: CylinderShape(halfExtents)
{
	m_upAxis = 0;
}

CylinderShapeZ::CylinderShapeZ(const Vec3& halfExtents)
	: CylinderShape(halfExtents)
{
	m_upAxis = 2;
}

void CylinderShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	Transform2Aabb(getHalfExtentsWithoutMargin(), getMargin(), t, aabbMin, aabbMax);
}

void CylinderShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
//Until drx3D 2.77 a box approximation was used, so uncomment this if you need backwards compatibility
//#define USE_BOX_INERTIA_APPROXIMATION 1
#ifndef USE_BOX_INERTIA_APPROXIMATION

	/*
	cylinder is defined as following:
	*
	* - principle axis aligned along y by default, radius in x, z-value not used
	* - for CylinderShapeX: principle axis aligned along x, radius in y direction, z-value not used
	* - for CylinderShapeZ: principle axis aligned along z, radius in x direction, y-value not used
	*
	*/

	Scalar radius2;                                    // square of cylinder radius
	Scalar height2;                                    // square of cylinder height
	Vec3 halfExtents = getHalfExtentsWithMargin();  // get cylinder dimension
	Scalar div12 = mass / 12.f;
	Scalar div4 = mass / 4.f;
	Scalar div2 = mass / 2.f;
	i32 idxRadius, idxHeight;

	switch (m_upAxis)  // get indices of radius and height of cylinder
	{
		case 0:  // cylinder is aligned along x
			idxRadius = 1;
			idxHeight = 0;
			break;
		case 2:  // cylinder is aligned along z
			idxRadius = 0;
			idxHeight = 2;
			break;
		default:  // cylinder is aligned along y
			idxRadius = 0;
			idxHeight = 1;
	}

	// calculate squares
	radius2 = halfExtents[idxRadius] * halfExtents[idxRadius];
	height2 = Scalar(4.) * halfExtents[idxHeight] * halfExtents[idxHeight];

	// calculate tensor terms
	Scalar t1 = div12 * height2 + div4 * radius2;
	Scalar t2 = div2 * radius2;

	switch (m_upAxis)  // set diagonal elements of inertia tensor
	{
		case 0:  // cylinder is aligned along x
			inertia.setVal(t2, t1, t1);
			break;
		case 2:  // cylinder is aligned along z
			inertia.setVal(t1, t1, t2);
			break;
		default:  // cylinder is aligned along y
			inertia.setVal(t1, t2, t1);
	}
#else   //USE_BOX_INERTIA_APPROXIMATION
	//approximation of box shape
	Vec3 halfExtents = getHalfExtentsWithMargin();

	Scalar lx = Scalar(2.) * (halfExtents.x());
	Scalar ly = Scalar(2.) * (halfExtents.y());
	Scalar lz = Scalar(2.) * (halfExtents.z());

	inertia.setVal(mass / (Scalar(12.0)) * (ly * ly + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + ly * ly));
#endif  //USE_BOX_INERTIA_APPROXIMATION
}

SIMD_FORCE_INLINE Vec3 CylinderLocalSupportX(const Vec3& halfExtents, const Vec3& v)
{
	i32k cylinderUpAxis = 0;
	i32k XX = 1;
	i32k YY = 0;
	i32k ZZ = 2;

	//mapping depends on how cylinder local orientation is
	// extents of the cylinder is: X,Y is for radius, and Z for height

	Scalar radius = halfExtents[XX];
	Scalar halfHeight = halfExtents[cylinderUpAxis];

	Vec3 tmp;
	Scalar d;

	Scalar s = Sqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
	if (s != Scalar(0.0))
	{
		d = radius / s;
		tmp[XX] = v[XX] * d;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = v[ZZ] * d;
		return tmp;
	}
	else
	{
		tmp[XX] = radius;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = Scalar(0.0);
		return tmp;
	}
}

inline Vec3 CylinderLocalSupportY(const Vec3& halfExtents, const Vec3& v)
{
	i32k cylinderUpAxis = 1;
	i32k XX = 0;
	i32k YY = 1;
	i32k ZZ = 2;

	Scalar radius = halfExtents[XX];
	Scalar halfHeight = halfExtents[cylinderUpAxis];

	Vec3 tmp;
	Scalar d;

	Scalar s = Sqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
	if (s != Scalar(0.0))
	{
		d = radius / s;
		tmp[XX] = v[XX] * d;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = v[ZZ] * d;
		return tmp;
	}
	else
	{
		tmp[XX] = radius;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = Scalar(0.0);
		return tmp;
	}
}

inline Vec3 CylinderLocalSupportZ(const Vec3& halfExtents, const Vec3& v)
{
	i32k cylinderUpAxis = 2;
	i32k XX = 0;
	i32k YY = 2;
	i32k ZZ = 1;

	//mapping depends on how cylinder local orientation is
	// extents of the cylinder is: X,Y is for radius, and Z for height

	Scalar radius = halfExtents[XX];
	Scalar halfHeight = halfExtents[cylinderUpAxis];

	Vec3 tmp;
	Scalar d;

	Scalar s = Sqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
	if (s != Scalar(0.0))
	{
		d = radius / s;
		tmp[XX] = v[XX] * d;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = v[ZZ] * d;
		return tmp;
	}
	else
	{
		tmp[XX] = radius;
		tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
		tmp[ZZ] = Scalar(0.0);
		return tmp;
	}
}

Vec3 CylinderShapeX::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	return CylinderLocalSupportX(getHalfExtentsWithoutMargin(), vec);
}

Vec3 CylinderShapeZ::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	return CylinderLocalSupportZ(getHalfExtentsWithoutMargin(), vec);
}
Vec3 CylinderShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	return CylinderLocalSupportY(getHalfExtentsWithoutMargin(), vec);
}

void CylinderShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i] = CylinderLocalSupportY(getHalfExtentsWithoutMargin(), vectors[i]);
	}
}

void CylinderShapeZ::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i] = CylinderLocalSupportZ(getHalfExtentsWithoutMargin(), vectors[i]);
	}
}

void CylinderShapeX::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i] = CylinderLocalSupportX(getHalfExtentsWithoutMargin(), vectors[i]);
	}
}
