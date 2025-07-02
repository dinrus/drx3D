
#ifndef D3_RAYCAST_INFO_H
#define D3_RAYCAST_INFO_H

#include <drx3D/Common/b3Vec3.h>

D3_ATTRIBUTE_ALIGNED16(struct)
b3RayInfo
{
	b3Vec3 m_from;
	b3Vec3 m_to;
};

D3_ATTRIBUTE_ALIGNED16(struct)
b3RayHit
{
	b3Scalar m_hitFraction;
	i32 m_hitBody;
	i32 m_hitResult1;
	i32 m_hitResult2;
	b3Vec3 m_hitPoint;
	b3Vec3 m_hitNormal;
};

#endif  //D3_RAYCAST_INFO_H
