

#include <drx3D/Common/shared/b3Int4.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3BvhSubtreeInfoData.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3QuantizedBvhNodeData.h>

// work-in-progress
void b3BvhTraversal(__global const b3Int4* pairs,
					__global const b3RigidBodyData* rigidBodies,
					__global const b3Collidable* collidables,
					__global b3Aabb* aabbs,
					__global b3Int4* concavePairsOut,
					__global  i32* numConcavePairsOut,
					__global const b3BvhSubtreeInfo* subtreeHeadersRoot,
					__global const b3QuantizedBvhNode* quantizedNodesRoot,
					__global const b3BvhInfo* bvhInfos,
					i32 numPairs,
					i32 maxNumConcavePairsCapacity,
					i32 id)
{
	i32 bodyIndexA = pairs[id].x;
	i32 bodyIndexB = pairs[id].y;
	i32 collidableIndexA = rigidBodies[bodyIndexA].m_collidableIdx;
	i32 collidableIndexB = rigidBodies[bodyIndexB].m_collidableIdx;

	//once the broadphase avoids static-static pairs, we can remove this test
	if ((rigidBodies[bodyIndexA].m_invMass == 0) && (rigidBodies[bodyIndexB].m_invMass == 0))
	{
		return;
	}

	if (collidables[collidableIndexA].m_shapeType != SHAPE_CONCAVE_TRIMESH)
		return;

	i32 shapeTypeB = collidables[collidableIndexB].m_shapeType;

	if (shapeTypeB != SHAPE_CONVEX_HULL &&
		shapeTypeB != SHAPE_SPHERE &&
		shapeTypeB != SHAPE_COMPOUND_OF_CONVEX_HULLS)
		return;

	b3BvhInfo bvhInfo = bvhInfos[collidables[collidableIndexA].m_numChildShapes];

	b3Float4 bvhAabbMin = bvhInfo.m_aabbMin;
	b3Float4 bvhAabbMax = bvhInfo.m_aabbMax;
	b3Float4 bvhQuantization = bvhInfo.m_quantization;
	i32 numSubtreeHeaders = bvhInfo.m_numSubTrees;
	__global const b3BvhSubtreeInfoData* subtreeHeaders = &subtreeHeadersRoot[bvhInfo.m_subTreeOffset];
	__global const b3QuantizedBvhNodeData* quantizedNodes = &quantizedNodesRoot[bvhInfo.m_nodeOffset];

	u16 quantizedQueryAabbMin[3];
	u16 quantizedQueryAabbMax[3];
	b3QuantizeWithClamp(quantizedQueryAabbMin, aabbs[bodyIndexB].m_minVec, false, bvhAabbMin, bvhAabbMax, bvhQuantization);
	b3QuantizeWithClamp(quantizedQueryAabbMax, aabbs[bodyIndexB].m_maxVec, true, bvhAabbMin, bvhAabbMax, bvhQuantization);

	for (i32 i = 0; i < numSubtreeHeaders; i++)
	{
		b3BvhSubtreeInfoData subtree = subtreeHeaders[i];

		i32 overlap = b3TestQuantizedAabbAgainstQuantizedAabbSlow(quantizedQueryAabbMin, quantizedQueryAabbMax, subtree.m_quantizedAabbMin, subtree.m_quantizedAabbMax);
		if (overlap != 0)
		{
			i32 startNodeIndex = subtree.m_rootNodeIndex;
			i32 endNodeIndex = subtree.m_rootNodeIndex + subtree.m_subtreeSize;
			i32 curIndex = startNodeIndex;
			i32 escapeIndex;
			i32 isLeafNode;
			i32 aabbOverlap;
			while (curIndex < endNodeIndex)
			{
				b3QuantizedBvhNodeData rootNode = quantizedNodes[curIndex];
				aabbOverlap = b3TestQuantizedAabbAgainstQuantizedAabbSlow(quantizedQueryAabbMin, quantizedQueryAabbMax, rootNode.m_quantizedAabbMin, rootNode.m_quantizedAabbMax);
				isLeafNode = b3IsLeaf(&rootNode);
				if (aabbOverlap)
				{
					if (isLeafNode)
					{
						i32 triangleIndex = b3GetTriangleIndex(&rootNode);
						if (shapeTypeB == SHAPE_COMPOUND_OF_CONVEX_HULLS)
						{
							i32 numChildrenB = collidables[collidableIndexB].m_numChildShapes;
							i32 pairIdx = b3AtomicAdd(numConcavePairsOut, numChildrenB);
							for (i32 b = 0; b < numChildrenB; b++)
							{
								if ((pairIdx + b) < maxNumConcavePairsCapacity)
								{
									i32 childShapeIndexB = collidables[collidableIndexB].m_shapeIndex + b;
									b3Int4 newPair = b3MakeInt4(bodyIndexA, bodyIndexB, triangleIndex, childShapeIndexB);
									concavePairsOut[pairIdx + b] = newPair;
								}
							}
						}
						else
						{
							i32 pairIdx = b3AtomicInc(numConcavePairsOut);
							if (pairIdx < maxNumConcavePairsCapacity)
							{
								b3Int4 newPair = b3MakeInt4(bodyIndexA, bodyIndexB, triangleIndex, 0);
								concavePairsOut[pairIdx] = newPair;
							}
						}
					}
					curIndex++;
				}
				else
				{
					if (isLeafNode)
					{
						curIndex++;
					}
					else
					{
						escapeIndex = b3GetEscapeIndex(&rootNode);
						curIndex += escapeIndex;
					}
				}
			}
		}
	}
}