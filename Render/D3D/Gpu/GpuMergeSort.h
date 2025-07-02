// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     05/12/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Render/GpuComputeBackend.h>
#include <drx3D/Render/D3D/GraphicsPipeline/Common/ComputeRenderPass.h>

namespace gpu
{
class CMergeSort
{
public:
	struct SMergeSortItem
	{
		u32 key;
		u32 payload;
	};

	struct CParams
	{
		i32 inputBlockSize;
	};

	CMergeSort(u32 maxElements);
	// numElements needs to be power-of-two and <= maxElements
	void Sort(u32 numElements, CDeviceCommandListRef RESTRICT_REFERENCE commandList);
	CGpuBuffer& GetBuffer() { return m_data.Get().GetBuffer(); }

private:
	void SyncParams(uint inputBlockSize);
	i32 m_maxElements;
	gpu::CTypedConstantBuffer<CParams> m_params;
	gpu::CDoubleBuffered<gpu::CTypedResource<SMergeSortItem, gpu::BufferFlagsReadWriteReadback>> m_data;
	CComputeRenderPass                 m_passesMergeSort[2];
};
}
