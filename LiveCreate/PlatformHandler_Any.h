// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIVECREATE_PLATFORMHANDLER_ANY_H_
#define _LIVECREATE_PLATFORMHANDLER_ANY_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/ILiveCreateCommon.h>
#include <drx3D/LiveCreate/PlatformHandlerBase.h>

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

// General handler than can handle any drx3D game
class PlatformHandler_Any : public PlatformHandlerBase
{
public:
	PlatformHandler_Any(IPlatformHandlerFactory* pFactory, tukk szTargetName);
	virtual ~PlatformHandler_Any();

	// IPlatformHandler interface implementation
	virtual void        Delete();
	virtual bool        IsOn() const;
	virtual bool        Launch(tukk pExeFilename, tukk pWorkingFolder, tukk pArgs) const;
	virtual bool        Reset(EResetMode aMode) const;
	virtual bool        ScanFolder(tukk pFolder, IPlatformHandlerFolderScan& outResult) const;
	virtual tukk GetRootPath() const;
};

// General handler than can handle any drx3D game (requires only connection)
class PlatformHandlerFactory_Any : public IPlatformHandlerFactory
{
public:
	// IPlatformHandlerFactory interface implementation
	virtual tukk       GetPlatformName() const;
	virtual IPlatformHandler* CreatePlatformHandlerInstance(tukk pTargetMachineName);
	virtual u32            ScanForTargets(TargetInfo* outTargets, const uint maxTargets);
	virtual bool              ResolveAddress(tukk pTargetMachineName, tuk pOutIpAddress, u32 aMaxOutIpAddressSize);
};
}

#endif
#endif
