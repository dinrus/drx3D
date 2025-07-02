// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/DX12/Resource/CDrxDX12Resource.hpp>

class CDrxDX12Texture1D : public CDrxDX12Resource<ID3D11Texture1DToImplement>
{
public:
	DX12_OBJECT(CDrxDX12Texture1D, CDrxDX12Resource<ID3D11Texture1DToImplement> );

	static CDrxDX12Texture1D* Create(CDrxDX12Device* pDevice);
	static CDrxDX12Texture1D* Create(CDrxDX12Device* pDevice, CDrxDX12SwapChain* pSwapChain, ID3D12Resource* pResource);
	static CDrxDX12Texture1D* Create(CDrxDX12Device* pDevice, const FLOAT cClearValue[4], const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

	#pragma region /* IDrxDX12Resource implementation */

	virtual EDX12ResourceType GetDX12ResourceType() const final
	{
		return eDX12RT_Texture1D;
	}

	virtual void STDMETHODCALLTYPE GetType(_Out_ D3D11_RESOURCE_DIMENSION* pResourceDimension) final
	{
		if (pResourceDimension)
		{
			*pResourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE1D;
		}
	}

	#pragma endregion

	#pragma region /* ID3D11Texture1D implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_TEXTURE1D_DESC* pDesc) FINALGFX
	{
		if (pDesc)
		{
			*pDesc = m_Desc11;
		}
	}

	#pragma endregion

protected:
	CDrxDX12Texture1D(CDrxDX12Device* pDevice, const D3D11_TEXTURE1D_DESC& desc11, ID3D12Resource* pResource, D3D12_RESOURCE_STATES eInitialState, const CD3DX12_RESOURCE_DESC& desc12, const D3D11_SUBRESOURCE_DATA* pInitialData = NULL, size_t numInitialData = 0);

private:
	D3D11_TEXTURE1D_DESC m_Desc11;
};
