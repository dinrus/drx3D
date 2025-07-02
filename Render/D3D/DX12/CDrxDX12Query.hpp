// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Resource/CDrxDX12Asynchronous.hpp>

class CDrxDX12Query : public CDrxDX12Asynchronous<ID3D11QueryToImplement>
{
public:
	DX12_OBJECT(CDrxDX12Query, CDrxDX12Asynchronous<ID3D11QueryToImplement> );

	static CDrxDX12Query* Create(ID3D12Device* pDevice, const D3D11_QUERY_DESC* pDesc);

	#pragma region /* ID3D11Asynchronous implementation */

	VIRTUALGFX UINT STDMETHODCALLTYPE GetDataSize() FINALGFX
	{
		if (m_Desc.Query == D3D11_QUERY_EVENT)
			return sizeof(BOOL);
		else if (m_Desc.Query == D3D11_QUERY_TIMESTAMP)
			return sizeof(UINT64);
		else if (m_Desc.Query == D3D11_QUERY_TIMESTAMP_DISJOINT)
			return sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT);
		else if (m_Desc.Query == D3D11_QUERY_OCCLUSION)
			return sizeof(UINT64);
		else if (m_Desc.Query == D3D11_QUERY_OCCLUSION_PREDICATE)
			return sizeof(UINT64);
		else if (m_Desc.Query == D3D11_QUERY_PIPELINE_STATISTICS)
			return sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS);
		else
			return 0;
	}

	#pragma endregion

	#pragma region /* ID3D11Query implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_QUERY_DESC* pDesc) FINALGFX;

	#pragma endregion

protected:
	CDrxDX12Query(const D3D11_QUERY_DESC* pDesc);

	D3D11_QUERY_DESC m_Desc;
};

class CDrxDX12EventQuery : public CDrxDX12Query
{
public:
	DX12_OBJECT(CDrxDX12EventQuery, CDrxDX12Query);

	CDrxDX12EventQuery(const D3D11_QUERY_DESC* pDesc);

	bool   Init(ID3D12Device* pDevice);

	UINT64 GetFenceValue() const
	{
		return m_FenceValue;
	}

	void SetFenceValue(UINT64 fenceValue)
	{
		m_FenceValue = fenceValue;
	}

private:
	UINT64 m_FenceValue;
};

class CDrxDX12ResourceQuery : public CDrxDX12EventQuery
{
public:
	DX12_OBJECT(CDrxDX12ResourceQuery, CDrxDX12EventQuery);

	CDrxDX12ResourceQuery(const D3D11_QUERY_DESC* pDesc);

	bool Init(ID3D12Device* pDevice);

	UINT GetQueryIndex() const
	{
		return m_QueryIndex;
	}

	void SetQueryIndex(UINT queryIndex)
	{
		m_QueryIndex = queryIndex;
	}

	ID3D12Resource* GetQueryResource() const
	{
		return m_pResource;
	}

	void SetQueryResource(ID3D12Resource* pResource)
	{
		m_pResource = pResource;
	}

private:
	UINT            m_QueryIndex;
	ID3D12Resource* m_pResource;
};
