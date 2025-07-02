#ifndef D3_UPDATE_AABBS_H
#define D3_UPDATE_AABBS_H

#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>

void b3ComputeWorldAabb(i32 bodyId, __global const b3RigidBodyData_t* bodies, __global const b3Collidable_t* collidables, __global const b3Aabb_t* localShapeAABB, __global b3Aabb_t* worldAabbs)
{
	__global const b3RigidBodyData_t* body = &bodies[bodyId];

	b3Float4 position = body->m_pos;
	b3Quat orientation = body->m_quat;

	i32 collidableIndex = body->m_collidableIdx;
	i32 shapeIndex = collidables[collidableIndex].m_shapeIndex;

	if (shapeIndex >= 0)
	{
		b3Aabb_t localAabb = localShapeAABB[collidableIndex];
		b3Aabb_t worldAabb;

		b3Float4 aabbAMinOut, aabbAMaxOut;
		float margin = 0.f;
		b3TransformAabb2(localAabb.m_minVec, localAabb.m_maxVec, margin, position, orientation, &aabbAMinOut, &aabbAMaxOut);

		worldAabb.m_minVec = aabbAMinOut;
		worldAabb.m_minIndices[3] = bodyId;
		worldAabb.m_maxVec = aabbAMaxOut;
		worldAabb.m_signedMaxIndices[3] = body[bodyId].m_invMass == 0.f ? 0 : 1;
		worldAabbs[bodyId] = worldAabb;
	}
}

#endif  //D3_UPDATE_AABBS_H
