// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_ILIVECREATECOMMON_H_
#define _H_ILIVECREATECOMMON_H_

#include <drx3D/Sys/IEngineModule.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// LiveCreate should not be enabled or compiled in server or release mode.
#if (defined(DEDICATED_SERVER) || defined(_RELEASE)) && !defined(NO_LIVECREATE)
	#define NO_LIVECREATE
#else
	#if DRX_PLATFORM_DESKTOP
		#define LIVECREATE_FOR_PC
	#endif
#endif

struct ILiveCreateEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(ILiveCreateEngineModule, "b93b314c-06b2-4660-adf0-c6a1cb3eaf26"_drx_guid);
};

namespace LiveCreate
{
struct IHost;
struct IHostInfo;
struct IUpr;
struct IPlatformHandler;
struct IPlatformHandlerFactory;

enum ELogMessageType
{
	eLogType_Normal,
	eLogType_Warning,
	eLogType_Error
};

struct IUprListenerEx
{
	// <interfuscator:shuffle>
	virtual ~IUprListenerEx(){};

	//! Upr was successfully connected to a remote host.
	virtual void OnHostConnected(IHostInfo* pHostInfo) {};

	//! Upr was disconnected from a remote host.
	virtual void OnHostDisconnected(IHostInfo* pHostInfo) {};

	//! Upr connection was confirmed and we are ready to send LiveCreate data.
	virtual void OnHostReady(IHostInfo* pHost) {};

	//! LiveCreate host is busy (probably loading the level).
	virtual void OnHostBusy(IHostInfo* pHost) {};

	//! Upr wide sending status flag has changed.
	virtual void OnSendingStatusChanged(bool bCanSend) {};

	//! Internal message logging.
	virtual void OnLogMessage(ELogMessageType aType, tukk pMessage) {};

	// </interfuscator:shuffle>
};
}

#endif
