// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/CDrxDX12Resource.hpp>

#include <drx3D/Render/DX12/Resource/Misc/CDrxDX12Buffer.hpp>

#include <drx3D/Render/DX12/API/DX12View.hpp>

// "view" must be ID3D11*View
// This is potentialy dangerous, but easy & fast...
#define DX12_EXTRACT_IDRXDX12VIEW(view) \
	((view) ? (static_cast<IDrxDX12View*>(reinterpret_cast<CDrxDX12RenderTargetView*>(view))) : NULL)

#define DX12_EXTRACT_DX12VIEW(view) \
	((view) ? &(static_cast<IDrxDX12View*>(reinterpret_cast<CDrxDX12RenderTargetView*>(view)))->GetDX12View() : NULL)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct IDrxDX12View
{
	virtual ID3D12Resource*  GetD3D12Resource() const = 0;

	virtual NDrxDX12::CView& GetDX12View() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class CDrxDX12View : public CDrxDX12DeviceChild<T>, public IDrxDX12View
{
public:
	DX12_OBJECT(CDrxDX12View, CDrxDX12DeviceChild<T> );

	virtual ~CDrxDX12View()
	{
		m_DX12View.Invalidate();
	}

	ID3D11Resource* GetD3D11Resource() const
	{
		return m_pResource11;
	}

	ID3D12Resource* GetD3D12Resource() const
	{
		return m_rDX12Resource.GetD3D12Resource();
	}

	NDrxDX12::CResource& GetDX12Resource() const
	{
		return m_rDX12Resource;
	}

	NDrxDX12::CView& GetDX12View()
	{
		return m_DX12View;
	}

	STxt GetResourceName()
	{
		IDrxDX12Resource* ires = DX12_EXTRACT_IDRXDX12RESOURCE(m_pResource11.get());
		CDrxDX12Resource<ID3D11ResourceToImplement>* cres = static_cast<CDrxDX12Resource<ID3D11ResourceToImplement>*>(ires);
		return cres ? cres->GetName() : "-";
	}

	template<class T>
	void SetResourceState(T* pCmdList, D3D12_RESOURCE_STATES desiredState)
	{
		pCmdList->SetResourceState(m_rDX12Resource, m_DX12View, desiredState);
	}

	#pragma region /* ID3D11View implementation */

	VIRTUALGFX void STDMETHODCALLTYPE GetResource(
	  _Out_ ID3D11Resource** ppResource) FINALGFX
	{
		if (m_pResource11)
		{
			*ppResource = m_pResource11.get();
			(*ppResource)->AddRef();
		}
		else
		{
			*ppResource = NULL;
		}
	}

	#pragma endregion

protected:
	CDrxDX12View(ID3D11Resource* pResource11, NDrxDX12::EViewType viewType)
		: Super(nullptr, nullptr)
		, m_pResource11(pResource11)
		, m_rDX12Resource(DX12_EXTRACT_IDRXDX12RESOURCE(pResource11)->GetDX12Resource())
	{
		m_DX12View.Init(m_rDX12Resource, viewType);
	}

	NDrxDX12::CView m_DX12View;

	DX12_PTR(ID3D11Resource) m_pResource11;

	NDrxDX12::CResource& m_rDX12Resource;
};
