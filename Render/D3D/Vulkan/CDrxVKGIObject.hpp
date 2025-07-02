// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

//Adapted from DX12 implementation. CDRXVK* objects are refcounted and mimic DXGI/DX behavior

#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

//-- Macros
//---------------------------------------------------------------------------------------------------------------------
#define IMPLEMENT_INTERFACES(...)                                         \
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(                       \
    REFIID riid,                                                          \
    uk * ppvObject)                                                     \
  {                                                                       \
    bool found = checkInterfaces<__VA_ARGS__>(this, riid, ppvObject); \
    return found ? S_OK : E_NOINTERFACE;                                  \
  }

//-- Helper functions
//---------------------------------------------------------------------------------------------------------------------
template<typename Interface>
bool checkInterfaces(uk instance, REFIID riid, uk * ppvObject)
{
	if (riid == __uuidof(Interface))
	{
		if (ppvObject)
		{
			*reinterpret_cast<Interface**>(ppvObject) = static_cast<Interface*>(instance);
			static_cast<Interface*>(instance)->AddRef();
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------
template<typename Interface1, typename Interface2, typename ... Interfaces>
bool checkInterfaces(uk instance, REFIID riid, uk * ppvObject)
{
	if (checkInterfaces<Interface1>(instance, riid, ppvObject))
	{
		return true;
	}
	else
	{
		return checkInterfaces<Interface2, Interfaces ...>(instance, riid, ppvObject);

	}
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
static T* PassAddRef(T* ptr)
{
	if (ptr)
	{
		ptr->AddRef();
	}

	return ptr;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
static T* PassAddRef(const _smart_ptr<T>& ptr)
{
	if (ptr)
	{
		ptr.get()->AddRef();
	}

	return ptr.get();
}

//-- CDrxVK* Base types
//---------------------------------------------------------------------------------------------------------------------
#ifndef DRX_PLATFORM_WINDOWS
#include <drx3D/Render/DX12\Includes\Unknwn_empty.h>
#else
#include <Unknwnbase.h>
#endif
//---------------------------------------------------------------------------------------------------------------------
class CDrxVKObject : public IUnknown
{
public:

	CDrxVKObject()
		: m_RefCount(0)
	{

	}

	virtual ~CDrxVKObject()
	{

	}

	#pragma region /* IUnknown implementation */

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return DrxInterlockedIncrement(&m_RefCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		ULONG RefCount;
		if (!(RefCount = DrxInterlockedDecrement(&m_RefCount)))
		{
			delete this;
			return 0;
		}

		return RefCount;
	}

	#pragma endregion

private:
	i32 m_RefCount;
};

//---------------------------------------------------------------------------------------------------------------------

class CDrxVKGIObject : public CDrxVKObject
{
public:
	CDrxVKGIObject() { AddRef(); } // High level code assumes GI objects are create with refcount 1

	#pragma region /* IDXGIObject implementation */

	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
	  _In_ REFGUID Name,
	  UINT DataSize,
	  _In_reads_bytes_(DataSize) ukk pData)
	{
		return -1;
	}

	virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
	  _In_ REFGUID Name,
	  _In_ const IUnknown* pUnknown)
	{
		return -1;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
	  _In_ REFGUID Name,
	  _Inout_ UINT* pDataSize,
	  _Out_writes_bytes_(*pDataSize) uk pData)
	{
		return -1;
	}

	virtual HRESULT STDMETHODCALLTYPE GetParent(
	  _In_ REFIID riid,
	  _Outvoid** ppParent)
	{
		return -1;
	}

	#pragma endregion
};
