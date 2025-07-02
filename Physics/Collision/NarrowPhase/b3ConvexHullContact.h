
#ifndef _CONVEX_HULL_CONTACT_H
#define _CONVEX_HULL_CONTACT_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3ConvexPolyhedronData.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Contact4.h>
#include <drx3D/Common/shared/b3Int2.h>
#include <drx3D/Common/shared/b3Int4.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3OptimizedBvh.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3BvhInfo.h>
#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>

struct GpuSatCollision
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;
	cl_kernel m_findSeparatingAxisKernel;
	cl_kernel m_mprPenetrationKernel;
	cl_kernel m_findSeparatingAxisUnitSphereKernel;

	cl_kernel m_findSeparatingAxisVertexFaceKernel;
	cl_kernel m_findSeparatingAxisEdgeEdgeKernel;

	cl_kernel m_findConcaveSeparatingAxisKernel;
	cl_kernel m_findConcaveSeparatingAxisVertexFaceKernel;
	cl_kernel m_findConcaveSeparatingAxisEdgeEdgeKernel;

	cl_kernel m_findCompoundPairsKernel;
	cl_kernel m_processCompoundPairsKernel;

	cl_kernel m_clipHullHullKernel;
	cl_kernel m_clipCompoundsHullHullKernel;

	cl_kernel m_clipFacesAndFindContacts;
	cl_kernel m_findClippingFacesKernel;

	cl_kernel m_clipHullHullConcaveConvexKernel;
	//	cl_kernel				m_extractManifoldAndAddContactKernel;
	cl_kernel m_newContactReductionKernel;

	cl_kernel m_bvhTraversalKernel;
	cl_kernel m_primitiveContactsKernel;
	cl_kernel m_findConcaveSphereContactsKernel;

	cl_kernel m_processCompoundPairsPrimitivesKernel;

	b3OpenCLArray<b3Vec3> m_unitSphereDirections;

	b3OpenCLArray<i32> m_totalContactsOut;

	b3OpenCLArray<b3Vec3> m_sepNormals;
	b3OpenCLArray<float> m_dmins;

	b3OpenCLArray<i32> m_hasSeparatingNormals;
	b3OpenCLArray<b3Vec3> m_concaveSepNormals;
	b3OpenCLArray<i32> m_concaveHasSeparatingNormals;
	b3OpenCLArray<i32> m_numConcavePairsOut;
	b3OpenCLArray<b3CompoundOverlappingPair> m_gpuCompoundPairs;
	b3OpenCLArray<b3Vec3> m_gpuCompoundSepNormals;
	b3OpenCLArray<i32> m_gpuHasCompoundSepNormals;
	b3OpenCLArray<i32> m_numCompoundPairsOut;

	GpuSatCollision(cl_context ctx, cl_device_id device, cl_command_queue q);
	virtual ~GpuSatCollision();

	void computeConvexConvexContactsGPUSAT(b3OpenCLArray<b3Int4>* pairs, i32 nPairs,
										   const b3OpenCLArray<b3RigidBodyData>* bodyBuf,
										   b3OpenCLArray<b3Contact4>* contactOut, i32& nContacts,
										   const b3OpenCLArray<b3Contact4>* oldContacts,
										   i32 maxContactCapacity,
										   i32 compoundPairCapacity,
										   const b3OpenCLArray<b3ConvexPolyhedronData>& hostConvexData,
										   const b3OpenCLArray<b3Vec3>& vertices,
										   const b3OpenCLArray<b3Vec3>& uniqueEdges,
										   const b3OpenCLArray<b3GpuFace>& faces,
										   const b3OpenCLArray<i32>& indices,
										   const b3OpenCLArray<b3Collidable>& gpuCollidables,
										   const b3OpenCLArray<b3GpuChildShape>& gpuChildShapes,

										   const b3OpenCLArray<b3Aabb>& clAabbsWorldSpace,
										   const b3OpenCLArray<b3Aabb>& clAabbsLocalSpace,

										   b3OpenCLArray<b3Vec3>& worldVertsB1GPU,
										   b3OpenCLArray<b3Int4>& clippingFacesOutGPU,
										   b3OpenCLArray<b3Vec3>& worldNormalsAGPU,
										   b3OpenCLArray<b3Vec3>& worldVertsA1GPU,
										   b3OpenCLArray<b3Vec3>& worldVertsB2GPU,
										   b3AlignedObjectArray<class b3OptimizedBvh*>& bvhData,
										   b3OpenCLArray<b3QuantizedBvhNode>* treeNodesGPU,
										   b3OpenCLArray<b3BvhSubtreeInfo>* subTreesGPU,
										   b3OpenCLArray<b3BvhInfo>* bvhInfo,
										   i32 numObjects,
										   i32 maxTriConvexPairCapacity,
										   b3OpenCLArray<b3Int4>& triangleConvexPairs,
										   i32& numTriConvexPairsOut);
};

#endif  //_CONVEX_HULL_CONTACT_H
