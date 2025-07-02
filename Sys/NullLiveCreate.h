// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/ILiveCreateHost.h>
#include <drx3D/LiveCreate/ILiveCreateUpr.h>

namespace LiveCreate
{

// Null implementation of the new manager
class CNullUprEx : public IUpr
{
public:
	CNullUprEx() {};
	virtual ~CNullUprEx() {};
	virtual bool                     IsNullImplementation() const                                                                               { return true; }
	virtual bool                     CanSend() const                                                                                            { return false; }
	virtual bool                     IsEnabled() const                                                                                          { return false; }
	virtual void                     SetEnabled(bool bIsEnabled)                                                                                {};
	virtual bool                     IsLogEnabled() const                                                                                       { return false; }
	virtual void                     SetLogEnabled(bool bIsEnabled)                                                                             {};
	virtual bool                     RegisterListener(IUprListenerEx* pListener)                                                            { return false; }
	virtual bool                     UnregisterListener(IUprListenerEx* pListener)                                                          { return false; }
	virtual bool                     FindMachineAddressByName(tukk pTargetMachineName, tuk pOutIpAddress, u32 aMaxOutIpAddressSize) { return false; }
	virtual uint                     GetNumHosts() const                                                                                        { return 0; }
	virtual struct IHostInfo*        GetHost(const uint index) const                                                                            { return NULL; }
	virtual uint                     GetNumPlatforms() const                                                                                    { return false; }
	virtual IPlatformHandlerFactory* GetPlatformFactory(const uint index) const                                                                 { return NULL; }
	virtual bool                     RemoveHost(IHostInfo* pHost)                                                                               { return false; }
	virtual struct IHostInfo* CreateHost(IPlatformHandler* pPlatform, tukk szValidIpAddress)                                      { return NULL; };
	virtual void              RescanForNewHosts()                                                                                        {};
	virtual void              Update()                                                                                                   {};
	virtual bool              SendCommand(const IRemoteCommand& command)                                                                 { return false; }
	virtual void              LogMessage(ELogMessageType aType, tukk pMessage)                                                    {}
};

// Null implementation of LiveCreate host
class CNullHost : public IHost
{
public:
	CNullHost() {};
	virtual ~CNullHost() {};
	virtual bool Initialize(u16k, u16k, u16k) { return false; }
	virtual void Close()                                              {};
	virtual void SuppressCommandExecution()                           {}
	virtual void ResumeCommandExecution()                             {}
	virtual void ExecuteCommands()                                    {};
	virtual void DrawOverlay()                                        {};
};

}
