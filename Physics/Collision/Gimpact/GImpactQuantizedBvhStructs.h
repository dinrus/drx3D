#ifndef GIM_QUANTIZED_SET_STRUCTS_H_INCLUDED
#define GIM_QUANTIZED_SET_STRUCTS_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/GImpactBvh.h>
#include <drx3D/Physics/Collision/Gimpact/Quantization.h>

//QuantizedBvhNode is a compressed aabb node, 16 bytes.
///Node can be used for leafnode or internal node. Leafnodes can point to 32-bit triangle index (non-negative range).
ATTRIBUTE_ALIGNED16 ( struct )
DRX3D_QUANTIZED_BVH_NODE
{
	//12 bytes
	u16 m_quantizedAabbMin[3];
	u16 m_quantizedAabbMax[3];
	//4 bytes
	i32 m_escapeIndexOrDataIndex;

	DRX3D_QUANTIZED_BVH_NODE()
	{
		m_escapeIndexOrDataIndex = 0;
	}

	SIMD_FORCE_INLINE bool isLeafNode() const
	{
		//skipindex is negative (internal node), triangleindex >=0 (leafnode)
		return ( m_escapeIndexOrDataIndex >= 0 );
	}

	SIMD_FORCE_INLINE i32 getEscapeIndex() const
	{
		//Assert(m_escapeIndexOrDataIndex < 0);
		return -m_escapeIndexOrDataIndex;
	}

	SIMD_FORCE_INLINE void setEscapeIndex ( i32 index )
	{
		m_escapeIndexOrDataIndex = -index;
	}

	SIMD_FORCE_INLINE i32 getDataIndex() const
	{
		//Assert(m_escapeIndexOrDataIndex >= 0);

		return m_escapeIndexOrDataIndex;
	}

	SIMD_FORCE_INLINE void setDataIndex ( i32 index )
	{
		m_escapeIndexOrDataIndex = index;
	}

	SIMD_FORCE_INLINE bool testQuantizedBoxOverlapp (
		unsigned short* quantizedMin, unsigned short* quantizedMax ) const
	{
		if ( m_quantizedAabbMin[0] > quantizedMax[0] ||
			 m_quantizedAabbMax[0] < quantizedMin[0] ||
			 m_quantizedAabbMin[1] > quantizedMax[1] ||
			 m_quantizedAabbMax[1] < quantizedMin[1] ||
			 m_quantizedAabbMin[2] > quantizedMax[2] ||
			 m_quantizedAabbMax[2] < quantizedMin[2] )
		{
			return false;
		}

		return true;
	}
};

#endif  // GIM_QUANTIZED_SET_STRUCTS_H_INCLUDED
