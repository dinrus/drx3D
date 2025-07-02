// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_ILIVECREATEHOST_H_
#define _H_ILIVECREATEHOST_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/ILiveCreateCommon.h>

namespace LiveCreate
{

//! Default port for LiveCreate host.
static u16k kDefaultHostListenPort = 30601;

//! Default port for UDP target discovery service.
static u16k kDefaultDiscoverySerivceListenPost = 30602;

//! Default port for TCP file transfer service.
static u16k kDefaultFileTransferServicePort = 30603;

//! Host information (send over the network based host discovery system).
struct CHostInfoPacket
{
	string platformName;
	string hostName;
	string buildExecutable;
	string buildDirectory;
	string gameFolder;
	string rootFolder;
	string currentLevel;
	u8  bAllowsLiveCreate;
	u8  bHasLiveCreateConnection;
	u16 screenWidth;
	u16 screenHeight;

	template<class T>
	void Serialize(T& stream)
	{
		stream << platformName;
		stream << hostName;
		stream << gameFolder;
		stream << buildExecutable;
		stream << buildDirectory;
		stream << rootFolder;
		stream << currentLevel;
		stream << bAllowsLiveCreate;
		stream << bHasLiveCreateConnection;
		stream << screenWidth;
		stream << screenHeight;
	}
};

//! Host-side (console) LiveCreate interface.
struct IHost
{
public:
	virtual ~IHost() {};

public:
	//! Initialize host at given listening port.
	virtual bool Initialize(u16k listeningPort = kDefaultHostListenPort,
	                        u16k discoveryServiceListeningPort = kDefaultDiscoverySerivceListenPost,
	                        u16k fileTransferServiceListeningPort = kDefaultFileTransferServicePort) = 0;

	//! Close the interface and disconnect all clients.
	virtual void Close() = 0;

	//! Enter a "loading mode" in which commands are not executed.
	virtual void SuppressCommandExecution() = 0;

	//! Exit a "loading mode" and restore command execution.
	virtual void ResumeCommandExecution() = 0;

	//! Execute pending commands.
	virtual void ExecuteCommands() = 0;

	//! Draw overlay information.
	virtual void DrawOverlay() = 0;
};

}

#endif
