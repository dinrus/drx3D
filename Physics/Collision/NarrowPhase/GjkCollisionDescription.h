#ifndef GJK_COLLISION_DESCRIPTION_H
#define GJK_COLLISION_DESCRIPTION_H

#include <drx3D/Maths/Linear/Vec3.h>

struct GjkCollisionDescription
{
	Vec3 m_firstDir;
	i32 m_maxGjkIterations;
	Scalar m_maximumDistanceSquared;
	Scalar m_gjkRelError2;
	GjkCollisionDescription()
		: m_firstDir(0, 1, 0),
		  m_maxGjkIterations(1000),
		  m_maximumDistanceSquared(1e30f),
		  m_gjkRelError2(1.0e-6)
	{
	}
	virtual ~GjkCollisionDescription()
	{
	}
};

#endif  //GJK_COLLISION_DESCRIPTION_H
