// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     05/12/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/GpuMergeSort.h>

namespace gpu
{

CMergeSort::CMergeSort(u32 maxElements) : m_data(maxElements), m_maxElements(maxElements)
{
	m_params.CreateDeviceBuffer();
	m_data.Initialize(true);
	CShader* pShader = gcpRendD3D.m_cEF.mfForName("GpuMergeSort", EF_SYSTEM, 0, 0);

	m_passesMergeSort[0].SetTechnique(pShader, CDrxNameTSCRC("MergeSort"), 0);
	m_passesMergeSort[0].SetInlineConstantBuffer(8, m_params.GetDeviceConstantBuffer());
	m_passesMergeSort[0].SetOutputUAV(0, &m_data.Get().GetBuffer());
	m_passesMergeSort[0].SetOutputUAV(1, &m_data.GetBackBuffer().GetBuffer());

	m_passesMergeSort[1].SetTechnique(pShader, CDrxNameTSCRC("MergeSort"), 0);
	m_passesMergeSort[1].SetInlineConstantBuffer(8, m_params.GetDeviceConstantBuffer());
	m_passesMergeSort[1].SetOutputUAV(0, &m_data.GetBackBuffer().GetBuffer());
	m_passesMergeSort[1].SetOutputUAV(1, &m_data.Get().GetBuffer());
}

void CMergeSort::Sort(u32 numElements, CDeviceCommandListRef RESTRICT_REFERENCE commandList)
{
	PROFILE_LABEL_SCOPE("GPU MERGE SORT");

	DRX_ASSERT(numElements <= m_maxElements);

	i32k blocks = gpu::GetNumberOfBlocksForArbitaryNumberOfThreads(numElements, 1024);

	u32 block = 1;
	while (block < numElements)
	{
		SyncParams(block);
		i32 currentPass = m_data.GetCurrentBufferId();
		m_passesMergeSort[currentPass].PrepareResourcesForUse(commandList);
		m_passesMergeSort[currentPass].SetDispatchSize(blocks, 1, 1);
		m_passesMergeSort[currentPass].Execute(commandList);
		m_data.Swap();
		block *= 2;
	}
}

void CMergeSort::SyncParams(uint inputBlockSize)
{
	m_params->inputBlockSize = inputBlockSize;
	m_params.CopyToDevice();
}

}
