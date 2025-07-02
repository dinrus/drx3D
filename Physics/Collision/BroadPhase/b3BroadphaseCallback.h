#ifndef D3_BROADPHASE_CALLBACK_H
#define D3_BROADPHASE_CALLBACK_H

#include <drx3D/Common/b3Vec3.h>
struct b3BroadphaseProxy;

struct b3BroadphaseAabbCallback
{
	virtual ~b3BroadphaseAabbCallback() {}
	virtual bool process(const b3BroadphaseProxy* proxy) = 0;
};

struct b3BroadphaseRayCallback : public b3BroadphaseAabbCallback
{
	///added some cached data to accelerate ray-AABB tests
	b3Vec3 m_rayDirectionInverse;
	u32 m_signs[3];
	b3Scalar m_lambda_max;

	virtual ~b3BroadphaseRayCallback() {}
};

#endif  //D3_BROADPHASE_CALLBACK_H
