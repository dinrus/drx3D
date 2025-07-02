#include <drx3D/Physics/Collision/Gimpact/gim_box_set.h>

GUINT GIM_BOX_TREE::_calc_splitting_axis(
	gim_array<GIM_AABB_DATA>& primitive_boxes, GUINT startIndex, GUINT endIndex)
{
	GUINT i;

	Vec3 means(Scalar(0.), Scalar(0.), Scalar(0.));
	Vec3 variance(Scalar(0.), Scalar(0.), Scalar(0.));
	GUINT numIndices = endIndex - startIndex;

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

GUINT GIM_BOX_TREE::_sort_and_calc_splitting_index(
	gim_array<GIM_AABB_DATA>& primitive_boxes, GUINT startIndex,
	GUINT endIndex, GUINT splitAxis)
{
	GUINT i;
	GUINT splitIndex = startIndex;
	GUINT numIndices = endIndex - startIndex;

	// average of centers
	Scalar splitValue = 0.0f;
	for (i = startIndex; i < endIndex; i++)
	{
		splitValue += 0.5f * (primitive_boxes[i].m_bound.m_max[splitAxis] +
							  primitive_boxes[i].m_bound.m_min[splitAxis]);
	}
	splitValue /= (Scalar)numIndices;

	//sort leafNodes so all values larger then splitValue comes first, and smaller values start from 'splitIndex'.
	for (i = startIndex; i < endIndex; i++)
	{
		Scalar center = 0.5f * (primitive_boxes[i].m_bound.m_max[splitAxis] +
								  primitive_boxes[i].m_bound.m_min[splitAxis]);
		if (center > splitValue)
		{
			//swap
			primitive_boxes.swap(i, splitIndex);
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
	GUINT rangeBalancedIndices = numIndices / 3;
	bool unbalanced = ((splitIndex <= (startIndex + rangeBalancedIndices)) || (splitIndex >= (endIndex - 1 - rangeBalancedIndices)));

	if (unbalanced)
	{
		splitIndex = startIndex + (numIndices >> 1);
	}

	Assert(!((splitIndex == startIndex) || (splitIndex == (endIndex))));

	return splitIndex;
}

void GIM_BOX_TREE::_build_sub_tree(gim_array<GIM_AABB_DATA>& primitive_boxes, GUINT startIndex, GUINT endIndex)
{
	GUINT current_index = m_num_nodes++;

	Assert((endIndex - startIndex) > 0);

	if ((endIndex - startIndex) == 1)  //we got a leaf
	{
		m_node_array[current_index].m_left = 0;
		m_node_array[current_index].m_right = 0;
		m_node_array[current_index].m_escapeIndex = 0;

		m_node_array[current_index].m_bound = primitive_boxes[startIndex].m_bound;
		m_node_array[current_index].m_data = primitive_boxes[startIndex].m_data;
		return;
	}

	//configure inner node

	GUINT splitIndex;

	//calc this node bounding box
	m_node_array[current_index].m_bound.invalidate();
	for (splitIndex = startIndex; splitIndex < endIndex; splitIndex++)
	{
		m_node_array[current_index].m_bound.merge(primitive_boxes[splitIndex].m_bound);
	}

	//calculate Best Splitting Axis and where to split it. Sort the incoming 'leafNodes' array within range 'startIndex/endIndex'.

	//split axis
	splitIndex = _calc_splitting_axis(primitive_boxes, startIndex, endIndex);

	splitIndex = _sort_and_calc_splitting_index(
		primitive_boxes, startIndex, endIndex, splitIndex);

	//configure this inner node : the left node index
	m_node_array[current_index].m_left = m_num_nodes;
	//build left child tree
	_build_sub_tree(primitive_boxes, startIndex, splitIndex);

	//configure this inner node : the right node index
	m_node_array[current_index].m_right = m_num_nodes;

	//build right child tree
	_build_sub_tree(primitive_boxes, splitIndex, endIndex);

	//configure this inner node : the escape index
	m_node_array[current_index].m_escapeIndex = m_num_nodes - current_index;
}

//! stackless build tree
void GIM_BOX_TREE::build_tree(
	gim_array<GIM_AABB_DATA>& primitive_boxes)
{
	// initialize node count to 0
	m_num_nodes = 0;
	// allocate nodes
	m_node_array.resize(primitive_boxes.size() * 2);

	_build_sub_tree(primitive_boxes, 0, primitive_boxes.size());
}
