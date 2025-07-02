// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIVECREATE_PLATFORMHANDLER_GAMEPC_H_
#define _LIVECREATE_PLATFORMHANDLER_GAMEPC_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/ILiveCreateCommon.h>
#include <drx3D/LiveCreate/PlatformHandlerBase.h>

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

// Local platform wrapper, assumes game is launched from the same directory as editor
class PlatformHandler_GamePC : public PlatformHandlerBase
{
private:
	string m_exeDirectory;
	string m_baseDirectory;
	string m_rootDirectory; // only the drive name

public:
	PlatformHandler_GamePC(IPlatformHandlerFactory* pFactory, tukk szTargetName);
	virtual ~PlatformHandler_GamePC();

	// IPlatformHandler interface implementation
	virtual void        Delete();
	virtual bool        IsOn() const;
	virtual bool        Launch(tukk pExeFilename, tukk pWorkingFolder, tukk pArgs) const;
	virtual bool        Reset(EResetMode aMode) const;
	virtual bool        ScanFolder(tukk pFolder, IPlatformHandlerFolderScan& outResult) const;
	virtual tukk GetRootPath() const;
};

// Local platform factory wrapper, supports local PC only
class PlatformHandlerFactory_GamePC : public IPlatformHandlerFactory
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
