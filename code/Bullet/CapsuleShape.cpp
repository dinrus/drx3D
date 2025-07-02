#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Maths/Linear/Quat.h>

CapsuleShape::CapsuleShape(Scalar radius, Scalar height) : ConvexInternalShape()
{
	m_collisionMargin = radius;
	m_shapeType = CAPSULE_SHAPE_PROXYTYPE;
	m_upAxis = 1;
	m_implicitShapeDimensions.setVal(radius, 0.5f * height, radius);
}

Vec3 CapsuleShape::localGetSupportingVertexWithoutMargin(const Vec3& vec0) const
{
	Vec3 supVec(0, 0, 0);

	Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));

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

	Vec3 vtx;
	Scalar newDot;

	{
		Vec3 pos(0, 0, 0);
		pos[getUpAxis()] = getHalfHeight();

		vtx = pos;
		newDot = vec.dot(vtx);
		if (newDot > maxDot)
		{
			maxDot = newDot;
			supVec = vtx;
		}
	}
	{
		Vec3 pos(0, 0, 0);
		pos[getUpAxis()] = -getHalfHeight();

		vtx = pos;
		newDot = vec.dot(vtx);
		if (newDot > maxDot)
		{
			maxDot = newDot;
			supVec = vtx;
		}
	}

	return supVec;
}

void CapsuleShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 j = 0; j < numVectors; j++)
	{
		Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));
		const Vec3& vec = vectors[j];

		Vec3 vtx;
		Scalar newDot;
		{
			Vec3 pos(0, 0, 0);
			pos[getUpAxis()] = getHalfHeight();
			vtx = pos;
			newDot = vec.dot(vtx);
			if (newDot > maxDot)
			{
				maxDot = newDot;
				supportVerticesOut[j] = vtx;
			}
		}
		{
			Vec3 pos(0, 0, 0);
			pos[getUpAxis()] = -getHalfHeight();
			vtx = pos;
			newDot = vec.dot(vtx);
			if (newDot > maxDot)
			{
				maxDot = newDot;
				supportVerticesOut[j] = vtx;
			}
		}
	}
}

void CapsuleShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	//as an approximation, take the inertia of the box that bounds the spheres

	Transform2 ident;
	ident.setIdentity();

	Scalar radius = getRadius();

	Vec3 halfExtents(radius, radius, radius);
	halfExtents[getUpAxis()] += getHalfHeight();

	Scalar lx = Scalar(2.) * (halfExtents[0]);
	Scalar ly = Scalar(2.) * (halfExtents[1]);
	Scalar lz = Scalar(2.) * (halfExtents[2]);
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(.08333333);

	inertia[0] = scaledmass * (y2 + z2);
	inertia[1] = scaledmass * (x2 + z2);
	inertia[2] = scaledmass * (x2 + y2);
}

CapsuleShapeX::CapsuleShapeX(Scalar radius, Scalar height)
{
	m_collisionMargin = radius;
	m_upAxis = 0;
	m_implicitShapeDimensions.setVal(0.5f * height, radius, radius);
}

CapsuleShapeZ::CapsuleShapeZ(Scalar radius, Scalar height)
{
	m_collisionMargin = radius;
	m_upAxis = 2;
	m_implicitShapeDimensions.setVal(radius, radius, 0.5f * height);
}
