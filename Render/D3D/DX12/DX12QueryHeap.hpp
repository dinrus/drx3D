// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12Base.hpp>

namespace NDrxDX12 {

class CQueryHeap : public CDeviceObject
{
public:
	CQueryHeap(CDevice* device);
	virtual ~CQueryHeap();

	bool             Init(CDevice* device, const D3D12_QUERY_HEAP_DESC& desc);
	void             Reset();

	ID3D12QueryHeap* GetD3D12QueryHeap() const { return /*PassAddRef*/ (m_pQueryHeap); }

	UINT             GetType() const           { return m_Desc12.Type; }
	UINT             GetCapacity() const       { return m_Desc12.Count; }

private:
	DX12_PTR(ID3D12QueryHeap) m_pQueryHeap;
	D3D12_QUERY_HEAP_DESC m_Desc12;
};

}
