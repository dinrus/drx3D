#include <drx3D/Physics/Collision/BroadPhase/AxisSweep3.h>

AxisSweep3::AxisSweep3(const Vec3& worldAabbMin, const Vec3& worldAabbMax, u16 maxHandles, OverlappingPairCache* pairCache, bool disableRaycastAccelerator)
	: AxisSweep3Internal<u16>(worldAabbMin, worldAabbMax, 0xfffe, 0xffff, maxHandles, pairCache, disableRaycastAccelerator)
{
	// 1 handle is reserved as sentinel
	Assert(maxHandles > 1 && maxHandles < 32767);
}

drx3D32BitAxisSweep3::drx3D32BitAxisSweep3(const Vec3& worldAabbMin, const Vec3& worldAabbMax, u32 maxHandles, OverlappingPairCache* pairCache, bool disableRaycastAccelerator)
	: AxisSweep3Internal<u32>(worldAabbMin, worldAabbMax, 0xfffffffe, 0x7fffffff, maxHandles, pairCache, disableRaycastAccelerator)
{
	// 1 handle is reserved as sentinel
	Assert(maxHandles > 1 && maxHandles < 2147483647);
}
