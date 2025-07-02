#ifndef D3_BVH_INFO_H
#define D3_BVH_INFO_H

#include <drx3D/Common/b3Vec3.h>

struct b3BvhInfo
{
	b3Vec3 m_aabbMin;
	b3Vec3 m_aabbMax;
	b3Vec3 m_quantization;
	i32 m_numNodes;
	i32 m_numSubTrees;
	i32 m_nodeOffset;
	i32 m_subTreeOffset;
};

#endif  //D3_BVH_INFO_H