#ifndef DRX3D_GIMPACT_BVH_H_INCLUDED
#define DRX3D_GIMPACT_BVH_H_INCLUDED

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <drx3D/Physics/Collision/Gimpact/BoxCollision.h>
#include <drx3D/Physics/Collision/Gimpact/TriangleShapeEx.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactBvhStructs.h>

//! A pairset array
class PairSet : public AlignedObjectArray<GIM_PAIR>
{
public:
	PairSet()
	{
		reserve(32);
	}
	inline void push_pair(i32 index1, i32 index2)
	{
		push_back(GIM_PAIR(index1, index2));
	}

	inline void push_pair_inv(i32 index1, i32 index2)
	{
		push_back(GIM_PAIR(index2, index1));
	}
};

class GIM_BVH_DATA_ARRAY : public AlignedObjectArray<GIM_BVH_DATA>
{
};

class GIM_BVH_TREE_NODE_ARRAY : public AlignedObjectArray<GIM_BVH_TREE_NODE>
{
};

//! Basic Box tree structure
class BvhTree
{
protected:
	i32 m_num_nodes;
	GIM_BVH_TREE_NODE_ARRAY m_node_array;

protected:
	i32 _sort_and_calc_splitting_index(
		GIM_BVH_DATA_ARRAY& primitive_boxes,
		i32 startIndex, i32 endIndex, i32 splitAxis);

	i32 _calc_splitting_axis(GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex);

	void _build_sub_tree(GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex);

public:
	BvhTree()
	{
		m_num_nodes = 0;
	}

	//! prototype functions for box tree management
	//!@{
	void build_tree(GIM_BVH_DATA_ARRAY& primitive_boxes);

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
		bound = m_node_array[nodeindex].m_bound;
	}

	SIMD_FORCE_INLINE void setNodeBound(i32 nodeindex, const AABB& bound)
	{
		m_node_array[nodeindex].m_bound = bound;
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

	SIMD_FORCE_INLINE const GIM_BVH_TREE_NODE* get_node_pointer(i32 index = 0) const
	{
		return &m_node_array[index];
	}

	//!@}
};

//! Prototype Base class for primitive classification
/*!
This class is a wrapper for primitive collections.
This tells relevant info for the Bounding Box set classes, which take care of space classification.
This class can manage Compound shapes and trimeshes, and if it is managing trimesh then the  Hierarchy Bounding Box classes will take advantage of primitive Vs Box overlapping tests for getting optimal results and less Per Box compairisons.
*/
class PrimitiveManagerBase
{
public:
	virtual ~PrimitiveManagerBase() {}

	//! determines if this manager consist on only triangles, which special case will be optimized
	virtual bool is_trimesh() const = 0;
	virtual i32 get_primitive_count() const = 0;
	virtual void get_primitive_box(i32 prim_index, AABB& primbox) const = 0;
	//! retrieves only the points of the triangle, and the collision margin
	virtual void get_primitive_triangle(i32 prim_index, PrimitiveTriangle& triangle) const = 0;
};

//! Structure for containing Boxes
/*!
This class offers an structure for managing a box tree of primitives.
Requires a Primitive prototype (like PrimitiveManagerBase )
*/
class GImpactBvh
{
protected:
	BvhTree m_box_tree;
	PrimitiveManagerBase* m_primitive_manager;

protected:
	//stackless refit
	void refit();

public:
	//! this constructor doesn't build the tree. you must call	buildSet
	GImpactBvh()
	{
		m_primitive_manager = NULL;
	}

	//! this constructor doesn't build the tree. you must call	buildSet
	GImpactBvh(PrimitiveManagerBase* primitive_manager)
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

	SIMD_FORCE_INLINE const GIM_BVH_TREE_NODE* get_node_pointer(i32 index = 0) const
	{
		return m_box_tree.get_node_pointer(index);
	}

#ifdef TRI_COLLISION_PROFILING
	static float getAverageTreeCollisionTime();
#endif  //TRI_COLLISION_PROFILING

	static void find_collision(GImpactBvh* boxset1, const Transform2& trans1,
							   GImpactBvh* boxset2, const Transform2& trans2,
							   PairSet& collision_pairs);
};

#endif  // DRX3D_GIMPACT_BVH_H_INCLUDED
