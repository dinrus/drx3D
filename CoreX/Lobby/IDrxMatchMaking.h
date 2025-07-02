// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/CoreX/Lobby/CommonIDrxMatchMaking.h>

// Overview of IDrxMatchMaking
// MatchMaking provides a way for gamers to find each other to play multi player game sessions.
// A gamer can host a session or search for a session to join.
// Custom game data can be added to a session to provide other gamers information about the game being played.
// When searching for games to join the results can be filtered so only games the gamer is interested in are returned.
//
// MatchMaking Setup
// Before MatchMaking can be used the games custom session data must be registered with a call to SessionRegisterUserData.
// The custom data is defined using an array of SDrxLobbyUserData and can have a maximum of 64 elements.
// m_id should be a unique id used to identify the data item.
// In Live the session user data has to be defined with the xlast program and uploaded to Microsoft before matchmaking can be used.
// The xlast program will produce an h file with ID's for the user data and these ID's should be used.
// In NP due to limitations of the service, you should just use any UID (the matchmaking service will then place the values
// appropriately to fit) Key note : Only the first 8 i32 fields registered will be searchable/filterable.
// In DrxLAN any UID can be used. It will happily use the same ID's defined for the underlying online service for each platform.
// The data types that can be used for custom session user data are
// eCLUDT_Int64, eCLUDT_Int32, eCLUDT_Int16, eCLUDT_Int8, eCLUDT_Float64, eCLUDT_Float32, eCLUDT_Int64NoEndianSwap
//
// MatchMaking on the host
// The host should start by calling SessionCreate specifying the data that describes the session.
// If the functions completes successfully this data will be returned to peers when they search for sessions.
// As well as the custom data, other session data that should be supplied is the number of public and private slots that are available for users to join.
// Private slots can only be joined by a users who are invited to the game while public slots can be joined by anybody.
// The session name should also be given.
// Some SDK's don't allow the session name to be set and will replace the name given, normally with the user name of the user who created the session.
// The final thing to set is if the match is ranked. In Live, ranked sessions do arbitration in the background.
//
// During a game it is common for the game data to change and hence the data returned by searches needs to change to reflect this. e.g The game moves on to a different map.
// The host has 2 functions it can call to update the changeable data. These functions can be called anytime during the lifetime of the session.
// SessionUpdate can be called to update the sessions custom user data and takes an array of SDrxLobbyUserData containing the data items to update.
// SessionUpdateSlots can be called to update the number of public and private slots available for users to join. e.g a private game may want to be made public.
// The new total number of slots should always be greater of equal to the current number of users in the session.
//
// When ready to start gameplay the host should call SessionStart and send a message to the other peers telling them to do the same.
// In ranked sessions after SessionStart has been called no more users are allowed to join the session. In unranked session users will still be allowed to join.
// When gameplay is finished SessionEnd should be called and a message should be sent to the other peers to tell them to do the same.
// If IDrxStats is being used for leaderboards then writing to the leaderboards should be done between SessionStart and SessionEnd.
// In Live the leaderboard data written is sent to the Live servers when SessionEnd is called.
// If the game wishes to start gameplay again with the same session then SessionStart can be called again. e.g the same group of players moving on to another round.
//
// When the session is complete SessionDelete should be called this will free up all resources used by the session and stop the session being adverised
// so it will no longer be returned in searches.
//
// MatchMaking on the peers
// Before a peer can join a session it must find a DrxSessionID of a session to join.
// There are 2 ways a DrxSessionID can be obtained.
// A call to SessionSearch can be made that will return sessions that match the SDrxSessionSearchParam structure passed in.
//
// The SDrxSessionSearchParam structure passed in contains all the information needed to filter the results returned so only sessions
// the user is interested in are returned.
// In Live all the search types must also be defined in the xlast program and uploaded to Microsoft before they can be used.
// The h file the xlast program creates will contain ID's for each search type. m_type should be set to the ID of the search being done.
// Other systems can use any ID.
// An array of SDrxSessionSearchData is also given that is used to filter sessions.
// An SDrxSessionSearchData contains an SDrxLobbyUserData that is compared against the equivalent item in the session using the EDrxSessionSearchOperator given.
// Only sessions that return true for all the SDrxSessionSearchData's given will be returned.
// Due to the limitations of PSN if PSN support is needed searches should only be defined using up to 8 i32 fields.
// m_numFreeSlots should be set to only return sessions with at least this many free public slots.
// m_ranked should be set to return ranked or unranked matches.
// m_maxNumReturn should be set to return at most this many sessions.
//
// The callback passed to SessionSearch will be called with the session information of each session found.
// If the error code is eCLE_SuccessContinue then a session is also provided and the callback will be called again.
// If the error code is eCLE_SuccessUnreachable then a session is also provided and the callback will be called again.
// This code indicates that a connection attempt to this session will fail most likely due to incompatible NATs.
// If the error code is eCLE_Success then the session will be NULL and the search is complete.
//
// The second way a DrxSessionID can be obtained is by the user accepting an invite or joining a session in progress via the underlying SDKs UI.
// To get the DrxSessionID when the user does this RegisterEventInterest should be called with eCLSE_InviteAccepted and a callback in IDrxLobby during startup.
// When the callback is called the game should join the given DrxSessionID.
//
// When a DrxSessionID has been obtained call SessionJoin to try and join this session.
// The join could fail if the session has become unjoinable since the DrxSessionID was obtained e.g Session has become full or the host has called SessionDelete.
// It can also fail due to incompatable NAT types.
//
// SessionStart should be called when gameplay starts. A message should be sent from the host to indicate when this is.
// SessionEnd should be called when gameplay ends. A message should be sent from the host to indicate when this is or if the user decides to leave.
//
// When the session is complete SessionDelete should be called this will free up all resources used by the session.
//

#include <drx3D/CoreX/Lobby/IDrxLobby.h>                                                 // <> required for Interfuscator

class CDrxLobbyPacket;

// Session create flags (system)
#define DRXSESSION_CREATE_FLAG_SEARCHABLE 0x00000001                   //!< If set the session will be searchable and returned in session searches. If set uses XSESSION_CREATE_USES_MATCHMAKING on live if not set uses SCE_NP_MATCHING2_ROOM_FLAG_ATTR_HIDDEN on NP.
#define DRXSESSION_CREATE_FLAG_INVITABLE  0x00000002                   //!< If set friends can be invited to this session. In Live only 1 session can be invitable.
#define DRXSESSION_CREATE_FLAG_NUB        0x00000004                   //!< Set if the session will be connected to a CNetNub.

// Session create flags (game)
#define DRXSESSION_CREATE_FLAG_MIGRATABLE           0x00010000         //!< Set if the session is migratable to another host.
#define DRXSESSION_CREATE_FLAG_CAN_SEND_SERVER_PING 0x00020000         //!< Session capable of sending server ping information.
#define DRXSESSION_CREATE_FLAG_JOIN_BY_INVITE       0x00040000         //!< Set if we were invited to this session.

#define DRXSESSION_CREATE_FLAG_SYSTEM_MASK          0x0000ffff         //!< Mask for the system flags (set when creating a session, not modifiable).
#define DRXSESSION_CREATE_FLAG_GAME_MASK            0xffff0000         //!< Mask for the game flags (can be modified after creating a session).
#define DRXSESSION_CREATE_FLAG_GAME_FLAGS_SHIFT     (16)

// Local flags (system)
#define DRXSESSION_LOCAL_FLAG_USED                          0x00000001 //!< Session is in use.
#define DRXSESSION_LOCAL_FLAG_HOST                          0x00000002 //!< This machine is the host of this session.
#define DRXSESSION_LOCAL_FLAG_USER_DATA_EVENTS_STARTED      0x00000004
#define DRXSESSION_LOCAL_FLAG_CAN_SEND_HOST_HINTS           0x00000008 //!< Session capable of sending host hint information.
#define DRXSESSION_LOCAL_FLAG_HOST_HINT_INFO_DIRTY          0x00000010 //!< Indicates the session needs to send host hint information.
#define DRXSESSION_LOCAL_FLAG_HOST_HINT_EXTERNAL_INFO_DIRTY 0x00000020 //!< Indicates the session needs to send host hint information to the lobby service provider (if supported).
#define DRXSESSION_LOCAL_FLAG_STARTED                       0x00000040 //!< Indicates this session has been started.

// Local flags (game)
#define DRXSESSION_LOCAL_FLAG_HOST_MIGRATION_CAN_BE_HOST 0x00010000    //!< Initially cleared until the game decides this machine has enough information to be able to become host during a host migration event.

#define DRXSESSION_LOCAL_FLAG_SYSTEM_MASK                0x0000ffff    //!< Mask for system controlled local session flags.
#define DRXSESSION_LOCAL_FLAG_GAME_MASK                  0xffff0000    //!< Mask for game controlled local session flags.

#define MAX_SESSION_NAME_LENGTH                          32

struct SDrxSessionData
{
	SDrxSessionData()
	{
		m_data = NULL;
		m_numData = 0;
		m_numPublicSlots = 0;
		m_numPrivateSlots = 0;
		memset(m_name, 0, MAX_SESSION_NAME_LENGTH);
		m_ranked = false;
	}

	SDrxLobbyUserData* m_data;
	u32             m_numData;
	u32             m_numPublicSlots;
	u32             m_numPrivateSlots;
	char               m_name[MAX_SESSION_NAME_LENGTH];
	bool               m_ranked;
};

enum EDrxSessionSearchOperator
{
	eCSSO_Equal,
	eCSSO_NotEqual,
	eCSSO_LessThan,
	eCSSO_LessThanEqual,
	eCSSO_GreaterThan,
	eCSSO_GreaterThanEqual,
	eCSSO_BitwiseAndNotEqualZero
};

//! The value returned for a user datum whose ID maps to eCGSK_RegionKey will have exactly two of these bits set: a narrow region and its containing wide region.
//! The m_region of a SDrxSessionSearchParam can have any combination of these bits set: 0 will perform no region filtering, and any
//! other value will cause a result to be omitted if the bitwise AND of its m_region with the search's m_region is 0.
enum EServerRegion
{
	eSR_Americas                        = BIT(0),
	eSR_NorthAmerica                    = BIT(1),
	eSR_Carribean                       = BIT(2),
	eSR_CentralAmerica                  = BIT(3),
	eSR_SouthAmerica                    = BIT(4),

	eSR_Africa                          = BIT(5),
	eSR_CentralAfrica                   = BIT(6),
	eSR_EastAfrica                      = BIT(7),
	eSR_NorthernAfrica                  = BIT(8),
	eSR_SouthernAfrica                  = BIT(9),
	eSR_WestAfrica                      = BIT(10),

	eSR_Asia                            = BIT(11),
	eSR_EastAsia                        = BIT(12),
	eSR_Pacific                         = BIT(13),
	eSR_SouthAsia                       = BIT(14),
	eSR_SouthEastAsia                   = BIT(15),

	eSR_Europe                          = BIT(16),
	eSR_BalticStates                    = BIT(17),
	eSR_CommonwealthOfIndependentStates = BIT(18),
	eSR_EasternEurope                   = BIT(19),
	eSR_MiddleEast                      = BIT(20),
	eSR_SouthEastEurope                 = BIT(21),
	eSR_WesternEurope                   = BIT(22),

	eSR_All                             = BIT(23) - 1
};

struct SDrxSessionSearchData
{
	SDrxLobbyUserData         m_data;
	EDrxSessionSearchOperator m_operator;
};

struct SDrxSessionSearchParam
{
	u32                 m_type;
	SDrxSessionSearchData* m_data;
	u32                 m_numData;
	u32                 m_numFreeSlots;
	u32                 m_maxNumReturn;
	bool                   m_ranked;
};

enum EDrxSessionSearchResultFlag
{
	eCSSRF_RequirePassword = BIT(0)
};

struct SDrxSessionSearchResult
{
	SDrxSessionData m_data;
	DrxSessionID    m_id;
	u32          m_numFilledSlots;
	u32          m_numFriends;
	u32          m_ping;                   //!< In milliseconds.
	u32          m_flags;
};

const uint64 DrxMatchMakingInvalidSessionSID = 0;

struct SDrxMatchMakingConnectionUID
{
	uint64 m_sid;
	u32 m_uid;

	SDrxMatchMakingConnectionUID()
	{
		m_sid = DrxMatchMakingInvalidSessionSID;
		m_uid = 0;
	}

	bool operator==(const SDrxMatchMakingConnectionUID& other) const
	{
		return m_sid == other.m_sid && m_uid == other.m_uid;
	}

	bool operator!=(const SDrxMatchMakingConnectionUID& other) const
	{
		return !(m_sid == other.m_sid && m_uid == other.m_uid);
	}

	// The operators below are purely for sorting purposes (they don't actually make sense from a uid point of view!).

	bool operator<(const SDrxMatchMakingConnectionUID& other) const
	{
		if (m_sid < other.m_sid)
		{
			return true;
		}
		else
		{
			if ((m_sid == other.m_sid) && (m_uid < other.m_uid))
			{
				return true;
			}
		}

		return false;
	}

	i32 operator-(const SDrxMatchMakingConnectionUID& other) const
	{
		if (*this < other)
			return -1;
		if (*this == other)
			return 0;
		return 1;
	}
};

const SDrxMatchMakingConnectionUID DrxMatchMakingInvalidConnectionUID; //!< Default construction creates an invalid ID, this is just a convenient name.

struct SDrxLobbyUserPacketData
{
	CDrxLobbyPacket*             pPacket;
	DrxSessionHandle             session;
	SDrxMatchMakingConnectionUID connection;
};

#define DRXLOBBY_USER_DATA_SIZE_IN_BYTES 16

struct SDrxUserInfoResult
{
	SDrxMatchMakingConnectionUID m_conID;
	DrxUserID                    m_userID;
	char                         m_userName[DRXLOBBY_USER_NAME_LENGTH];
	u8                        m_userData[DRXLOBBY_USER_DATA_SIZE_IN_BYTES];
	bool                         m_isDedicated;
};

struct SDrxLobbySessionUserData
{
	DrxSessionHandle   session;
	SDrxUserInfoResult data;
};

struct SDrxLobbySessionEventData
{
	DrxSessionHandle session;
};

typedef uk DrxSessionRequesterID;
const DrxSessionRequesterID DrxSessionInvalidRequesterID = NULL;

struct SDrxLobbySessionRequestInfo
{
	CDrxLobbyPacket*      pPacket;
	DrxSessionHandle      session;
	DrxSessionRequesterID requester;
};

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param h           If successful a handle to the session that owns the flags.
//! \param flags       The flags for the session.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingFlagsCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle h, u32 flags, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param h           If successful a handle to the session created used to identify the session in other matchmaking functions.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionCreateCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle h, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code.
//!                        eCLE_SuccessContinue if session contains a valid result and search is continuing.
//!                        eCLE_SuccessUnreachable if the session contains a valid result but it has been found that a connection attempt will fail and the search is continuing.
//!                        eCLE_Success and session NULL if the search has finished and function succeeded or an error that occurred while processing the function.
//! \param session     Pointer to a session search result if error was eCLE_SuccessContinue.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionSearchCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxSessionSearchResult* session, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code.
//!                        eCLE_SuccessContinue if session contains a valid result and search is continuing.
//!                        eCLE_Success and session NULL if the search has finished and function succeeded or an error that occurred while processing the function.
//! \param session     Pointer to a session search result if error was eCLE_SuccessContinue.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionQueryCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxSessionSearchResult* session, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code. eCLE_Success if session contains a valid result.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionGetUsersCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxUserInfoResult* session, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param h           If successful a handle to the session created used to identify the session in other matchmaking functions.
//! Ip, port   If successful the address that can be used to communicate with the host.
//! NAT traversal will have been done by the underlying SDK so communication will always be possible with this address.
//! But game connection could still fail if host quits before game connection is done.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionJoinCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle h, u32 ip, u16 port, uk arg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param ip          If successful, the IP address that can be used to connect to the dedicated server.
//! \param port        If successful, the port that can be used to connect to the dedicated server.
//! \param pArg        Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionSetupDedicatedServerCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, u32 ip, u16 port, uk pArg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param packet      If error is eCLE_Success the packet sent back from the host of the session. NULL otherwise.
//! \param pArg        Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionSendRequestInfoCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, CDrxLobbyPacket* pPacket, uk pArg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pData       Resulting data block.
//! \param dataSize    Size of data block.
//! \param arg         Pointer to application-specified data.
typedef void (* DrxMatchmakingSessionGetAdvertisementDataCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, u8* pData, u32 dataSize, uk arg);

struct IHostMigrationEventListener;
struct DrxUserID;
namespace Live
{
namespace State
{
struct INetworkingUser;
}
}

struct IDrxMatchMaking : public IDrxMatchMakingPrivate, public IDrxMatchMakingConsoleCommands
{
public:
	virtual ~IDrxMatchMaking(){}
	//! This function must be called by all peers before any other matchmaking calls.
	//! It defines the applications custom data used for it's sessions.
	//! This data is advertised when a host creates a session and gets returned when a session search is performed.
	//! \param data			   Pointer to an array of SDrxLobbyUserData that defines the applications custom data for a session.
	//! \param numData	   Number of items in the data array.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionRegisterUserData(SDrxLobbyUserData* data, u32 numData, DrxLobbyTaskID* taskID, DrxMatchmakingCallback cb, uk cbArg) = 0;

#if DRX_PLATFORM_DURANGO
	//! Durango specific. This is to feed the multiplayer system the user state interface which contains session information.
	//! The matchmaking system only really needs the secure device address in order to function, and the async match flow is different enough.
	//! From the way the existing matchmaking works that the surface area required is about 80% game side.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SetINetworkingUser(Live::State::INetworkingUser* user) = 0;
#endif

	//! Called by the host to create a session and make it available for others to join.
	//! \param users		   Pointer to an array of local users who will be joining the session after creation. The first user will be the session owner.
	//! The numbers given should be the pad numbers the users are using.
	//! It is important that these are correct on live as multiple accounts can be signed in one for each pad.
	//! If the wrong pad numbers are passed in then the wrong accounts will be used for the session.
	//! Remember even on a single account machine and single player game the user doesn't have to be using and signed in on pad 0.
	//! \param numUsers	   Number of users in the users array.
	//! \param flags		   Session create flags that control how the session behaves.
	//! \param data			   Pointer to a SDrxSessionData that describes the session.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionCreate(u32* users, i32 numUsers, u32 flags, SDrxSessionData* data, DrxLobbyTaskID* taskID, DrxMatchmakingSessionCreateCallback cb, uk cbArg) = 0;

	//! Called by the migrating host to ensure the lobby information is correct for the migrated session.
	//! \param h				   Handle of the current session we are migrating.
	//! \param users		   Pointer to an array of local users who will be joining the session after creation. The first user will be the session owner.
	//! The numbers given should be the pad numbers the users are using.
	//! It is important that these are correct on live as multiple accounts can be signed in one for each pad.
	//! If the wrong pad numbers are passed in then the wrong accounts will be used for the session.
	//! Remember even on a single account machine and single player game the user doesn't have to be using and signed in on pad 0.
	//! \param numUsers	   Number of users in the users array.
	//! \param flags		   Session create flags that control how the session behaves.
	//! \param data			   Pointer to a SDrxSessionData that describes the session.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionMigrate(DrxSessionHandle h, u32* pUsers, i32 numUsers, u32 flags, SDrxSessionData* pData, DrxLobbyTaskID* pTaskID, DrxMatchmakingSessionCreateCallback pCB, uk pCBArg) = 0;

	//! Called by the host to update the applications custom data. Only the data items that need updating need to be given.
	//! \param h				   Handle to the session to be updated.
	//! \param data			   Pointer to an array of SDrxLobbyUserData that contains the applications custom data that will be updated for the session.
	//! \param numData	   Number of items in the data array.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionUpdate(DrxSessionHandle h, SDrxLobbyUserData* data, u32 numData, DrxLobbyTaskID* taskID, DrxMatchmakingCallback cb, uk cbArg) = 0;

	//! Called by the host to update the number of public and private slots.
	//! Any users currently in the session will be kept in the session if there are suitable slots for them.
	//! \param h				   Handle to the session to be updated.
	//! \param numPublic   The number of public slots to change to.
	//! \param numPrivate  The number of private slots to change to.
	//! \param pTaskID	   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB			   Callback function that is called when function completes.
	//! \param pCBArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionUpdateSlots(DrxSessionHandle h, u32 numPublic, u32 numPrivate, DrxLobbyTaskID* pTaskID, DrxMatchmakingCallback pCB, uk pCBArg) = 0;

	//! Called by any one to retrieve the current state of the session.
	//! \param h				   Handle to the session to query.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionQuery(DrxSessionHandle h, DrxLobbyTaskID* taskID, DrxMatchmakingSessionQueryCallback cb, uk cbArg) = 0;

	//! Called by any one to retrieve the list of users in the session.
	//! \param h				   Handle to the session to get user information from.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionGetUsers(DrxSessionHandle h, DrxLobbyTaskID* taskID, DrxMatchmakingSessionGetUsersCallback cb, uk cbArg) = 0;

	//! Called by all peers when gameplay starts.
	//! In Live ranked sessions the session becomes unjoinable between SessionStart and SessionEnd.
	//! \param h				   Handle to the session to start.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionStart(DrxSessionHandle h, DrxLobbyTaskID* taskID, DrxMatchmakingCallback cb, uk cbArg) = 0;

	//! Called by all peers when gameplay ends.
	//! \param h				   Handle to the session to end.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionEnd(DrxSessionHandle h, DrxLobbyTaskID* taskID, DrxMatchmakingCallback cb, uk cbArg) = 0;

	//! Set the local user data for the given user. This data gets sent around the system and is returned for each user in a session with a call to SessionGetUsers.
	//! \param h				   The handle of the session to set the user data for.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param user			   The pad number of the user whose data to set.
	//! \param pData		   The data you wish to set for the user.
	//! \param dataSize	   The size of the data max DRXLOBBY_USER_DATA_SIZE_IN_BYTES.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	virtual EDrxLobbyError SessionSetLocalUserData(DrxSessionHandle h, DrxLobbyTaskID* pTaskID, u32 user, u8* pData, u32 dataSize, DrxMatchmakingCallback pCB, uk pCBArg) = 0;

	//! Called by all members of the session to delete it when they have finished with it.
	//! \param h				   Handle to the session to delete.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionDelete(DrxSessionHandle h, DrxLobbyTaskID* taskID, DrxMatchmakingCallback cb, uk cbArg) = 0;

	//! Performs a search for sessions that match the specified search criteria.
	//! \param user			   The pad number of the user doing the search.
	//! In Live the TrueSkill system is used. Each session has a skill value worked out from the members skill values.
	//! The skill value of the user doing the search is used to return sessions that have skill values closest to the users value.
	//! \param param		   Pointer to a SDrxSessionSearchParam that defines how sessions should be filtered.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSearch(u32 user, SDrxSessionSearchParam* param, DrxLobbyTaskID* taskID, DrxMatchmakingSessionSearchCallback cb, uk cbArg) = 0;

	//! Called to join a session previously created by a session host. Session id's can be obtained from a session search or from a session invite notification.
	//! \param users			 Pointer to an array of pad numbers of the local users who will be joining the session.
	//! \param numUsers	   Number of users in the users array.
	//! \param flags		   Session create flags that control how the session behaves.
	//! \param id					 A DrxSessionID of the session that will be joined.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionJoin(u32* users, i32 numUsers, u32 flags, DrxSessionID id, DrxLobbyTaskID* taskID, DrxMatchmakingSessionJoinCallback cb, uk cbArg) = 0;

	//! When DrxDedicatedServerArbitrator is being used the host calls this function to setup and get connection info for a dedicated server for their session.
	//! \param h				   Handle to the session to setup a dedicated server for.
	//! \param pPacket	   A packet that gets forwarded to the dedicated server. Contains game specific data that is used to setup the dedicated server so it is ready to accept connections.
	//! \param pTaskID	   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB			   Callback function that is called when function completes.
	//! \param pCBArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSetupDedicatedServer(DrxSessionHandle h, CDrxLobbyPacket* pPacket, DrxLobbyTaskID* pTaskID, DrxMatchmakingSessionSetupDedicatedServerCallback pCB, uk pCBArg) = 0;

	//! Called by the host to release the dedicated server allocated to the given session by SessionSetupDedicatedServer.
	//! \param h				   Handle to the session to release the dedicated server for.
	//! \param pTaskID	   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB			   Callback function that is called when function completes.
	//! \param pCBArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionReleaseDedicatedServer(DrxSessionHandle h, DrxLobbyTaskID* pTaskID, DrxMatchmakingCallback pCB, uk pCBArg) = 0;

	//! When not connected to the session send a packet to the session host and wait for a response.
	//! The session host needs to have registered an event interest for eCLSE_SessionRequestInfo.
	//! \param pPacket	   The packet to send. Contains game specific data to specify the data being requested.
	//! \param id					 A DrxSessionID of the session that the packet will be sent to.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSendRequestInfo(CDrxLobbyPacket* pPacket, DrxSessionID id, DrxLobbyTaskID* pTaskID, DrxMatchmakingSessionSendRequestInfoCallback pCB, uk pCBArg) = 0;

	//! Send a packet back to the requester when an eCLSE_SessionRequestInfo event is received.
	//! \param pPacket	   The packet to send. Contains the game specific data requested.
	//! \param requester   A DrxSessionRequesterID of the requester to send the packet to.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSendRequestInfoResponse(CDrxLobbyPacket* pPacket, DrxSessionRequesterID requester) = 0;

	//! Attempts to join the first searchable hosted session at the ip:port specified by cl_serveraddr.
	virtual void SessionJoinFromConsole(void) = 0;

	//! Uses the internal host hinting to dictate whether to migrate the session.
	//! Should really only be used in the pre-game lobby as the player list should be relatively stable during the game.
	//! \param gh				   Game session handle.
	//! \param taskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb				   Callback function that is called when function completes.
	//! \param pCbArg		   Pointer to application-specified data that is passed to the callback.
	//! \return		   eCLS_SUCCESS if best host determined.
	virtual EDrxLobbyError SessionEnsureBestHost(DrxSessionHandle gh, DrxLobbyTaskID* pTaskID, DrxMatchmakingCallback pCB, uk pCBArg) = 0;

	//! This function should be called when a group of connections (including the host connection) are about to leave the
	//! Session as an atomic operation. It will change the hints for the specified connections so that they're deemed
	//! Unsuitable as candidates for becoming the new host, synchronise the hints across the connections and then stop.
	//! Sending hint updates for that session. The specified connections can then leave the session and a new host will.
	//! Be chosen from the remaining connections, and hinting will then resume.
	//! \param gh						   Game session handle.
	//! \param pConnections	   Array of connections that are about to be disconnected.
	//! \param numConnections  Number of connections that are about to be disconnected.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB					   Callback function that is called when function completes.
	//! \param pCBArg				   Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionTerminateHostHintingForGroup(DrxSessionHandle gh, SDrxMatchMakingConnectionUID* pConnections, u32 numConnections, DrxLobbyTaskID* pTaskID, DrxMatchmakingCallback pCB, uk pCBArg) = 0;

	//! Sets the specified local game flags for the specified session.
	//! \param gh						   Game session handle.
	//! \param flags				   Local game flags.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB					   Callback function that is called when function completes.
	//! \param pCBArg				   Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSetLocalFlags(DrxSessionHandle gh, u32 flags, DrxLobbyTaskID* pTaskID, DrxMatchmakingFlagsCallback pCB, uk pCBArg) = 0;

	//! Clears the specified local game flags for the specified session.
	//! \param gh						   Game session handle.
	//! \param flags				   Local game flags.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB					   Callback function that is called when function completes.
	//! \param pCBArg				   Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionClearLocalFlags(DrxSessionHandle gh, u32 flags, DrxLobbyTaskID* pTaskID, DrxMatchmakingFlagsCallback pCB, uk pCBArg) = 0;

	//! Gets the local game flags for the specified session.
	//! \param gh						   Game session handle.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCB					   Callback function that is called when function completes.
	//! \param pCBArg				   Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionGetLocalFlags(DrxSessionHandle gh, DrxLobbyTaskID* pTaskID, DrxMatchmakingFlagsCallback pCB, uk pCBArg) = 0;

	//! Sets a data block that can be remotely retrieved without joining a session.
	//! \param gh						   Game session handle.
	//! \param pData				   Pointer to data block to set.
	//! \param dataSize			   Size of the data block.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb					   Callback function that is called when the function completes.
	//! \param pCbArg					 Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionSetAdvertisementData(DrxSessionHandle gh, u8* pData, u32 dataSize, DrxLobbyTaskID* pTaskID, DrxMatchmakingCallback pCb, uk pCbArg) = 0;

	//! Gets a remote data block for session, without needing to join that session.
	//! \param sessionId		   SessionID for the remote session.
	//! \param pTaskID			   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb					   Callback function that is called when the function completes.
	//! \param pCbArg					 Pointer to application-specified data that is passed to the callback.
	//! \return				   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SessionGetAdvertisementData(DrxSessionID sessionId, DrxLobbyTaskID* pTaskID, DrxMatchmakingSessionGetAdvertisementDataCallback pCb, uk pCbArg) = 0;

	//! Get the session address from a session ID.
	//! \param sessionID		 Session ID.
	//! \param hostIP				 Output session IP address in host byte order.
	//! \param hostPort			 Output session port in host byte order.
	//! \return				 eCLE_Success if successful, otherwise an error.
	virtual EDrxLobbyError GetSessionAddressFromSessionID(DrxSessionID sessionID, u32& hostIP, u16& hostPort) = 0;

	//! Cancel the given task. The task will still be running in the background but the callback will not be called when it finishes.
	//! \param taskID		   The task to cancel.
	virtual void CancelTask(DrxLobbyTaskID taskID) = 0;

	//! Send a packet from a client to the server of the given session.
	//! \param pPacket	   The packet to send.
	//! \param h				   The handle of the session to send to the server of.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SendToServer(CDrxLobbyPacket* pPacket, DrxSessionHandle h) = 0;

	//! Send a packet from the server to all the clients of the given session.
	//! \param pPacket	   The packet to send.
	//! \param h				   The handle of the session to send to all the clients of.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SendToAllClients(CDrxLobbyPacket* pPacket, DrxSessionHandle h) = 0;

	//! Send a packet from the server to the given client of the given session.
	//! \param pPacket	   The packet to send.
	//! \param h				   The handle of the session to send to a client of.
	//! \param uid			   The connection uid of the client to send to.
	//! \return		   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError SendToClient(CDrxLobbyPacket* pPacket, DrxSessionHandle h, SDrxMatchMakingConnectionUID uid) = 0;

	//! Get the DrxSessionID from a DrxSessionHandle.
	//! \param h				   The handle of the session to get the DrxSessionID of.
	//! \return		   The DrxSessionID of the session.
	virtual DrxSessionID SessionGetDrxSessionIDFromDrxSessionHandle(DrxSessionHandle h) = 0;

	//! Returns the size of DrxSessionID in a packet for specific platform.
	//! \return		   The size of the platform specific DrxSessionID.
	virtual u32 GetSessionIDSizeInPacket() const = 0;

	//! Adds the sessionId to a packet so it can be send across the network.
	//! \param sessionId	   SessionId you want to add to the packet.
	//! \param pPacket		   Packet you want to add the sessionId to.
	//! \return		   eCLS_Success if function it writes it to the packet.
	virtual EDrxLobbyError WriteSessionIDToPacket(DrxSessionID sessionId, CDrxLobbyPacket* pPacket) const = 0;

	//! Gets a platform specific sessionsId from a packet.
	//! \param pPacket		   packet you want to read the sessionID from.
	//! \return		   DrxSessionID that was inside the packet.
	virtual DrxSessionID ReadSessionIDFromPacket(CDrxLobbyPacket* pPacket) const = 0;

	//! Gets a matchmaking connection UID from the game session handle and the channel ID.
	//! \param gh				   Game session handle.
	//! \param channelID	 Channel ID.
	virtual SDrxMatchMakingConnectionUID GetConnectionUIDFromGameSessionHandleAndChannelID(DrxSessionHandle gh, u16 channelID) = 0;

	//! Gets the matchmaking connection UID of the host of the given session.
	//! \param gh				   Game session handle.
	//! \return		   SDrxMatchMakingConnectionUID of the host.
	virtual SDrxMatchMakingConnectionUID GetHostConnectionUID(DrxSessionHandle gh) = 0;

	//! Terminates host migration for the specified session.
	virtual void TerminateHostMigration(DrxSessionHandle gh) = 0;

	//! \return The host migration state of the specified session.
	virtual eHostMigrationState GetSessionHostMigrationState(DrxSessionHandle gh) = 0;

	//! Get the ping value for a player in a session.
	//! \param uid				 Session and player id.
	//! \param pPing			 Ping.
	//! \return			 eCLE_Success of successful, otherwise an error.
	virtual EDrxLobbyError GetSessionPlayerPing(SDrxMatchMakingConnectionUID uid, DrxPing* const pPing) = 0;

	//! Get a DrxSessionID from a session URL.
	//! \param pSessionURL Session URL.
	//! \param pSessionID	 Session ID.
	//! \return			 eCLE_Success if successful, otherwise an error.
	virtual EDrxLobbyError GetSessionIDFromSessionURL(tukk const pSessionURL, DrxSessionID* const pSessionID) = 0;

	//! Get a DrxSessionID from an ASCII IP address.
	//! \param pAddr			 IP address.
	//! \param pSessionID	 Session ID.
	//! \return			 eCLE_Success if successful, otherwise an error.
	virtual EDrxLobbyError GetSessionIDFromIP(tukk const pAddr, DrxSessionID* const pSessionID) = 0;

	//! Returns the time since we last received a packet in milliseconds from the server for the given session handle.
	//! \param gh				   Game session handle.
	//! \return		   Time since packet received in milliseconds.
	virtual u32 GetTimeSincePacketFromServerMS(DrxSessionHandle gh) = 0;

	//! Forces the server connection to disconnect.
	//! \param gh				   Game session handle.
	virtual void DisconnectFromServer(DrxSessionHandle gh) = 0;

	//! \param pUserID DrxUserID* of the user to kick.
	//! \param cause Reason for the kick.
	virtual void Kick(const DrxUserID* pUserID, EDisconnectionCause cause) = 0;

	//! \param pUserID Pointer to the DrxUserID of the user to ban.
	//! \param timeout Ban timeout in minutes.
	virtual void Ban(const DrxUserID* pUserID, float timeout) = 0;

	//! Retrieve the user name from a matchmaking connection.
	//! \param gh Game session handle.
	virtual tukk GetNameFromGameSessionHandle(DrxSessionHandle gh) = 0;
	// </interfuscator:shuffle>
};
