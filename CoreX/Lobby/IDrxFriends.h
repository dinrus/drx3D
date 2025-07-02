// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

<drx3D/CoreX/Lobby/IDrxLobby.h> // <> required for Interfuscator

#define DRXLOBBY_PRESENCE_MAX_SIZE 256

struct SFriendInfo
{
	DrxUserID          userID;
	char               name[DRXLOBBY_USER_NAME_LENGTH];
	char               presence[DRXLOBBY_PRESENCE_MAX_SIZE];
	ELobbyFriendStatus status;
};

//! \param TaskID   Task ID allocated when the function was executed.
//! \param Error    Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param PArg     Pointer to application-specified data.
typedef void (* DrxFriendsCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

//! \param TaskID       Task ID allocated when the function was executed.
//! \param Error        Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param PFriendInfo  Pointer to an array of SFriendInfo containing info about the friends retrieved.
//! \param NumFriends   Number of items in the pFriendInfo array.
//! \param PArg         Pointer to application-specified data.
typedef void (* DrxFriendsGetFriendsListCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendInfo* pFriendInfo, u32 numFriends, uk pArg);

struct IDrxFriends
{
	// <interfuscator:shuffle>
	virtual ~IDrxFriends(){}

	//! Retrieves the Friends list of the specified user.
	//! \param User      The pad number of the user to retrieve the friends list for.
	//! \param Start     The start index to retrieve from. First friend is 0.
	//! \param Num       Maximum number of friends to retrieve.
	//! \param PTaskID   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param Cb        Callback function that is called when function completes.
	//! \param PCbArg    Pointer to application-specified data that is passed to the callback.
	//! \return eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsGetFriendsList(u32 user, u32 start, u32 num, DrxLobbyTaskID* pTaskID, DrxFriendsGetFriendsListCallback cb, uk pCbArg) = 0;

	//! Send game invites to the given list of users for the given session.
	//! \param User        The pad number of the user sending the game invites.
	//! \param H           The handle to the session the invites are for.
	//! \param PUserIDs    Pointer to an array of user ids to send invites to.
	//! \param NumUserIDs  The number of users to invite.
	//! \param PTaskID     Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param Cb          Callback function that is called when function completes.
	//! \param PCbArg      Pointer to application-specified data that is passed to the callback.
	//! \return	eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError FriendsSendGameInvite(u32 user, DrxSessionHandle h, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxFriendsCallback cb, uk pCbArg) = 0;
	// </interfuscator:shuffle>
};
