#ifndef DRX3D_AXIS_SWEEP_3_H
#define DRX3D_AXIS_SWEEP_3_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCallback.h>
#include <drx3D/Physics/Collision/BroadPhase/DbvtBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/AxisSweep3Internal.h>

/// The AxisSweep3 is an efficient implementation of the 3d axis sweep and prune broadphase.
/// It uses arrays rather then lists for storage of the 3 axis. Also it operates using 16 bit integer coordinates instead of floats.
/// For large worlds and many objects, use drx3D32BitAxisSweep3 or DbvtBroadphase instead. drx3D32BitAxisSweep3 has higher precision and allows more then 16384 objects at the cost of more memory and bit of performance.
class AxisSweep3 : public AxisSweep3Internal<u16>
{
public:
	AxisSweep3(const Vec3& worldAabbMin, const Vec3& worldAabbMax, u16 maxHandles = 16384, OverlappingPairCache* pairCache = 0, bool disableRaycastAccelerator = false);
};

/// The drx3D32BitAxisSweep3 allows higher precision quantization and more objects compared to the AxisSweep3 sweep and prune.
/// This comes at the cost of more memory per handle, and a bit slower performance.
/// It uses arrays rather then lists for storage of the 3 axis.
class drx3D32BitAxisSweep3 : public AxisSweep3Internal<u32>
{
public:
	drx3D32BitAxisSweep3(const Vec3& worldAabbMin, const Vec3& worldAabbMax, u32 maxHandles = 1500000, OverlappingPairCache* pairCache = 0, bool disableRaycastAccelerator = false);
};

#endif
