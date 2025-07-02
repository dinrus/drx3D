// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/DX12/API/DX12Base.hpp>

#define DX12_BASE_OBJECT(className, interfaceName)         \
	typedef className     Class;                           \
	typedef               DX12_PTR (Class) Ptr;            \
	typedef               DX12_PTR (const Class) ConstPtr; \
	typedef interfaceName Super;                           \
	typedef interfaceName Interface;

#define DX12_OBJECT(className, superName)              \
	typedef className Class;                           \
	typedef           DX12_PTR (Class) Ptr;            \
	typedef           DX12_PTR (const Class) ConstPtr; \
	typedef superName Super;

#include <drx3D/Render/DrxDX12Guid.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDrxDX12Buffer;
class CDrxDX12DepthStencilView;
class CDrxDX12Device;
class CDrxDX12DeviceContext;
class CDrxDX12MemoryUpr;
class CDrxDX12Query;
class CDrxDX12RenderTargetView;
class CDrxDX12SamplerState;
class CDrxDX12Shader;
class CDrxDX12ShaderResourceView;
class CDrxDX12SwapChain;
class CDrxDX12Texture1D;
class CDrxDX12Texture2D;
class CDrxDX12Texture3D;
class CDrxDX12UnorderedAccessView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class CDrxDX12Object : public T
{
public:
	DX12_BASE_OBJECT(CDrxDX12Object, T);

	CDrxDX12Object()
		: m_RefCount(0)
	{

	}

	virtual ~CDrxDX12Object()
	{
		DX12_LOG(g_nPrintDX12, "CDrxDX12 object destroyed: %p", this);
	}

	#pragma region /* IUnknown implementation */

	VIRTUALGFX ULONG STDMETHODCALLTYPE AddRef() FINALGFX
	{
		return DrxInterlockedIncrement(&m_RefCount);
	}

	VIRTUALGFX ULONG STDMETHODCALLTYPE Release() FINALGFX
	{
		ULONG RefCount;
		if (!(RefCount = DrxInterlockedDecrement(&m_RefCount)))
		{
			delete this;
			return 0;
		}

		return RefCount;
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE QueryInterface(
	  REFIID riid,
	  uk * ppvObject) FINALGFX
	{
		if (
		  (riid == __uuidof(T)) ||
		  (riid == __uuidof(ID3D11Device       ) && __uuidof(ID3D11Device1ToImplement       ) == __uuidof(T)) ||
		  (riid == __uuidof(ID3D11DeviceContext) && __uuidof(ID3D11DeviceContext1ToImplement) == __uuidof(T))
		)
		{
			if (ppvObject)
			{
				*reinterpret_cast<T**>(ppvObject) = static_cast<T*>(this);
				static_cast<T*>(this)->AddRef();
			}

			return S_OK;
		}

		return E_NOINTERFACE;
	}

	#pragma endregion

private:
	i32 m_RefCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class CDrxDX12GIObject : public T
{
public:
	DX12_BASE_OBJECT(CDrxDX12GIObject, T);

	CDrxDX12GIObject()
		: m_RefCount(0)
	{

	}

	virtual ~CDrxDX12GIObject()
	{

	}

	#pragma region /* IUnknown implementation */

	VIRTUALGFX ULONG STDMETHODCALLTYPE AddRef() FINALGFX
	{
		return DrxInterlockedIncrement(&m_RefCount);
	}

	VIRTUALGFX ULONG STDMETHODCALLTYPE Release() FINALGFX
	{
		ULONG RefCount;
		if (!(RefCount = DrxInterlockedDecrement(&m_RefCount)))
		{
			delete this;
			return 0;
		}

		return RefCount;
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE QueryInterface(
	  REFIID riid,
	  uk * ppvObject) FINALGFX
	{
		if (
		  (riid == __uuidof(T)) ||
#if !DRX_PLATFORM_DURANGO
		  (riid == __uuidof(IDXGIDevice   ) && __uuidof(IDXGIDevice3   ) == __uuidof(T)) ||
#endif
		  (riid == __uuidof(IDXGIFactory  ) && __uuidof(IDXGIFactory4  ) == __uuidof(T)) ||
		  (riid == __uuidof(IDXGIAdapter  ) && __uuidof(IDXGIAdapter3  ) == __uuidof(T)) ||
		  (riid == __uuidof(IDXGIOutput   ) && __uuidof(IDXGIOutput4   ) == __uuidof(T)) ||
		  (riid == __uuidof(IDXGISwapChain) && __uuidof(IDXGISwapChain3) == __uuidof(T))
		)
		{
			if (ppvObject)
			{
				*reinterpret_cast<T**>(ppvObject) = static_cast<T*>(this);
				static_cast<T*>(this)->AddRef();
			}

			return S_OK;
		}

		return E_NOINTERFACE;
	}

	#pragma endregion

	#pragma region /* IDXGIObject implementation */

	VIRTUALGFX HRESULT STDMETHODCALLTYPE SetPrivateData(
	  _In_ REFGUID Name,
	  UINT DataSize,
	  _In_reads_bytes_(DataSize) ukk pData) FINALGFX
	{
		return -1;
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
	  _In_ REFGUID Name,
	  _In_ const IUnknown* pUnknown) FINALGFX
	{
		return -1;
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE GetPrivateData(
	  _In_ REFGUID Name,
	  _Inout_ UINT* pDataSize,
	  _Out_writes_bytes_(*pDataSize) uk pData) FINALGFX
	{
		return -1;
	}

	VIRTUALGFX HRESULT STDMETHODCALLTYPE GetParent(
	  _In_ REFIID riid,
	  _Outvoid** ppParent) FINALGFX
	{
		return -1;
	}

	#pragma endregion

private:
	i32 m_RefCount;
};
