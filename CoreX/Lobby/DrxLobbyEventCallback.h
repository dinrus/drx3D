// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "CommonIDrxMatchMaking.h"

class CDrxLobbyPacket;

enum EDrxLobbySystemEvent
{
	eCLSE_EthernetState,                //!< returns DrxLobbyEthernetStateData	(gives status of ethernet cable e.g. unplugged)
	eCLSE_OnlineState,                  //!< returns DrxLobbyOnlineStateData		(gives status of online connection e.g. signed in)
	eCLSE_InviteAccepted,               //!< returns DrxLobbyInviteAcceptedData
	eCLSE_UserInviteAccepted,           //!< returns DrxLobbyUserInviteAcceptedData
	eCLSE_NatType,                      //!< returns DrxLobbyNatTypeData
	eCLSE_UserPacket,                   //!< returns DrxLobbyUserPacketData
	eCLSE_RoomOwnerChanged,             //!< returns DrxLobbyRoomOwnerChanged
	eCLSE_SessionUserJoin,              //!< returns SDrxLobbySessionUserData
	eCLSE_SessionUserLeave,             //!< returns SDrxLobbySessionUserData
	eCLSE_SessionUserUpdate,            //!< returns SDrxLobbySessionUserData
	eCLSE_PartyMembers,                 //!< returns SDrxLobbyPartyMembers. Called whenever the Live party list changes.
	eCLSE_UserProfileChanged,           //!< returns SDrxLobbyUserProfileChanged. Called whenever the user changes a profile setting using the platform UI.
	eCLSE_OnlineNameRejected,           //!< returns SDrxLobbyOnlineNameRejectedData.
	eCLSE_FriendOnlineState,            //!< returns SDrxLobbyFriendOnlineStateData
	eCLSE_FriendRequest,                //!< returns SDrxLobbyFriendIDData
	eCLSE_FriendMessage,                //!< returns SDrxLobbyFriendMessageData
	eCLSE_FriendAuthorised,             //!< returns SDrxLobbyFriendIDData
	eCLSE_FriendRevoked,                //!< returns SDrxLobbyFriendIDData
	eCLSE_FriendsListUpdated,           //!< returns NULL
	eCLSE_ReceivedInvite,               //!< returns SDrxLobbyFriendMessageData
	eCLSE_SessionClosed,                //!< returns SDrxLobbySessionEventData
	eCLSE_ChatRestricted,               //!< returns SDrxLobbyChatRestrictedData
	eCLSE_ForcedFromRoom,               //!< returns SDrxLobbyForcedFromRoomData;
	eCLSE_LoginFailed,                  //!< returns SDrxLobbyOnlineStateData; for services that don't use the eOS_SigningIn intermediate state
	eCLSE_SessionRequestInfo,           //!< returns SDrxLobbySessionRequestInfo
	eCLSE_KickedFromSession,            //!< returns SDrxLobbySessionEventData
	eCLSE_KickedHighPing,               //!< returns SDrxLobbySessionEventData
	eCLSE_KickedReservedUser,           //!< returns SDrxLobbySessionEventData
	eCLSE_KickedLocalBan,               //!< returns SDrxLobbySessionEventData
	eCLSE_KickedGlobalBan,              //!< returns SDrxLobbySessionEventData
	eCLSE_KickedGlobalBanStage1,        //!< returns SDrxLobbySessionEventData
	eCLSE_KickedGlobalBanStage2,        //!< returns SDrxLobbySessionEventData
	eCLSE_StartJoiningGameByPlaygroup,  //!< returns NULL
	eCLSE_JoiningGameByPlaygroupResult, //!< returns SDrxLobbyJoinedGameByPlaygroupData
	eCLSE_LeavingGameByPlaygroup,       //!< returns SDrxLobbySessionEventData
	eCLSE_PlaygroupUserJoin,            //!< returns SDrxLobbyPlaygroupUserData
	eCLSE_PlaygroupUserLeave,           //!< returns SDrxLobbyPlaygroupUserData
	eCLSE_PlaygroupUserUpdate,          //!< returns SDrxLobbyPlaygroupUserData
	eCLSE_PlaygroupClosed,              //!< returns SDrxLobbyPlaygroupEventData
	eCLSE_PlaygroupLeaderChanged,       //!< returns SDrxLobbyPlaygroupUserData
	eCLSE_KickedFromPlaygroup,          //!< returns SDrxLobbyPlaygroupEventData
	eCLSE_PlaygroupUpdated,             //!< returns SDrxLobbyPlaygroupEventData
	eCLSE_DedicatedServerSetup,         //!< returns SDrxLobbyDedicatedServerSetupData; Dedicated servers allocated via CDrxDedicatedServerArbitrator will receive this event when they are setup by the host calling SessionSetupDedicatedServer.
	eCLSE_DedicatedServerRelease,       //!< returns NULL; Dedicated servers allocated via CDrxDedicatedServerArbitrator will receive this event when they are released by the host calling SessionReleaseDedicatedServer or when the server losses all of its connections.
	eCLSE_SessionQueueUpdate,           //!< returns SDrxLobbySessionQueueData
};

struct SDrxLobbyEthernetStateData;
struct SDrxLobbyOnlineStateData;
struct SDrxLobbyInviteAcceptedData;
struct SDrxLobbyUserInviteAcceptedData;
struct SDrxLobbyNatTypeData;
struct SDrxLobbyUserPacketData;
struct SDrxLobbyRoomOwnerChanged;
struct SDrxLobbySessionUserData;
struct SDrxLobbyPlaygroupUserData;
struct SDrxLobbyPartyMembers;
struct SDrxLobbyUserProfileChanged;
struct SDrxLobbyOnlineNameRejectedData;
struct SDrxLobbyFriendIDData;
struct SDrxLobbyFriendOnlineStateData;
struct SDrxLobbyFriendMessageData;
struct SDrxLobbySessionEventData;
struct SDrxLobbyPlaygroupEventData;
struct SDrxLobbyChatRestrictedData;
struct SDrxLobbyForcedFromRoomData;
struct SDrxLobbySessionRequestInfo;
struct SDrxLobbySessionQueueData;
struct SDrxLobbyDedicatedServerSetupData;
struct SDrxLobbyDedicatedServerReleaseData;
struct SDrxLobbyJoinedGameByPlaygroupData;


union UDrxLobbyEventData
{
	SDrxLobbyEthernetStateData* pEthernetStateData;
	SDrxLobbyOnlineStateData* pOnlineStateData;
	SDrxLobbyInviteAcceptedData* pInviteAcceptedData;
	SDrxLobbyUserInviteAcceptedData* pUserInviteAcceptedData;
	SDrxLobbyNatTypeData* pNatTypeData;
	SDrxLobbyUserPacketData* pUserPacketData;
	SDrxLobbyRoomOwnerChanged*  pRoomOwnerChanged;
	SDrxLobbySessionUserData*   pSessionUserData;
	SDrxLobbyPlaygroupUserData* pPlaygroupUserData;
	SDrxLobbyPartyMembers* pPartyMembers;
	SDrxLobbyUserProfileChanged* pUserProfileChanged;
	SDrxLobbyOnlineNameRejectedData* pSuggestedNames;
	SDrxLobbyFriendIDData* pFriendIDData;
	SDrxLobbyFriendOnlineStateData* pFriendOnlineStateData;
	SDrxLobbyFriendMessageData*  pFriendMesssageData;
	SDrxLobbySessionEventData*   pSessionEventData;
	SDrxLobbyPlaygroupEventData* pPlaygroupEventData;
	SDrxLobbyChatRestrictedData* pChatRestrictedData;
	SDrxLobbyForcedFromRoomData* pForcedFromRoomData;
	SDrxLobbySessionRequestInfo* pSessionRequestInfo;
	SDrxLobbySessionQueueData*   pSessionQueueData;
	SDrxLobbyDedicatedServerSetupData* pDedicatedServerSetupData;
	SDrxLobbyDedicatedServerReleaseData* pDedicatedServerReleaseData;
	SDrxLobbyJoinedGameByPlaygroupData*  pJoinedByPlaygroupData;
};

typedef void (* DrxLobbyEventCallback)(UDrxLobbyEventData eventData, uk userParam);
