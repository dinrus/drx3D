#include <drx3D/Physics/Collision/Gimpact/GImpactBvh.h>
#include <drx3D/Maths/Linear/Quickprof.h>

#ifdef TRI_COLLISION_PROFILING

Clock g_tree_clock;

float g_accum_tree_collision_time = 0;
i32 g_count_traversing = 0;

void drx3d_begin_gim02_tree_time()
{
	g_tree_clock.reset();
}

void drx3d_end_gim02_tree_time()
{
	g_accum_tree_collision_time += g_tree_clock.getTimeMicroseconds();
	g_count_traversing++;
}

//! Gets the average time in miliseconds of tree collisions
float GImpactBvh::getAverageTreeCollisionTime()
{
	if (g_count_traversing == 0) return 0;

	float avgtime = g_accum_tree_collision_time;
	avgtime /= (float)g_count_traversing;

	g_accum_tree_collision_time = 0;
	g_count_traversing = 0;
	return avgtime;

	//	float avgtime = g_count_traversing;
	//	g_count_traversing = 0;
	//	return avgtime;
}

#endif  //TRI_COLLISION_PROFILING

/////////////////////// BvhTree /////////////////////////////////

i32 BvhTree::_calc_splitting_axis(
	GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex)
{
	i32 i;

	Vec3 means(Scalar(0.), Scalar(0.), Scalar(0.));
	Vec3 variance(Scalar(0.), Scalar(0.), Scalar(0.));
	i32 numIndices = endIndex - startIndex;

	for (i = startIndex; i < endIndex; i++)
	{
		Vec3 center = Scalar(0.5) * (primitive_boxes[i].m_bound.m_max +
											primitive_boxes[i].m_bound.m_min);
		means += center;
	}
	means *= (Scalar(1.) / (Scalar)numIndices);

	for (i = startIndex; i < endIndex; i++)
	{
		Vec3 center = Scalar(0.5) * (primitive_boxes[i].m_bound.m_max +
											primitive_boxes[i].m_bound.m_min);
		Vec3 diff2 = center - means;
		diff2 = diff2 * diff2;
		variance += diff2;
	}
	variance *= (Scalar(1.) / ((Scalar)numIndices - 1));

	return variance.maxAxis();
}

i32 BvhTree::_sort_and_calc_splitting_index(
	GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex,
	i32 endIndex, i32 splitAxis)
{
	i32 i;
	i32 splitIndex = startIndex;
	i32 numIndices = endIndex - startIndex;

	// average of centers
	Scalar splitValue = 0.0f;

	Vec3 means(Scalar(0.), Scalar(0.), Scalar(0.));
	for (i = startIndex; i < endIndex; i++)
	{
		Vec3 center = Scalar(0.5) * (primitive_boxes[i].m_bound.m_max +
											primitive_boxes[i].m_bound.m_min);
		means += center;
	}
	means *= (Scalar(1.) / (Scalar)numIndices);

	splitValue = means[splitAxis];

	//sort leafNodes so all values larger then splitValue comes first, and smaller values start from 'splitIndex'.
	for (i = startIndex; i < endIndex; i++)
	{
		Vec3 center = Scalar(0.5) * (primitive_boxes[i].m_bound.m_max +
											primitive_boxes[i].m_bound.m_min);
		if (center[splitAxis] > splitValue)
		{
			//swap
			primitive_boxes.swap(i, splitIndex);
			//swapLeafNodes(i,splitIndex);
			splitIndex++;
		}
	}

	//if the splitIndex causes unbalanced trees, fix this by using the center in between startIndex and endIndex
	//otherwise the tree-building might fail due to stack-overflows in certain cases.
	//unbalanced1 is unsafe: it can cause stack overflows
	//bool unbalanced1 = ((splitIndex==startIndex) || (splitIndex == (endIndex-1)));

	//unbalanced2 should work too: always use center (perfect balanced trees)
	//bool unbalanced2 = true;

	//this should be safe too:
	i32 rangeBalancedIndices = numIndices / 3;
	bool unbalanced = ((splitIndex <= (startIndex + rangeBalancedIndices)) || (splitIndex >= (endIndex - 1 - rangeBalancedIndices)));

	if (unbalanced)
	{
		splitIndex = startIndex + (numIndices >> 1);
	}

	Assert(!((splitIndex == startIndex) || (splitIndex == (endIndex))));

	return splitIndex;
}

void BvhTree::_build_sub_tree(GIM_BVH_DATA_ARRAY& primitive_boxes, i32 startIndex, i32 endIndex)
{
	i32 curIndex = m_num_nodes;
	m_num_nodes++;

	Assert((endIndex - startIndex) > 0);

	if ((endIndex - startIndex) == 1)
	{
		//We have a leaf node
		setNodeBound(curIndex, primitive_boxes[startIndex].m_bound);
		m_node_array[curIndex].setDataIndex(primitive_boxes[startIndex].m_data);

		return;
	}
	//calculate Best Splitting Axis and where to split it. Sort the incoming 'leafNodes' array within range 'startIndex/endIndex'.

	//split axis
	i32 splitIndex = _calc_splitting_axis(primitive_boxes, startIndex, endIndex);

	splitIndex = _sort_and_calc_splitting_index(
		primitive_boxes, startIndex, endIndex,
		splitIndex  //split axis
	);

	//calc this node bounding box

	AABB node_bound;
	node_bound.invalidate();

	for (i32 i = startIndex; i < endIndex; i++)
	{
		node_bound.merge(primitive_boxes[i].m_bound);
	}

	setNodeBound(curIndex, node_bound);

	//build left branch
	_build_sub_tree(primitive_boxes, startIndex, splitIndex);

	//build right branch
	_build_sub_tree(primitive_boxes, splitIndex, endIndex);

	m_node_array[curIndex].setEscapeIndex(m_num_nodes - curIndex);
}

//! stackless build tree
void BvhTree::build_tree(
	GIM_BVH_DATA_ARRAY& primitive_boxes)
{
	// initialize node count to 0
	m_num_nodes = 0;
	// allocate nodes
	m_node_array.resize(primitive_boxes.size() * 2);

	_build_sub_tree(primitive_boxes, 0, primitive_boxes.size());
}

////////////////////////////////////class GImpactBvh

void GImpactBvh::refit()
{
	i32 nodecount = getNodeCount();
	while (nodecount--)
	{
		if (isLeafNode(nodecount))
		{
			AABB leafbox;
			m_primitive_manager->get_primitive_box(getNodeData(nodecount), leafbox);
			setNodeBound(nodecount, leafbox);
		}
		else
		{
			//const GIM_BVH_TREE_NODE * nodepointer = get_node_pointer(nodecount);
			//get left bound
			AABB bound;
			bound.invalidate();

			AABB temp_box;

			i32 child_node = getLeftNode(nodecount);
			if (child_node)
			{
				getNodeBound(child_node, temp_box);
				bound.merge(temp_box);
			}

			child_node = getRightNode(nodecount);
			if (child_node)
			{
				getNodeBound(child_node, temp_box);
				bound.merge(temp_box);
			}

			setNodeBound(nodecount, bound);
		}
	}
}

//! this rebuild the entire set
void GImpactBvh::buildSet()
{
	//obtain primitive boxes
	GIM_BVH_DATA_ARRAY primitive_boxes;
	primitive_boxes.resize(m_primitive_manager->get_primitive_count());

	for (i32 i = 0; i < primitive_boxes.size(); i++)
	{
		m_primitive_manager->get_primitive_box(i, primitive_boxes[i].m_bound);
		primitive_boxes[i].m_data = i;
	}

	m_box_tree.build_tree(primitive_boxes);
}

//! returns the indices of the primitives in the m_primitive_manager
bool GImpactBvh::boxQuery(const AABB& box, AlignedObjectArray<i32>& collided_results) const
{
	i32 curIndex = 0;
	i32 numNodes = getNodeCount();

	while (curIndex < numNodes)
	{
		AABB bound;
		getNodeBound(curIndex, bound);

		//catch bugs in tree data

		bool aabbOverlap = bound.has_collision(box);
		bool isleafnode = isLeafNode(curIndex);

		if (isleafnode && aabbOverlap)
		{
			collided_results.push_back(getNodeData(curIndex));
		}

		if (aabbOverlap || isleafnode)
		{
			//next subnode
			curIndex++;
		}
		else
		{
			//skip node
			curIndex += getEscapeNodeIndex(curIndex);
		}
	}
	if (collided_results.size() > 0) return true;
	return false;
}

//! returns the indices of the primitives in the m_primitive_manager
bool GImpactBvh::rayQuery(
	const Vec3& ray_dir, const Vec3& ray_origin,
	AlignedObjectArray<i32>& collided_results) const
{
	i32 curIndex = 0;
	i32 numNodes = getNodeCount();

	while (curIndex < numNodes)
	{
		AABB bound;
		getNodeBound(curIndex, bound);

		//catch bugs in tree data

		bool aabbOverlap = bound.collide_ray(ray_origin, ray_dir);
		bool isleafnode = isLeafNode(curIndex);

		if (isleafnode && aabbOverlap)
		{
			collided_results.push_back(getNodeData(curIndex));
		}

		if (aabbOverlap || isleafnode)
		{
			//next subnode
			curIndex++;
		}
		else
		{
			//skip node
			curIndex += getEscapeNodeIndex(curIndex);
		}
	}
	if (collided_results.size() > 0) return true;
	return false;
}

SIMD_FORCE_INLINE bool _node_collision(
	GImpactBvh* boxset0, GImpactBvh* boxset1,
	const DRX3D_BOX_BOX_TRANSFORM_CACHE& trans_cache_1to0,
	i32 node0, i32 node1, bool complete_primitive_tests)
{
	AABB box0;
	boxset0->getNodeBound(node0, box0);
	AABB box1;
	boxset1->getNodeBound(node1, box1);

	return box0.overlapping_trans_cache(box1, trans_cache_1to0, complete_primitive_tests);
	//	box1.appy_transform_trans_cache(trans_cache_1to0);
	//	return box0.has_collision(box1);
}

//stackless recursive collision routine
static void _find_collision_pairs_recursive(
	GImpactBvh* boxset0, GImpactBvh* boxset1,
	PairSet* collision_pairs,
	const DRX3D_BOX_BOX_TRANSFORM_CACHE& trans_cache_1to0,
	i32 node0, i32 node1, bool complete_primitive_tests)
{
	if (_node_collision(
			boxset0, boxset1, trans_cache_1to0,
			node0, node1, complete_primitive_tests) == false) return;  //avoid colliding internal nodes

	if (boxset0->isLeafNode(node0))
	{
		if (boxset1->isLeafNode(node1))
		{
			// collision result
			collision_pairs->push_pair(
				boxset0->getNodeData(node0), boxset1->getNodeData(node1));
			return;
		}
		else
		{
			//collide left recursive

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				node0, boxset1->getLeftNode(node1), false);

			//collide right recursive
			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				node0, boxset1->getRightNode(node1), false);
		}
	}
	else
	{
		if (boxset1->isLeafNode(node1))
		{
			//collide left recursive
			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getLeftNode(node0), node1, false);

			//collide right recursive

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getRightNode(node0), node1, false);
		}
		else
		{
			//collide left0 left1

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getLeftNode(node0), boxset1->getLeftNode(node1), false);

			//collide left0 right1

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getLeftNode(node0), boxset1->getRightNode(node1), false);

			//collide right0 left1

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getRightNode(node0), boxset1->getLeftNode(node1), false);

			//collide right0 right1

			_find_collision_pairs_recursive(
				boxset0, boxset1,
				collision_pairs, trans_cache_1to0,
				boxset0->getRightNode(node0), boxset1->getRightNode(node1), false);

		}  // else if node1 is not a leaf
	}      // else if node0 is not a leaf
}

void GImpactBvh::find_collision(GImpactBvh* boxset0, const Transform2& trans0,
								  GImpactBvh* boxset1, const Transform2& trans1,
								  PairSet& collision_pairs)
{
	if (boxset0->getNodeCount() == 0 || boxset1->getNodeCount() == 0) return;

	DRX3D_BOX_BOX_TRANSFORM_CACHE trans_cache_1to0;

	trans_cache_1to0.calc_from_homogenic(trans0, trans1);

#ifdef TRI_COLLISION_PROFILING
	drx3d_begin_gim02_tree_time();
#endif  //TRI_COLLISION_PROFILING

	_find_collision_pairs_recursive(
		boxset0, boxset1,
		&collision_pairs, trans_cache_1to0, 0, 0, true);
#ifdef TRI_COLLISION_PROFILING
	drx3d_end_gim02_tree_time();
#endif  //TRI_COLLISION_PROFILING
}
