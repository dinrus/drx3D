#include <drx3D/Physics/Collision/Shapes/UniformScalingShape.h>

UniformScalingShape::UniformScalingShape(ConvexShape* convexChildShape, Scalar uniformScalingFactor) : ConvexShape(), m_childConvexShape(convexChildShape), m_uniformScalingFactor(uniformScalingFactor)
{
	m_shapeType = UNIFORM_SCALING_SHAPE_PROXYTYPE;
}

UniformScalingShape::~UniformScalingShape()
{
}

Vec3 UniformScalingShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	Vec3 tmpVertex;
	tmpVertex = m_childConvexShape->localGetSupportingVertexWithoutMargin(vec);
	return tmpVertex * m_uniformScalingFactor;
}

void UniformScalingShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	m_childConvexShape->batchedUnitVectorGetSupportingVertexWithoutMargin(vectors, supportVerticesOut, numVectors);
	i32 i;
	for (i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i] = supportVerticesOut[i] * m_uniformScalingFactor;
	}
}

Vec3 UniformScalingShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 tmpVertex;
	tmpVertex = m_childConvexShape->localGetSupportingVertex(vec);
	return tmpVertex * m_uniformScalingFactor;
}

void UniformScalingShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	///this linear upscaling is not realistic, but we don't deal with large mass ratios...
	Vec3 tmpInertia;
	m_childConvexShape->calculateLocalInertia(mass, tmpInertia);
	inertia = tmpInertia * m_uniformScalingFactor;
}

///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
void UniformScalingShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	getAabbSlow(trans, aabbMin, aabbMax);
}

void UniformScalingShape::getAabbSlow(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
#if 1
	Vec3 _directions[] =
		{
			Vec3(1., 0., 0.),
			Vec3(0., 1., 0.),
			Vec3(0., 0., 1.),
			Vec3(-1., 0., 0.),
			Vec3(0., -1., 0.),
			Vec3(0., 0., -1.)};

	Vec3 _supporting[] =
		{
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.)};

	for (i32 i = 0; i < 6; i++)
	{
		_directions[i] = _directions[i] * t.getBasis();
	}

	batchedUnitVectorGetSupportingVertexWithoutMargin(_directions, _supporting, 6);

	Vec3 aabbMin1(0, 0, 0), aabbMax1(0, 0, 0);

	for (i32 i = 0; i < 3; ++i)
	{
		aabbMax1[i] = t(_supporting[i])[i];
		aabbMin1[i] = t(_supporting[i + 3])[i];
	}
	Vec3 marginVec(getMargin(), getMargin(), getMargin());
	aabbMin = aabbMin1 - marginVec;
	aabbMax = aabbMax1 + marginVec;

#else

	Scalar margin = getMargin();
	for (i32 i = 0; i < 3; i++)
	{
		Vec3 vec(Scalar(0.), Scalar(0.), Scalar(0.));
		vec[i] = Scalar(1.);
		Vec3 sv = localGetSupportingVertex(vec * t.getBasis());
		Vec3 tmp = t(sv);
		aabbMax[i] = tmp[i] + margin;
		vec[i] = Scalar(-1.);
		sv = localGetSupportingVertex(vec * t.getBasis());
		tmp = t(sv);
		aabbMin[i] = tmp[i] - margin;
	}

#endif
}

void UniformScalingShape::setLocalScaling(const Vec3& scaling)
{
	m_childConvexShape->setLocalScaling(scaling);
}

const Vec3& UniformScalingShape::getLocalScaling() const
{
	return m_childConvexShape->getLocalScaling();
}

void UniformScalingShape::setMargin(Scalar margin)
{
	m_childConvexShape->setMargin(margin);
}
Scalar UniformScalingShape::getMargin() const
{
	return m_childConvexShape->getMargin() * m_uniformScalingFactor;
}

i32 UniformScalingShape::getNumPreferredPenetrationDirections() const
{
	return m_childConvexShape->getNumPreferredPenetrationDirections();
}

void UniformScalingShape::getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const
{
	m_childConvexShape->getPreferredPenetrationDirection(index, penetrationVector);
}
