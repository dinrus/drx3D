// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "CommonIDrxMatchMaking.h"

class CDrxLobbyPacket;

//! If the underlying SDK has a frontend invites can be accepted from then this callback will be called if an invite is accepted this way.
//! When this call back is called the game must switch to the given service and join the session.
//! If the user is in a session already then the old session must be deleted before the join is started.
//! Structure returned when a registered callback is triggered for invite acceptance.
struct SDrxLobbyInviteAcceptedData
{
	EDrxLobbyService m_service;           //!< Which of the DrxLobby services this is for.
	u32           m_user;              //!< Id of local user this pertains to.
	DrxSessionID     m_id;                //!< Session identifier of which session to join.
	EDrxLobbyError   m_error;
};

enum EDrxLobbyInviteType
{
	eCLIT_InviteToSquad,
	eCLIT_JoinSessionInProgress,
	eCLIT_InviteToSession,
};

//! Separate data type for platforms where invites revolve around users, not sessions.
struct SDrxLobbyUserInviteAcceptedData
{
	EDrxLobbyService    m_service;
	u32              m_user;
	DrxUserID           m_inviterId;
	EDrxLobbyError      m_error;
	EDrxLobbyInviteType m_type;
};

enum ECableState
{
	eCS_Unknown   = -1,
	eCS_Unplugged = 0,
	eCS_Disconnected,                   //!< Indicates the network link is down (same as unplugged but means the game was using wireless).
	eCS_Connected,
};

enum EOnlineState
{
	eOS_Unknown   = -1,
	eOS_SignedOut = 0,
	eOS_SigningIn,
	eOS_SignedIn,
};

enum ENatType
{
	eNT_Unknown = -1,
	eNT_Open    = 0,
	eNT_Moderate,
	eNT_Strict
};

enum EForcedFromRoomReason
{
	eFFRR_Unknown = 0,
	eFFRR_Left,
	eFFRR_Kicked,
	eFFRR_ServerForced,
	eFFRR_ServerInternalError,
	eFFRR_ConnectionError,
	eFFRR_SignedOut
};

//! curState will indicate the new state of the ethernet interface.
struct SDrxLobbyEthernetStateData
{
	ECableState m_curState;
};

//! m_curState will indicate the new state of the user identified by user (expect multiple of these).
//! m_reason gives you a reason for the state change. Only really meaningful if m_curState is eOS_SignedOut.
struct SDrxLobbyOnlineStateData
{
	u32       m_user;
	EOnlineState m_curState;
	u32       m_reason;
	bool         m_serviceConnected;
};

struct SDrxLobbyNatTypeData
{
	ENatType m_curState;            //!< Current nat type.
};

struct SDrxLobbyRoomOwnerChanged
{
	DrxSessionHandle m_session;
	u32           m_ip;
	u16           m_port;
};

struct SDrxLobbyOnlineNameRejectedData
{
	u32 m_user;                  //!< Local user index.
	size_t m_numSuggestedNames;     //!< Do not modify. May be 0.
	tuk* m_ppSuggestedNames;      //!< Do not modify. Will be NULL if m_numSuggestedNames is 0.
};

struct SDrxLobbyUserProfileChanged
{
	u32 m_user;
};

struct SDrxLobbyChatRestrictedData
{
	u32 m_user;
	bool   m_chatRestricted;
};

struct SDrxLobbyForcedFromRoomData
{
	DrxSessionHandle      m_session;
	EForcedFromRoomReason m_why;
};

struct SDrxLobbySessionQueueData
{
	i32 m_placeInQueue;
};

struct SDrxLobbyDedicatedServerSetupData
{
	CDrxLobbyPacket* pPacket;
	DrxSessionHandle session;
};

struct SDrxLobbyDedicatedServerReleaseData
{
	DrxSessionHandle session;
};

struct SDrxLobbyFriendIDData
{
	DrxUserID m_user;
};

struct SDrxLobbyFriendOnlineStateData : public SDrxLobbyFriendIDData
{
	EOnlineState m_curState; //!< Indicates the new state of the user identified by m_user.
};

#define LOBBY_MESSAGE_SIZE (256)
struct SDrxLobbyFriendMessageData : public SDrxLobbyFriendIDData
{
	char m_message[LOBBY_MESSAGE_SIZE];
};

enum ELobbyFriendStatus
{
	eLFS_Offline,
	eLFS_Online,
	eLFS_OnlineSameTitle,
	eLFS_Pending
};

typedef u32 DrxLobbyUserIndex;
const DrxLobbyUserIndex DrxLobbyInvalidUserIndex = 0xffffffff;

struct SDrxLobbyPartyMember
{
	DrxUserID          m_userID;
	char               m_name[DRXLOBBY_USER_NAME_LENGTH];
	ELobbyFriendStatus m_status;
	DrxLobbyUserIndex  m_userIndex;
};

struct SDrxLobbyPartyMembers
{
	u32                m_numMembers;
	SDrxLobbyPartyMember* m_pMembers;
};
