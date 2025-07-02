#ifndef GIM_QUANTIZED_SET_H_INCLUDED
#define GIM_QUANTIZED_SET_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/GImpactBvh.h>
#include <drx3D/Physics/Collision/Gimpact/Quantization.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactQuantizedBvhStructs.h>

class GIM_QUANTIZED_BVH_NODE_ARRAY : public AlignedObjectArray<DRX3D_QUANTIZED_BVH_NODE>
{
};

//! Basic Box tree structure
class QuantizedBvhTree
{
protected:
	i32 m_num_nodes;
	GIM_QUANTIZED_BVH_NODE_ARRAY m_node_array;
	AABB m_global_bound;
	Vec3 m_bvhQuantization;

protected:
	void calc_quantization(GIM_BVH_DATA_ARRAY& primitive_boxes, Scalar boundMargin = Scalar(1.0));

	i32 _sort_and_calc_splitting_index(
		GIM_BVH_DATA_ARRAY& primitive_boxes,
		i32 startIndex, i32 endIndex, i32 splitAxis);

	i32 _calc_splitting_axis(GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex);

	void _build_sub_tree(GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex);

public:
	QuantizedBvhTree()
	{
		m_num_nodes = 0;
	}

	//! prototype functions for box tree management
	//!@{
	void build_tree(GIM_BVH_DATA_ARRAY& primitive_boxes);

	SIMD_FORCE_INLINE void quantizePoint(
		unsigned short* quantizedpoint, const Vec3& point) const
	{
		drx3d_quantize_clamp(quantizedpoint, point, m_global_bound.m_min, m_global_bound.m_max, m_bvhQuantization);
	}

	SIMD_FORCE_INLINE bool testQuantizedBoxOverlapp(
		i32 node_index,
		unsigned short* quantizedMin, unsigned short* quantizedMax) const
	{
		return m_node_array[node_index].testQuantizedBoxOverlapp(quantizedMin, quantizedMax);
	}

	SIMD_FORCE_INLINE void clearNodes()
	{
		m_node_array.clear();
		m_num_nodes = 0;
	}

	//! node count
	SIMD_FORCE_INLINE i32 getNodeCount() const
	{
		return m_num_nodes;
	}

	//! tells if the node is a leaf
	SIMD_FORCE_INLINE bool isLeafNode(i32 nodeindex) const
	{
		return m_node_array[nodeindex].isLeafNode();
	}

	SIMD_FORCE_INLINE i32 getNodeData(i32 nodeindex) const
	{
		return m_node_array[nodeindex].getDataIndex();
	}

	SIMD_FORCE_INLINE void getNodeBound(i32 nodeindex, AABB& bound) const
	{
		bound.m_min = drx3d_unquantize(
			m_node_array[nodeindex].m_quantizedAabbMin,
			m_global_bound.m_min, m_bvhQuantization);

		bound.m_max = drx3d_unquantize(
			m_node_array[nodeindex].m_quantizedAabbMax,
			m_global_bound.m_min, m_bvhQuantization);
	}

	SIMD_FORCE_INLINE void setNodeBound(i32 nodeindex, const AABB& bound)
	{
		drx3d_quantize_clamp(m_node_array[nodeindex].m_quantizedAabbMin,
						  bound.m_min,
						  m_global_bound.m_min,
						  m_global_bound.m_max,
						  m_bvhQuantization);

		drx3d_quantize_clamp(m_node_array[nodeindex].m_quantizedAabbMax,
						  bound.m_max,
						  m_global_bound.m_min,
						  m_global_bound.m_max,
						  m_bvhQuantization);
	}

	SIMD_FORCE_INLINE i32 getLeftNode(i32 nodeindex) const
	{
		return nodeindex + 1;
	}

	SIMD_FORCE_INLINE i32 getRightNode(i32 nodeindex) const
	{
		if (m_node_array[nodeindex + 1].isLeafNode()) return nodeindex + 2;
		return nodeindex + 1 + m_node_array[nodeindex + 1].getEscapeIndex();
	}

	SIMD_FORCE_INLINE i32 getEscapeNodeIndex(i32 nodeindex) const
	{
		return m_node_array[nodeindex].getEscapeIndex();
	}

	SIMD_FORCE_INLINE const DRX3D_QUANTIZED_BVH_NODE* get_node_pointer(i32 index = 0) const
	{
		return &m_node_array[index];
	}

	//!@}
};

//! Structure for containing Boxes
/*!
This class offers an structure for managing a box tree of primitives.
Requires a Primitive prototype (like PrimitiveManagerBase )
*/
class GImpactQuantizedBvh
{
protected:
	QuantizedBvhTree m_box_tree;
	PrimitiveManagerBase* m_primitive_manager;

protected:
	//stackless refit
	void refit();

public:
	//! this constructor doesn't build the tree. you must call	buildSet
	GImpactQuantizedBvh()
	{
		m_primitive_manager = NULL;
	}

	//! this constructor doesn't build the tree. you must call	buildSet
	GImpactQuantizedBvh(PrimitiveManagerBase* primitive_manager)
	{
		m_primitive_manager = primitive_manager;
	}

	SIMD_FORCE_INLINE AABB getGlobalBox() const
	{
		AABB totalbox;
		getNodeBound(0, totalbox);
		return totalbox;
	}

	SIMD_FORCE_INLINE void setPrimitiveManager(PrimitiveManagerBase* primitive_manager)
	{
		m_primitive_manager = primitive_manager;
	}

	SIMD_FORCE_INLINE PrimitiveManagerBase* getPrimitiveManager() const
	{
		return m_primitive_manager;
	}

	//! node manager prototype functions
	///@{

	//! this attemps to refit the box set.
	SIMD_FORCE_INLINE void update()
	{
		refit();
	}

	//! this rebuild the entire set
	void buildSet();

	//! returns the indices of the primitives in the m_primitive_manager
	bool boxQuery(const AABB& box, AlignedObjectArray<i32>& collided_results) const;

	//! returns the indices of the primitives in the m_primitive_manager
	SIMD_FORCE_INLINE bool boxQueryTrans(const AABB& box,
										 const Transform2& transform, AlignedObjectArray<i32>& collided_results) const
	{
		AABB transbox = box;
		transbox.appy_transform(transform);
		return boxQuery(transbox, collided_results);
	}

	//! returns the indices of the primitives in the m_primitive_manager
	bool rayQuery(
		const Vec3& ray_dir, const Vec3& ray_origin,
		AlignedObjectArray<i32>& collided_results) const;

	//! tells if this set has hierarcht
	SIMD_FORCE_INLINE bool hasHierarchy() const
	{
		return true;
	}

	//! tells if this set is a trimesh
	SIMD_FORCE_INLINE bool isTrimesh() const
	{
		return m_primitive_manager->is_trimesh();
	}

	//! node count
	SIMD_FORCE_INLINE i32 getNodeCount() const
	{
		return m_box_tree.getNodeCount();
	}

	//! tells if the node is a leaf
	SIMD_FORCE_INLINE bool isLeafNode(i32 nodeindex) const
	{
		return m_box_tree.isLeafNode(nodeindex);
	}

	SIMD_FORCE_INLINE i32 getNodeData(i32 nodeindex) const
	{
		return m_box_tree.getNodeData(nodeindex);
	}

	SIMD_FORCE_INLINE void getNodeBound(i32 nodeindex, AABB& bound) const
	{
		m_box_tree.getNodeBound(nodeindex, bound);
	}

	SIMD_FORCE_INLINE void setNodeBound(i32 nodeindex, const AABB& bound)
	{
		m_box_tree.setNodeBound(nodeindex, bound);
	}

	SIMD_FORCE_INLINE i32 getLeftNode(i32 nodeindex) const
	{
		return m_box_tree.getLeftNode(nodeindex);
	}

	SIMD_FORCE_INLINE i32 getRightNode(i32 nodeindex) const
	{
		return m_box_tree.getRightNode(nodeindex);
	}

	SIMD_FORCE_INLINE i32 getEscapeNodeIndex(i32 nodeindex) const
	{
		return m_box_tree.getEscapeNodeIndex(nodeindex);
	}

	SIMD_FORCE_INLINE void getNodeTriangle(i32 nodeindex, PrimitiveTriangle& triangle) const
	{
		m_primitive_manager->get_primitive_triangle(getNodeData(nodeindex), triangle);
	}

	SIMD_FORCE_INLINE const DRX3D_QUANTIZED_BVH_NODE* get_node_pointer(i32 index = 0) const
	{
		return m_box_tree.get_node_pointer(index);
	}

#ifdef TRI_COLLISION_PROFILING
	static float getAverageTreeCollisionTime();
#endif  //TRI_COLLISION_PROFILING

	static void find_collision(const GImpactQuantizedBvh* boxset1, const Transform2& trans1,
							   const GImpactQuantizedBvh* boxset2, const Transform2& trans2,
							   PairSet& collision_pairs);
};

#endif  // GIM_BOXPRUNING_H_INCLUDED
