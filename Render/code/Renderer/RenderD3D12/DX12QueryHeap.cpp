// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DX12QueryHeap.hpp>

namespace NDrxDX12
{

//---------------------------------------------------------------------------------------------------------------------
CQueryHeap::CQueryHeap(CDevice* device)
	: CDeviceObject(device)
{

}

//---------------------------------------------------------------------------------------------------------------------
CQueryHeap::~CQueryHeap()
{

}

//---------------------------------------------------------------------------------------------------------------------
bool CQueryHeap::Init(CDevice* device, const D3D12_QUERY_HEAP_DESC& desc)
{
	if (!m_pQueryHeap)
	{
		SetDevice(device);

		ID3D12QueryHeap* heap;
		GetDevice()->GetD3D12Device()->CreateQueryHeap(&desc, IID_GFX_ARGS(&heap));

		m_pQueryHeap = heap;
		heap->Release();

		m_Desc12 = desc /*m_pQueryHeap->GetDesc()*/;
	}

	Reset();
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void CQueryHeap::Reset()
{
}

}
