// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "CommonIDrxLobby.h"

#define DRXLOBBY_USER_NAME_LENGTH        32
#define DRXLOBBY_USER_GUID_STRING_LENGTH 40

struct SDrxUserID : public CMultiThreadRefCount
{
	virtual bool                                              operator==(const SDrxUserID& other) const = 0;
	virtual bool                                              operator<(const SDrxUserID& other) const = 0;

	virtual DrxFixedStringT<DRXLOBBY_USER_GUID_STRING_LENGTH> GetGUIDAsString() const
	{
		return DrxFixedStringT<DRXLOBBY_USER_GUID_STRING_LENGTH>("");
	}
};

struct DrxUserID
{
	DrxUserID() : userID()
	{}

	DrxUserID(SDrxUserID* ptr) : userID(ptr)
	{}

	const SDrxUserID* get() const
	{
		return userID.get();
	}

	bool operator!=(const DrxUserID& other) const
	{
		return !(*this == other);
	}

	bool operator==(const DrxUserID& other) const
	{
		if (other.IsValid() && IsValid())
		{
			return ((*other.userID) == (*userID));
		}
		if ((!other.IsValid()) && (!IsValid()))
		{
			return true;
		}
		return false;
	}

	bool operator<(const DrxUserID& other) const
	{
		// In the case where one is invalid, the invalid one is considered less than the valid one
		if (other.IsValid())
		{
			if (IsValid())
			{
				return (*userID) < (*other.userID);
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	bool IsValid() const
	{
		return (userID.get() != NULL);
	}

	_smart_ptr<SDrxUserID> userID;
};

const DrxUserID DrxUserInvalidID = NULL;

class CLobbyString : public string
{
public:
	CLobbyString() : string() {}
	CLobbyString(u32 n) : string() { Format("%u", n); }
	CLobbyString(string s) : string(s) {}
	CLobbyString(u16 n) : string() { Format("%u", n); }
	CLobbyString(i32 n) : string() { Format("%i", n); }
	bool operator==(const CLobbyString& other) { return compare(other) == 0; }
	bool operator==(CLobbyString& other) { return compare(other) == 0; }
	CLobbyString(const CLobbyString& s) : string(s) {}
private:
	CLobbyString(tukk p) {}
};
typedef CLobbyString DrxLobbyUserDataID;

enum EDrxLobbyUserDataType
{
	eCLUDT_Int64,
	eCLUDT_Int32,
	eCLUDT_Int16,
	eCLUDT_Int8,
	eCLUDT_Float64,
	eCLUDT_Float32,
	eCLUDT_Int64NoEndianSwap
};

struct SDrxLobbyUserData
{
	DrxLobbyUserDataID    m_id;
	EDrxLobbyUserDataType m_type;

	SDrxLobbyUserData()
	{
		m_id = DrxLobbyUserDataID();
		m_type = eCLUDT_Int64;
		m_int64 = 0;
	}

	union
	{
		int64 m_int64;
		f64   m_f64;
		i32 m_int32;
		f32   m_f32;
		i16 m_int16;
		int8  m_int8;
	};

	const SDrxLobbyUserData& operator=(const SDrxLobbyUserData& src)
	{
		m_id = src.m_id;
		m_type = src.m_type;

		switch (m_type)
		{
		case eCLUDT_Int64:
			m_int64 = src.m_int64;
			break;
		case eCLUDT_Int32:
			m_int32 = src.m_int32;
			break;
		case eCLUDT_Int16:
			m_int16 = src.m_int16;
			break;
		case eCLUDT_Int8:
			m_int8 = src.m_int8;
			break;
		case eCLUDT_Float64:
			m_f64 = src.m_f64;
			break;
		case eCLUDT_Float32:
			m_f32 = src.m_f32;
			break;
		case eCLUDT_Int64NoEndianSwap:
			m_int64 = src.m_int64;
			break;
		default:
			DrxLog("Unhandled EDrxLobbyUserDataType %d", m_type);
			break;
		}

		return *this;
	};

	bool operator==(const SDrxLobbyUserData& other)
	{
		if ((m_id == other.m_id) && (m_type == other.m_type))
		{
			switch (m_type)
			{
			case eCLUDT_Int64:
				return m_int64 == other.m_int64;
			case eCLUDT_Int32:
				return m_int32 == other.m_int32;
			case eCLUDT_Int16:
				return m_int16 == other.m_int16;
			case eCLUDT_Int8:
				return m_int8 == other.m_int8;
			case eCLUDT_Float64:
				return m_f64 == other.m_f64;
			case eCLUDT_Float32:
				return m_f32 == other.m_f32;
			case eCLUDT_Int64NoEndianSwap:
				return m_int64 == other.m_int64;
			default:
				DrxLog("Unhandled EDrxLobbyUserDataType %d", m_type);
				return false;
			}
		}

		return false;
	}

	bool operator!=(const SDrxLobbyUserData& other)
	{
		if ((m_id == other.m_id) && (m_type == other.m_type))
		{
			switch (m_type)
			{
			case eCLUDT_Int64:
				return m_int64 != other.m_int64;
			case eCLUDT_Int32:
				return m_int32 != other.m_int32;
			case eCLUDT_Int16:
				return m_int16 != other.m_int16;
			case eCLUDT_Int8:
				return m_int8 != other.m_int8;
			case eCLUDT_Float64:
				return m_f64 != other.m_f64;
			case eCLUDT_Float32:
				return m_f32 != other.m_f32;
			case eCLUDT_Int64NoEndianSwap:
				return m_int64 != other.m_int64;
			default:
				DrxLog("Unhandled EDrxLobbyUserDataType %d", m_type);
				return true;
			}
		}

		return true;
	}
};


typedef u32 DrxStatsLeaderBoardID;

//! The score column for a leader board entry.
//! In Live the score id is the property id from the xlast program given to the score column when defining a leaderboard.
struct SDrxStatsLeaderBoardScore
{
	SDrxLobbyUserData  score;
	DrxLobbyUserDataID id;
};

//! The user defined columns for a leaderboard.
//! In Live a leaderboard can have up to 64 user defined columns.
//! The DrxLobbyUserDataID inside data is the property id from the xlast program given to the column when defining a leaderboard
//! columnID is the id output by the xlast program when defining a leaderboard.
struct SDrxStatsLeaderBoardUserColumn
{
	SDrxLobbyUserData  data;
	DrxLobbyUserDataID columnID;
};

//! A leaderboard row contains a score column and 0 or more custom columns.
struct SDrxStatsLeaderBoardData
{
	SDrxStatsLeaderBoardScore       score;
	SDrxStatsLeaderBoardUserColumn* pColumns;
	u32                          numColumns;
};

//! In Live the leaderboard id is output by the xlast program when defining a leaderboard.
struct SDrxStatsLeaderBoardWrite
{
	SDrxStatsLeaderBoardData data;
	DrxStatsLeaderBoardID    id;
};

struct SDrxStatsLeaderBoardReadRow
{
	SDrxStatsLeaderBoardData data;
	u32                   rank;
	DrxUserID                userID;
	char                     name[DRXLOBBY_USER_NAME_LENGTH];
};

struct SDrxStatsLeaderBoardReadResult
{
	DrxStatsLeaderBoardID        id;
	SDrxStatsLeaderBoardReadRow* pRows;
	u32                       numRows;
	u32                       totalNumBoardRows;
};

//! \param taskID	   Task ID allocated when the function was executed
//! \param error		 Error code    eCLE_Success if the function succeeded or an error that occurred while processing the function
//! \param pArg			 Pointer to application-specified data
typedef void(*DrxStatsCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

//! \param taskID	   Task ID allocated when the function was executed
//! \param error		 Error code    eCLE_Success if the function succeeded or an error that occurred while processing the function
//! \param pResult	 If error is eCLE_Success a pointer to a SDrxStatsLeaderBoardReadResult which contains the information read from the leaderboard.
//! \param pArg			 Pointer to application-specified data
typedef void(*DrxStatsReadLeaderBoardCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxStatsLeaderBoardReadResult* pResult, uk pArg);

//! \param taskID	   Task ID allocated when the function was executed
//! \param error		 Error code    eCLE_Success if the function succeeded or an error that occurred while processing the function
//! \param pData		 Pointer to an array of SDrxLobbyUserData that will match the data registered and contain the last data written.
//! \param numData	 The number of SDrxLobbyUserData returned.
//! \param pArg			 Pointer to application-specified data
typedef void(*DrxStatsReadUserDataCallback)(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUserData* pData, u32 numData, uk pArg);

enum EDrxLobbyLeaderboardType
{
	eCLLT_P2P,
	eCLLT_Dedicated,
	eCLLT_Num
};

struct IDrxStats
{
	// <interfuscator:shuffle>
	virtual ~IDrxStats() {}

	//! This function must be called before any other leaderboard functions.
	//! It defines the applications custom data used for it's leaderboards.
	//! \param pBoards		   Pointer to an array of SDrxStatsLeaderBoardWrite that defines the applications leaderboards
	//! \param numBoards	   Number of leaderboards to register
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback
	//! \param cb					   Callback function that is called when function completes
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start
	virtual EDrxLobbyError StatsRegisterLeaderBoards(SDrxStatsLeaderBoardWrite* pBoards, u32 numBoards, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Write one or more leaderboard entries for the given user.
	//! In Live this call must be made between SessionStart and SessionEnd.
	//! \param session		   The session the user is in and the stats are for.
	//! \param user				   The pad number of the local user the stats are being written for.
	//! \param pBoards		   Pointer to an array of leaderboard entires to be written.
	//! \param numBoards	   Number of leaderboard entries to be written.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsWriteLeaderBoards(DrxSessionHandle session, u32 user, SDrxStatsLeaderBoardWrite* pBoards, u32 numBoards, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Write one or more leaderboard entries for one or more users.
	//! In Live this call must be made between SessionStart and SessionEnd.
	//! \param session		   The session the users are in and the stats are for.
	//! \param pUserIDs		   Pointer to an array of DrxUserID of the users the stats are for.
	//! \param ppBoards		   Pointer to an array of arrays of leaderboard entries to be written. One array to be written for each user.
	//! \param pNumBoards	   The number of leaderboard entries to be written for each user.
	//! \param numUserIDs	   The number of users being written.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsWriteLeaderBoards(DrxSessionHandle session, DrxUserID* pUserIDs, SDrxStatsLeaderBoardWrite** ppBoards, u32* pNumBoards, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Retrieves a list of entires in the order of their ranking within a leaderboard, starting with a specified rank value.
	//! \param board			   The leaderboard to read from.
	//! \param startRank	   The rank to start retrieving from.
	//! \param num				   The number of entires to retrieve.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsReadLeaderBoardByRankForRange(DrxStatsLeaderBoardID board, u32 startRank, u32 num, DrxLobbyTaskID* pTaskID, DrxStatsReadLeaderBoardCallback cb, uk pCbArg) = 0;

	//! Retrieves a list of entires in the order of their ranking within a leaderboard, with the given local user appearing in the middle.
	//! \param board			   The leaderboard to read from.
	//! \param user				   The pad number of the local user who will appear in the middle of the list.
	//! \param num				   The number of entires to retrieve.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsReadLeaderBoardByRankForUser(DrxStatsLeaderBoardID board, u32 user, u32 num, DrxLobbyTaskID* pTaskID, DrxStatsReadLeaderBoardCallback cb, uk pCbArg) = 0;

	//! Retrieves a list of entires for a given list of users in the order of their ranking within a leaderboard.
	//! \param board			   The leaderboard to read from.
	//! \param pUserIDs		   Pointer to an array of DrxUserID for the users to read.
	//! \param numUserIDs	   Number of users to read.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsReadLeaderBoardByUserID(DrxStatsLeaderBoardID board, DrxUserID* pUserIDs, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxStatsReadLeaderBoardCallback cb, uk pCbArg) = 0;

	//! This function must be called before any other user data functions.
	//! It defines the applications custom data used for it's users.
	//! In Live a maximum of 3000 bytes of custom user data can be stored.
	//! \param pData			   Pointer to an array of SDrxLobbyUserData that defines the user data the application wants to store for each user.
	//! \param numData		   Number of items to store.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsRegisterUserData(SDrxLobbyUserData* pData, u32 numData, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Write the user data for a local user.
	//! \param user				   The pad number of the local user to have their data written.
	//! \param pData			   Pointer to an array of SDrxLobbyUserData that defines the user data to write.
	//! \param numData		   Number of data items to write.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsWriteUserData(u32 user, SDrxLobbyUserData* pData, u32 numData, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Write the user data for an array of users.
	//! \param pUserIDs		   The user IDs to have their data written.
	//! \param ppData			   Ragged 2D array of SDrxLobbyUserData that defines the user data to write.
	//! \param pNumData		   Array of lengths of rows in ppData.
	//! \param numUserIDs	   The number of user IDs.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsWriteUserData(DrxUserID* pUserIDs, SDrxLobbyUserData** ppData, u32* pNumData, u32 numUserIDs, DrxLobbyTaskID* pTaskID, DrxStatsCallback cb, uk pCbArg) = 0;

	//! Read the user data for a local user.
	//! \param user				   The pad number of the local user to read data for.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsReadUserData(u32 user, DrxLobbyTaskID* pTaskID, DrxStatsReadUserDataCallback cb, uk pCbArg) = 0;

	//! Read the user data for a given DrxUserID.
	//! \param user				   The pad number of the local user who is doing the read.
	//! \param userID			   The DrxUserID of the user to read data for.
	//! \param pTaskID		   Pointer to buffer to store the task ID to identify this call in the callback.
	//! \param cb					   Callback function that is called when function completes.
	//! \param pCbArg			   Pointer to application-specified data that is passed to the callback.
	//! \return			   eCLE_Success if function successfully started or an error code if function failed to start.
	virtual EDrxLobbyError StatsReadUserData(u32 user, DrxUserID userID, DrxLobbyTaskID* pTaskID, DrxStatsReadUserDataCallback cb, uk pCbArg) = 0;

	//! Cancel the given task. The task will still be running in the background but the callback will not be called when it finishes.
	//! \param taskID			   The task to cancel
	virtual void CancelTask(DrxLobbyTaskID taskID) = 0;

	//! Set the leaderboard type.
	//! \param leaderboardType	   The leaderboard type
	virtual void SetLeaderboardType(EDrxLobbyLeaderboardType leaderboardType) = 0;

	//! Associates a name with an ID for API's that handle leaderboard by string.
	//! \param name    leaderboard name API requires
	//! \param id    the ID passed to DrxStats when reading/writing
	virtual void RegisterLeaderboardNameIdPair(string name, u32 id) = 0;

	//! Get the leaderboard type.
	virtual EDrxLobbyLeaderboardType GetLeaderboardType() = 0;

	// </interfuscator:shuffle>
};
