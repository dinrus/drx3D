// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Network/NetAddress.h>
#include <drx3D/Network/DrxSocketError.h>
#include <drx3D/Network/DrxSocks.h>

typedef u32 DrxSessionHandle;
const DrxSessionHandle DrxSessionInvalidHandle = 0xffffffff;

enum EDisconnectionCause
{
	//! Эта причина следует первой! - произошёл таймаут.
	eDC_Timeout = 0,
	//! Несовместимые протоколы.
	eDC_ProtocolError,
	//! Не удалось разрешить адрес.
	eDC_ResolveFailed,
	//! Несовпадение версий.
	eDC_VersionMismatch,
	//! Сервер занят.
	eDC_ServerFull,
	//! User initiated kick.
	eDC_Kicked,
	//! Teamkill ban/ admin ban.
	eDC_Banned,
	//! Context database mismatch.
	eDC_ContextCorruption,
	//! Password mismatch, cdkey bad, etc.
	eDC_AuthenticationFailed,
	//! Misc. game error.
	eDC_GameError,
	//! DX11 not found.
	eDC_NotDX11Capable,
	//! The nub has been destroyed.
	eDC_NubDestroyed,
	//! Icmp reported error.
	eDC_ICMPError,
	//! NAT negotiation error.
	eDC_NatNegError,
	//! Punk buster detected something bad.
	eDC_PunkDetected,
	//! Demo playback finished.
	eDC_DemoPlaybackFinished,
	//! Demo playback file not found.
	eDC_DemoPlaybackFileNotFound,
	//! User decided to stop playing.
	eDC_UserRequested,
	//! User should have controller connected.
	eDC_NoController,
	//! Unable to connect to server.
	eDC_CantConnect,
	//! Arbitration failed in a live arbitrated session.
	eDC_ArbitrationFailed,
	//! Failed to successfully join migrated game.
	eDC_FailedToMigrateToNewHost,
	//! The session has just been deleted.
	eDC_SessionDeleted,
	//! Kicked due to having a high ping.
	eDC_KickedHighPing,
	//! Kicked due to reserved user joining.
	eDC_KickedReservedUser,
	//! Class registry mismatch.
	eDC_ClassRegistryMismatch,
	//! Global ban.
	eDC_GloballyBanned,
	//! Global ban stage 1 messaging.
	eDC_Global_Ban1,
	//! Global ban stage 2 messaging.
	eDC_Global_Ban2,
	//! This cause must be last! - unknown cause.
	eDC_Unknown
};

enum eStatusCmdMode
{
	eSCM_AllSessions = 1,
	eSCM_LegacyRConCompatabilityConnectionsOnly,
	eSCM_LegacyRConCompatabilityNoNub
};

struct SDrxSessionID : public CMultiThreadRefCount
{
	// <interfuscator:shuffle>
	virtual bool operator==(const SDrxSessionID& other) = 0;
	virtual bool operator<(const SDrxSessionID& other) = 0;
	virtual bool IsFromInvite() const = 0;
	virtual void AsCStr(tuk pOutString, i32 inBufferSize) const = 0;
	// </interfuscator:shuffle>
};

typedef _smart_ptr<SDrxSessionID> DrxSessionID;
const DrxSessionID DrxSessionInvalidID = NULL;

struct IDrxMatchMakingConsoleCommands
{
public:
	//! Returns the sessionID from the first searchable hosted session at the ip:port specified by cl_serveraddr.
	//! \return Valid sessionId or DrxSessionInvalidID if it fails to find a session.
	virtual DrxSessionID GetSessionIDFromConsole(void) = 0;

	//! Logs all connections for all sessions to the console (used by 'status' command).
	//! \param mode Documented in eStatusCmdMode.
	//! \see eStatusCmdMode.
	virtual void StatusCmd(eStatusCmdMode mode) = 0;

	//! Kick console command.
	//! \param cxID - connection ID to kick (or DrxLobbyInvalidConnectionID to ignore this param).
	//! \param userID - e.g. GPProfile id to kick (or 0 to ignore this param).
	//! \param pName - user name to ban (or NULL to ignore this param).
	//! \param cause - reason for the kick.
	virtual void KickCmd(u32 cxID, uint64 userID, tukk pName, EDisconnectionCause cause) = 0;

	//! Ban console command.
	//! \param userID  e.g. GPProfile id to ban.
	//! \param timeout   Ban timeout in minutes.
	virtual void BanCmd(uint64 userID, float timeout) = 0;

	//! Ban console command.
	//! \param pUserName User name to ban.
	//! \param timeout   Ban timeout in minutes.
	virtual void BanCmd(tukk pUserName, float timeout) = 0;

	//! Unban console command.
	//! UserID - e.g. GPProfile id to unban.
	virtual void UnbanCmd(uint64 userID) = 0;

	//! Unban console command.	//! \param pUserName User name to unban.
	virtual void UnbanCmd(tukk pUserName) = 0;

	//! Logs all banned users to the console.
	virtual void BanStatus(void) = 0;
};

struct IDrxMatchMakingPrivate
{
public:

	virtual TNetAddress       GetHostAddressFromSessionHandle(DrxSessionHandle h) = 0;
	virtual EDrxLobbyService  GetLobbyServiceTypeForNubSession() = 0;
	virtual void              SessionDisconnectRemoteConnectionFromNub(DrxSessionHandle gh, EDisconnectionCause cause) = 0;
	virtual u16            GetChannelIDFromGameSessionHandle(DrxSessionHandle gh) = 0;

	virtual void         LobbyAddrIDTick() {}                // Need to implement if matchmaking service uses LobbyIdAddr's rather than SIPV4Addr
	virtual bool         LobbyAddrIDHasPendingData() { return false; } // Need to implement if matchmaking service uses LobbyIdAddr's rather than SIPV4Addr

	virtual ESocketError LobbyAddrIDSend(u8k* buffer, u32 size, const TNetAddress& addr) { return eSE_SocketClosed; } // Need to implement if matchmaking service uses LobbyIdAddr's rather than SIPV4Addr
	virtual void         LobbyAddrIDRecv(void(*cb)(uk , u8* buffer, u32 size, DRXSOCKET sock, TNetAddress& addr), uk cbData) {}                           // Need to implement if matchmaking service uses LobbyIdAddr's rather than SIPV4Addr

	virtual bool         IsNetAddressInLobbyConnection(const TNetAddress& addr) { return true; }
};
