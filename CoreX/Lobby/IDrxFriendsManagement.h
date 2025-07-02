// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

<drx3D/CoreX/Lobby/IDrxFriends.h> // <> required for Interfuscator

//! Used to return IsFriend/IsUserBlocked results.
struct SFriendManagementInfo
{
	DrxUserID userID;
	bool      result;
};

//! Used to pass in user to search for.
struct SFriendManagementSearchInfo
{
	char name[DRXLOBBY_USER_NAME_LENGTH];
};

#define DRXLOBBY_FRIEND_STATUS_STRING_SIZE   256
#define DRXLOBBY_FRIEND_LOCATION_STRING_SIZE 256

enum EFriendManagementStatus
{
	eFMS_Unknown,
	eFMS_Offline,
	eFMS_Online,
	eFMS_Playing,
	eFMS_Staging,
	eFMS_Chatting,
	eFMS_Away
};

struct SFriendStatusInfo
{
	DrxUserID               userID;
	EFriendManagementStatus status;
	char                    statusString[DRXLOBBY_FRIEND_STATUS_STRING_SIZE];
	char                    locationString[DRXLOBBY_FRIEND_LOCATION_STRING_SIZE];
};

//! \param taskID   Task ID allocated when the function was executed.
//! \param error    Error code eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg     Pointer to application-specified data.
typedef void (* DrxFriendsManagementCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pCbArg);

//! \param taskID    Task ID allocated when the function was executed.
//! \param error     Error code eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg      Pointer to application-specified data.
typedef void (* DrxFriendsManagementInfoCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendManagementInfo* pInfo, u32 numUserIDs, uk pCbArg);

//! \param taskID    Task ID allocated when the function was executed.
//! \param error     Error code   eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg      Pointer to application-specified data.
typedef void (* DrxFriendsManagementSearchCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendInfo* pInfo, u32 numUserIDs, uk pCbArg);

//! \param taskID      Task ID allocated when the function was executed.
//! \param error       Error code   eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pInfo       Array of returned friend status info.
//! \param numUserIDs  Number of elements in array.
//! \param pArg        Pointer to application-specified data.
typedef void (* DrxFriendsManagementStatusCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendStatusInfo* pInfo, u32 numUserIDs, uk pCbArg);

struct IDrxFriendsManagement
{
	// <interfuscator:shuffle>
	virtual ~IDrxFriendsManagement() {}

	//! Sends a friend request to the specified list of users.
	//! \param User         The pad number of the user sending the friend requests.
	//! \param pUserIDs     Pointer to an array of user ids to send friend requests to.
	//! \param numUserIDs   The number of users to send friend requests to.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb          Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementSendFriendRequest(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Accepts friend requests from the specified list of users.
	//! \param User         The pad number of the user accepting the friend requests.
	//! \param pUserIDs     Pointer to an array of user ids to accept friend requests from.
	//! \param numUserIDs   The number of users to accept friend requests from.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb          Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementAcceptFriendRequest(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Rejects friend requests from the specified list of users.
	//! \param User         The pad number of the user rejecting the friend requests.
	//! \param pUserIDs     Pointer to an array of user ids to reject friend requests from.
	//! \param numUserIDs   The number of users to reject friend requests from.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb          Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementRejectFriendRequest(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Revokes friendship status from the specified list of users.
	//! \param User         The pad number of the user revoking friend status.
	//! \param pUserIDs     Pointer to an array of user ids to revoke friend status from.
	//! \param numUserIDs   The number of users to revoke friend status from.
	//! \param pTaskID      Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb          Callback function that is called when function completes.
	//! \param pCbArg       Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementRevokeFriendStatus(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Determines friendship status of the specified list of users.
	//! \param User          The pad number of the user determining friend status.
	//! \param pUserIDs      Pointer to an array of user ids to determine friend status for.
	//! \param numUserIDs    The number of users to determine friend status for.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementIsUserFriend(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementInfoCallback pCb, uk pCbArg) = 0;

	//! Attempt to find the users specified.
	//! \param User          The pad number of the user attempting to find other users.
	//! \param pUserIDs      Pointer to an array of user ids to attempt to find.
	//! \param numUserIDs    The number of users to attempt to find.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementFindUser(u32 user, SFriendManagementSearchInfo* pUserName, u32 maxResults, DrxLobbyTaskID* pTaskID, DrxFriendsManagementSearchCallback pCb, uk pCbArg) = 0;

	//! Add the specified users to the blocked list.
	//! \param User          The pad number of the user adding users to their block list.
	//! \param pUserIDs      Pointer to an array of user ids to block.
	//! \param numUserIDs    The number of users to block.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementBlockUser(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Remove the specified users from the blocked list.
	//! \param User          The pad number of the user removing users from their block list.
	//! \param pUserIDs      Pointer to an array of user ids to unblock.
	//! \param numUserIDs    The number of users to unblock.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementUnblockUser(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Determines if the specified users are in the blocked list.
	//! \param User          The pad number of the user determining blocked status.
	//! \param pUserIDs      Pointer to an array of user ids to determine blocked status for.
	//! \param numUserIDs    The number of users to determine blocked status for.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementIsUserBlocked(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementInfoCallback pCb, uk pCbArg) = 0;

	//! Accepts the invitation from the specified user.
	//! \param User          The pad number of the user accepting the invite.
	//! \param PUserID       Pointer to the user id to accept the invite from.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementAcceptInvite(u32 user, DrxUserID* pUserID, DrxLobbyTaskID* pTaskID, DrxFriendsManagementCallback pCb, uk pCbArg) = 0;

	//! Gets the names of the specified list of user IDs.
	//! \param User          The pad number of the user requesting names.
	//! \param pUserIDs      Pointer to an array of user ids to get names for.
	//! \param numUserIDs    The number of users to get names for.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementGetName(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementSearchCallback pCb, uk pCbArg) = 0;

	//! Gets the status of the specified list of user IDs.
	//! \param User          The pad number of the user requesting names.
	//! \param pUserIDs      Pointer to an array of user ids to get names for.
	//! \param numUserIDs    The number of users to get names for.
	//! \param pTaskID       Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param pCb           Callback function that is called when function completes.
	//! \param pCbArg        Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsManagementGetStatus(u32 user, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsManagementStatusCallback pCb, uk pCbArg) = 0;

	//! Cancels a task.
	virtual void CancelTask(DrxLobbyTaskID) = 0;
	// </interfuscator:shuffle>
};
