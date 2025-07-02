
#ifndef D3_BVH_SUBTREE_INFO_DATA_H
#define D3_BVH_SUBTREE_INFO_DATA_H

typedef struct b3BvhSubtreeInfoData b3BvhSubtreeInfoData_t;

struct b3BvhSubtreeInfoData
{
	//12 bytes
	u16 m_quantizedAabbMin[3];
	u16 m_quantizedAabbMax[3];
	//4 bytes, points to the root of the subtree
	i32 m_rootNodeIndex;
	//4 bytes
	i32 m_subtreeSize;
	i32 m_padding[3];
};

#endif  //D3_BVH_SUBTREE_INFO_DATA_H
