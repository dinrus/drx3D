// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GpuComputeBackend.h>
#include <drx3D/Render/D3D/GraphicsPipeline/Common/ComputeRenderPass.h>

namespace gpu
{
// same definition as in shader
const UINT BITONIC_BLOCK_SIZE = 1024;
const UINT TRANSPOSE_BLOCK_SIZE = 16;
const UINT NUM_ELEMENTS = (BITONIC_BLOCK_SIZE * BITONIC_BLOCK_SIZE);

class CBitonicSort
{
public:
	struct SBitonicSortItem
	{
		u32 key;
		u32 payload;
	};

	struct CParams
	{
		u32 iLevel;
		u32 iLevelMask;
		u32 iWidth;
		u32 iHeight;
	};
	CBitonicSort();
	void Sort(u32 numElements, CDeviceCommandListRef RESTRICT_REFERENCE commandList);
	CGpuBuffer& GetBuffer() { return m_data.GetBuffer(); }
private:
	void        SyncParams(u32 iLevel, u32 iLevelMask, u32 iWidth, u32 iHeight);
	CTypedConstantBuffer<CParams>                          m_params;
	CStructuredResource<SBitonicSortItem, BufferFlagsReadWrite> m_data;
	CStructuredResource<SBitonicSortItem, BufferFlagsReadWrite> m_transposeData;
	CComputeRenderPass m_computePassBitonicSort;
	CComputeRenderPass m_computePassBitonicTranspose;
};
}
