// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_ILIVECREATEMANAGER_H_
#define _H_ILIVECREATEMANAGER_H_

#include <drx3D/LiveCreate/ILiveCreateCommon.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct IRemoteCommand;

namespace LiveCreate
{

struct IPlatformHandler;

//! Host-side (PC) LiveCreate interface.
struct IHostInfo
{
protected:
	virtual ~IHostInfo() {};

public:
	//! Host target name.
	virtual tukk GetTargetName() const = 0;

	//! Get resolved network address (NULL for non existing hosts).
	virtual tukk GetAddress() const = 0;

	//! Get platform name.
	virtual tukk GetPlatformName() const = 0;

	//! Are we connected to this host?
	virtual bool IsConnected() const = 0;

	//! Is the connection to this host confirmed (i.e. remote side has responded to our messages)?
	virtual bool IsReady() const = 0;

	//! Get the platform related interface (never NULL).
	virtual IPlatformHandler* GetPlatformInterface() = 0;

	//! Connect to remote manager.
	virtual bool Connect() = 0;

	//! Disconnect from remote manager.
	virtual void Disconnect() = 0;

	//! Add reference to object (refcounted interface).
	virtual void AddRef() = 0;

	//! Release reference to object (refcounted interface).
	virtual void Release() = 0;
};

//! Host-side (PC) LiveCreate manager interface.
struct IUpr
{
public:
	virtual ~IUpr() {};

public:
	//! This is a hint that the LiveCreate is the null implementation that does nothing.
	//! Useful when we dont want to ship DrxLiveCreate.dll and the system falls back to LiveCreate::CNullUpr.
	virtual bool IsNullImplementation() const = 0;

	//! Can we send any data to LiveCreate hosts?
	virtual bool CanSend() const = 0;

	//! Is the manager enabled?
	virtual bool IsEnabled() const = 0;

	//! Enable/Disable manager.
	virtual void SetEnabled(bool bIsEnabled) = 0;

	//! Is the logging of debug messages enabled.
	virtual bool IsLogEnabled() const = 0;

	//! Enable/Disable logging of debug messages.
	virtual void SetLogEnabled(bool bIsEnabled) = 0;

	//! Register a new listener.
	virtual bool RegisterListener(IUprListenerEx* pListener) = 0;

	//! Unregister a listener.
	virtual bool UnregisterListener(IUprListenerEx* pListener) = 0;

	//! Get number of host connections.
	virtual uint GetNumHosts() const = 0;

	//! Get host interface.
	virtual IHostInfo* GetHost(const uint index) const = 0;

	//! Get number of registered platforms.
	virtual uint GetNumPlatforms() const = 0;

	//! Get platform factory at given index.
	virtual IPlatformHandlerFactory* GetPlatformFactory(const uint index) const = 0;

	//! Create a host interface from valid platform interface and a valid address.
	virtual IHostInfo* CreateHost(IPlatformHandler* pPlatform, tukk szValidIpAddress) = 0;

	//! Remove host from manager.
	virtual bool RemoveHost(IHostInfo* pHost) = 0;

	//! Update manager.
	virtual void Update() = 0;

	//! Send remote command to connected hosts.
	virtual bool SendCommand(const IRemoteCommand& command) = 0;

	//! Log LiveCreate message.
	virtual void LogMessage(ELogMessageType aType, tukk pMessage) = 0;
};

}

#endif
