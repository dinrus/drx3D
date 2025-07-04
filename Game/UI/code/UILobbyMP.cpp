// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UILobbyMP.cpp
//  Version:     v1.00
//  Created:     08/06/2012 by Michiel Meesters.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/UILobbyMP.h>

#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/Network/Lobby/GameBrowser.h>
#include <drx3D/Game/Network/Lobby/GameLobbyUpr.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/CoreX/Lobby/IDrxLobby.h>
#include <drx3D/CoreX/Lobby/IDrxReward.h>
#include <drx3D/CoreX/Lobby/IDrxStats.h>
#include <drx3D/CoreX/Lobby/IDrxFriends.h>
#include <drx3D/CoreX/Lobby/IDrxVoice.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>

static CUILobbyMP *pUILObbyMP = NULL;
static std::map<i32, std::vector<DrxUserID> > m_FriendsMap; // containerId -> friendList with userID's for invites

////////////////////////////////////////////////////////////////////////////
CUILobbyMP::CUILobbyMP()
	: m_pUIEvents(NULL)
	, m_pUIFunctions(NULL)
{
	pUILObbyMP = this;
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::InitEventSystem()
{
	if (!gEnv->pFlashUI)
		return;

	ICVar* pServerVar = gEnv->pConsole->GetCVar("cl_serveraddr");

	// events to send from this class to UI flowgraphs
	m_pUIFunctions = gEnv->pFlashUI->CreateEventSystem("LobbyMP", IUIEventSystem::eEST_SYSTEM_TO_UI);
	m_eventSender.Init(m_pUIFunctions);

	{
		SUIEventDesc evtDesc("ServerFound", "Triggered when server is found");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("ServerId", "ID of the server");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("ServerName", "Name of the server");
		m_eventSender.RegisterEvent<eUIE_ServerFound>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerList", "Triggered when GetPlayerList is fired");
		evtDesc.SetDynamic("PlayerList", "List of players");
		m_eventSender.RegisterEvent<eUIE_PlayerListReturn>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("PlayerIDList", "Triggered when GetPlayerList is fired");
		evtDesc.SetDynamic("PlayerIDs", "List of player ids");
		m_eventSender.RegisterEvent<eUIE_PlayerIdListReturn>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("InviteAccepted", "An invite to a game was accepted");
		m_eventSender.RegisterEvent<eUIE_InviteAccepted>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("SearchStarted", "A lobby search query was started");
		m_eventSender.RegisterEvent<eUIE_SearchStarted>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("SearchCompleted", "A lobby search query was completed");
		m_eventSender.RegisterEvent<eUIE_SearchCompleted>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("NatTypeUpdated", "Triggered when new NAT info is available (psn/xlive)");
		m_eventSender.RegisterEvent<eUIE_NatTypeUpdated>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("ShowLoadingDialog", "Requests to show loading dialog - when something takes longer then usual");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("LoadingDialog", "tag specifies which loading dialog");
		m_eventSender.RegisterEvent<eUIE_ShowLoadingDialog>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("HideLoadingDialog", "Requests to hide a loading dialog - when something took longer then usual");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("LoadingDialog", "tag specifies which loading dialog");
		m_eventSender.RegisterEvent<eUIE_HideLoadingDialog>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("LeaderboardEntry", "Triggered when ReadLeaderboard is fired");
		evtDesc.SetDynamic("LeaderboardEntries", "Extra custom columns");
		m_eventSender.RegisterEvent<eUIE_LeaderboardEntryReturn>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("GetFriendsCompleted", "Triggered when friends are found and stored in container");
		m_eventSender.RegisterEvent<eUIE_GetFriendsCompleted>(evtDesc);
	}

	{
		SUIEventDesc evtDesc("UserStatRead", "Triggered when a user stat is returned (from GetUserStats)");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("Name", "Name of the stat");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Any>("Val", "Value of the stat");
		m_eventSender.RegisterEvent<eUIE_UserStatRead>(evtDesc);
	}

	// events that can be sent from UI flowgraphs to this class
	m_pUIEvents = gEnv->pFlashUI->CreateEventSystem("LobbyMP", IUIEventSystem::eEST_UI_TO_SYSTEM);
	m_eventDispatcher.Init(m_pUIEvents, this, "CUILobbyMP");

	{
		SUIEventDesc evtDesc("HostGame", "Host a game, providing a map-path");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Bool>("LAN", "1: Lan, 0: Internet");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("MapPath", "path to desired map");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("Gamerules", "desired Gamerules to be used");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::HostGame);
	}


	{
		SUIEventDesc evtDesc("JoinGame", "Join a game");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("serverID", "provided from search servers");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::JoinGame);
	}

	{
		SUIEventDesc evtDesc("MutePlayer", "Mutes a Player");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("PlayerID", "the channel ID");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Bool>("shouldMute", "1=mute, 0 =unmute");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::MutePlayer);
	}

	{
		SUIEventDesc evtDesc("LeaveGame", "Leave game session");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::LeaveGame);
	}

	{
		SUIEventDesc evtDesc("SetMultiplayer", "Sets Multiplayer");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Bool>("isMultiplayer", "true: multiplayer - false: singleplayer");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::SetMultiplayer);
	}

	{
		SUIEventDesc evtDesc("LockController", "Locks or Unlocks the exclusive controller index");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Bool>("LockController", "true: lock - false: unlock");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::LockController);
	}

	{
		SUIEventDesc evtDesc("SearchGames", "Start quick-game with default settings");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Bool>("LAN", "1: LAN or 0: Internet");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::SearchGames);
	}

	{
		SUIEventDesc evtDesc("AwardTrophy", "Test award trophy");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("TrophyNr", "Index of trophy");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::AwardTrophy);
	}

	{
		SUIEventDesc evtDesc("GetPlayerlist", "Get list of all players in lobby");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::GetPlayerList);
	}


	{
		SUIEventDesc evtDesc("ReadLeaderboard", "Read from leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("LeaderboardIdx", "Index of leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("Mode", "0=Top ranked, 1=From User");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("NrOfEntries", "How many rows you want to read");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::ReadLeaderBoard);
	}

	{
		SUIEventDesc evtDesc("RegisterLeaderboard", "Register a leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("Name", "Name of leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("LeaderboardIdx", "Index of leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("NrOfColumns", "Nr of custom columns for this leaderboard");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::RegisterLeaderBoard);
	}

	{
		SUIEventDesc evtDesc("WriteLeaderboard", "Write current player's score to a leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("LeaderboardIdx", "Index of leaderboard");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_String>("Params", "Delimiter: |  - Score, Custom col 1, Custom col 2,...");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::WriteLeaderBoard);
	}


	{
		SUIEventDesc evtDesc("GetFriends", "Get online friends");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("Container", "Which container to fill up");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::GetFriends);
	}

	{
		SUIEventDesc evtDesc("InviteFriend", "Get online friends");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("Container", "Which container to pick from");
		evtDesc.AddParam<SUIParameterDesc::eUIPT_Int>("FriendIdx", "Index in the container");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::InviteFriends);
	}

	{
		SUIEventDesc evtDesc("GetUserStats", "Get online stats");
		m_eventDispatcher.RegisterEvent(evtDesc, &CUILobbyMP::GetUserStats);
	}

	gEnv->pFlashUI->RegisterModule(this, "CUILobbyMP");
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::UnloadEventSystem()
{
	if (gEnv->pFlashUI)
		gEnv->pFlashUI->UnregisterModule(this);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::Reset()
{
}

////////////////////////////////////////////////////////////////////////////
// ui events
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::ServerFound(SDrxSessionSearchResult session, string sServerName)
{
	if (gEnv->pFlashUI)
	{
		m_eventSender.SendEvent<eUIE_ServerFound>((i32)m_FoundServers.size(), sServerName);
		m_FoundServers.push_back(session);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::PlayerListReturn(const SUIArguments& players, const SUIArguments& playerids)
{
	if (gEnv->pFlashUI)
	{
		m_eventSender.SendEvent<eUIE_PlayerListReturn>(players);
		m_eventSender.SendEvent<eUIE_PlayerIdListReturn>(playerids);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::InviteAccepted()
{
	if (gEnv->pFlashUI)
		m_eventSender.SendEvent<eUIE_InviteAccepted>();
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::SearchStarted()
{
	if (gEnv->pFlashUI)
		m_eventSender.SendEvent<eUIE_SearchStarted>();
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::SearchCompleted()
{
	if (gEnv->pFlashUI)
		m_eventSender.SendEvent<eUIE_SearchCompleted>();
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::ShowLoadingDialog(const  tuk sLoadingDialog)
{
	if (gEnv->pFlashUI)
		m_eventSender.SendEvent<eUIE_ShowLoadingDialog>(sLoadingDialog);
};

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::HideLoadingDialog(const  tuk sLoadingDialog)
{
	if (gEnv->pFlashUI)
		m_eventSender.SendEvent<eUIE_HideLoadingDialog>(sLoadingDialog);
};

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::UpdateNatType()
{
	CGameBrowser *pGameBrowser = g_pGame->GetGameBrowser();
	if(pGameBrowser && 	gEnv->pFlashUI)
	{
		string natString = pGameBrowser->GetNatTypeString();
		m_eventSender.SendEvent<eUIE_NatTypeUpdated>(natString);
	}
}

////////////////////////////////////////////////////////////////////////////
// ui functions
////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void CUILobbyMP::JoinGame(u32 sessionID)
{
	bool result = false;

	CGameLobby *pGameLobby = g_pGame->GetGameLobby();
	CGameLobbyUpr *pGameLobbyMgr = g_pGame->GetGameLobbyUpr();
	CSquadUpr *pSquadMgr = g_pGame->GetSquadUpr();

	if(pGameLobbyMgr)
	{
		pGameLobbyMgr->SetMultiplayer(true);
	}

	if(pSquadMgr)
	{
		pSquadMgr->SetMultiplayer(true);
	}

	if(pGameLobby && m_FoundServers.size() > sessionID)
		result = pGameLobby->JoinServer(m_FoundServers[sessionID].m_id, m_FoundServers[sessionID].m_data.m_name, DrxMatchMakingInvalidConnectionUID, false);

	return;
}

//////////////////////////////////////////////////////////////////////////
void CUILobbyMP::HostGame(bool bLan, string sMapPath, string sGameRules)
{
	if (CGameLobbyUpr* pGameLobbyUpr = g_pGame->GetGameLobbyUpr())
	{
		pGameLobbyUpr->SetMultiplayer(true);
	}

	if (CSquadUpr* pSquadUpr = g_pGame->GetSquadUpr())
	{
		pSquadUpr->SetMultiplayer(true);
	}

	if (CGameLobby* pGameLobby = g_pGame->GetGameLobby())
	{
		CGameLobby::SetLobbyService(bLan ? eCLS_LAN : eCLS_Online);
				
		if(!g_pGame->GetIGameFramework()->GetIGameRulesSystem()->HaveGameRules(sGameRules))
		{
			 sGameRules = "DeathMatch";
		}

		pGameLobby->CreateSessionFromSettings(sGameRules, sMapPath);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::SearchGames(bool bLan)
{
	//Clear results
	m_FoundServers.clear();
	
	CGameBrowser *pGameBrowser = g_pGame->GetGameBrowser();
	if (pGameBrowser)
	{
		if(bLan)
			CGameLobby::SetLobbyService(eCLS_LAN);
		else
			CGameLobby::SetLobbyService(eCLS_Online);

		pGameBrowser->StartSearchingForServers(CUILobbyMP::MatchmakingSessionSearchCallback);
	}

}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::GetPlayerList()
{
	CGameLobby* pGameLobby = g_pGame->GetGameLobby();
	SSessionNames sessions = pGameLobby->GetSessionNames();
	SUIArguments players, ids;
	for (uint i = 0; i < sessions.Size(); i++)
	{
		SSessionNames::SSessionName &player = sessions.m_sessionNames[i];
		players.AddArgument( string(player.m_name) );
		ids.AddArgument(player.m_conId.m_uid);
	}
	
	PlayerListReturn(players, ids);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::MutePlayer(i32 playerId, bool mute)
{
	CGameLobby* pGameLobby = g_pGame->GetGameLobby();
	SSessionNames sessions = pGameLobby->GetSessionNames();

	SSessionNames::SSessionName player = sessions.m_sessionNames[playerId];

	if(gEnv->pNetwork->GetLobby() && gEnv->pNetwork->GetLobby()->GetVoice())
	{
		gEnv->pNetwork->GetLobby()->GetVoice()->MuteLocalPlayer(0, mute);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::LeaveGame()
{
	CGameLobby* pGameLobby = g_pGame->GetGameLobby();
	if (pGameLobby)
	{
    	pGameLobby->LeaveSession(true, true);
		pGameLobby->SetMatchmakingGame(false);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::MatchmakingSessionSearchCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxSessionSearchResult* session, uk arg)
{
	i32 breakhere = 0;
	if (error == eCLE_SuccessContinue || error == eCLE_Success)
	{
		if(session)
		{
			CGameBrowser *pGameBrowser = g_pGame->GetGameBrowser();
			CGameLobby *pGameLobby = g_pGame->GetGameLobby();
			pUILObbyMP->ServerFound(*session, session->m_data.m_name);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::AwardTrophy(i32 trophy)
{
	IDrxLobby* pLobby = gEnv->pNetwork->GetLobby();
	u32 user = g_pGame->GetExclusiveControllerDeviceIndex();
	if(user != IPlatformOS::Unknown_User)
	{
		u32 achievementId = trophy;

		//-- Award() only puts awards into a queue to be awarded.
		//-- This code here only asserts that the award was added to the queue successfully, not if the award was successfully unlocked.
		//-- Function has been modified for a DrxLobbyTaskId, callback and callback args parameters, similar to most other lobby functions,
		//-- to allow for callback to trigger when the award is successfully unlocked (eCLE_Success) or failed to unlock (eCLE_InternalError).
		//-- In the case of trying to unlock an award that has already been unlocked, the callback will return eCLE_Success.
		//-- Successful return value of the Award function has been changed from incorrect eCRE_Awarded to more sensible eCRE_Queued.
	
		//achievements are only for online lobbies
		if(pLobby && pLobby->GetLobbyService(eCLS_Online) && pLobby->GetLobbyService(eCLS_Online)->GetReward())
		{
			EDrxRewardError error = pLobby->GetLobbyService(eCLS_Online)->GetReward()->Award(user, achievementId, NULL, NULL, NULL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::RegisterLeaderBoard(string leaderboardName, i32 leaderBoardIdx, i32 nrOfColumns)
{
	IDrxLobby *Lobby = gEnv->pNetwork->GetLobby();
	IDrxLobbyService *Service = (Lobby) ? Lobby->GetLobbyService(eCLS_Online) : NULL;
	IDrxStats *pStats = (Service != NULL) ? Service->GetStats() : NULL;

	if(!pStats)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP::RegisterLeaderBoard: online Stats/Service not available");
		return;
	}
	
	EDrxLobbyError Error;

	SDrxStatsLeaderBoardUserColumn* columns = new SDrxStatsLeaderBoardUserColumn[nrOfColumns];
	for (i32 i = 0; i < nrOfColumns; i++)
	{
		columns[i].columnID = i;
		columns[i].data.m_type = eCLUDT_Int32;
	}

	SDrxStatsLeaderBoardWrite leaderboards;
	leaderboards.id = leaderBoardIdx;
	leaderboards.data.numColumns = nrOfColumns;
	leaderboards.data.pColumns = columns;

	pStats->RegisterLeaderboardNameIdPair(leaderboardName, leaderBoardIdx);
	Error = pStats->StatsRegisterLeaderBoards(&leaderboards, 1, NULL, CUILobbyMP::RegisterLeaderboardCB, NULL);

	delete [] columns;
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::GetFriends(i32 containerIdx)
{
	//Retrieve friendlist
	IDrxFriends* pFriends = gEnv->pNetwork->GetLobby()->GetFriends();
	u32 user = g_pGame->GetExclusiveControllerDeviceIndex();
	if(pFriends)
	{
		pFriends->FriendsGetFriendsList(user, 0, 10, NULL, CUILobbyMP::GetFriendsCB, (uk )((EXPAND_PTR)containerIdx));
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::GetFriendsCB(DrxLobbyTaskID taskID, EDrxLobbyError error, SFriendInfo* pFriendInfo, u32 numFriends, uk pArg)
{
	if(error == eCLE_Success)
	{
		IDrxFriends* pFriends = gEnv->pNetwork->GetLobby()->GetFriends();
		i32 containerIdx = (i32)((TRUNCATE_PTR)pArg);

		IFlowSystemContainerPtr pContainer = gEnv->pFlowSystem->GetContainer(containerIdx);
		if (pContainer)
		{
			std::vector<DrxUserID> userIDList;
			for (i32 i = 0; i < numFriends; i++)
			{
				// load up container
				TFlowInputData name;
				name.Set(string(pFriendInfo[i].name));
				pContainer->AddItem(name);
				userIDList.push_back(pFriendInfo[i].userID);
			}
			m_FriendsMap[containerIdx] = userIDList;
			
			pUILObbyMP->m_eventSender.SendEvent<eUIE_GetFriendsCompleted>();
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Provided container not found, no friends stored");
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::InviteFriends(i32 containerIdx, i32 friendIdx)
{
	u32 user = g_pGame->GetExclusiveControllerDeviceIndex();
	IDrxFriends* pFriends = gEnv->pNetwork->GetLobby()->GetFriends();
	IFlowSystemContainerPtr pContainer = gEnv->pFlowSystem->GetContainer(containerIdx);

	// assume they send the userId - should be a string
	string userId;
	if(pContainer)
		pContainer->GetItem(friendIdx).GetValueWithConversion(userId);

	DrxSessionHandle handle = g_pGame->GetGameLobby()->GetCurrentSessionHandle();
	EDrxLobbyError error = pFriends->FriendsSendGameInvite(user, handle, &(m_FriendsMap[containerIdx][friendIdx]), 1, NULL, CUILobbyMP::InviteFriendsCB, NULL);
	
	DrxLog("UILobbyMP: Sending friend invite to: %s, error: %d", userId.c_str(), error);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::InviteFriendsCB(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg)
{
	DrxLog("UILobbyMP: Sending friend invite callback error: %d", error);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::RegisterLeaderboardCB(DrxLobbyTaskID TaskID, EDrxLobbyError Error, uk Arg)
{
	if (Error > eCLE_SuccessContinue)
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP::RegisterLeaderboardCB: Failed to register, code: %i", Error);
}


////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::WriteLeaderBoard(i32 leaderboardIdx, tukk values)
{
	IDrxLobby *Lobby = gEnv->pNetwork->GetLobby();
	CGameLobby* pGameLobby = g_pGame->GetGameLobby();
	IDrxLobbyService *Service = (Lobby) ? Lobby->GetLobbyService(eCLS_Online) : NULL;
	IDrxStats *Stats = (Service != NULL) ? Service->GetStats() : NULL;

	if(!Stats)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP::WriteLeaderBoard: online Stats/Service not available");
		return;
	}
	
	EDrxLobbyError Error;

	SUIArguments args;
	args.AddArguments(values, false);
	i32 nrOfArgs = args.GetArgCount();

	if(nrOfArgs < 1)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP::WriteLeaderBoard: not enough arguments provided (minimum 1 required)");
		return;
	}

	SDrxStatsLeaderBoardUserColumn* columns = NULL;
	if(nrOfArgs > 1)
	{
		// skip score (first element) from the list
		columns = new SDrxStatsLeaderBoardUserColumn[nrOfArgs-1];
		for(i32 i = 0; i < nrOfArgs-1; i++)
		{
			string val;
			args.GetArg(i+1, val);
			columns[i].columnID = i;
			columns[i].data.m_int32 = atoi(val);
			columns[i].data.m_type = eCLUDT_Int32;	
		}
	}

	string scoreValue;
	args.GetArg(0, scoreValue);

	SDrxStatsLeaderBoardWrite leaderboard;
	leaderboard.id = leaderboardIdx;
	leaderboard.data.numColumns = nrOfArgs-1;
	leaderboard.data.pColumns = columns;
	leaderboard.data.score.score.m_int32 = atoi(scoreValue);
	leaderboard.data.score.score.m_type = eCLUDT_Int32;
	leaderboard.data.score.id = 0;

	Error = Stats->StatsWriteLeaderBoards(pGameLobby->GetCurrentSessionHandle(), 0, &leaderboard, 1, NULL, CUILobbyMP::WriteLeaderboardCallback, this);

	if(columns)
	{
		delete [] columns;
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::WriteLeaderboardCallback(DrxLobbyTaskID TaskID, EDrxLobbyError Error, uk Arg)
{
	if (Error > eCLE_SuccessContinue)
		GameWarning("CQuerySystem::WriteLeaderboardCallback: Failed to write, code: %i", Error);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::ReadLeaderBoard(i32 leaderboardIdx, i32 mode, i32 nrOfEntries)
{
	IDrxLobby *Lobby = gEnv->pNetwork->GetLobby();
	IDrxLobbyService *Service = (Lobby) ? Lobby->GetLobbyService(eCLS_Online) : NULL;
	IDrxStats *Stats = (Service != NULL) ? Service->GetStats() : NULL;

	if(!Service || !Stats)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP::ReadLeaderBoard: online Stats/Service not available");
		return;
	}

	const DrxStatsLeaderBoardID id = leaderboardIdx;
	EDrxLobbyError Error;	

	switch(mode)
	{
	case 0:
		Error = Stats->StatsReadLeaderBoardByRankForUser(id, 1, 12, NULL, CUILobbyMP::ReadLeaderBoardCB, NULL);
		break;
	case 1:
		Error = Stats->StatsReadLeaderBoardByRankForRange(id, 1, 10, NULL, CUILobbyMP::ReadLeaderBoardCB, NULL);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::ReadLeaderBoardCB(DrxLobbyTaskID TaskID, EDrxLobbyError Error, SDrxStatsLeaderBoardReadResult *Result, uk Arg)
{
	if(Error != eCLE_Success)
	{
		DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP: Read leaderboard failed.");
		return;
	}

	for (i32 i = 0; i < Result->numRows; i++)
	{
		SUIArguments customColumns;
		i32 score	= 0;

		switch(Result->pRows[i].data.score.score.m_type)
		{
			case eCLUDT_Int64:
				{
					score = (i32)Result->pRows[i].data.score.score.m_int64;
				}
				break;
			case eCLUDT_Int32:
				{
					score = Result->pRows[i].data.score.score.m_int32;
				}
				break;
			default:
				{
					DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP: Leaderboard read fail, column type not supported (only i32/int64)");
				}
		}

		customColumns.AddArgument(string(Result->pRows[i].name));
		customColumns.AddArgument(score);
		
		for (i32 j = 0; j < Result->pRows[i].data.numColumns; j++)
		{
			i32 columnVal = 0;

			switch(Result->pRows[i].data.pColumns[j].data.m_type)
			{
				case eCLUDT_Int64:
					{
						columnVal = (i32)Result->pRows[i].data.pColumns[j].data.m_int64;
					}
					break;
				case eCLUDT_Int32:
					{
						columnVal = Result->pRows[i].data.pColumns[j].data.m_int32;
					}
					break;
				default:
					{
						DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CUILobbyMP: Leaderboard read fail, column type not supported (only i32/int64)");
					}
			}
			
			customColumns.AddArgument(columnVal);
		}

		pUILObbyMP->LeaderboardEntryReturn(customColumns);
	}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::LeaderboardEntryReturn(const SUIArguments& customColumns)
{
	m_eventSender.SendEvent<eUIE_LeaderboardEntryReturn>(customColumns);
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::SetMultiplayer( bool bIsMultiplayer )
{
	CGameLobbyUpr *pGameLobbyMgr = g_pGame->GetGameLobbyUpr();
	CSquadUpr *pSquadMgr = g_pGame->GetSquadUpr();

	if(pGameLobbyMgr)
	{
		pGameLobbyMgr->SetMultiplayer(bIsMultiplayer);
	}

	if(pSquadMgr)
	{
		pSquadMgr->SetMultiplayer(bIsMultiplayer);
	}

	gEnv->bMultiplayer = bIsMultiplayer;
	g_pGameCVars->g_multiplayerDefault = bIsMultiplayer;
	g_pGame->InitGameType(bIsMultiplayer);
}

void CUILobbyMP::GetUserStats()
{
	u32 user = g_pGame->GetExclusiveControllerDeviceIndex();
	if(gEnv->pLobby->GetLobbyService(eCLS_Online)->GetStats())
		gEnv->pLobby->GetLobbyService(eCLS_Online)->GetStats()->StatsReadUserData(user, NULL, CUILobbyMP::ReadUserDataCallback, this);
}

void CUILobbyMP::SendUserStat(SUIArguments& arg)
{
	m_eventSender.SendEvent<eUIE_UserStatRead>(arg);
}

void CUILobbyMP::ReadUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, SDrxLobbyUserData* pData, u32 numData, uk pArg)
{
	// 3 - ignore the checksum and version number of the stats (these are added for the profile manager)
	for (i32 i = 3; i < numData; i++)
	{
			SUIArguments arg;
			arg.AddArgument( pData[i].m_id);

			switch(pData[i].m_type)
			{
			case eCLUDT_Float32:
				arg.AddArgument(pData[i].m_f32);
				break;
			case eCLUDT_Int32:
				arg.AddArgument(pData[i].m_int32);
				break;
			}

			CUILobbyMP* pInstance = (CUILobbyMP*)(pArg);
			pInstance->SendUserStat(arg);
		}
}

////////////////////////////////////////////////////////////////////////////
void CUILobbyMP::LockController( bool bLock )
{
	if(bLock)
		g_pGame->SetExclusiveControllerFromPreviousInput();
	else
		g_pGame->RemoveExclusiveController();
}

////////////////////////////////////////////////////////////////////////////
REGISTER_UI_EVENTSYSTEM( CUILobbyMP );
