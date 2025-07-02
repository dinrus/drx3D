// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

enum EDrxRewardError
{
	eCRE_Queued = 0,     //!< Reward successfully queued.
	eCRE_Busy,           //!< Reward queue full - try again later.
	eCRE_Failed          //!< Reward process failed.
};

//! \param taskID		Task ID allocated when the function was executed.
//! \param error		Error code - eCLE_Success if the function succeeded or an error that occurred while processing the function.
//! \param pArg			Pointer to application-specified data.
typedef void (* DrxRewardCallback)(DrxLobbyTaskID taskID, u32 playerID, u32 awardID, EDrxLobbyError error, bool alreadyAwarded, uk pArg);

struct IDrxReward
{
	// <interfuscator:shuffle>
	virtual ~IDrxReward(){}

	//! Awards an achievement/trophy/reward to the specified player.
	//! \param playerID	Player ID.
	//! \param awardID	Award ID (probably implemented as an enumerated type).
	//! \return Informs the caller that the award was added to the pending queue or not.
	virtual EDrxRewardError Award(u32 playerID, u32 awardID, DrxLobbyTaskID* pTaskID, DrxRewardCallback cb, uk pCbArg) = 0;
	// </interfuscator:shuffle>
};
