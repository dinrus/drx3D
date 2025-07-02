// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLBase.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the reference counted base class for all
//               DXGL interface implementations
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLBASE__
#define __DRXDXGLBASE__

#include <drx3D/Render/D3D/DXGL/DrxDXGLGuid.hpp>
#include <drx3D/Render/D3D/DXGL/IDrxDXGLUnknown.hpp>

template<typename Interface>
struct SingleInterface
{
	template<typename Object>
	static bool Query(Object* pThis, REFIID riid, uk * ppvObject)
	{
		if (riid == __uuidof(Interface))
		{
			*reinterpret_cast<Interface**>(ppvObject) = static_cast<Interface*>(pThis);
			static_cast<Interface*>(pThis)->AddRef();
			return true;
		}
		return false;
	}
};

#include <drx3D/Render/D3D/DXGL/DXEmulation.hpp>

////////////////////////////////////////////////////////////////////////////
// Definition of basic types
////////////////////////////////////////////////////////////////////////////

class CDrxDXGLBase
#if !DXGL_FULL_EMULATION
	: public IUnknown
#endif //!DXGL_FULL_EMULATION
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLBase, Unknown)

	CDrxDXGLBase();
	virtual ~CDrxDXGLBase();

#if DXGL_FULL_EMULATION
	SAggregateNode&           GetAggregateHead();
#else
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject);
#endif //!DXGL_FULL_EMULATION
	ULONG STDMETHODCALLTYPE   AddRef();
	ULONG STDMETHODCALLTYPE   Release();
protected:
	u32         m_uRefCount;
#if DXGL_FULL_EMULATION
	SAggregateNode m_kAggregateHead;
#endif //DXGL_FULL_EMULATION
};

class CDrxDXGLPrivateDataContainer
{
public:
	CDrxDXGLPrivateDataContainer();
	~CDrxDXGLPrivateDataContainer();

	HRESULT GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData);
	HRESULT SetPrivateData(REFGUID guid, UINT DataSize, ukk pData);
	HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);
protected:
	void    RemovePrivateData(REFGUID guid);
protected:
	struct SPrivateData;

	struct SGuidHashCompare
	{
		size_t operator()(const GUID& kGuid) const;
		bool   operator()(const GUID& kLeft, const GUID& kRight) const;
	};

	typedef std::unordered_map<GUID, SPrivateData*, SGuidHashCompare, SGuidHashCompare> TPrivateDataMap;
	TPrivateDataMap m_kPrivateDataMap;
};

#endif //__DRXDXGLBASE__
