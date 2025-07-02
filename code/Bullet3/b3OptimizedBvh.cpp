#include <drx3D/Physics/Collision/NarrowPhase/b3OptimizedBvh.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3StridingMeshInterface.h>
#include <drx3D/Geometry/b3AabbUtil.h>

b3OptimizedBvh::b3OptimizedBvh()
{
}

b3OptimizedBvh::~b3OptimizedBvh()
{
}

void b3OptimizedBvh::build(b3StridingMeshInterface* triangles, bool useQuantizedAabbCompression, const b3Vec3& bvhAabbMin, const b3Vec3& bvhAabbMax)
{
	m_useQuantization = useQuantizedAabbCompression;

	// NodeArray	triangleNodes;

	struct NodeTriangleCallback : public b3InternalTriangleIndexCallback
	{
		NodeArray& m_triangleNodes;

		NodeTriangleCallback& operator=(NodeTriangleCallback& other)
		{
			m_triangleNodes.copyFromArray(other.m_triangleNodes);
			return *this;
		}

		NodeTriangleCallback(NodeArray& triangleNodes)
			: m_triangleNodes(triangleNodes)
		{
		}

		virtual void internalProcessTriangleIndex(b3Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			b3OptimizedBvhNode node;
			b3Vec3 aabbMin, aabbMax;
			aabbMin.setVal(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));
			aabbMax.setVal(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
			aabbMin.setMin(triangle[0]);
			aabbMax.setMax(triangle[0]);
			aabbMin.setMin(triangle[1]);
			aabbMax.setMax(triangle[1]);
			aabbMin.setMin(triangle[2]);
			aabbMax.setMax(triangle[2]);

			//with quantization?
			node.m_aabbMinOrg = aabbMin;
			node.m_aabbMaxOrg = aabbMax;

			node.m_escapeIndex = -1;

			//for child nodes
			node.m_subPart = partId;
			node.m_triangleIndex = triangleIndex;
			m_triangleNodes.push_back(node);
		}
	};
	struct QuantizedNodeTriangleCallback : public b3InternalTriangleIndexCallback
	{
		QuantizedNodeArray& m_triangleNodes;
		const b3QuantizedBvh* m_optimizedTree;  // for quantization

		QuantizedNodeTriangleCallback& operator=(QuantizedNodeTriangleCallback& other)
		{
			m_triangleNodes.copyFromArray(other.m_triangleNodes);
			m_optimizedTree = other.m_optimizedTree;
			return *this;
		}

		QuantizedNodeTriangleCallback(QuantizedNodeArray& triangleNodes, const b3QuantizedBvh* tree)
			: m_triangleNodes(triangleNodes), m_optimizedTree(tree)
		{
		}

		virtual void internalProcessTriangleIndex(b3Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			// The partId and triangle index must fit in the same (positive) integer
			drx3DAssert(partId < (1 << MAX_NUM_PARTS_IN_BITS));
			drx3DAssert(triangleIndex < (1 << (31 - MAX_NUM_PARTS_IN_BITS)));
			//negative indices are reserved for escapeIndex
			drx3DAssert(triangleIndex >= 0);

			b3QuantizedBvhNode node;
			b3Vec3 aabbMin, aabbMax;
			aabbMin.setVal(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));
			aabbMax.setVal(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
			aabbMin.setMin(triangle[0]);
			aabbMax.setMax(triangle[0]);
			aabbMin.setMin(triangle[1]);
			aabbMax.setMax(triangle[1]);
			aabbMin.setMin(triangle[2]);
			aabbMax.setMax(triangle[2]);

			//PCK: add these checks for zero dimensions of aabb
			const b3Scalar MIN_AABB_DIMENSION = b3Scalar(0.002);
			const b3Scalar MIN_AABB_HALF_DIMENSION = b3Scalar(0.001);
			if (aabbMax.getX() - aabbMin.getX() < MIN_AABB_DIMENSION)
			{
				aabbMax.setX(aabbMax.getX() + MIN_AABB_HALF_DIMENSION);
				aabbMin.setX(aabbMin.getX() - MIN_AABB_HALF_DIMENSION);
			}
			if (aabbMax.getY() - aabbMin.getY() < MIN_AABB_DIMENSION)
			{
				aabbMax.setY(aabbMax.getY() + MIN_AABB_HALF_DIMENSION);
				aabbMin.setY(aabbMin.getY() - MIN_AABB_HALF_DIMENSION);
			}
			if (aabbMax.getZ() - aabbMin.getZ() < MIN_AABB_DIMENSION)
			{
				aabbMax.setZ(aabbMax.getZ() + MIN_AABB_HALF_DIMENSION);
				aabbMin.setZ(aabbMin.getZ() - MIN_AABB_HALF_DIMENSION);
			}

			m_optimizedTree->quantize(&node.m_quantizedAabbMin[0], aabbMin, 0);
			m_optimizedTree->quantize(&node.m_quantizedAabbMax[0], aabbMax, 1);

			node.m_escapeIndexOrTriangleIndex = (partId << (31 - MAX_NUM_PARTS_IN_BITS)) | triangleIndex;

			m_triangleNodes.push_back(node);
		}
	};

	i32 numLeafNodes = 0;

	if (m_useQuantization)
	{
		//initialize quantization values
		setQuantizationValues(bvhAabbMin, bvhAabbMax);

		QuantizedNodeTriangleCallback callback(m_quantizedLeafNodes, this);

		triangles->InternalProcessAllTriangles(&callback, m_bvhAabbMin, m_bvhAabbMax);

		//now we have an array of leafnodes in m_leafNodes
		numLeafNodes = m_quantizedLeafNodes.size();

		m_quantizedContiguousNodes.resize(2 * numLeafNodes);
	}
	else
	{
		NodeTriangleCallback callback(m_leafNodes);

		b3Vec3 aabbMin = b3MakeVector3(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
		b3Vec3 aabbMax = b3MakeVector3(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));

		triangles->InternalProcessAllTriangles(&callback, aabbMin, aabbMax);

		//now we have an array of leafnodes in m_leafNodes
		numLeafNodes = m_leafNodes.size();

		m_contiguousNodes.resize(2 * numLeafNodes);
	}

	m_curNodeIndex = 0;

	buildTree(0, numLeafNodes);

	///if the entire tree is small then subtree size, we need to create a header info for the tree
	if (m_useQuantization && !m_SubtreeHeaders.size())
	{
		b3BvhSubtreeInfo& subtree = m_SubtreeHeaders.expand();
		subtree.setAabbFromQuantizeNode(m_quantizedContiguousNodes[0]);
		subtree.m_rootNodeIndex = 0;
		subtree.m_subtreeSize = m_quantizedContiguousNodes[0].isLeafNode() ? 1 : m_quantizedContiguousNodes[0].getEscapeIndex();
	}

	//PCK: update the copy of the size
	m_subtreeHeaderCount = m_SubtreeHeaders.size();

	//PCK: clear m_quantizedLeafNodes and m_leafNodes, they are temporary
	m_quantizedLeafNodes.clear();
	m_leafNodes.clear();
}

void b3OptimizedBvh::refit(b3StridingMeshInterface* meshInterface, const b3Vec3& aabbMin, const b3Vec3& aabbMax)
{
	if (m_useQuantization)
	{
		setQuantizationValues(aabbMin, aabbMax);

		updateBvhNodes(meshInterface, 0, m_curNodeIndex, 0);

		///now update all subtree headers

		i32 i;
		for (i = 0; i < m_SubtreeHeaders.size(); i++)
		{
			b3BvhSubtreeInfo& subtree = m_SubtreeHeaders[i];
			subtree.setAabbFromQuantizeNode(m_quantizedContiguousNodes[subtree.m_rootNodeIndex]);
		}
	}
	else
	{
	}
}

void b3OptimizedBvh::refitPartial(b3StridingMeshInterface* meshInterface, const b3Vec3& aabbMin, const b3Vec3& aabbMax)
{
	//incrementally initialize quantization values
	drx3DAssert(m_useQuantization);

	drx3DAssert(aabbMin.getX() > m_bvhAabbMin.getX());
	drx3DAssert(aabbMin.getY() > m_bvhAabbMin.getY());
	drx3DAssert(aabbMin.getZ() > m_bvhAabbMin.getZ());

	drx3DAssert(aabbMax.getX() < m_bvhAabbMax.getX());
	drx3DAssert(aabbMax.getY() < m_bvhAabbMax.getY());
	drx3DAssert(aabbMax.getZ() < m_bvhAabbMax.getZ());

	///we should update all quantization values, using updateBvhNodes(meshInterface);
	///but we only update chunks that overlap the given aabb

	unsigned short quantizedQueryAabbMin[3];
	unsigned short quantizedQueryAabbMax[3];

	quantize(&quantizedQueryAabbMin[0], aabbMin, 0);
	quantize(&quantizedQueryAabbMax[0], aabbMax, 1);

	i32 i;
	for (i = 0; i < this->m_SubtreeHeaders.size(); i++)
	{
		b3BvhSubtreeInfo& subtree = m_SubtreeHeaders[i];

		//PCK: unsigned instead of bool
		unsigned overlap = b3TestQuantizedAabbAgainstQuantizedAabb(quantizedQueryAabbMin, quantizedQueryAabbMax, subtree.m_quantizedAabbMin, subtree.m_quantizedAabbMax);
		if (overlap != 0)
		{
			updateBvhNodes(meshInterface, subtree.m_rootNodeIndex, subtree.m_rootNodeIndex + subtree.m_subtreeSize, i);

			subtree.setAabbFromQuantizeNode(m_quantizedContiguousNodes[subtree.m_rootNodeIndex]);
		}
	}
}

void b3OptimizedBvh::updateBvhNodes(b3StridingMeshInterface* meshInterface, i32 firstNode, i32 endNode, i32 index)
{
	(void)index;

	drx3DAssert(m_useQuantization);

	i32 curNodeSubPart = -1;

	//get access info to trianglemesh data
	u8k* vertexbase = 0;
	i32 numverts = 0;
	PHY_ScalarType type = PHY_INTEGER;
	i32 stride = 0;
	u8k* indexbase = 0;
	i32 indexstride = 0;
	i32 numfaces = 0;
	PHY_ScalarType indicestype = PHY_INTEGER;

	b3Vec3 triangleVerts[3];
	b3Vec3 aabbMin, aabbMax;
	const b3Vec3& meshScaling = meshInterface->getScaling();

	i32 i;
	for (i = endNode - 1; i >= firstNode; i--)
	{
		b3QuantizedBvhNode& curNode = m_quantizedContiguousNodes[i];
		if (curNode.isLeafNode())
		{
			//recalc aabb from triangle data
			i32 nodeSubPart = curNode.getPartId();
			i32 nodeTriangleIndex = curNode.getTriangleIndex();
			if (nodeSubPart != curNodeSubPart)
			{
				if (curNodeSubPart >= 0)
					meshInterface->unLockReadOnlyVertexBase(curNodeSubPart);
				meshInterface->getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride, &indexbase, indexstride, numfaces, indicestype, nodeSubPart);

				curNodeSubPart = nodeSubPart;
			}
			//triangles->getLockedReadOnlyVertexIndexBase(vertexBase,numVerts,

			u32* gfxbase = (u32*)(indexbase + nodeTriangleIndex * indexstride);

			for (i32 j = 2; j >= 0; j--)
			{
				i32 graphicsindex;
                                switch (indicestype) {
                                        case PHY_INTEGER: graphicsindex = gfxbase[j]; break;
                                        case PHY_SHORT: graphicsindex = ((unsigned short*)gfxbase)[j]; break;
                                        case PHY_UCHAR: graphicsindex = ((u8*)gfxbase)[j]; break;
                                        default: drx3DAssert(0);
                                }
				if (type == PHY_FLOAT)
				{
					float* graphicsbase = (float*)(vertexbase + graphicsindex * stride);
					triangleVerts[j] = b3MakeVector3(
						graphicsbase[0] * meshScaling.getX(),
						graphicsbase[1] * meshScaling.getY(),
						graphicsbase[2] * meshScaling.getZ());
				}
				else
				{
					double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);
					triangleVerts[j] = b3MakeVector3(b3Scalar(graphicsbase[0] * meshScaling.getX()), b3Scalar(graphicsbase[1] * meshScaling.getY()), b3Scalar(graphicsbase[2] * meshScaling.getZ()));
				}
			}

			aabbMin.setVal(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));
			aabbMax.setVal(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
			aabbMin.setMin(triangleVerts[0]);
			aabbMax.setMax(triangleVerts[0]);
			aabbMin.setMin(triangleVerts[1]);
			aabbMax.setMax(triangleVerts[1]);
			aabbMin.setMin(triangleVerts[2]);
			aabbMax.setMax(triangleVerts[2]);

			quantize(&curNode.m_quantizedAabbMin[0], aabbMin, 0);
			quantize(&curNode.m_quantizedAabbMax[0], aabbMax, 1);
		}
		else
		{
			//combine aabb from both children

			b3QuantizedBvhNode* leftChildNode = &m_quantizedContiguousNodes[i + 1];

			b3QuantizedBvhNode* rightChildNode = leftChildNode->isLeafNode() ? &m_quantizedContiguousNodes[i + 2] : &m_quantizedContiguousNodes[i + 1 + leftChildNode->getEscapeIndex()];

			{
				for (i32 i = 0; i < 3; i++)
				{
					curNode.m_quantizedAabbMin[i] = leftChildNode->m_quantizedAabbMin[i];
					if (curNode.m_quantizedAabbMin[i] > rightChildNode->m_quantizedAabbMin[i])
						curNode.m_quantizedAabbMin[i] = rightChildNode->m_quantizedAabbMin[i];

					curNode.m_quantizedAabbMax[i] = leftChildNode->m_quantizedAabbMax[i];
					if (curNode.m_quantizedAabbMax[i] < rightChildNode->m_quantizedAabbMax[i])
						curNode.m_quantizedAabbMax[i] = rightChildNode->m_quantizedAabbMax[i];
				}
			}
		}
	}

	if (curNodeSubPart >= 0)
		meshInterface->unLockReadOnlyVertexBase(curNodeSubPart);
}

///deSerializeInPlace loads and initializes a BVH from a buffer in memory 'in place'
b3OptimizedBvh* b3OptimizedBvh::deSerializeInPlace(uk i_alignedDataBuffer, u32 i_dataBufferSize, bool i_swapEndian)
{
	b3QuantizedBvh* bvh = b3QuantizedBvh::deSerializeInPlace(i_alignedDataBuffer, i_dataBufferSize, i_swapEndian);

	//we don't add additional data so just do a static upcast
	return static_cast<b3OptimizedBvh*>(bvh);
}
