// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UILobbyMP.h
//  Version:     v1.00
//  Created:     08/06/2012 by Michiel Meesters.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UILOBBYMP_H_
#define __UILOBBYMP_H_

#include <drx3D/Game/IUIGameEventSystem.h>
#include <drx3D/Sys/Scaleform/IFlashUI.h>
#include <drx3D/CoreX/Lobby/IDrxStats.h>
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/CoreX/Lobby/IDrxFriends.h>

class CUILobbyMP 
	: public IUIGameEventSystem
	, public IUIModule
{
public:
	CUILobbyMP();

	// IUIGameEventSystem
	UIEVENTSYSTEM( "UILobbyMP" );
	virtual void InitEventSystem() override;
	virtual void UnloadEventSystem() override;

	// IUIModule
	virtual void Reset() override;
	// ~IUIModule

	// UI functions
	void InviteAccepted();
	void SearchCompleted();
	void SearchStarted();
	void UpdateNatType();
	void ServerFound(SDrxSessionSearchResult session, string sServerName);
	void PlayerListReturn(const SUIArguments& players, const SUIArguments& playerids);
	void ReadLeaderBoard(i32 leaderboardIdx, i32 mode, i32 nrOfEntries);
	void WriteLeaderBoard(i32 leaderboardIdx, tukk values);
	void RegisterLeaderBoard(string leaderboardName, i32 leaderBoardIdx, i32 nrOfColumns);
	void ShowLoadingDialog(const  tuk sLoadingDialog);
	void HideLoadingDialog(const  tuk sLoadingDialog);
	void LeaderboardEntryReturn(const SUIArguments& customColumns);
	void InviteFriends(i32 containerIdx, i32 friendIdx);
	void SendUserStat(SUIArguments& arg);

	//Callback when session is found
	static void MatchmakingSessionSearchCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxSessionSearchResult* session, uk arg);
	static void ReadLeaderBoardCB(DrxLobbyTaskID TaskID, EDrxLobbyError Error, SDrxStatsLeaderBoardReadResult *Result, uk Arg);
	static void RegisterLeaderboardCB(DrxLobbyTaskID TaskID, EDrxLobbyError Error, uk Arg);
	static void WriteLeaderboardCallback(DrxLobbyTaskID TaskID, EDrxLobbyError Error, uk Arg);

	static void ReadUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUserData* pData, u32 numData, uk pArg);

	static void GetFriendsCB(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendInfo* pFriendInfo, u32 numFriends, uk pArg);
	static void InviteFriendsCB(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

private:
	// UI events
	void GetUserStats();
	void SearchGames(bool bLan);
	void AwardTrophy(i32 trophy);
	void JoinGame(u32 sessionID);
	void HostGame(bool bLan, string sMapPath, string sGameRules);
	void SetMultiplayer(bool bIsMultiplayer);
	void LockController( bool bLock );
	void LeaveGame();
	void GetPlayerList();
	void MutePlayer(i32 playerId, bool mute);
	void GetFriends(i32 containerIdx);

private:
	enum EUIEvent
	{
		eUIE_ServerFound = 0,
		eUIE_PlayerListReturn,
		eUIE_PlayerIdListReturn,
		eUIE_InviteAccepted,
		eUIE_SearchStarted,
		eUIE_SearchCompleted,
		eUIE_NatTypeUpdated,
		eUIE_ShowLoadingDialog,
		eUIE_HideLoadingDialog,
		eUIE_LeaderboardEntryReturn,
		eUIE_GetFriendsCompleted,
		eUIE_UserStatRead,
	};

	SUIEventReceiverDispatcher<CUILobbyMP> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent> m_eventSender;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;
	std::vector<SDrxSessionSearchResult> m_FoundServers;
};


#endif // __UILOBBYMP_H_
