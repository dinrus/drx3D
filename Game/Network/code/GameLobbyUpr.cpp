// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/CoreX/Lobby/IDrxSignIn.h>
#include <drx3D/Game/GameLobbyUpr.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/GameLobby.h>

#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>

#include <drx3D/Act/IPlayerProfiles.h>

#include <drx3D/Game/MatchMakingHandler.h>

CGameLobbyUpr::CGameLobbyUpr() : REGISTER_GAME_MECHANISM(CGameLobbyUpr)
{
	m_primaryLobby = new CGameLobby(this);
	m_nextLobby = NULL;

	for (i32 i=0; i<MAX_LOCAL_USERS; ++i)
	{
		m_onlineState[i] = eOS_SignedOut;
	}

	m_multiplayer = false;
	
	m_pendingPrimarySessionDelete = false;
	m_pendingNextSessionDelete = false;
	
	m_isCableConnected = true;
	m_isChatRestricted = false;
	m_bMergingIsComplete = false;

	m_signOutTaskID= DrxLobbyInvalidTaskID;

	m_pMatchMakingHandler = new CMatchMakingHandler();

	IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
	pLobby->RegisterEventInterest(eCLSE_OnlineState, CGameLobbyUpr::OnlineCallback, this);
	pLobby->RegisterEventInterest(eCLSE_EthernetState, CGameLobbyUpr::EthernetStateCallback, this);
	pLobby->RegisterEventInterest(eCLSE_ChatRestricted, CGameLobbyUpr::ChatRestrictedCallback, this);
}

CGameLobbyUpr::~CGameLobbyUpr()
{
	IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
	pLobby->UnregisterEventInterest(eCLSE_OnlineState, CGameLobbyUpr::OnlineCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_EthernetState, CGameLobbyUpr::EthernetStateCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_ChatRestricted, CGameLobbyUpr::ChatRestrictedCallback, this);

	SAFE_DELETE(m_primaryLobby);
	SAFE_DELETE(m_nextLobby);
	SAFE_DELETE(m_pMatchMakingHandler);
}

CGameLobby* CGameLobbyUpr::GetGameLobby() const
{
	DRX_ASSERT(m_primaryLobby);
	return m_primaryLobby;
}

CGameLobby* CGameLobbyUpr::GetNextGameLobby() const
{
	return m_nextLobby;
}

bool CGameLobbyUpr::IsPrimarySession(CGameLobby *pLobby)
{
	return pLobby == m_primaryLobby;
}

bool CGameLobbyUpr::IsNewSession(CGameLobby *pLobby)
{
	return pLobby == m_nextLobby;
}

//when a session host wants to merge sessions it requests to join another one
bool CGameLobbyUpr::NewSessionRequest(CGameLobby* pLobby, DrxSessionID sessionId)
{
	if(m_nextLobby == NULL)
	{
		m_bMergingIsComplete = false;
		DRX_ASSERT(pLobby == m_primaryLobby);
		m_nextLobby = new CGameLobby(this);
		m_nextLobby->JoinServer(sessionId, "", DrxMatchMakingInvalidConnectionUID, false);
		m_nextLobby->SetMatchmakingGame(true);
		DrxLogAlways("CGameLobbyUpr::NewSessionRequest Success");
		return true;
	}

	DrxLogAlways("CGameLobbyUpr::NewSessionRequest Failed");
	return false;
}

//Next lobby joins (and reserves slots for everyone)
void CGameLobbyUpr::NewSessionResponse(CGameLobby* pLobby, DrxSessionID sessionId)
{
	DrxLogAlways("CGameLobbyUpr::NewSessionResponse %d", sessionId != DrxSessionInvalidID);

	DRX_ASSERT(pLobby == m_nextLobby);
	DRX_ASSERT(m_primaryLobby && m_nextLobby);

	if(sessionId != DrxSessionInvalidID)
	{
		m_bMergingIsComplete = true;
		CompleteMerge(sessionId);
	}
	else
	{
		m_nextLobby->LeaveSession(true, false);
	}
}

void CGameLobbyUpr::CompleteMerge(DrxSessionID sessionId)
{
	DrxLog("CGameLobbyUpr::CompleteMerge()");
	m_primaryLobby->FindGameMoveSession(sessionId);
}

//Hosted session doesn't want to merge anymore (received new players) so cancels the switch
void CGameLobbyUpr::CancelMoveSession(CGameLobby* pLobby)
{
	if (m_nextLobby)
	{
		DrxLogAlways("CGameLobbyUpr::CancelMoveSession");
		DRX_ASSERT(pLobby == m_primaryLobby);
		m_nextLobby->LeaveSession(true, false);
		m_bMergingIsComplete = false;
	}
}

//When a game lobby session deletes it tells the manager (this)
void CGameLobbyUpr::DeletedSession(CGameLobby* pLobby)
{
	DrxLog("CGameLobbyUpr::DeletedSession() pLobby:%p, primaryLobby:%p, nextLobby:%p", pLobby, m_primaryLobby, m_nextLobby);
	DRX_ASSERT(m_primaryLobby);
	DRX_ASSERT(pLobby == m_primaryLobby || pLobby == m_nextLobby);
	DRX_ASSERT(m_primaryLobby != m_nextLobby);

	// Can't delete the lobby now because we're in the middle of updating it, do the delete a bit later!
	if (pLobby == m_primaryLobby)
	{
		m_pendingPrimarySessionDelete = true;
	}
	else if (pLobby == m_nextLobby)
	{
		m_pendingNextSessionDelete = true;
	}
}

void CGameLobbyUpr::DoPendingDeleteSession(CGameLobby *pLobby)
{
	DrxLog("CGameLobbyUpr::DoPendingDeleteSession() pLobby:%p", pLobby);

	if(pLobby == m_primaryLobby)
	{
		if(m_nextLobby)
		{
			DRX_ASSERT(m_primaryLobby && m_nextLobby);

			SAFE_DELETE(m_primaryLobby);
			m_primaryLobby = m_nextLobby;
			m_nextLobby = NULL;

			m_primaryLobby->SwitchToPrimaryLobby();

			SetPrivateGame(m_primaryLobby, m_primaryLobby->IsPrivateGame());

			DrxLog("CGameLobbyUpr::DoPendingDeleteSession - Moved to next session");
		}
		else
		{
			DrxLog("CGameLobbyUpr::DoPendingDeleteSession - No sessions left");
#if 0 // old frontend
			CFlashFrontEnd *pFlashMenu = g_pGame->GetFlashMenu();
			if (pFlashMenu)
			{
				// Have to go all the way back to main then forward to play_online because the stack may
				// not include play_online (destroyed when we do a level rotation)
				if (IsMultiplayer() && pFlashMenu->IsScreenInStack("game_lobby"))
				{
					if (CMPMenuHub *pMPMenu = CMPMenuHub::GetMPMenuHub())
					{
						pMPMenu->GoToCurrentLobbyServiceScreen(); // go to correct lobby service screen - play_online or play_lan
					}
				}
			}
#endif 
			CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();
			if (pSquadUpr)
			{
				pSquadUpr->GameSessionIdChanged(CSquadUpr::eGSC_LeftSession, DrxSessionInvalidID);
			}

			SetPrivateGame(NULL, false);
		}
	}
	else if(pLobby == m_nextLobby)
	{
		DrxLog("CGameLobbyUpr::DoPendingDeleteSession - Next Lobby deleted");
		SAFE_DELETE(m_nextLobby);
	}

	m_bMergingIsComplete = false;
}

//Handles online state changes - when you sign out it returns you
void CGameLobbyUpr::OnlineCallback(UDrxLobbyEventData eventData, uk arg)
{
	if (g_pGameCVars->g_ProcessOnlineCallbacks == 0)
		return;

	if(eventData.pOnlineStateData)
	{
		CGameLobbyUpr *pLobbyUpr = static_cast<CGameLobbyUpr*>(arg);

#if defined(DEDICATED_SERVER)
		EOnlineState previousState = pLobbyUpr->m_onlineState[eventData.pOnlineStateData->m_user];
#endif

		DRX_ASSERT(eventData.pOnlineStateData->m_user < MAX_LOCAL_USERS);
		pLobbyUpr->m_onlineState[eventData.pOnlineStateData->m_user] = eventData.pOnlineStateData->m_curState;

		u32 userIndex = g_pGame->GetExclusiveControllerDeviceIndex();

		IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();

#ifndef _RELEASE
		tukk pEventType = "eOS_Unknown";
		if (eventData.pOnlineStateData->m_curState == eOS_SignedOut)
		{
			pEventType = "eOS_SignedOut";
		}
		else if (eventData.pOnlineStateData->m_curState == eOS_SigningIn)
		{
			pEventType = "eOS_SigningIn";
		}
		else if (eventData.pOnlineStateData->m_curState == eOS_SignedIn)
		{
			pEventType = "eOS_SignedIn";
		}
		DrxLog("[GameLobbyUpr] OnlineCallback: eventType=%s, user=%u, currentUser=%u", pEventType, eventData.pOnlineStateData->m_user, userIndex);

		if (g_pGameCVars->autotest_enabled && pLobby != NULL && (pLobby->GetLobbyServiceType() == eCLS_LAN))
		{
			// Don't care about signing out if we're in the autotester and in LAN mode
			return;
		}
#endif

		{
			EOnlineState onlineState = eventData.pOnlineStateData->m_curState;
			if(onlineState == eOS_SignedOut)
			{
				if(pLobby && pLobby->GetLobbyServiceType() == eCLS_Online)
				{
					if(eventData.pOnlineStateData->m_reason != eCLE_CyclingForInvite)
					{
						CErrorHandling *pErrorHandling = CErrorHandling::GetInstance();
						if (pErrorHandling)
						{
							pErrorHandling->OnFatalError(CErrorHandling::eFE_PlatformServiceSignedOut);
						}
						else
						{
							pLobbyUpr->LeaveGameSession(eLSR_SignedOut);
						}

						IPlayerProfileUpr *pPPM = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
						if(pPPM)
						{
							pPPM->ClearOnlineAttributes();
						}

#if 0 // old frontend
						CWarningsUpr *pWM = g_pGame->GetWarnings();
						if(pWM)
						{
							pWM->RemoveWarning("ChatRestricted");
						}
#endif 
					}


#if defined(DEDICATED_SERVER)
					if (previousState != eOS_SignedOut)
					{
						DrxLogAlways("We've been signed out, reason=%u, bailing", eventData.pOnlineStateData->m_reason);
						gEnv->pConsole->ExecuteString("quit", false, true);
					}
#endif
				}
			}					
		}
	}
}

void CGameLobbyUpr::EthernetStateCallback(UDrxLobbyEventData eventData, uk arg)
{
	if(eventData.pEthernetStateData)
	{
		DrxLog("[GameLobbyUpr] EthernetStateCallback state %d", eventData.pEthernetStateData->m_curState);

		CGameLobbyUpr *pGameLobbyUpr = (CGameLobbyUpr*)arg;
		DRX_ASSERT(pGameLobbyUpr);

		if(g_pGame->HasExclusiveControllerIndex())
		{
			ECableState newState = eventData.pEthernetStateData->m_curState;
#if 0 // old frontend
			if(pMPMenuHub)
			{
				pMPMenuHub->EthernetStateChanged(newState);
			}
#endif

			// cable has been removed, clear dialog
			if ((newState == eCS_Unplugged) || (newState == eCS_Disconnected))
			{
				CErrorHandling *pErrorHandling = CErrorHandling::GetInstance();
				if (pErrorHandling)
				{
					pErrorHandling->OnFatalError(CErrorHandling::eFE_EthernetCablePulled);
				}
#if 0 // old frontend
				CWarningsUpr *pWM = g_pGame->GetWarnings();
				if(pWM)
				{
					pWM->RemoveWarning("ChatRestricted");
				}
#endif
			}
		}

		pGameLobbyUpr->m_isCableConnected = (eventData.pEthernetStateData->m_curState == eCS_Connected) ? true : false;
	}
}

void CGameLobbyUpr::ChatRestrictedCallback(UDrxLobbyEventData eventData, uk arg)
{
	SDrxLobbyChatRestrictedData *pChatRestrictedData = eventData.pChatRestrictedData;
	if(pChatRestrictedData)
	{
		DrxLog("[GameLobbyUpr] ChatRestrictedCallback user %d isChatRestricted %d", pChatRestrictedData->m_user, pChatRestrictedData->m_chatRestricted);

		CGameLobbyUpr *pGameLobbyUpr = (CGameLobbyUpr*)arg;
		DRX_ASSERT(pGameLobbyUpr);
	
		u32 userIndex = g_pGame->GetExclusiveControllerDeviceIndex();

		if(pChatRestrictedData->m_user == userIndex)
		{
			pGameLobbyUpr->m_isChatRestricted = pChatRestrictedData->m_chatRestricted;
		}
	}
}

void CGameLobbyUpr::Update( float dt )
{
	if (m_primaryLobby)
	{
		m_primaryLobby->Update(dt);
	}
	if (m_nextLobby)
	{
		m_nextLobby->Update(dt);
	}

	m_pMatchMakingHandler->Update( dt );

	// Do these in reverse order because the next lobby become the primary lobby if the current primary is deleted
	if (m_pendingNextSessionDelete)
	{
		DoPendingDeleteSession(m_nextLobby);
		m_pendingNextSessionDelete = false;
	}
	if (m_pendingPrimarySessionDelete)
	{
		if (m_primaryLobby && m_primaryLobby->GetState() == eLS_None)
		{
			DoPendingDeleteSession(m_primaryLobby);
		}
		else
		{
			DrxLog("CGameLobbyUpr::Update() primary lobby deletion requested but it's no longer in the eLS_None state, aborting the delete");
		}
		m_pendingPrimarySessionDelete = false;
	}
}

//-------------------------------------------------------------------------
void CGameLobbyUpr::LeaveGameSession(ELeavingSessionReason reason)
{
	DrxLog("CGameLobbyUpr::LeaveGameSession() reason=%i multiplayer %d", (i32) reason, gEnv->bMultiplayer);
	// Tell the game lobby that we want to quit
	// Note: If we're leaving in a group (because the squad is quitting) then we need to do special behaviour:
	//					If we're the squad session host then we need to tell the rest of the squad to quit
	//					If we're the game session host then the rest of the squad has to quit before we do

	if(gEnv->bMultiplayer || reason == eLSR_SwitchGameType)
	{
		if (IsLobbyMerging())
		{
			CancelMoveSession(m_primaryLobby);
		}

		bool canLeaveNow = true;
		CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();

		ELobbyState lobbyState = m_primaryLobby->GetState();
		if (lobbyState != eLS_Game)
		{
			// If we're in a squad and we're the game session host then we need to be careful about leaving
			if (pSquadUpr != NULL && pSquadUpr->HaveSquadMates())
			{
				if (reason == eLSR_Menu)
				{
					// User requested quit
					//	- If we're not the squad session host then we're the only person leaving so we can do it straight away
					//	- If we're not the game session host then there are no ordering issues so again, we can just leave
					if (pSquadUpr->InCharge())
					{
						if (m_primaryLobby->IsServer())
						{
							DrxLog("  we're trying to leave but we're the squad and game host, need to leave last");
							pSquadUpr->TellMembersToLeaveGameSession();
							canLeaveNow = false;
						}
						else
						{
							DrxLog("  we're not the game host, can leave now");
							pSquadUpr->TellMembersToLeaveGameSession();
						}
					}
				}
				else if (reason == eLSR_ReceivedSquadLeavingFromSquadHost)
				{
					// Squad leader requested quit
					if (m_primaryLobby->IsServer())
					{
						canLeaveNow = false;
					}
				}
				else if (reason == eLSR_ReceivedSquadLeavingFromGameHost)
				{
					pSquadUpr->TellMembersToLeaveGameSession();
				}
			}
		}
		else
		{
			DrxLog("  leaving a game that is not in the lobby state, tell the squad manager");
			pSquadUpr->LeftGameSessionInProgress();
		}

		if (canLeaveNow)
		{
			m_primaryLobby->LeaveSession(true, (reason == eLSR_SwitchGameType) || (reason == eLSR_Menu));
		}
		else
		{
			m_primaryLobby->LeaveAfterSquadMembers();
		}
	}
}

//-------------------------------------------------------------------------
bool CGameLobbyUpr::HaveActiveLobby(bool includeLeaving/*=true*/) const
{
	if (m_nextLobby)
	{
		return true;
	}
	else if (m_primaryLobby)
	{
		if ((includeLeaving) || (m_primaryLobby->GetState() != eLS_Leaving))
		{
			if (m_primaryLobby->GetState() != eLS_None)
			{
				return true;
			}
		}
	}
	return false;
}

//-------------------------------------------------------------------------
void CGameLobbyUpr::MoveUsers(CGameLobby *pFromLobby)
{
	DrxLog("[GameLobbyUpr] MoveUsers pFromLobby %p pToLobby %p", pFromLobby, m_nextLobby);

	if(m_nextLobby)
	{
		m_nextLobby->MoveUsers(pFromLobby);
	}
}

//------------------------------------------------------------------------
void CGameLobbyUpr::SetMultiplayer(const bool multiplayer)
{
	m_multiplayer = multiplayer;
	if(!multiplayer)
	{
		SetPrivateGame(NULL, false);	//can't be a private game anymore
	}
}

//------------------------------------------------------------------------
void CGameLobbyUpr::AddPrivateGameListener(IPrivateGameListener* pListener)
{
	DRX_ASSERT(pListener);

	stl::push_back_unique(m_privateGameListeners, pListener);

	bool privateGame = m_primaryLobby ? m_primaryLobby->IsPrivateGame() : false;
	pListener->SetPrivateGame(privateGame);
}

//------------------------------------------------------------------------
void CGameLobbyUpr::RemovePrivateGameListener(IPrivateGameListener* pListener)
{
	stl::find_and_erase(m_privateGameListeners, pListener);
}

//------------------------------------------------------------------------
void CGameLobbyUpr::SetPrivateGame(CGameLobby *pLobby, const bool privateGame)
{
#ifndef _RELEASE
	if(privateGame)
	{
		DRX_ASSERT_MESSAGE(IsPrimarySession(pLobby), "PrivateGame logic is broken!");
	}
#endif

	if(pLobby == NULL || IsPrimarySession(pLobby))
	{
		if(!m_privateGameListeners.empty())
		{
			TPrivateGameListenerVec::iterator iter = m_privateGameListeners.begin();
			while (iter != m_privateGameListeners.end())
			{
				(*iter)->SetPrivateGame(privateGame);
				++iter;
			}
		}
	}
}

EDrxLobbyError CGameLobbyUpr::DoUserSignOut()
{
	DrxLog("CGameLobbyUpr::DoUserSignOut");
	EDrxLobbyError error = eCLE_Success;

	if(m_signOutTaskID == DrxLobbyInvalidTaskID)
	{
		IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
		IDrxLobbyService *pLobbyService = pLobby ? pLobby->GetLobbyService() : NULL;

		if(pLobbyService)
		{
			IDrxSignIn* pSignIn = pLobbyService->GetSignIn();

			if ( pSignIn )
			{
				error = pSignIn->SignOutUser( g_pGame->GetExclusiveControllerDeviceIndex(), &m_signOutTaskID, UserSignoutCallback, this );

				if(error == eCLE_Success)
				{
					// Notify UI? michiel
				}
			}
		}
	}
	else
	{
		DrxLog("  not starting signout task as we already have one in progress");
	}

	return error;
}

// static
void CGameLobbyUpr::UserSignoutCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, u32 user, uk pArg)
{
	DrxLog("UserSignoutCallback error %d", error);

	CGameLobbyUpr *pLobbyMgr =(CGameLobbyUpr*)pArg;

	if(pLobbyMgr && pLobbyMgr->m_signOutTaskID == taskID)
	{
		pLobbyMgr->m_signOutTaskID = DrxLobbyInvalidTaskID;
#if 0 // old frontend
		g_pGame->GetWarnings()->RemoveWarning("MyCrysisSigningOut");
#endif
	}
}
