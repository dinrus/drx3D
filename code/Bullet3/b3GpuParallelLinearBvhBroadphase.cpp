
#include <drx3D/Physics/Collision/BroadPhase/b3GpuParallelLinearBvhBroadphase.h>

b3GpuParallelLinearBvhBroadphase::b3GpuParallelLinearBvhBroadphase ( cl_context context,
		cl_device_id device, cl_command_queue queue ) :
		m_plbvh ( context, device, queue ),

		m_overlappingPairsGpu ( context, queue ),

		m_aabbsGpu ( context, queue ),
		m_smallAabbsMappingGpu ( context, queue ),
		m_largeAabbsMappingGpu ( context, queue )
{
}

void b3GpuParallelLinearBvhBroadphase::createProxy ( const b3Vec3& aabbMin,
		const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup,
		i32 collisionFilterMask )
{
	i32 newAabbIndex = m_aabbsCpu.size();

	b3SapAabb aabb;
	aabb.m_minVec = aabbMin;
	aabb.m_maxVec = aabbMax;

	aabb.m_minIndices[3] = userPtr;
	aabb.m_signedMaxIndices[3] = newAabbIndex;

	m_smallAabbsMappingCpu.push_back ( newAabbIndex );

	m_aabbsCpu.push_back ( aabb );
}

void b3GpuParallelLinearBvhBroadphase::createLargeProxy ( const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask )
{
	i32 newAabbIndex = m_aabbsCpu.size();

	b3SapAabb aabb;
	aabb.m_minVec = aabbMin;
	aabb.m_maxVec = aabbMax;

	aabb.m_minIndices[3] = userPtr;
	aabb.m_signedMaxIndices[3] = newAabbIndex;

	m_largeAabbsMappingCpu.push_back ( newAabbIndex );

	m_aabbsCpu.push_back ( aabb );
}

void b3GpuParallelLinearBvhBroadphase::calculateOverlappingPairs ( i32 maxPairs )
{
	//Reconstruct BVH
	m_plbvh.build ( m_aabbsGpu, m_smallAabbsMappingGpu, m_largeAabbsMappingGpu );

	//
	m_overlappingPairsGpu.resize ( maxPairs );
	m_plbvh.calculateOverlappingPairs ( m_overlappingPairsGpu );
}

void b3GpuParallelLinearBvhBroadphase::calculateOverlappingPairsHost ( i32 maxPairs )
{
	drx3DAssert ( 0 );  //CPU version not implemented
}

void b3GpuParallelLinearBvhBroadphase::writeAabbsToGpu()
{
	m_aabbsGpu.copyFromHost ( m_aabbsCpu );
	m_smallAabbsMappingGpu.copyFromHost ( m_smallAabbsMappingCpu );
	m_largeAabbsMappingGpu.copyFromHost ( m_largeAabbsMappingCpu );
}
