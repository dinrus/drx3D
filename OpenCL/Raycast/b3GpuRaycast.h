#ifndef D3_GPU_RAYCAST_H
#define D3_GPU_RAYCAST_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3RaycastInfo.h>

class b3GpuRaycast
{
protected:
	struct b3GpuRaycastInternalData* m_data;

public:
	b3GpuRaycast(cl_context ctx, cl_device_id device, cl_command_queue q);
	virtual ~b3GpuRaycast();

	void castRaysHost(const b3AlignedObjectArray<b3RayInfo>& raysIn, b3AlignedObjectArray<b3RayHit>& hitResults,
					  i32 numBodies, const struct b3RigidBodyData* bodies, i32 numCollidables, const struct b3Collidable* collidables,
					  const struct b3GpuNarrowPhaseInternalData* narrowphaseData);

	void castRays(const b3AlignedObjectArray<b3RayInfo>& rays, b3AlignedObjectArray<b3RayHit>& hitResults,
				  i32 numBodies, const struct b3RigidBodyData* bodies, i32 numCollidables, const struct b3Collidable* collidables,
				  const struct b3GpuNarrowPhaseInternalData* narrowphaseData, class b3GpuBroadphaseInterface* broadphase);
};

#endif  //D3_GPU_RAYCAST_H
