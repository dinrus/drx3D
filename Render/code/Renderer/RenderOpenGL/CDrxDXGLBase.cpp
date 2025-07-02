// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLBase.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the reference counted base class for all
//               DXGL interface implementations
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLBase.hpp>
#include <drx3D/Render/Implementation/GLCommon.hpp>

CDrxDXGLBase::CDrxDXGLBase()
	: m_uRefCount(1)
{
	DXGL_INITIALIZE_INTERFACE(Unknown);
}

CDrxDXGLBase::~CDrxDXGLBase()
{
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of IUnknown
////////////////////////////////////////////////////////////////////////////////

#if DXGL_FULL_EMULATION

SAggregateNode& CDrxDXGLBase::GetAggregateHead()
{
	return m_kAggregateHead;
}

#else

HRESULT CDrxDXGLBase::QueryInterface(REFIID riid, uk * ppvObject)
{
	return E_NOINTERFACE;
}

#endif

ULONG CDrxDXGLBase::AddRef(void)
{
	return ++m_uRefCount;
}

ULONG CDrxDXGLBase::Release(void)
{
	--m_uRefCount;
	if (m_uRefCount == 0)
	{
		delete this;
		return 0;
	}
	return m_uRefCount;
}

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLPrivateDataContainer
////////////////////////////////////////////////////////////////////////////////

struct CDrxDXGLPrivateDataContainer::SPrivateData
{
	union
	{
		u8*    m_pBuffer;
		IUnknown* m_pInterface;
	};
	u32 m_uSize;
	bool   m_bInterface;

	SPrivateData(ukk pData, u32 uSize)
	{
		m_pBuffer = new u8[uSize];
		m_uSize = uSize;
		m_bInterface = false;
		NDrxOpenGL::Memcpy(m_pBuffer, pData, uSize);
	}

	SPrivateData(IUnknown* pInterface)
	{
		pInterface->AddRef();
		m_pInterface = pInterface;
		m_uSize = sizeof(IUnknown*);
		m_bInterface = true;
	}

	~SPrivateData()
	{
		if (m_bInterface)
			m_pInterface->Release();
		else
			delete[] m_pBuffer;
	}
};

CDrxDXGLPrivateDataContainer::CDrxDXGLPrivateDataContainer()
{
}

CDrxDXGLPrivateDataContainer::~CDrxDXGLPrivateDataContainer()
{
	TPrivateDataMap::iterator kPrivateIter(m_kPrivateDataMap.begin());
	TPrivateDataMap::iterator kPrivateEnd(m_kPrivateDataMap.end());
	for (; kPrivateIter != kPrivateEnd; ++kPrivateIter)
	{
		delete kPrivateIter->second;
	}
}

HRESULT CDrxDXGLPrivateDataContainer::GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData)
{
	if (pData == NULL)
	{
		if (*pDataSize != 0)
			return E_FAIL;
		RemovePrivateData(guid);
	}
	else
	{
		assert(pDataSize != NULL);
		TPrivateDataMap::const_iterator kFound(m_kPrivateDataMap.find(guid));
		if (kFound == m_kPrivateDataMap.end() || *pDataSize < kFound->second->m_uSize)
			return E_FAIL;

		if (kFound->second->m_bInterface)
		{
			kFound->second->m_pInterface->AddRef();
			*static_cast<IUnknown**>(pData) = kFound->second->m_pInterface;
		}
		else
			NDrxOpenGL::Memcpy(pData, kFound->second->m_pBuffer, kFound->second->m_uSize);
		*pDataSize = kFound->second->m_uSize;
	}

	return S_OK;
}

HRESULT CDrxDXGLPrivateDataContainer::SetPrivateData(REFGUID guid, UINT DataSize, ukk pData)
{
	RemovePrivateData(guid);

	m_kPrivateDataMap.insert(TPrivateDataMap::value_type(guid, new SPrivateData(pData, DataSize)));
	return S_OK;
}

HRESULT CDrxDXGLPrivateDataContainer::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
	RemovePrivateData(guid);

	m_kPrivateDataMap.insert(TPrivateDataMap::value_type(guid, new SPrivateData(const_cast<IUnknown*>(pData))));  // The specification requires that IUnknown::AddRef, Release are called on pData thus the const cast
	return S_OK;
}

void CDrxDXGLPrivateDataContainer::RemovePrivateData(REFGUID guid)
{
	TPrivateDataMap::iterator kFound(m_kPrivateDataMap.find(guid));
	if (kFound != m_kPrivateDataMap.end())
	{
		delete kFound->second;
		m_kPrivateDataMap.erase(kFound);
	}
}

size_t CDrxDXGLPrivateDataContainer::SGuidHashCompare::operator()(const GUID& kGuid) const
{
	return (size_t)NDrxOpenGL::GetCRC32(&kGuid, sizeof(kGuid));
}

bool CDrxDXGLPrivateDataContainer::SGuidHashCompare::operator()(const GUID& kLeft, const GUID& kRight) const
{
	return memcmp(&kLeft, &kRight, sizeof(kLeft)) == 0;
}
