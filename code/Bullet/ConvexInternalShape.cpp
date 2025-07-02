#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>

ConvexInternalShape::ConvexInternalShape()
	: m_localScaling(Scalar(1.), Scalar(1.), Scalar(1.)),
	  m_collisionMargin(CONVEX_DISTANCE_MARGIN)
{
}

void ConvexInternalShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling.absolute();
}

void ConvexInternalShape::getAabbSlow(const Transform2& trans, Vec3& minAabb, Vec3& maxAabb) const
{
#ifndef __SPU__
	//use localGetSupportingVertexWithoutMargin?
	Scalar margin = getMargin();
	for (i32 i = 0; i < 3; i++)
	{
		Vec3 vec(Scalar(0.), Scalar(0.), Scalar(0.));
		vec[i] = Scalar(1.);

		Vec3 sv = localGetSupportingVertex(vec * trans.getBasis());

		Vec3 tmp = trans(sv);
		maxAabb[i] = tmp[i] + margin;
		vec[i] = Scalar(-1.);
		tmp = trans(localGetSupportingVertex(vec * trans.getBasis()));
		minAabb[i] = tmp[i] - margin;
	}
#endif
}

Vec3 ConvexInternalShape::localGetSupportingVertex(const Vec3& vec) const
{
#ifndef __SPU__

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

#else
	Assert(0);
	return Vec3(0, 0, 0);
#endif  //__SPU__
}

ConvexInternalAabbCachingShape::ConvexInternalAabbCachingShape()
	: ConvexInternalShape(),
	  m_localAabbMin(1, 1, 1),
	  m_localAabbMax(-1, -1, -1),
	  m_isLocalAabbValid(false)
{
}

void ConvexInternalAabbCachingShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	getNonvirtualAabb(trans, aabbMin, aabbMax, getMargin());
}

void ConvexInternalAabbCachingShape::setLocalScaling(const Vec3& scaling)
{
	ConvexInternalShape::setLocalScaling(scaling);
	recalcLocalAabb();
}

void ConvexInternalAabbCachingShape::recalcLocalAabb()
{
	m_isLocalAabbValid = true;

#if 1
	static const Vec3 _directions[] =
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

	batchedUnitVectorGetSupportingVertexWithoutMargin(_directions, _supporting, 6);

	for (i32 i = 0; i < 3; ++i)
	{
		m_localAabbMax[i] = _supporting[i][i] + m_collisionMargin;
		m_localAabbMin[i] = _supporting[i + 3][i] - m_collisionMargin;
	}

#else

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
#endif
}
