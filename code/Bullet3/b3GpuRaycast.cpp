
#include <drx3D/OpenCL/Raycast/b3GpuRaycast.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/OpenCL/RigidBody/b3GpuNarrowPhaseInternalData.h>

#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>
#include <drx3D/Physics/Collision/BroadPhase/b3GpuBroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/b3GpuParallelLinearBvh.h>

#include <drx3D/OpenCL/Raycast/kernels/rayCastKernels.h>

#define D3_RAYCAST_PATH "drx3D/Bullet3/OpenCL/Raycast/kernels/rayCastKernels.cl"

struct b3GpuRaycastInternalData
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_q;
	cl_kernel m_raytraceKernel;
	cl_kernel m_raytracePairsKernel;
	cl_kernel m_findRayRigidPairIndexRanges;

	b3GpuParallelLinearBvh* m_plbvh;
	b3RadixSort32CL* m_radixSorter;
	b3FillCL* m_fill;

	//1 element per ray
	b3OpenCLArray<b3RayInfo>* m_gpuRays;
	b3OpenCLArray<b3RayHit>* m_gpuHitResults;
	b3OpenCLArray<i32>* m_firstRayRigidPairIndexPerRay;
	b3OpenCLArray<i32>* m_numRayRigidPairsPerRay;

	//1 element per (ray index, rigid index) pair, where the ray intersects with the rigid's AABB
	b3OpenCLArray<i32>* m_gpuNumRayRigidPairs;
	b3OpenCLArray<b3Int2>* m_gpuRayRigidPairs;  //x == ray index, y == rigid index

	i32 m_test;
};

b3GpuRaycast::b3GpuRaycast(cl_context ctx, cl_device_id device, cl_command_queue q)
{
	m_data = new b3GpuRaycastInternalData;
	m_data->m_context = ctx;
	m_data->m_device = device;
	m_data->m_q = q;
	m_data->m_raytraceKernel = 0;
	m_data->m_raytracePairsKernel = 0;
	m_data->m_findRayRigidPairIndexRanges = 0;

	m_data->m_plbvh = new b3GpuParallelLinearBvh(ctx, device, q);
	m_data->m_radixSorter = new b3RadixSort32CL(ctx, device, q);
	m_data->m_fill = new b3FillCL(ctx, device, q);

	m_data->m_gpuRays = new b3OpenCLArray<b3RayInfo>(ctx, q);
	m_data->m_gpuHitResults = new b3OpenCLArray<b3RayHit>(ctx, q);
	m_data->m_firstRayRigidPairIndexPerRay = new b3OpenCLArray<i32>(ctx, q);
	m_data->m_numRayRigidPairsPerRay = new b3OpenCLArray<i32>(ctx, q);
	m_data->m_gpuNumRayRigidPairs = new b3OpenCLArray<i32>(ctx, q);
	m_data->m_gpuRayRigidPairs = new b3OpenCLArray<b3Int2>(ctx, q);

	{
		cl_int errNum = 0;
		cl_program prog = b3OpenCLUtils::compileCLProgramFromString(m_data->m_context, m_data->m_device, rayCastKernelCL, &errNum, "", D3_RAYCAST_PATH);
		drx3DAssert(errNum == CL_SUCCESS);
		m_data->m_raytraceKernel = b3OpenCLUtils::compileCLKernelFromString(m_data->m_context, m_data->m_device, rayCastKernelCL, "rayCastKernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_data->m_raytracePairsKernel = b3OpenCLUtils::compileCLKernelFromString(m_data->m_context, m_data->m_device, rayCastKernelCL, "rayCastPairsKernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_data->m_findRayRigidPairIndexRanges = b3OpenCLUtils::compileCLKernelFromString(m_data->m_context, m_data->m_device, rayCastKernelCL, "findRayRigidPairIndexRanges", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		clReleaseProgram(prog);
	}
}

b3GpuRaycast::~b3GpuRaycast()
{
	clReleaseKernel(m_data->m_raytraceKernel);
	clReleaseKernel(m_data->m_raytracePairsKernel);
	clReleaseKernel(m_data->m_findRayRigidPairIndexRanges);

	delete m_data->m_plbvh;
	delete m_data->m_radixSorter;
	delete m_data->m_fill;

	delete m_data->m_gpuRays;
	delete m_data->m_gpuHitResults;
	delete m_data->m_firstRayRigidPairIndexPerRay;
	delete m_data->m_numRayRigidPairsPerRay;
	delete m_data->m_gpuNumRayRigidPairs;
	delete m_data->m_gpuRayRigidPairs;

	delete m_data;
}

bool sphere_intersect(const b3Vec3& spherePos, b3Scalar radius, const b3Vec3& rayFrom, const b3Vec3& rayTo, float& hitFraction)
{
	b3Vec3 rs = rayFrom - spherePos;
	b3Vec3 rayDir = rayTo - rayFrom;

	float A = b3Dot(rayDir, rayDir);
	float B = b3Dot(rs, rayDir);
	float C = b3Dot(rs, rs) - (radius * radius);

	float D = B * B - A * C;

	if (D > 0.0)
	{
		float t = (-B - sqrt(D)) / A;

		if ((t >= 0.0f) && (t < hitFraction))
		{
			hitFraction = t;
			return true;
		}
	}
	return false;
}

bool rayConvex(const b3Vec3& rayFromLocal, const b3Vec3& rayToLocal, const b3ConvexPolyhedronData& poly,
			   const b3AlignedObjectArray<b3GpuFace>& faces, float& hitFraction, b3Vec3& hitNormal)
{
	float exitFraction = hitFraction;
	float enterFraction = -0.1f;
	b3Vec3 curHitNormal = b3MakeVector3(0, 0, 0);
	for (i32 i = 0; i < poly.m_numFaces; i++)
	{
		const b3GpuFace& face = faces[poly.m_faceOffset + i];
		float fromPlaneDist = b3Dot(rayFromLocal, face.m_plane) + face.m_plane.w;
		float toPlaneDist = b3Dot(rayToLocal, face.m_plane) + face.m_plane.w;
		if (fromPlaneDist < 0.f)
		{
			if (toPlaneDist >= 0.f)
			{
				float fraction = fromPlaneDist / (fromPlaneDist - toPlaneDist);
				if (exitFraction > fraction)
				{
					exitFraction = fraction;
				}
			}
		}
		else
		{
			if (toPlaneDist < 0.f)
			{
				float fraction = fromPlaneDist / (fromPlaneDist - toPlaneDist);
				if (enterFraction <= fraction)
				{
					enterFraction = fraction;
					curHitNormal = face.m_plane;
					curHitNormal.w = 0.f;
				}
			}
			else
			{
				return false;
			}
		}
		if (exitFraction <= enterFraction)
			return false;
	}

	if (enterFraction < 0.f)
		return false;

	hitFraction = enterFraction;
	hitNormal = curHitNormal;
	return true;
}

void b3GpuRaycast::castRaysHost(const b3AlignedObjectArray<b3RayInfo>& rays, b3AlignedObjectArray<b3RayHit>& hitResults,
								i32 numBodies, const struct b3RigidBodyData* bodies, i32 numCollidables, const struct b3Collidable* collidables, const struct b3GpuNarrowPhaseInternalData* narrowphaseData)
{
	//	return castRays(rays,hitResults,numBodies,bodies,numCollidables,collidables);

	D3_PROFILE("castRaysHost");
	for (i32 r = 0; r < rays.size(); r++)
	{
		b3Vec3 rayFrom = rays[r].m_from;
		b3Vec3 rayTo = rays[r].m_to;
		float hitFraction = hitResults[r].m_hitFraction;

		i32 hitBodyIndex = -1;
		b3Vec3 hitNormal;

		for (i32 b = 0; b < numBodies; b++)
		{
			const b3Vec3& pos = bodies[b].m_pos;
			//const b3Quat& orn = bodies[b].m_quat;

			switch (collidables[bodies[b].m_collidableIdx].m_shapeType)
			{
				case SHAPE_SPHERE:
				{
					b3Scalar radius = collidables[bodies[b].m_collidableIdx].m_radius;
					if (sphere_intersect(pos, radius, rayFrom, rayTo, hitFraction))
					{
						hitBodyIndex = b;
						b3Vec3 hitPoint;
						hitPoint.setInterpolate3(rays[r].m_from, rays[r].m_to, hitFraction);
						hitNormal = (hitPoint - bodies[b].m_pos).normalize();
					}
				}
				case SHAPE_CONVEX_HULL:
				{
					b3Transform convexWorldTransform;
					convexWorldTransform.setIdentity();
					convexWorldTransform.setOrigin(bodies[b].m_pos);
					convexWorldTransform.setRotation(bodies[b].m_quat);
					b3Transform convexWorld2Local = convexWorldTransform.inverse();

					b3Vec3 rayFromLocal = convexWorld2Local(rayFrom);
					b3Vec3 rayToLocal = convexWorld2Local(rayTo);

					i32 shapeIndex = collidables[bodies[b].m_collidableIdx].m_shapeIndex;
					const b3ConvexPolyhedronData& poly = narrowphaseData->m_convexPolyhedra[shapeIndex];
					if (rayConvex(rayFromLocal, rayToLocal, poly, narrowphaseData->m_convexFaces, hitFraction, hitNormal))
					{
						hitBodyIndex = b;
					}

					break;
				}
				default:
				{
					static bool once = true;
					if (once)
					{
						once = false;
						drx3DWarning("Raytest: unsupported shape type\n");
					}
				}
			}
		}
		if (hitBodyIndex >= 0)
		{
			hitResults[r].m_hitFraction = hitFraction;
			hitResults[r].m_hitPoint.setInterpolate3(rays[r].m_from, rays[r].m_to, hitFraction);
			hitResults[r].m_hitNormal = hitNormal;
			hitResults[r].m_hitBody = hitBodyIndex;
		}
	}
}
///todo: add some acceleration structure (AABBs, tree etc)
void b3GpuRaycast::castRays(const b3AlignedObjectArray<b3RayInfo>& rays, b3AlignedObjectArray<b3RayHit>& hitResults,
							i32 numBodies, const struct b3RigidBodyData* bodies, i32 numCollidables, const struct b3Collidable* collidables,
							const struct b3GpuNarrowPhaseInternalData* narrowphaseData, class b3GpuBroadphaseInterface* broadphase)
{
	//castRaysHost(rays,hitResults,numBodies,bodies,numCollidables,collidables,narrowphaseData);

	D3_PROFILE("castRaysGPU");

	{
		D3_PROFILE("raycast copyFromHost");
		m_data->m_gpuRays->copyFromHost(rays);
		m_data->m_gpuHitResults->copyFromHost(hitResults);
	}

	i32 numRays = hitResults.size();
	{
		m_data->m_firstRayRigidPairIndexPerRay->resize(numRays);
		m_data->m_numRayRigidPairsPerRay->resize(numRays);

		m_data->m_gpuNumRayRigidPairs->resize(1);
		m_data->m_gpuRayRigidPairs->resize(numRays * 16);
	}

	//run kernel
	const bool USE_BRUTE_FORCE_RAYCAST = false;
	if (USE_BRUTE_FORCE_RAYCAST)
	{
		D3_PROFILE("raycast launch1D");

		b3LauncherCL launcher(m_data->m_q, m_data->m_raytraceKernel, "m_raytraceKernel");
		i32 numRays = rays.size();
		launcher.setConst(numRays);

		launcher.setBuffer(m_data->m_gpuRays->getBufferCL());
		launcher.setBuffer(m_data->m_gpuHitResults->getBufferCL());

		launcher.setConst(numBodies);
		launcher.setBuffer(narrowphaseData->m_bodyBufferGPU->getBufferCL());
		launcher.setBuffer(narrowphaseData->m_collidablesGPU->getBufferCL());
		launcher.setBuffer(narrowphaseData->m_convexFacesGPU->getBufferCL());
		launcher.setBuffer(narrowphaseData->m_convexPolyhedraGPU->getBufferCL());

		launcher.launch1D(numRays);
		clFinish(m_data->m_q);
	}
	else
	{
		m_data->m_plbvh->build(broadphase->getAllAabbsGPU(), broadphase->getSmallAabbIndicesGPU(), broadphase->getLargeAabbIndicesGPU());

		m_data->m_plbvh->testRaysAgainstBvhAabbs(*m_data->m_gpuRays, *m_data->m_gpuNumRayRigidPairs, *m_data->m_gpuRayRigidPairs);

		i32 numRayRigidPairs = -1;
		m_data->m_gpuNumRayRigidPairs->copyToHostPointer(&numRayRigidPairs, 1);
		if (numRayRigidPairs > m_data->m_gpuRayRigidPairs->size())
		{
			numRayRigidPairs = m_data->m_gpuRayRigidPairs->size();
			m_data->m_gpuNumRayRigidPairs->copyFromHostPointer(&numRayRigidPairs, 1);
		}

		m_data->m_gpuRayRigidPairs->resize(numRayRigidPairs);  //Radix sort needs b3OpenCLArray::size() to be correct

		//Sort ray-rigid pairs by ray index
		{
			D3_PROFILE("sort ray-rigid pairs");
			m_data->m_radixSorter->execute(*reinterpret_cast<b3OpenCLArray<b3SortData>*>(m_data->m_gpuRayRigidPairs));
		}

		//detect start,count of each ray pair
		{
			D3_PROFILE("detect ray-rigid pair index ranges");

			{
				D3_PROFILE("reset ray-rigid pair index ranges");

				m_data->m_fill->execute(*m_data->m_firstRayRigidPairIndexPerRay, numRayRigidPairs, numRays);  //atomic_min used to find first index
				m_data->m_fill->execute(*m_data->m_numRayRigidPairsPerRay, 0, numRays);
				clFinish(m_data->m_q);
			}

			b3BufferInfoCL bufferInfo[] =
				{
					b3BufferInfoCL(m_data->m_gpuRayRigidPairs->getBufferCL()),

					b3BufferInfoCL(m_data->m_firstRayRigidPairIndexPerRay->getBufferCL()),
					b3BufferInfoCL(m_data->m_numRayRigidPairsPerRay->getBufferCL())};

			b3LauncherCL launcher(m_data->m_q, m_data->m_findRayRigidPairIndexRanges, "m_findRayRigidPairIndexRanges");
			launcher.setBuffers(bufferInfo, sizeof(bufferInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numRayRigidPairs);

			launcher.launch1D(numRayRigidPairs);
			clFinish(m_data->m_q);
		}

		{
			D3_PROFILE("ray-rigid intersection");

			b3BufferInfoCL bufferInfo[] =
				{
					b3BufferInfoCL(m_data->m_gpuRays->getBufferCL()),
					b3BufferInfoCL(m_data->m_gpuHitResults->getBufferCL()),
					b3BufferInfoCL(m_data->m_firstRayRigidPairIndexPerRay->getBufferCL()),
					b3BufferInfoCL(m_data->m_numRayRigidPairsPerRay->getBufferCL()),

					b3BufferInfoCL(narrowphaseData->m_bodyBufferGPU->getBufferCL()),
					b3BufferInfoCL(narrowphaseData->m_collidablesGPU->getBufferCL()),
					b3BufferInfoCL(narrowphaseData->m_convexFacesGPU->getBufferCL()),
					b3BufferInfoCL(narrowphaseData->m_convexPolyhedraGPU->getBufferCL()),

					b3BufferInfoCL(m_data->m_gpuRayRigidPairs->getBufferCL())};

			b3LauncherCL launcher(m_data->m_q, m_data->m_raytracePairsKernel, "m_raytracePairsKernel");
			launcher.setBuffers(bufferInfo, sizeof(bufferInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numRays);

			launcher.launch1D(numRays);
			clFinish(m_data->m_q);
		}
	}

	//copy results
	{
		D3_PROFILE("raycast copyToHost");
		m_data->m_gpuHitResults->copyToHost(hitResults);
	}
}