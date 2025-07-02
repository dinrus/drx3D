#ifndef GIM_BOX_SET_STRUCT_H_INCLUDED
#define GIM_BOX_SET_STRUCT_H_INCLUDED

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Gimpact/BoxCollision.h>
#include <drx3D/Physics/Collision/Gimpact/TriangleShapeEx.h>
#include <drx3D/Physics/Collision/Gimpact/gim_pair.h> //for GIM_PAIR

///GIM_BVH_DATA is an internal GIMPACT collision structure to contain axis aligned bounding box
struct GIM_BVH_DATA
{
	AABB m_bound;
	i32 m_data;
};

//! Node Structure for trees
class GIM_BVH_TREE_NODE
{
public:
	AABB m_bound;

protected:
	i32 m_escapeIndexOrDataIndex;

public:
	GIM_BVH_TREE_NODE()
	{
		m_escapeIndexOrDataIndex = 0;
	}

	SIMD_FORCE_INLINE bool isLeafNode() const
	{
		//skipindex is negative (internal node), triangleindex >=0 (leafnode)
		return (m_escapeIndexOrDataIndex >= 0);
	}

	SIMD_FORCE_INLINE i32 getEscapeIndex() const
	{
		//Assert(m_escapeIndexOrDataIndex < 0);
		return -m_escapeIndexOrDataIndex;
	}

	SIMD_FORCE_INLINE void setEscapeIndex(i32 index)
	{
		m_escapeIndexOrDataIndex = -index;
	}

	SIMD_FORCE_INLINE i32 getDataIndex() const
	{
		//Assert(m_escapeIndexOrDataIndex >= 0);

		return m_escapeIndexOrDataIndex;
	}

	SIMD_FORCE_INLINE void setDataIndex(i32 index)
	{
		m_escapeIndexOrDataIndex = index;
	}
};

#endif  // GIM_BOXPRUNING_H_INCLUDED
