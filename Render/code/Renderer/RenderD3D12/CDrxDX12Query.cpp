// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12Query.hpp>

CDrxDX12Query* CDrxDX12Query::Create(ID3D12Device* pDevice, const D3D11_QUERY_DESC* pDesc)
{
	switch (pDesc->Query)
	{
	case D3D11_QUERY_EVENT:
		{
			auto pResult = DX12_NEW_RAW(CDrxDX12EventQuery(pDesc));
			if (!pResult->Init(pDevice))
				SAFE_RELEASE(pResult);

			return pResult;
		}
		break;
	case D3D11_QUERY_TIMESTAMP:
	case D3D11_QUERY_OCCLUSION:
	case D3D11_QUERY_OCCLUSION_PREDICATE:
		{
			auto pResult = DX12_NEW_RAW(CDrxDX12ResourceQuery(pDesc));
			if (!pResult->Init(pDevice))
				SAFE_RELEASE(pResult);

			return pResult;
		}
		break;

	case D3D11_QUERY_TIMESTAMP_DISJOINT:
	default:
		return DX12_NEW_RAW(CDrxDX12Query(pDesc));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12Query::CDrxDX12Query(const D3D11_QUERY_DESC* pDesc)
	: Super()
	, m_Desc(*pDesc)
{

}

/* ID3D11Query implementation */

void STDMETHODCALLTYPE CDrxDX12Query::GetDesc(
  _Out_ D3D11_QUERY_DESC* pDesc)
{
	*pDesc = m_Desc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12EventQuery::CDrxDX12EventQuery(const D3D11_QUERY_DESC* pDesc)
	: Super(pDesc)
	, m_FenceValue(0)
{
}

bool CDrxDX12EventQuery::Init(ID3D12Device* pDevice)
{
	m_FenceValue = 0LL;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12ResourceQuery::CDrxDX12ResourceQuery(const D3D11_QUERY_DESC* pDesc)
	: Super(pDesc)
	, m_QueryIndex(0)
	, m_pResource(nullptr)
{
}

bool CDrxDX12ResourceQuery::Init(ID3D12Device* pDevice)
{
	Super::Init(pDevice);
	m_QueryIndex = 0;
	m_pResource = nullptr;
	return true;
}
