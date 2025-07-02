#ifndef DRX3D_QUANTIZED_BVH_H
#define DRX3D_QUANTIZED_BVH_H

class Serializer;

//#define DEBUG_CHECK_DEQUANTIZATION 1
#ifdef DEBUG_CHECK_DEQUANTIZATION
#ifdef __SPU__
#define printf spu_printf
#endif  //__SPU__

#include <stdio.h>
#include <stdlib.h>
#endif  //DEBUG_CHECK_DEQUANTIZATION

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define QuantizedBvhData QuantizedBvhDoubleData
#define OptimizedBvhNodeData OptimizedBvhNodeDoubleData
#define QuantizedBvhDataName "QuantizedBvhDoubleData"
#else
#define QuantizedBvhData QuantizedBvhFloatData
#define OptimizedBvhNodeData OptimizedBvhNodeFloatData
#define QuantizedBvhDataName "QuantizedBvhFloatData"
#endif

//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vclrf__m128.asp

//Note: currently we have 16 bytes per quantized node
#define MAX_SUBTREE_SIZE_IN_BYTES 2048

// 10 gives the potential for 1024 parts, with at most 2^21 (2097152) (minus one
// actually) triangles each (since the sign bit is reserved
#define MAX_NUM_PARTS_IN_BITS 10

//QuantizedBvhNode is a compressed aabb node, 16 bytes.
///Node can be used for leafnode or internal node. Leafnodes can point to 32-bit triangle index (non-negative range).
ATTRIBUTE_ALIGNED16(struct)
QuantizedBvhNode
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	//12 bytes
	u16 m_quantizedAabbMin[3];
	u16 m_quantizedAabbMax[3];
	//4 bytes
	i32 m_escapeIndexOrTriangleIndex;

	bool isLeafNode() const
	{
		//skipindex is negative (internal node), triangleindex >=0 (leafnode)
		return (m_escapeIndexOrTriangleIndex >= 0);
	}
	i32 getEscapeIndex() const
	{
		Assert(!isLeafNode());
		return -m_escapeIndexOrTriangleIndex;
	}
	i32 getTriangleIndex() const
	{
		Assert(isLeafNode());
		u32 x = 0;
		u32 y = (~(x & 0)) << (31 - MAX_NUM_PARTS_IN_BITS);
		// Get only the lower bits where the triangle index is stored
		return (m_escapeIndexOrTriangleIndex & ~(y));
	}
	i32 getPartId() const
	{
		Assert(isLeafNode());
		// Get only the highest bits where the part index is stored
		return (m_escapeIndexOrTriangleIndex >> (31 - MAX_NUM_PARTS_IN_BITS));
	}
};

/// OptimizedBvhNode contains both internal and leaf node information.
/// Total node size is 44 bytes / node. You can use the compressed version of 16 bytes.
ATTRIBUTE_ALIGNED16(struct)
OptimizedBvhNode
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	//32 bytes
	Vec3 m_aabbMinOrg;
	Vec3 m_aabbMaxOrg;

	//4
	i32 m_escapeIndex;

	//8
	//for child nodes
	i32 m_subPart;
	i32 m_triangleIndex;

	//pad the size to 64 bytes
	char m_padding[20];
};

//BvhSubtreeInfo provides info to gather a subtree of limited size
ATTRIBUTE_ALIGNED16(class)
BvhSubtreeInfo
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	//12 bytes
	u16 m_quantizedAabbMin[3];
	u16 m_quantizedAabbMax[3];
	//4 bytes, points to the root of the subtree
	i32 m_rootNodeIndex;
	//4 bytes
	i32 m_subtreeSize;
	i32 m_padding[3];

	BvhSubtreeInfo()
	{
		//memset(&m_padding[0], 0, sizeof(m_padding));
	}

	void setAabbFromQuantizeNode(const QuantizedBvhNode& quantizedNode)
	{
		m_quantizedAabbMin[0] = quantizedNode.m_quantizedAabbMin[0];
		m_quantizedAabbMin[1] = quantizedNode.m_quantizedAabbMin[1];
		m_quantizedAabbMin[2] = quantizedNode.m_quantizedAabbMin[2];
		m_quantizedAabbMax[0] = quantizedNode.m_quantizedAabbMax[0];
		m_quantizedAabbMax[1] = quantizedNode.m_quantizedAabbMax[1];
		m_quantizedAabbMax[2] = quantizedNode.m_quantizedAabbMax[2];
	}
};

class NodeOverlapCallback
{
public:
	virtual ~NodeOverlapCallback(){};

	virtual void processNode(i32 subPart, i32 triangleIndex) = 0;
};

#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///for code readability:
typedef AlignedObjectArray<OptimizedBvhNode> NodeArray;
typedef AlignedObjectArray<QuantizedBvhNode> QuantizedNodeArray;
typedef AlignedObjectArray<BvhSubtreeInfo> BvhSubtreeInfoArray;

///The QuantizedBvh class stores an AABB tree that can be quickly traversed on CPU and Cell SPU.
///It is used by the BvhTriangleMeshShape as midphase.
///It is recommended to use quantization for better performance and lower memory requirements.
ATTRIBUTE_ALIGNED16(class)
QuantizedBvh
{
public:
	enum TraversalMode
	{
		TRAVERSAL_STACKLESS = 0,
		TRAVERSAL_STACKLESS_CACHE_FRIENDLY,
		TRAVERSAL_RECURSIVE
	};

protected:
	Vec3 m_bvhAabbMin;
	Vec3 m_bvhAabbMax;
	Vec3 m_bvhQuantization;

	i32 m_bulletVersion;  //for serialization versioning. It could also be used to detect endianess.

	i32 m_curNodeIndex;
	//quantization data
	bool m_useQuantization;

	NodeArray m_leafNodes;
	NodeArray m_contiguousNodes;
	QuantizedNodeArray m_quantizedLeafNodes;
	QuantizedNodeArray m_quantizedContiguousNodes;

	TraversalMode m_traversalMode;
	BvhSubtreeInfoArray m_SubtreeHeaders;

	//This is only used for serialization so we don't have to add serialization directly to AlignedObjectArray
	mutable i32 m_subtreeHeaderCount;

	///two versions, one for quantized and normal nodes. This allows code-reuse while maintaining readability (no template/macro!)
	///this might be refactored into a virtual, it is usually not calculated at run-time
	void setInternalNodeAabbMin(i32 nodeIndex, const Vec3& aabbMin)
	{
		if (m_useQuantization)
		{
			quantize(&m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[0], aabbMin, 0);
		}
		else
		{
			m_contiguousNodes[nodeIndex].m_aabbMinOrg = aabbMin;
		}
	}
	void setInternalNodeAabbMax(i32 nodeIndex, const Vec3& aabbMax)
	{
		if (m_useQuantization)
		{
			quantize(&m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[0], aabbMax, 1);
		}
		else
		{
			m_contiguousNodes[nodeIndex].m_aabbMaxOrg = aabbMax;
		}
	}

	Vec3 getAabbMin(i32 nodeIndex) const
	{
		if (m_useQuantization)
		{
			return unQuantize(&m_quantizedLeafNodes[nodeIndex].m_quantizedAabbMin[0]);
		}
		//non-quantized
		return m_leafNodes[nodeIndex].m_aabbMinOrg;
	}
	Vec3 getAabbMax(i32 nodeIndex) const
	{
		if (m_useQuantization)
		{
			return unQuantize(&m_quantizedLeafNodes[nodeIndex].m_quantizedAabbMax[0]);
		}
		//non-quantized
		return m_leafNodes[nodeIndex].m_aabbMaxOrg;
	}

	void setInternalNodeEscapeIndex(i32 nodeIndex, i32 escapeIndex)
	{
		if (m_useQuantization)
		{
			m_quantizedContiguousNodes[nodeIndex].m_escapeIndexOrTriangleIndex = -escapeIndex;
		}
		else
		{
			m_contiguousNodes[nodeIndex].m_escapeIndex = escapeIndex;
		}
	}

	void mergeInternalNodeAabb(i32 nodeIndex, const Vec3& newAabbMin, const Vec3& newAabbMax)
	{
		if (m_useQuantization)
		{
			u16 quantizedAabbMin[3];
			u16 quantizedAabbMax[3];
			quantize(quantizedAabbMin, newAabbMin, 0);
			quantize(quantizedAabbMax, newAabbMax, 1);
			for (i32 i = 0; i < 3; i++)
			{
				if (m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[i] > quantizedAabbMin[i])
					m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[i] = quantizedAabbMin[i];

				if (m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[i] < quantizedAabbMax[i])
					m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[i] = quantizedAabbMax[i];
			}
		}
		else
		{
			//non-quantized
			m_contiguousNodes[nodeIndex].m_aabbMinOrg.setMin(newAabbMin);
			m_contiguousNodes[nodeIndex].m_aabbMaxOrg.setMax(newAabbMax);
		}
	}

	void swapLeafNodes(i32 firstIndex, i32 secondIndex);

	void assignInternalNodeFromLeafNode(i32 internalNode, i32 leafNodeIndex);

protected:
	void buildTree(i32 startIndex, i32 endIndex);

	i32 calcSplittingAxis(i32 startIndex, i32 endIndex);

	i32 sortAndCalcSplittingIndex(i32 startIndex, i32 endIndex, i32 splitAxis);

	void walkStacklessTree(NodeOverlapCallback * nodeCallback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	void walkStacklessQuantizedTreeAgainstRay(NodeOverlapCallback * nodeCallback, const Vec3& raySource, const Vec3& rayTarget, const Vec3& aabbMin, const Vec3& aabbMax, i32 startNodeIndex, i32 endNodeIndex) const;
	void walkStacklessQuantizedTree(NodeOverlapCallback * nodeCallback, u16* quantizedQueryAabbMin, u16* quantizedQueryAabbMax, i32 startNodeIndex, i32 endNodeIndex) const;
	void walkStacklessTreeAgainstRay(NodeOverlapCallback * nodeCallback, const Vec3& raySource, const Vec3& rayTarget, const Vec3& aabbMin, const Vec3& aabbMax, i32 startNodeIndex, i32 endNodeIndex) const;

	///tree traversal designed for small-memory processors like PS3 SPU
	void walkStacklessQuantizedTreeCacheFriendly(NodeOverlapCallback * nodeCallback, u16* quantizedQueryAabbMin, u16* quantizedQueryAabbMax) const;

	///use the 16-byte stackless 'skipindex' node tree to do a recursive traversal
	void walkRecursiveQuantizedTreeAgainstQueryAabb(const QuantizedBvhNode* currentNode, NodeOverlapCallback* nodeCallback, u16* quantizedQueryAabbMin, u16* quantizedQueryAabbMax) const;

	///use the 16-byte stackless 'skipindex' node tree to do a recursive traversal
	void walkRecursiveQuantizedTreeAgainstQuantizedTree(const QuantizedBvhNode* treeNodeA, const QuantizedBvhNode* treeNodeB, NodeOverlapCallback* nodeCallback) const;

	void updateSubtreeHeaders(i32 leftChildNodexIndex, i32 rightChildNodexIndex);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	QuantizedBvh();

	virtual ~QuantizedBvh();

	///***************************************** expert/internal use only *************************
	void setQuantizationValues(const Vec3& bvhAabbMin, const Vec3& bvhAabbMax, Scalar quantizationMargin = Scalar(1.0));
	QuantizedNodeArray& getLeafNodeArray() { return m_quantizedLeafNodes; }
	///buildInternal is expert use only: assumes that setQuantizationValues and LeafNodeArray are initialized
	void buildInternal();
	///***************************************** expert/internal use only *************************

	void reportAabbOverlappingNodex(NodeOverlapCallback * nodeCallback, const Vec3& aabbMin, const Vec3& aabbMax) const;
	void reportRayOverlappingNodex(NodeOverlapCallback * nodeCallback, const Vec3& raySource, const Vec3& rayTarget) const;
	void reportBoxCastOverlappingNodex(NodeOverlapCallback * nodeCallback, const Vec3& raySource, const Vec3& rayTarget, const Vec3& aabbMin, const Vec3& aabbMax) const;

	SIMD_FORCE_INLINE void quantize(unsigned short* out, const Vec3& point, i32 isMax) const
	{
		Assert(m_useQuantization);

		Assert(point.getX() <= m_bvhAabbMax.getX());
		Assert(point.getY() <= m_bvhAabbMax.getY());
		Assert(point.getZ() <= m_bvhAabbMax.getZ());

		Assert(point.getX() >= m_bvhAabbMin.getX());
		Assert(point.getY() >= m_bvhAabbMin.getY());
		Assert(point.getZ() >= m_bvhAabbMin.getZ());

		Vec3 v = (point - m_bvhAabbMin) * m_bvhQuantization;
		///Make sure rounding is done in a way that unQuantize(quantizeWithClamp(...)) is conservative
		///end-points always set the first bit, so that they are sorted properly (so that neighbouring AABBs overlap properly)
		///@todo: double-check this
		if (isMax)
		{
			out[0] = (unsigned short)(((unsigned short)(v.getX() + Scalar(1.)) | 1));
			out[1] = (unsigned short)(((unsigned short)(v.getY() + Scalar(1.)) | 1));
			out[2] = (unsigned short)(((unsigned short)(v.getZ() + Scalar(1.)) | 1));
		}
		else
		{
			out[0] = (unsigned short)(((unsigned short)(v.getX()) & 0xfffe));
			out[1] = (unsigned short)(((unsigned short)(v.getY()) & 0xfffe));
			out[2] = (unsigned short)(((unsigned short)(v.getZ()) & 0xfffe));
		}

#ifdef DEBUG_CHECK_DEQUANTIZATION
		Vec3 newPoint = unQuantize(out);
		if (isMax)
		{
			if (newPoint.getX() < point.getX())
			{
				printf("unconservative X, diffX = %f, oldX=%f,newX=%f\n", newPoint.getX() - point.getX(), newPoint.getX(), point.getX());
			}
			if (newPoint.getY() < point.getY())
			{
				printf("unconservative Y, diffY = %f, oldY=%f,newY=%f\n", newPoint.getY() - point.getY(), newPoint.getY(), point.getY());
			}
			if (newPoint.getZ() < point.getZ())
			{
				printf("unconservative Z, diffZ = %f, oldZ=%f,newZ=%f\n", newPoint.getZ() - point.getZ(), newPoint.getZ(), point.getZ());
			}
		}
		else
		{
			if (newPoint.getX() > point.getX())
			{
				printf("unconservative X, diffX = %f, oldX=%f,newX=%f\n", newPoint.getX() - point.getX(), newPoint.getX(), point.getX());
			}
			if (newPoint.getY() > point.getY())
			{
				printf("unconservative Y, diffY = %f, oldY=%f,newY=%f\n", newPoint.getY() - point.getY(), newPoint.getY(), point.getY());
			}
			if (newPoint.getZ() > point.getZ())
			{
				printf("unconservative Z, diffZ = %f, oldZ=%f,newZ=%f\n", newPoint.getZ() - point.getZ(), newPoint.getZ(), point.getZ());
			}
		}
#endif  //DEBUG_CHECK_DEQUANTIZATION
	}

	SIMD_FORCE_INLINE void quantizeWithClamp(unsigned short* out, const Vec3& point2, i32 isMax) const
	{
		Assert(m_useQuantization);

		Vec3 clampedPoint(point2);
		clampedPoint.setMax(m_bvhAabbMin);
		clampedPoint.setMin(m_bvhAabbMax);

		quantize(out, clampedPoint, isMax);
	}

	SIMD_FORCE_INLINE Vec3 unQuantize(const unsigned short* vecIn) const
	{
		Vec3 vecOut;
		vecOut.setVal(
			(Scalar)(vecIn[0]) / (m_bvhQuantization.getX()),
			(Scalar)(vecIn[1]) / (m_bvhQuantization.getY()),
			(Scalar)(vecIn[2]) / (m_bvhQuantization.getZ()));
		vecOut += m_bvhAabbMin;
		return vecOut;
	}

	///setTraversalMode let's you choose between stackless, recursive or stackless cache friendly tree traversal. Note this is only implemented for quantized trees.
	void setTraversalMode(TraversalMode traversalMode)
	{
		m_traversalMode = traversalMode;
	}

	SIMD_FORCE_INLINE QuantizedNodeArray& getQuantizedNodeArray()
	{
		return m_quantizedContiguousNodes;
	}

	SIMD_FORCE_INLINE BvhSubtreeInfoArray& getSubtreeInfoArray()
	{
		return m_SubtreeHeaders;
	}

	////////////////////////////////////////////////////////////////////

	/////Calculate space needed to store BVH for serialization
	unsigned calculateSerializeBufferSize() const;

	/// Data buffer MUST be 16 byte aligned
	virtual bool serialize(uk o_alignedDataBuffer, unsigned i_dataBufferSize, bool i_swapEndian) const;

	///deSerializeInPlace loads and initializes a BVH from a buffer in memory 'in place'
	static QuantizedBvh* deSerializeInPlace(uk i_alignedDataBuffer, u32 i_dataBufferSize, bool i_swapEndian);

	static u32 getAlignmentSerializationPadding();
	//////////////////////////////////////////////////////////////////////

	virtual i32 calculateSerializeBufferSizeNew() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	virtual void deSerializeFloat(struct QuantizedBvhFloatData & quantizedBvhFloatData);

	virtual void deSerializeDouble(struct QuantizedBvhDoubleData & quantizedBvhDoubleData);

	////////////////////////////////////////////////////////////////////

	SIMD_FORCE_INLINE bool isQuantized()
	{
		return m_useQuantization;
	}

private:
	// Special "copy" constructor that allows for in-place deserialization
	// Prevents Vec3's default constructor from being called, but doesn't inialize much else
	// ownsMemory should most likely be false if deserializing, and if you are not, don't call this (it also changes the function signature, which we need)
	QuantizedBvh(QuantizedBvh & other, bool ownsMemory);
};

// clang-format off
// parser needs * with the name
struct BvhSubtreeInfoData
{
	i32 m_rootNodeIndex;
	i32 m_subtreeSize;
	unsigned short m_quantizedAabbMin[3];
	unsigned short m_quantizedAabbMax[3];
};

struct OptimizedBvhNodeFloatData
{
	Vec3FloatData m_aabbMinOrg;
	Vec3FloatData m_aabbMaxOrg;
	i32 m_escapeIndex;
	i32 m_subPart;
	i32 m_triangleIndex;
	char m_pad[4];
};

struct OptimizedBvhNodeDoubleData
{
	Vec3DoubleData m_aabbMinOrg;
	Vec3DoubleData m_aabbMaxOrg;
	i32 m_escapeIndex;
	i32 m_subPart;
	i32 m_triangleIndex;
	char m_pad[4];
};


struct QuantizedBvhNodeData
{
	unsigned short m_quantizedAabbMin[3];
	unsigned short m_quantizedAabbMax[3];
	i32	m_escapeIndexOrTriangleIndex;
};

struct	QuantizedBvhFloatData
{
	Vec3FloatData			m_bvhAabbMin;
	Vec3FloatData			m_bvhAabbMax;
	Vec3FloatData			m_bvhQuantization;
	i32					m_curNodeIndex;
	i32					m_useQuantization;
	i32					m_numContiguousLeafNodes;
	i32					m_numQuantizedContiguousNodes;
	OptimizedBvhNodeFloatData	*m_contiguousNodesPtr;
	QuantizedBvhNodeData		*m_quantizedContiguousNodesPtr;
	BvhSubtreeInfoData	*m_subTreeInfoPtr;
	i32					m_traversalMode;
	i32					m_numSubtreeHeaders;
	
};

struct	QuantizedBvhDoubleData
{
	Vec3DoubleData			m_bvhAabbMin;
	Vec3DoubleData			m_bvhAabbMax;
	Vec3DoubleData			m_bvhQuantization;
	i32							m_curNodeIndex;
	i32							m_useQuantization;
	i32							m_numContiguousLeafNodes;
	i32							m_numQuantizedContiguousNodes;
	OptimizedBvhNodeDoubleData	*m_contiguousNodesPtr;
	QuantizedBvhNodeData			*m_quantizedContiguousNodesPtr;

	i32							m_traversalMode;
	i32							m_numSubtreeHeaders;
	BvhSubtreeInfoData		*m_subTreeInfoPtr;
};
// clang-format on

SIMD_FORCE_INLINE i32 QuantizedBvh::calculateSerializeBufferSizeNew() const
{
	return sizeof(QuantizedBvhData);
}

#endif  //DRX3D_QUANTIZED_BVH_H
