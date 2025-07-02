// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/StdAfx.h>
#include <drx3D/LiveCreate/PlatformHandlerBase.h>

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

// Reimplement missing functions from drx3D
	#ifdef DRXLIVECREATEPLATFORMS_EXPORTS
namespace
{
i32 DrxInterlockedIncrement( i32* value)
{
	return (i32)InterlockedIncrement((long*)value);
}

i32 DrxInterlockedDecrement( i32* value)
{
	return (i32)InterlockedDecrement((long*)value);
}
}
	#endif

PlatformHandlerBase::PlatformHandlerBase(IPlatformHandlerFactory* pFactory, tukk szTargetName)
	: m_pFactory(pFactory)
	, m_targetName(szTargetName)
	, m_refCount(1)
	, m_flags(0)
{
}

void PlatformHandlerBase::AddRef()
{
	DrxInterlockedIncrement(( i32*)&m_refCount);
}

void PlatformHandlerBase::Release()
{
	if (0 == DrxInterlockedDecrement(( i32*)&m_refCount))
	{
		Delete();
	}
}

PlatformHandlerBase::~PlatformHandlerBase()
{
}

tukk PlatformHandlerBase::GetTargetName() const
{
	return m_targetName.c_str();
}

tukk PlatformHandlerBase::GetPlatformName() const
{
	return m_pFactory->GetPlatformName();
}

IPlatformHandlerFactory* PlatformHandlerBase::GetFactory() const
{
	return m_pFactory;
}

bool PlatformHandlerBase::IsFlagSet(u32 aFlag) const
{
	return 0 != (m_flags & aFlag);
}

bool PlatformHandlerBase::Launch(tukk pExeFilename, tukk pWorkingFolder, tukk pArgs) const
{
	return false;
}

bool PlatformHandlerBase::Reset(EResetMode aMode) const
{
	return false;
}

bool PlatformHandlerBase::IsOn() const
{
	return false;
}

bool PlatformHandlerBase::ScanFolder(tukk pFolder, IPlatformHandlerFolderScan& outResult) const
{
	return false;
}

bool PlatformHandlerBase::ResolveAddress(tuk pOutIpAddress, u32 aMaxOutIpAddressSize) const
{
	return m_pFactory->ResolveAddress(m_targetName.c_str(), pOutIpAddress, aMaxOutIpAddressSize);
}

}

#endif
