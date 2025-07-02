// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Session handler for squads (similar to parties from other popular online shooters)

-------------------------------------------------------------------------
История:
- 05:03:2010 : Created By Ben Parbury

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/SquadUpr.h>

#include <drx3D/CoreX/TypeInfo_impl.h>

#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/Network/Lobby/GameLobbyUpr.h>
#include <drx3D/Game/Network/GameNetworkUtils.h>
#include <IPlayerProfiles.h>
#include <drx3D/Game/PlayerProgression.h>
#include <drx3D/Game/PlaylistUpr.h>
#include <drx3D/Game/UI/WarningsUpr.h>

#define SQUADMGR_CREATE_SQUAD_RETRY_TIMER		10.f

static i32 sm_enable = 0;
static i32 sm_debug = 0;
static float sm_inviteJoinTimeout = 2.f;

//---------------------------------------
CSquadUpr::CSquadUpr() : REGISTER_GAME_MECHANISM(CSquadUpr)
{
	m_squadHandle = DrxSessionInvalidHandle;
	m_currentGameSessionId = DrxSessionInvalidID;
	m_requestedGameSessionId = DrxSessionInvalidID;
	m_inviteSessionId = DrxSessionInvalidID;
	m_nameList.Clear();
	m_squadLeader = false;
	m_isNewSquad = false;
	m_squadLeaderId = DrxUserInvalidID;
	m_pendingKickUserId = DrxUserInvalidID;
	m_leaderLobbyState = eLS_None;
	m_currentTaskId = DrxLobbyInvalidTaskID;
	m_bMultiplayerGame = false;
	m_pendingInvite = false;
	m_bSessionStarted = false;
	m_bGameSessionStarted = false;
	m_slotType = eSST_Public;
	m_requestedSlotType = eSST_Public;
	m_inProgressSlotType = eSST_Public;
	m_sessionIsInvalid = false;
	m_leavingSession = false;
	m_inviteJoiningTime = 0.f;

	IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
	pLobby->RegisterEventInterest(eCLSE_UserPacket, CSquadUpr::UserPacketCallback, this);
	pLobby->RegisterEventInterest(eCLSE_OnlineState, CSquadUpr::OnlineCallback, this);
	pLobby->RegisterEventInterest(eCLSE_SessionUserJoin, CSquadUpr::JoinUserCallback, this);
	pLobby->RegisterEventInterest(eCLSE_SessionUserLeave, CSquadUpr::LeaveUserCallback, this);
	pLobby->RegisterEventInterest(eCLSE_SessionUserUpdate, CSquadUpr::UpdateUserCallback, this);
	pLobby->RegisterEventInterest(eCLSE_SessionClosed, CSquadUpr::SessionClosedCallback, this);
	pLobby->RegisterEventInterest(eCLSE_ForcedFromRoom, CSquadUpr::ForcedFromRoomCallback, this);

	gEnv->pNetwork->AddHostMigrationEventListener(this, "SquadUpr", ELPT_PostEngine);

#if !defined(_RELEASE)
	if (gEnv->pConsole)
	{
		REGISTER_CVAR(sm_enable, sm_enable, 0, "Enables and disables squad");
		REGISTER_CVAR(sm_debug, 0, 0, "Enable squad manager debug watches and logs");
		REGISTER_CVAR(sm_inviteJoinTimeout, sm_inviteJoinTimeout, VF_CHEAT, "Time to wait for squadmates to leave before following an invite");
		gEnv->pConsole->AddCommand("sm_create", CmdCreate, 0, "Create a squad session");
		gEnv->pConsole->AddCommand("sm_leave", CmdLeave, 0, "Leave a squad session");
		gEnv->pConsole->AddCommand("sm_kick", CmdKick, 0, "Kick a player from the squad");
	}
#endif
	if(gEnv->IsDedicated())
	{
		sm_enable = 0;
	}

	m_taskQueue.Init(TaskStartedCallback, this);
}

//---------------------------------------
void CSquadUpr::RegisterSquadEventListener(ISquadEventListener *pListener)
{
	stl::push_back_unique(m_squadEventListeners, pListener);
}

//---------------------------------------
void CSquadUpr::UnregisterSquadEventListener(ISquadEventListener *pListener)
{
	stl::find_and_erase(m_squadEventListeners, pListener);
}

//---------------------------------------
void CSquadUpr::EventListenerSquaddieAdded(DrxUserID userId)
{
	const size_t numListeners = m_squadEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_squadEventListeners[i]->AddedSquaddie(userId);
	}
}

//---------------------------------------
void CSquadUpr::EventListenerSquaddieRemoved(DrxUserID userId)
{
	const size_t numListeners = m_squadEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_squadEventListeners[i]->RemovedSquaddie(userId);
	}
}

//---------------------------------------
void CSquadUpr::EventListenerSquaddieUpdated(DrxUserID userId)
{
	const size_t numListeners = m_squadEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_squadEventListeners[i]->UpdatedSquaddie(userId);
	}
}

//---------------------------------------
void CSquadUpr::EventListenerSquadLeaderChanged(DrxUserID userId)
{
	g_pGame->GetGameLobby()->OnSquadChanged();

	const size_t numListeners = m_squadEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_squadEventListeners[i]->SquadLeaderChanged(userId);
	}
}

//---------------------------------------
void CSquadUpr::EventListenerSquadEvent(ISquadEventListener::ESquadEventType eventType)
{
	const size_t numListeners = m_squadEventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_squadEventListeners[i]->SquadEvent(eventType);
	}
}

//---------------------------------------
void CSquadUpr::Enable(bool enable, bool allowCreate)
{
	if ( enable != ( !sm_enable ) )
	{
		return;
	}

	sm_enable = enable;

	DrxLog("CSquadUpr::Enable() enable='%s'", enable ? "true" : "false");
	if (enable)
	{
		if(!m_pendingInvite)
		{
			if(allowCreate)
			{
				if (m_bMultiplayerGame)
				{
					if(m_taskQueue.GetCurrentTask() != CLobbyTaskQueue::eST_Create)	
					{
						if (m_squadHandle == DrxSessionInvalidHandle)
						{
							IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
							if (pLobby)
							{
								const bool allowJoinMultipleSessions = pLobby->GetLobbyServiceFlag(eCLS_Online, eCLSF_AllowMultipleSessions);
								if (allowJoinMultipleSessions)	// we could 
								{
									m_taskQueue.AddTask(CLobbyTaskQueue::eST_Create, false);
								}
								else
								{
									DrxLog("    online lobby service doesn't support multiple sessions");
								} // allowJoinMultipleSessions
							} // pLobby
						}
						else
						{
							DrxLog("    squad already created");
						} // m_squadHandle
					} // task is not create squad
					else
					{
						DrxLog("   squad create already in progress");
					}
				} // m_bMultiplayerGame
			} // allowCreate
		} // pendingInvite
	}
	else
	{
		DeleteSession();
	}
}

//---------------------------------------
CSquadUpr::~CSquadUpr()
{
	if (gEnv->pConsole)
	{
#if !defined(_RELEASE)
		gEnv->pConsole->UnregisterVariable("sm_enable", true);
		gEnv->pConsole->UnregisterVariable("sm_create", true);
		gEnv->pConsole->UnregisterVariable("sm_leave", true);
#endif
	}

	IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
	pLobby->UnregisterEventInterest(eCLSE_UserPacket, CSquadUpr::UserPacketCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_OnlineState, CSquadUpr::OnlineCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_SessionUserJoin, CSquadUpr::JoinUserCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_SessionUserLeave, CSquadUpr::LeaveUserCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_SessionUserUpdate, CSquadUpr::UpdateUserCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_SessionClosed, CSquadUpr::SessionClosedCallback, this);
	pLobby->UnregisterEventInterest(eCLSE_ForcedFromRoom, CSquadUpr::ForcedFromRoomCallback, this);
	
	gEnv->pNetwork->RemoveHostMigrationEventListener(this);
}

//---------------------------------------
void CSquadUpr::Update(float dt)
{
	if (sm_enable)
	{
#ifndef _RELEASE
		if (sm_debug > 0)
		{
			if (InSquad())
			{
				//if(HaveSquadMates())
				{
					DrxWatch("Squad");
					i32k nameSize = m_nameList.Size();
					for(i32 i = 0; i < nameSize; i++)
					{
						DrxWatch("\t%d - %s", i + 1, m_nameList.m_sessionNames[i].m_name);
					}
				}
			}
		}
#endif

		SPendingGameJoin *pPendingGameJoin = &m_pendingGameJoin;
		if(pPendingGameJoin->IsValid())
		{
			IGameFramework *pFramework = g_pGame->GetIGameFramework();
			//CFlashFrontEnd *pFFE = g_pGame->GetFlashMenu();
			if(!pFramework->StartingGameContext())
			{
				DrxLog("[squad]  calling delayed SquadJoinGame");
				SquadJoinGame(pPendingGameJoin->m_sessionID, pPendingGameJoin->m_isMatchmakingGame, pPendingGameJoin->m_playlistID, pPendingGameJoin->m_restrictRank, pPendingGameJoin->m_requireRank);
				pPendingGameJoin->Invalidate();
			}
		}

		if (m_inviteSessionId && (m_inviteJoiningTime > 0.f))
		{
			m_inviteJoiningTime -= dt;
			if (m_inviteJoiningTime <= 0.f)
			{
				JoinInvite();
			}
		}
	}

	// Have to do this block regardless of sm_enable since we could be trying to delete the squad as a result
	// of being disabled
	m_taskQueue.Update();
}

//---------------------------------------
bool CSquadUpr::IsSquadMateByUserId(DrxUserID userId)
{
	bool result = false;

	if (InSquad() && userId.IsValid() && gEnv->pNetwork && gEnv->pNetwork->GetLobby())
	{
		SSessionNames *pNamesList = &m_nameList;
		i32k numPlayers = pNamesList->Size();
		for (i32 i = 0; i < numPlayers; ++ i)
		{
			SSessionNames::SSessionName *pPlayer = &pNamesList->m_sessionNames[i];
			if (pPlayer->m_userId == userId)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

//---------------------------------------
void CSquadUpr::GameSessionIdChanged(EGameSessionChange eventType, DrxSessionID gameSessionId)
{
	DrxLog("CSquadUpr::GameSessionIdChanged() eventType=%i", (i32) eventType);

	m_currentGameSessionId = gameSessionId;

	if (!HaveSquadMates())
	{
		// If we're not in an active squad (one with more than just us in) then we don't care what the game session does
		DrxLog("  not in an active squad");
		return;
	}

	switch (eventType)
	{
	case eGSC_JoinedNewSession:
		{
			if (InCharge())
			{
				// Reserve slots
				DrxLog("  joined a new session and we're in charge of the squad, making reservations");
				g_pGame->GetGameLobby()->MakeReservations(&m_nameList, true);
			}
		}
		break;
	case eGSC_LeftSession:
		{
			if ((!InCharge()) && (GameNetworkUtils::CompareDrxSessionId(m_requestedGameSessionId, gameSessionId) == false))
			{
				DrxLog("  left game session when we weren't expecting to, leaving the squad");
				DeleteSession();
				m_requestedGameSessionId = DrxSessionInvalidID;
			}
		}
		break;
	case eGSC_LobbyMerged:
		{
			// TODO: Tell anyone who didn't make it into the original session that they need to use a different DrxSessionID
		}
		break;
	case eGSC_LobbyMigrated:
		{
			// TODO: Tell anyone not in this game session that they need to use a different DrxSessionID
		}
		break;
	}
}

//---------------------------------------
void CSquadUpr::ReservationsFinished(EReservationResult result)
{
	DrxLog("CSquadUpr::ReservationsFinished() result=%i", result);

	if (result == eRR_Success)
	{
		if(SquadsSupported())
		{
			DrxLog("  reservations for squad successful, sending eGUPD_SquadJoinGame packet!");
			SendSquadPacket(eGUPD_SquadJoinGame);
		}
		else
		{
			DrxLog("  host is in a game that does not support squads, sending eGUPD_SquadNotSupported!");
			SendSquadPacket(eGUPD_SquadNotSupported);
		}
	}
	else
	{
		DrxLog("  reservations for squad failed, deleting session");
		HandleCustomError("SquadUprError", "@ui_menu_squad_error_not_enough_room", true, true);
	}
}

//---------------------------------------
void CSquadUpr::SetSquadHandle(DrxSessionHandle handle)
{
	DrxLog("[tlh] CSquadUpr::SetSquadHandle(DrxSessionHandle handle = %d) previousHandle=%d", (i32)handle, i32(m_squadHandle));
	m_squadHandle = handle;

	m_taskQueue.AddTask(CLobbyTaskQueue::eST_SetLocalUserData, false);
}

//---------------------------------------
void CSquadUpr::LocalUserDataUpdated()
{
	if (InSquad())
	{
		m_taskQueue.AddTask(CLobbyTaskQueue::eST_SetLocalUserData, false);
	}
}

//---------------------------------------
EDrxLobbyError CSquadUpr::DoUpdateLocalUserData(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	DrxLog("CSquadUpr::DoUpdateLocalUserData()");
	DRX_ASSERT(m_taskQueue.GetCurrentTask() == CLobbyTaskQueue::eST_SetLocalUserData);

	u8 localUserData[DRXLOBBY_USER_DATA_SIZE_IN_BYTES] = {0};
	CGameLobby::SetLocalUserData(localUserData);

	u32 userIndex = g_pGame->GetExclusiveControllerDeviceIndex();

	EDrxLobbyError result = pMatchMaking->SessionSetLocalUserData(m_squadHandle, &taskId, userIndex, localUserData, sizeof(localUserData), CSquadUpr::UpdateLocalUserDataCallback, this);
	return result;
}

//static---------------------------------------
void CSquadUpr::UpdateLocalUserDataCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, uk arg )
{
	DrxLog("CSquadUpr::UpdateLocalUserDataCallback(DrxLobbyTaskID taskID, EDrxLobbyError error = %d, uk arg)", error);

	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(arg);
	DRX_ASSERT(pSquadUpr);

	pSquadUpr->CallbackReceived(taskID, error);
}

//---------------------------------------
void CSquadUpr::JoinGameSession(DrxSessionID gameSessionId, bool bIsMatchmakingSession)
{
	DrxLog("CSquadUpr::JoinGameSession(DrxSessionID gameSessionId = %p)", gameSessionId.get());
	SSessionNames &playerList = m_nameList;
	if (playerList.Size() > 0)
	{
		SSessionNames::SSessionName &localPlayer = playerList.m_sessionNames[0];
		// Reservation will have been made using the squad session UID
		CGameLobby *pGameLobby = g_pGame->GetGameLobby();
		pGameLobby->SetMatchmakingGame(bIsMatchmakingSession);
		pGameLobby->JoinServer(gameSessionId, "Squad Leader", localPlayer.m_conId, false);

		if( bIsMatchmakingSession )
		{
			pGameLobby->InitGameMatchmaking();
		}
	}
}

//---------------------------------------
EDrxLobbyError CSquadUpr::DoCreateSquad(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	DrxLog("CSquadUpr::CreateSquad()");

	EDrxLobbyError result = eCLE_ServiceNotSupported;

	SDrxSessionData session;
	session.m_data = NULL;
	session.m_numData = 0;
	session.m_numPublicSlots = SQUADMGR_MAX_SQUAD_SIZE;
	session.m_numPrivateSlots = 0;
	session.m_ranked = false;
	
	m_slotType = eSST_Public;
	m_requestedSlotType = eSST_Public;
	m_inProgressSlotType = eSST_Public;

	IPlayerProfileUpr *pPlayerProfileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if (pPlayerProfileUpr)
	{
		drx_strcpy(session.m_name, pPlayerProfileUpr->GetCurrentUser());
		u32 userIndex = g_pGame->GetExclusiveControllerDeviceIndex(); //pad number
		result = pMatchMaking->SessionCreate(&userIndex, 1, DRXSESSION_CREATE_FLAG_INVITABLE | DRXSESSION_CREATE_FLAG_MIGRATABLE, &session, &taskId, CSquadUpr::CreateCallback, this);
	}
	else
	{
		result = eCLE_InvalidUser;
	}

	return result;
}

//static---------------------------------------
void CSquadUpr::CreateCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle handle, uk arg)
{
	DrxLog("CSquadUpr::CreateCallback(DrxLobbyTaskID taskID, EDrxLobbyError error = %d, DrxSessionHandle handle = %d, uk arg)", (i32)error, (i32)handle);

	CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
	DRX_ASSERT(pSquadUpr);

	if (pSquadUpr->CallbackReceived(taskID, error))
	{
		pSquadUpr->SetSquadHandle(handle);
		pSquadUpr->m_squadLeader = true;
		pSquadUpr->m_isNewSquad = true;
		DrxLog("CSquadUpr session created %d", (i32)handle);

		pSquadUpr->m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionSetLocalFlags, false);

		pSquadUpr->OnSquadLeaderChanged();
		pSquadUpr->EventListenerSquadEvent(ISquadEventListener::eSET_CreatedSquad);

		if (pSquadUpr->m_bGameSessionStarted)
		{
			DrxLog("  game session has already started, starting a SessionStart task");
			pSquadUpr->m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionStart, false);
		}		

		// we no longer delete sessions if they are in the process of being created
		// joined, so we need to clean up after them when the task has completed
		if(!sm_enable)
		{
			DrxLog("   deleting created session. squad manager is no longer enabled");

			// this will clear out the task queue for us
			pSquadUpr->DeleteSession();
		}
	}
	else if(error != eCLE_TimeOut)
	{
		if(gEnv->bMultiplayer)
		{
			gEnv->pGame->AddGameWarning("SquadUprError", "Cannot create squad");
		}
	}
}

//---------------------------------------
EDrxLobbyError CSquadUpr::DoJoinSquad(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	DrxLog("CSquadUpr::JoinSquad(DrxSessionID squadSessionId = %p)", m_inviteSessionId.get());

	u32 userIndex = g_pGame->GetExclusiveControllerDeviceIndex(); //pad number
	EDrxLobbyError result = pMatchMaking->SessionJoin(&userIndex, 1, DRXSESSION_CREATE_FLAG_INVITABLE, m_inviteSessionId, &taskId, CSquadUpr::JoinCallback, this);

	if ((result != eCLE_Success) && (result != eCLE_TooManyTasks))
	{
		JoinSessionFinished(taskId, result, DrxSessionInvalidHandle);
	}

	return result;
}

//---------------------------------------
void CSquadUpr::JoinSessionFinished(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle hdl)
{
	if (CallbackReceived(taskID, error))
	{
		SetSquadHandle(hdl);

		m_squadLeader = false;
		m_isNewSquad = true;

		DrxLog("CSquadUpr session joined %d", (i32)hdl);
		SendSquadPacket(eGUPD_SquadJoin);

		EventListenerSquadEvent(ISquadEventListener::eSET_JoinedSquad);

		m_inviteSessionId = DrxSessionInvalidID;
			
		if(!sm_enable || m_sessionIsInvalid)
		{
			DrxLog("   deleting joined session sm_enable %d sessionIsInvalid %d", sm_enable, m_sessionIsInvalid);
			m_sessionIsInvalid = false;
			DeleteSession();
		}
	}
	else if (error != eCLE_TimeOut)
	{
		// If we timed out then the task will be restarted, if we failed for any other reason we need to go back
		// to our own squad
		m_inviteSessionId = DrxSessionInvalidID;
		m_taskQueue.AddTask(CLobbyTaskQueue::eST_Create, false);
		ReportError(error);
	}
}

//static---------------------------------------
void CSquadUpr::JoinCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle handle, u32 ip, u16 port, uk arg)
{
	DrxLog("CSquadUpr::JoinCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle handle = %d, u32 ip, u16 port, uk arg)", (i32)handle);

	CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
	DRX_ASSERT(pSquadUpr);

	pSquadUpr->m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionSetLocalFlags, false);

	pSquadUpr->JoinSessionFinished(taskID, error, handle);
}

//static---------------------------------------
void CSquadUpr::SessionChangeSlotTypeCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg)
{
	DrxLog("CSquadUpr::SessionUpdateSlotCallback(DrxLobbyTaskID %d EDrxLobbyError %d)", taskID, error);

	CSquadUpr *pSquadUpr = (CSquadUpr*)pArg;
	pSquadUpr->SessionChangeSlotTypeFinished(taskID, error);	
}

//---------------------------------------
EDrxLobbyError CSquadUpr::DoLeaveSquad(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	DrxLog("CSquadUpr::LeaveSquad()");

	EDrxLobbyError result = pMatchMaking->SessionDelete(m_squadHandle, &taskId, CSquadUpr::DeleteCallback, this);

	m_pendingGameJoin.Invalidate();

	if ((result != eCLE_Success) && (result != eCLE_TooManyTasks))
	{
		SquadSessionDeleted(m_currentTaskId, result);
	}
	return result;
}

//---------------------------------------
void CSquadUpr::CleanUpSession()
{
	EventListenerSquadEvent(ISquadEventListener::eSET_LeftSquad);

	// finish off the deletion...
	m_squadHandle = DrxSessionInvalidHandle;

	m_squadLeaderId = DrxUserInvalidID;
	m_squadLeader = false;

	m_requestedGameSessionId = DrxSessionInvalidID;
	m_bSessionStarted = false;

	DRX_ASSERT_MESSAGE(m_nameList.Size() <= 1, "[tlh] SANITY FAIL! i thought all remote connections should've been removed by the LeaveUserCallback by this point, so it should only be me in this list...? need rethink.");
	m_nameList.Clear();

	// Make sure we tell the GameLobby that we've left our current squad (need to do this after ensuring that the names list is empty
	OnSquadLeaderChanged();
}

//----------------------------------------
void CSquadUpr::SquadSessionDeleted(DrxLobbyTaskID taskID, EDrxLobbyError error)
{
	CallbackReceived(taskID, error);

	if (error != eCLE_TimeOut)
	{
		m_leavingSession = false;

		CleanUpSession();		// If we didn't timeout, cleanup (if something went wrong then we need to start from scratch)
		
		if (sm_enable)
		{
		  	if(!m_pendingInvite)
			{
					// ...now join the next session (if there's one set)
					if (m_inviteSessionId != DrxSessionInvalidID)
					{
							m_taskQueue.AddTask(CLobbyTaskQueue::eST_Join, false);
					}
					else
					{
							// ...else need to recreate our own squad session - shouldn't ever be not in a squad
							m_taskQueue.AddTask(CLobbyTaskQueue::eST_Create, false);
					}
			}
		}
	}
}

//static---------------------------------------
void CSquadUpr::DeleteCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk arg)
{

	DrxLog("CSquadUpr::DeleteCallback(DrxLobbyTaskID taskID, EDrxLobbyError error = %d, uk arg)", error);

	CSquadUpr*  pSquadUpr = (CSquadUpr*) arg;
	DRX_ASSERT(pSquadUpr);

	pSquadUpr->SquadSessionDeleted(taskID, error);

}

//---------------------------------------
void CSquadUpr::InviteAccepted(DrxSessionID id)
{
	DrxLog("[Invite] SquadUpr::InviteAccepted id %p", id.get());

	CLobbyTaskQueue::ESessionTask currentTask = m_taskQueue.GetCurrentTask();

	m_pendingInvite = false;
	m_inviteSessionId = id;

	if ((m_squadHandle == DrxSessionInvalidHandle) && (!m_taskQueue.HasTaskInProgress()))
	{
		m_taskQueue.Reset();
		m_taskQueue.AddTask(CLobbyTaskQueue::eST_Join, false);
	}
	else if(currentTask == CLobbyTaskQueue::eST_Create || currentTask == CLobbyTaskQueue::eST_Join)
	{
		DrxLog("[invite] currently %s a session , adding delete task for when done", (currentTask == CLobbyTaskQueue::eST_Create) ? "creating" : "joining");
		m_taskQueue.AddTask(CLobbyTaskQueue::eST_Delete, false);
		m_taskQueue.AddTask(CLobbyTaskQueue::eST_Join, false);
	}
	else
	{
		i32k numMembers = m_nameList.Size();
		if (m_squadLeader && (numMembers > 1))
		{
			SendSquadPacket(eGUPD_SquadMerge, DrxMatchMakingInvalidConnectionUID);
			m_inviteJoiningTime = sm_inviteJoinTimeout;
		}
		else
		{
			JoinInvite();
		}
	}
}

//---------------------------------------
void CSquadUpr::JoinInvite()
{
	m_inviteJoiningTime = 0.f;
	DeleteSession();
	m_taskQueue.AddTask(CLobbyTaskQueue::eST_Join, false);
}

//static---------------------------------------
void CSquadUpr::JoinUserCallback(UDrxLobbyEventData eventData, uk arg)
{
	CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
	DRX_ASSERT(pSquadUpr);
	if(eventData.pSessionUserData->session == pSquadUpr->m_squadHandle)
	{
		pSquadUpr->JoinUser(&eventData.pSessionUserData->data);
	}
}

//static---------------------------------------
void CSquadUpr::LeaveUserCallback(UDrxLobbyEventData eventData, uk arg)
{
	CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
	DRX_ASSERT(pSquadUpr);
	if(eventData.pSessionUserData->session == pSquadUpr->m_squadHandle)
	{
		pSquadUpr->LeaveUser(&eventData.pSessionUserData->data);
	}
}

//static---------------------------------------
void CSquadUpr::UpdateUserCallback(UDrxLobbyEventData eventData, uk arg)
{
	CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
	DRX_ASSERT(pSquadUpr);
	if(eventData.pSessionUserData->session == pSquadUpr->m_squadHandle)
	{
		pSquadUpr->UpdateUser(&eventData.pSessionUserData->data);
	}
}

//static---------------------------------------
void CSquadUpr::OnlineCallback(UDrxLobbyEventData eventData, uk arg)
{
	DrxLog("CSquadUpr::OnlineCallback(UDrxLobbyEventData eventData, uk arg)");

	if(eventData.pOnlineStateData)
	{
		CSquadUpr* pSquadUpr = (CSquadUpr*)arg;
		DRX_ASSERT(pSquadUpr);

		const EOnlineState onlineState = eventData.pOnlineStateData->m_curState;
		u32 user = eventData.pOnlineStateData->m_user;

		DrxLog("    onlineState=%i, currentTask=%i", i32(onlineState), i32(pSquadUpr->m_taskQueue.GetCurrentTask()));

		if(user == g_pGame->GetExclusiveControllerDeviceIndex())	// only do this for the game user
		{
			if (onlineState == eOS_SignedIn)
			{
				IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
				bool allowSquadCreate = (pSquadUpr->m_bMultiplayerGame) && (sm_enable) && (pSquadUpr->m_squadHandle == DrxSessionInvalidHandle) && (pLobby && pLobby->GetLobbyServiceFlag(eCLS_Online, eCLSF_AllowMultipleSessions));

				// ensure we can actually create a squad at this point in time
				if (allowSquadCreate)
				{
					DrxLog("    finished signing in, calling CreateSquad()");
					pSquadUpr->m_taskQueue.AddTask(CLobbyTaskQueue::eST_Create, false);
				}
			}
			else if (onlineState == eOS_SignedOut)
			{
				pSquadUpr->DeleteSession();
				pSquadUpr->Enable(false, false);
			}
		}
	}
}

//static--------------------------------------------
void CSquadUpr::UpdateOfflineState(CSquadUpr *pSquadUpr)
{
	DrxLog("[SquadUpr] UpdateOflineState");

	if (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle)
	{
			DrxLog("    signed out and not currently in a task");
			pSquadUpr->CleanUpSession();
	}
}

//static---------------------------------------
void CSquadUpr::UserPacketCallback(UDrxLobbyEventData eventData, uk userParam)
{
	if(!sm_enable)
		return;

	DrxLog("[tlh] CSquadUpr::UserPacketCallback(UDrxLobbyEventData, void)");

	if(eventData.pUserPacketData)
	{
		CSquadUpr* pSquadUpr = (CSquadUpr*)userParam;
		DRX_ASSERT(pSquadUpr);

		if (eventData.pUserPacketData->session != DrxSessionInvalidHandle)
		{
			const DrxSessionHandle  lobbySessionHandle = g_pGame->GetGameLobby()->GetCurrentSessionHandle();

			if (eventData.pUserPacketData->session == pSquadUpr->m_squadHandle)
			{
				pSquadUpr->ReadSquadPacket(&eventData.pUserPacketData);
			}
		}
		else
		{
			DrxLog("[tlh]   err: session handle in packet data is INVALID");
		}
	}
	else
	{
		DrxLog("[tlh]   err: packet data is NULL");
	}
}

//---------------------------------------
void CSquadUpr::SendSquadPacket(GameUserPacketDefinitions packetType, SDrxMatchMakingConnectionUID connectionUID /*=DrxMatchMakingInvalidConnectionUID*/)
{
	DrxLog("[tlh] CSquadUpr::SendSquadPacket(packetType = '%d', ...)", packetType);

	IDrxLobbyService *pLobbyService = gEnv->pNetwork->GetLobby()->GetLobbyService(eCLS_Online);
	if (!pLobbyService)
	{
		DrxLog("    failed to find online lobby service");
		return;
	}
	IDrxMatchMaking *pMatchmaking = pLobbyService->GetMatchMaking();

	CDrxLobbyPacket packet;

	switch(packetType)
	{
	case eGUPD_SquadJoin:
		{
			DrxLog("[tlh]   sending packet of type 'eGUPD_SquadJoin'");
			DRX_ASSERT(!m_squadLeader);

			u32k MaxBufferSize = DrxLobbyPacketHeaderSize + DrxLobbyPacketUINT32Size;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
				packet.WriteUINT32(GameLobbyData::GetVersion());
			}
		}
		break;
	case eGUPD_SquadJoinGame:
		{
			DrxLog("[tlh]   sending packet of type 'eGUPD_SquadJoinGame'");
			DRX_ASSERT(m_squadLeader);

			u32k MaxBufferSize = DrxLobbyPacketHeaderSize + pMatchmaking->GetSessionIDSizeInPacket() + DrxLobbyPacketBoolSize + DrxLobbyPacketUINT32Size + DrxLobbyPacketUINT32Size + DrxLobbyPacketUINT32Size;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
				EDrxLobbyError error = pMatchmaking->WriteSessionIDToPacket(m_currentGameSessionId, &packet);

				bool bIsMatchmakingGame = false;

				CGameLobby *pGameLobby = g_pGame->GetGameLobby();
				if (pGameLobby)
				{
					bIsMatchmakingGame = pGameLobby->IsMatchmakingGame();
				}

				packet.WriteBool(bIsMatchmakingGame);

				u32 playlistId = GameLobbyData::GetPlaylistId();
				packet.WriteUINT32(playlistId);

				CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();

				i32 restrictRank = 0;
				i32 requireRank = 0;

				if(pPlaylistUpr)
				{
					i32k activeVariant = pPlaylistUpr->GetActiveVariantIndex();
					const SGameVariant *pGameVariant = (activeVariant >= 0) ? pPlaylistUpr->GetVariant(activeVariant) : NULL;
					restrictRank = pGameVariant ? pGameVariant->m_restrictRank : 0;
					requireRank = pGameVariant ? pGameVariant->m_requireRank : 0;
				}

				packet.WriteUINT32(restrictRank);
				packet.WriteUINT32(requireRank);

				DRX_ASSERT(error == eCLE_Success);
			}
		}
		break;
	case eGUPD_SquadLeaveGame:
		{
			DrxLog("CSquadUpr::SendSquadPacket() sending packet of type 'eGUPD_SquadLeaveGame'");
			m_requestedGameSessionId = DrxSessionInvalidID;

			u32k MaxBufferSize = DrxLobbyPacketHeaderSize;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
			}
		}
		break;
	case eGUPD_SquadNotSupported:
		{
			DrxLog("CSquadUpr::SendSquadPacket sending packet of type 'eGUPD_SquadNotSupported'");
			DRX_ASSERT(m_squadLeader);

			u32k MaxBufferSize = DrxLobbyPacketHeaderSize;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
			}

			break;
		}
		case eGUPD_SquadNotInGame:
		{
			DrxLog("CSquadUpr::SendSquadPacket sending packet of type 'eGUPD_SquadNotInGame'");
			u32k MaxBufferSize = DrxLobbyPacketHeaderSize;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
			}
			break;
		}
		case eGUPD_SquadDifferentVersion:
		{
			DrxLog("CSquadUpr::SendSquadPacket sending packet of type 'eGUPD_SquadDifferentVersion'");
			u32k MaxBufferSize = DrxLobbyPacketHeaderSize;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
			}
			break;
		}
		case eGUPD_SquadKick:
		{
			DrxLog("CSquadUpr::SendSquadPacket sending packet of type 'eGUPD_SquadKick'");
			u32k MaxBufferSize = DrxLobbyPacketHeaderSize;
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
			}
			break;
		}
		case eGUPD_SquadMerge:
		{
			DrxLog("CSquadUpr::SendSquadPacket sending packet of type 'eGUPD_SquadMerge'");
			u32k MaxBufferSize = DrxLobbyPacketHeaderSize + pMatchmaking->GetSessionIDSizeInPacket();
			if (packet.CreateWriteBuffer(MaxBufferSize))
			{
				packet.StartWrite(packetType, true);
				EDrxLobbyError error = pMatchmaking->WriteSessionIDToPacket(m_inviteSessionId, &packet);
				DRX_ASSERT(error == eCLE_Success);
			}
			break;
		}
	}

	DRX_ASSERT_MESSAGE(packet.GetWriteBuffer() != NULL, "Haven't written any data");
	DRX_ASSERT_MESSAGE(packet.GetWriteBufferPos() == packet.GetReadBufferSize(), "Packet size doesn't match data size");
	DRX_ASSERT_MESSAGE(packet.GetReliable(), "Unreliable packet sent");

	if(m_squadLeader)
	{
		if(connectionUID != DrxMatchMakingInvalidConnectionUID)
		{
			pMatchmaking->SendToClient(&packet, m_squadHandle, connectionUID);
		}
		else
		{
			GameNetworkUtils::SendToAll(&packet, m_squadHandle, m_nameList, true);
		}
	}
	else
	{
		pMatchmaking->SendToServer(&packet, m_squadHandle);
	}
}

//---------------------------------------
void CSquadUpr::ReadSquadPacket(SDrxLobbyUserPacketData** ppPacketData)
{
	SDrxLobbyUserPacketData* pPacketData = (*ppPacketData);
	CDrxLobbyPacket* pPacket = pPacketData->pPacket;
	DRX_ASSERT_MESSAGE(pPacket->GetReadBuffer() != NULL, "No packet data");

	u32 packetType = pPacket->StartRead();
	DrxLog("CSquadUpr::ReadSquadPacket() packetType = '%d'", packetType);

	switch(packetType)
	{
	case eGUPD_SquadJoin:
		{
			DrxLog("  reading packet of type 'eGUPD_SquadJoin'");
			DRX_ASSERT(m_squadLeader);

			u32k nameIndex = m_nameList.Find(pPacketData->connection);
			if (nameIndex != SSessionNames::k_unableToFind)
			{
				m_nameList.m_sessionNames[nameIndex].m_bFullyConnected = true;
			}

			u32 clientVersion = pPacket->ReadUINT32();
			u32 ownVersion = GameLobbyData::GetVersion();

			if (clientVersion == ownVersion)
			{
				if (m_currentGameSessionId != DrxSessionInvalidID)
				{
					if(SquadsSupported())
					{
						DrxLog("   host is in a session that does support squads, sending, eGUPD_SquadJoinGame");
						SendSquadPacket(eGUPD_SquadJoinGame,  pPacketData->connection);
					}
					else
					{
						DrxLog("   host is in a session that does not support squads, sending, eGUPD_SquadNotSupported");
						SendSquadPacket(eGUPD_SquadNotSupported, pPacketData->connection);
					}
				}
				else
				{
					SendSquadPacket(eGUPD_SquadNotInGame, pPacketData->connection);
				}
			}
			else
			{
				SendSquadPacket(eGUPD_SquadDifferentVersion, pPacketData->connection);
			}
		}
		break;
	case eGUPD_SquadJoinGame:
		{
			DrxLog("  reading packet of type 'eGUPD_SquadJoinGame'");
			DRX_ASSERT(!m_squadLeader);

			IDrxLobbyService *pLobbyService = gEnv->pNetwork->GetLobby()->GetLobbyService(eCLS_Online);
			if (pLobbyService)
			{
				IDrxMatchMaking *pMatchmaking = pLobbyService->GetMatchMaking();
				DrxSessionID requestedGameSessionID = pMatchmaking->ReadSessionIDFromPacket(pPacket);
				
				const bool bIsMatchmakingGame = pPacket->ReadBool();
				u32k playlistId = pPacket->ReadUINT32();
				i32k restrictRank = (i32)pPacket->ReadUINT32();
				i32k requireRank = (i32)pPacket->ReadUINT32();

				IGameFramework *pFramework = g_pGame->GetIGameFramework();
				if(!pFramework->StartingGameContext())
				{	
					DrxLog("[squad]  calling squad join game straight away");
					SquadJoinGame(requestedGameSessionID, bIsMatchmakingGame, playlistId, restrictRank, requireRank);
				}
				else
				{
					DrxLog("[squad]  delaying squad join game call");
					m_pendingGameJoin.Set(requestedGameSessionID, bIsMatchmakingGame, playlistId, restrictRank, requireRank, true);
				}
			}
		}
		break;
	case eGUPD_SquadLeaveGame:
		{
			DrxLog("  reading 'eGUPD_SquadLeaveGame' packet");
			m_requestedGameSessionId = DrxSessionInvalidID;
			CGameLobbyUpr *pGameLobbyUpr = g_pGame->GetGameLobbyUpr();
			if (pGameLobbyUpr)
			{
				pGameLobbyUpr->LeaveGameSession(CGameLobbyUpr::eLSR_ReceivedSquadLeavingFromSquadHost);
				
				m_pendingGameJoin.Invalidate();
			}
		}
		break;
	case eGUPD_SquadNotSupported:
		{
			DrxLog("  reading 'eGUPD_SquadNotSupported' packet");
			RequestLeaveSquad();

			g_pGame->AddGameWarning("SquadNotSupported", NULL);

		}
		break;
	case eGUPD_SquadNotInGame:
		{
			DrxLog("  reading 'eGUPD_SquadNotInGame' packet");
			m_requestedGameSessionId = DrxSessionInvalidID;
			CGameLobbyUpr *pGameLobbyUpr = g_pGame->GetGameLobbyUpr();
			if (pGameLobbyUpr)
			{
				pGameLobbyUpr->LeaveGameSession(CGameLobbyUpr::eLSR_AcceptingInvite);
			}
		}
		break;
	case eGUPD_SquadDifferentVersion:
		{
			DrxLog("  reading 'eGUPD_SquadDifferentVersion' packet");
			RequestLeaveSquad();

			g_pGame->AddGameWarning("WrongSquadVersion", NULL);

		}
		break;
	case eGUPD_SquadKick:
		{
			DrxLog("  reading 'eGUPD_SquadKick' packet");

			IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
			if (pLobby)
			{
				IDrxMatchMaking *pMatchmaking = pLobby->GetMatchMaking();
				if (pMatchmaking)
				{
					DrxSessionID currentSessionId = pMatchmaking->SessionGetDrxSessionIDFromDrxSessionHandle(m_squadHandle);
					CTimeValue now = gEnv->pTimer->GetFrameStartTime();

					SKickedSession session;
					session.m_sessionId = currentSessionId;
					session.m_timeKicked = now;

					// If the user has changed, reset the list
					IPlayerProfileUpr *pPlayerProfileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
					if (pPlayerProfileUpr)
					{
						tukk pCurrentUser = pPlayerProfileUpr->GetCurrentUser();
						if (strcmp(pCurrentUser, m_kickedSessionsUsername.c_str()))
						{
							m_kickedSessionsUsername = pCurrentUser;
							m_kickedSessions.clear();
						}
					}

					i32k numKickedSessions = m_kickedSessions.size();
					if (numKickedSessions < SQUADMGR_NUM_STORED_KICKED_SESSION)
					{
						m_kickedSessions.push_back(session);
					}
					else
					{
						i32 slotToUse = 0;
						CTimeValue oldestTime = now;
						for (i32 i = 0; i < numKickedSessions; ++ i)
						{
							if (m_kickedSessions[i].m_timeKicked < oldestTime)
							{
								oldestTime = m_kickedSessions[i].m_timeKicked;
								slotToUse = i;
							}
						}

						m_kickedSessions[slotToUse] = session;
					}
				}
			}

			RequestLeaveSquad();
			g_pGame->AddGameWarning("KickedFromSquad", NULL);

			CGameLobby *pGameLobby = g_pGame->GetGameLobby();
			if (pGameLobby != NULL && pGameLobby->IsPrivateGame())
			{
				g_pGame->GetGameLobbyUpr()->LeaveGameSession(CGameLobbyUpr::eLSR_KickedFromSquad);
			}
		}
		break;
	case eGUPD_SquadMerge:
		{
			DrxLog("  reading 'eGUPD_SquadMerge' packet");
			IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
			IDrxMatchMaking *pMatchmaking = pLobby->GetMatchMaking();
			DrxSessionID joinSessionId = pMatchmaking->ReadSessionIDFromPacket(pPacket);
			if (m_inviteSessionId == NULL)
			{
				InviteAccepted(joinSessionId);
			}
		}
		break;
	}

	DRX_ASSERT_MESSAGE(pPacket->GetReadBufferSize() == pPacket->GetReadBufferPos(), "Haven't read all the data");
}

//---------------------------------------
void CSquadUpr::OnSquadLeaderChanged()
{
	DrxLog("[tlh] CSquadUpr::OnSquadLeaderChanged()");

	// Need to tell the game lobby who our squad leader is
	IDrxLobbyService *pLobbyService = gEnv->pNetwork->GetLobby()->GetLobbyService(eCLS_Online);
	if (pLobbyService)
	{
		IDrxMatchMaking *pMatchMaking = pLobbyService->GetMatchMaking();

		SDrxMatchMakingConnectionUID hostUID = pMatchMaking->GetHostConnectionUID(m_squadHandle);
		i32 hostIdx = m_nameList.Find(hostUID);
		if (hostIdx != SSessionNames::k_unableToFind)
		{
			DrxUserID hostUserId = m_nameList.m_sessionNames[hostIdx].m_userId;
			DRX_ASSERT_MESSAGE(hostUserId.IsValid(), "Failed to find a valid hostUserId, probably attempting to use squads on a LAN!");
			if (hostUserId.IsValid())
			{
				m_squadLeaderId = hostUserId;
				EventListenerSquadLeaderChanged(hostUserId);
				m_isNewSquad = false;
			}
		}
		else
		{
			m_squadLeaderId = DrxUserInvalidID;
			EventListenerSquadLeaderChanged(DrxUserInvalidID);
		}
	}
}

//---------------------------------------
void CSquadUpr::JoinUser(SDrxUserInfoResult* user)
{
	DrxLog("CSquadUpr::JoinUser() user='%s'", user->m_userName);

	m_nameList.Insert(user->m_userID, user->m_conID, user->m_userName, user->m_userData, user->m_isDedicated);
	EventListenerSquaddieAdded(user->m_userID);

	if(m_isNewSquad)
	{
		OnSquadLeaderChanged();
	}

	// if we're the leader and we're on the variant screen, then refresh so
	// options are updated, (e.g. solo gets disabled if necessary)
	if(m_squadLeader)
	{
		// TODO: michiel - old frontend
	}
}

//---------------------------------------
void CSquadUpr::UpdateUser(SDrxUserInfoResult* user)
{
	DrxLog("CSquadUpr::UpdateUser(SDrxUserInfoResult* user)");
	m_nameList.Update(user->m_userID, user->m_conID, user->m_userName, user->m_userData, user->m_isDedicated, false);

	EventListenerSquaddieUpdated(user->m_userID);
}

//---------------------------------------
void CSquadUpr::LeaveUser(SDrxUserInfoResult* user)
{
	DrxLog("CSquadUpr::LeaveUser(), user='%s'", user->m_userName);
	m_nameList.Remove(user->m_conID);
	EventListenerSquaddieRemoved(user->m_userID);

	if ((m_inviteSessionId != NULL) && (m_nameList.Size() == 1))
	{
		JoinInvite();
	}
}

//---------------------------------------
DrxUserID CSquadUpr::GetUserIDFromChannelID(i32 channelId)
{
	DrxUserID userId = DrxUserInvalidID;

	if (gEnv->pNetwork && gEnv->pNetwork->GetLobby())
	{
		if (IDrxMatchMaking* pMatchMaking = gEnv->pNetwork->GetLobby()->GetMatchMaking())
		{
			SDrxMatchMakingConnectionUID conId = pMatchMaking->GetConnectionUIDFromGameSessionHandleAndChannelID(m_squadHandle, channelId);
			SSessionNames::SSessionName *pSessionName = m_nameList.GetSessionName(conId, true);
			if (pSessionName)
			{
				userId = pSessionName->m_userId;
			}
		}
	}

#if !defined(_RELEASE)
	if (userId == DrxUserInvalidID)
	{
		DrxLog("Failed to get DrxUserID for squad channel %d", channelId);
	}
#endif

	return userId;
}

//-------------------------------------------------------------------------
bool CSquadUpr::GetSquadCommonDLCs(u32 &commonDLCs)
{
	if(InSquad())
	{
		i32 numPlayers = m_nameList.Size();
		if(numPlayers)
		{
			commonDLCs = ~0;
			for (i32 i=0; i<numPlayers; ++i)
			{
				SSessionNames::SSessionName &player = m_nameList.m_sessionNames[i];
				commonDLCs &= player.m_userData[eLUD_LoadedDLCs];
			}
			return true;
		}
	}
	commonDLCs = 0;
	return false;
}

//-------------------------------------------------------------------------
void CSquadUpr::TaskStartedCallback( CLobbyTaskQueue::ESessionTask task, uk pArg )
{
	DrxLog("CSquadUpr::TaskStartedCallback(task=%u)", task);
	INDENT_LOG_DURING_SCOPE();

	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(pArg);

	EDrxLobbyError result = eCLE_ServiceNotSupported;
	bool bMatchMakingTaskStarted = false;
	DrxLobbyTaskID taskId = DrxLobbyInvalidTaskID;

	IDrxLobby *pDrxLobby = gEnv->pNetwork->GetLobby();
	if (pDrxLobby != NULL && pDrxLobby->GetLobbyService(eCLS_Online))
	{
		IDrxMatchMaking *pMatchMaking = pDrxLobby->GetLobbyService(eCLS_Online)->GetMatchMaking();
		if (pMatchMaking)
		{
			switch (task)
			{
			case CLobbyTaskQueue::eST_Create:
				if (sm_enable && (pSquadUpr->m_squadHandle == DrxSessionInvalidHandle))
				{
					DrxLog("    creating a squad");
					bMatchMakingTaskStarted = true;
					result = pSquadUpr->DoCreateSquad(pMatchMaking, taskId);
				}
				break;

			case CLobbyTaskQueue::eST_Join:
				if (sm_enable && (pSquadUpr->m_inviteSessionId != DrxSessionInvalidID) && (pSquadUpr->m_squadHandle == DrxSessionInvalidHandle))
				{
					DrxLog("    joining a squad");
					bMatchMakingTaskStarted = true;
					result = pSquadUpr->DoJoinSquad(pMatchMaking, taskId);
				}
				break;

			case CLobbyTaskQueue::eST_Delete:
				if (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle)
				{
					DrxLog("    deleting current squad");
					bMatchMakingTaskStarted = true;
					result = pSquadUpr->DoLeaveSquad(pMatchMaking, taskId);
				}
				else
				{
					pSquadUpr->m_leavingSession = false;
				}
				break;

			case CLobbyTaskQueue::eST_SetLocalUserData:
				{
					if (sm_enable && (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle))
					{
						DrxLog("    setting local user data");
						bMatchMakingTaskStarted = true;
						result = pSquadUpr->DoUpdateLocalUserData(pMatchMaking, taskId);
					}
				}
				break;

			case CLobbyTaskQueue::eST_SessionStart:
				{
					if (sm_enable && (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle))
					{
						if (!pSquadUpr->m_bSessionStarted)
						{
							DrxLog("    starting session");
							bMatchMakingTaskStarted = true;
							result = pSquadUpr->DoStartSession(pMatchMaking, taskId);
						}
					}
				}
				break;

			case CLobbyTaskQueue::eST_SessionEnd:						// Deliberate fall-through
				{
					if (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle)
					{
						if (pSquadUpr->m_bSessionStarted)
						{
							DrxLog("    ending session");
							bMatchMakingTaskStarted = true;
							result = pSquadUpr->DoEndSession(pMatchMaking, taskId);
						}
					}
				}
				break;
			case CLobbyTaskQueue::eST_SessionUpdateSlotType:
				{
					if(sm_enable && (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle))	// don't really need to do this if disabled or no session
					{
						if(pSquadUpr->m_slotType != pSquadUpr->m_requestedSlotType)
						{
							DrxLog("    session update slot type");
							bMatchMakingTaskStarted = true;
							result = pSquadUpr->DoSessionChangeSlotType(pMatchMaking, taskId);
						}
					}
					break;
				}
			case CLobbyTaskQueue::eST_SessionSetLocalFlags:
				{
					if (sm_enable && (pSquadUpr->m_squadHandle != DrxSessionInvalidHandle))
					{
						DrxLog("    setting local session flags");
						bMatchMakingTaskStarted = true;
						result = pSquadUpr->DoSessionSetLocalFlags(pMatchMaking, taskId);
					}
					break;
				}
			}
		}
	}

	if (bMatchMakingTaskStarted)
	{
		if (result == eCLE_Success)
		{
			pSquadUpr->m_currentTaskId = taskId;
		}
		else if(result == eCLE_SuccessInvalidSession)
		{
			pSquadUpr->m_taskQueue.TaskFinished();
		}
		else if (result == eCLE_TooManyTasks)
		{
			DrxLog("  too many tasks, restarting next frame");
			pSquadUpr->m_taskQueue.RestartTask();
		}
		else
		{
			ReportError(result);
			pSquadUpr->m_taskQueue.TaskFinished();
		}
	}
	else
	{
		// Task failed to start
		DrxLog("  ERROR: task failed to start, we're in an incorrect state sm_enable=%i, squadHandle=%u, sessionStarted=%s",
			sm_enable, pSquadUpr->m_squadHandle, pSquadUpr->m_bGameSessionStarted ? "true" : "false");

		pSquadUpr->m_taskQueue.TaskFinished();
	}
}

//-------------------------------------------------------------------------
void CSquadUpr::TaskFinished()
{
	DrxLog("CSquadUpr::TaskFinished() task=%u", m_taskQueue.GetCurrentTask());

	m_taskQueue.TaskFinished();
}

//static---------------------------------------
void CSquadUpr::ReportError(EDrxLobbyError error)
{
	DrxLogAlways("CSquadUpr::ReportError(EDrxLobbyError error = %d)", error);

	if(error != eCLE_Success)
	{
		if ((error != eCLE_UserNotSignedIn) &&
			(error != eCLE_InsufficientPrivileges) &&
			(error != eCLE_CableNotConnected) &&
			(error != eCLE_InternetDisabled))
		{
			CGameLobbyUpr *pGameLobbyUpr = g_pGame->GetGameLobbyUpr();
			if (!pGameLobbyUpr || pGameLobbyUpr->IsCableConnected())
			{
				if (error == eCLE_UserNotInSession)
				{
					gEnv->pGame->AddGameWarning("SquadUprError", "@ui_menu_error_squad_not_found");
				}
				else if (error == eCLE_SessionFull)
				{
					gEnv->pGame->AddGameWarning("SquadUprError", "@ui_menu_error_squad_full");
				}
				else
				{
					gEnv->pGame->AddGameWarning("SquadUprError", NULL);
				}
			}
		}
	}
}

//static---------------------------------------
void CSquadUpr::HandleCustomError(tukk dialogName, tukk msgPreLoc, const bool deleteSession, const bool returnToMainMenu)
{
	if (deleteSession)
	{
		g_pGame->GetGameLobby()->LeaveSession(true, false);
	}

	gEnv->pGame->AddGameWarning(dialogName, msgPreLoc);
}

//-------------------------------------------------------------------------
bool CSquadUpr::CallbackReceived( DrxLobbyTaskID taskId, EDrxLobbyError result )
{
	if (m_currentTaskId == taskId)
	{
		m_currentTaskId = DrxLobbyInvalidTaskID;
	}
	else
	{
		DrxLog("CSquadUpr::CallbackReceived() received callback with an unexpected taskId=%d, expected=%d", taskId, m_currentTaskId);
	}

	if (result == eCLE_TimeOut)
	{
		if(sm_enable || m_taskQueue.GetCurrentTask() == CLobbyTaskQueue::eST_Delete)
		{
			DrxLog("CSquadUpr::CallbackReceived() task timed out, restarting");
			m_taskQueue.RestartTask();
		}
		else
		{
			DrxLog("CSquadUpr::CallbackReceived() task timed out, finishing as squad manager is disabled");
			m_taskQueue.TaskFinished();
		}
	}
	else
	{
		m_taskQueue.TaskFinished();

		if (result != eCLE_Success && result != eCLE_SuccessInvalidSession)
		{
			DrxLog("CSquadUpr::CallbackReceived() task unsuccessful, result=%u", result);
			ReportError(result);
		}
	}

	return (result == eCLE_Success) || (result == eCLE_SuccessInvalidSession);
}

//-------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CSquadUpr::OnPromoteToServer( SHostMigrationInfo& hostMigrationInfo, u32& state )
{
	if (m_squadHandle == hostMigrationInfo.m_session)
	{
		u32k numUsers = m_nameList.Size();
		for (u32 i = 0; i < numUsers; ++ i)
		{
			m_nameList.m_sessionNames[i].m_bFullyConnected = true;
		}

		m_pendingGameJoin.Invalidate();
	}

	return IHostMigrationEventListener::Listener_Done;
}

//-------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CSquadUpr::OnFinalise( SHostMigrationInfo& hostMigrationInfo, u32& state )
{
	if (m_squadHandle == hostMigrationInfo.m_session)
	{
		m_squadLeader = hostMigrationInfo.IsNewHost();
		DrxLog("CSquadUpr::OnFinalise() squad session has changed host, isNewHost=%s", m_squadLeader ? "true" : "false");
		OnSquadLeaderChanged();
		EventListenerSquadEvent(ISquadEventListener::eSET_MigratedSquad);
	}

	return IHostMigrationEventListener::Listener_Done;
}

//-------------------------------------------------------------------------
void CSquadUpr::SetMultiplayer( bool multiplayer )
{
#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS 
	return;
#endif
	bool shouldBeEnabled = false;

	if (m_bMultiplayerGame != multiplayer)
	{
		m_bMultiplayerGame = multiplayer;
		if (multiplayer)
		{
			// Only enable if we're using online service
			IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
			if (pLobby)
			{
				const bool allowJoinMultipleSessions = pLobby->GetLobbyServiceFlag(eCLS_Online, eCLSF_AllowMultipleSessions);
				if (allowJoinMultipleSessions)
				{
					shouldBeEnabled = true;
				}
			}
		}
	}

	Enable(shouldBeEnabled, false);
}

//-------------------------------------------------------------------------
void CSquadUpr::TellMembersToLeaveGameSession()
{
	DrxLog("CSquadUpr::TellMembersToLeaveGameSession()");
	SendSquadPacket(eGUPD_SquadLeaveGame);
}

//-------------------------------------------------------------------------
void CSquadUpr::RequestLeaveSquad()
{
	DrxLog("CSquadUpr::RequestLeaveSquad()");
	DeleteSession();
}

#if !defined(_RELEASE)
//static---------------------------------------
void CSquadUpr::CmdCreate(IConsoleCmdArgs* pCmdArgs)
{
	CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();
	pSquadUpr->m_taskQueue.AddTask(CLobbyTaskQueue::eST_Create, false);
}

//static---------------------------------------
void CSquadUpr::CmdLeave(IConsoleCmdArgs* pCmdArgs)
{
	CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();
	pSquadUpr->DeleteSession();
}

//static---------------------------------------
void CSquadUpr::CmdKick(IConsoleCmdArgs* pCmdArgs)
{
	if (pCmdArgs->GetArgCount() == 2)
	{
		tukk pPlayerName = pCmdArgs->GetArg(1);
		CSquadUpr *pSquadUpr = g_pGame->GetSquadUpr();

		if (pSquadUpr->m_squadLeader)
		{
			SSessionNames *pSessionNames = &pSquadUpr->m_nameList;
			i32k numMembers = pSessionNames->Size();
			for (i32 i = 0; i < numMembers; ++ i)
			{
				SSessionNames::SSessionName &player = pSessionNames->m_sessionNames[i];
				if (!stricmp(player.m_name, pPlayerName))
				{
					pSquadUpr->KickPlayer(player.m_userId);
				}
			}
		}
		else
		{
			DrxLog("Only the squad leader can kick players");
		}
	}
	else
	{
		DrxLog("Invalid format, use 'sm_kick <playername>'");
	}
}
#endif

//-------------------------------------------------------------------------
void CSquadUpr::OnGameSessionStarted()
{
	if (!sm_enable)
		return;

	m_bGameSessionStarted = true;
	m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionStart, false);
}

//-------------------------------------------------------------------------
void CSquadUpr::OnGameSessionEnded()
{
	if (!sm_enable)
		return;

	m_bGameSessionStarted = false;
	m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionEnd, false);
}

//-------------------------------------------------------------------------
void CSquadUpr::DeleteSession()
{
	DrxLog("CSquadUpr::DeleteSession()");

	// We want to leave the current session, clear/cancel any pending tasks that aren't related
	// to deleting the session

	CLobbyTaskQueue::ESessionTask currentTask = m_taskQueue.GetCurrentTask();

	// need to let create/join finish their tasks before deleting, or we can get the session
	// in a bad state, they will initiate delete session on completion if need be
	if(currentTask == CLobbyTaskQueue::eST_Create)
	{
		DrxLog(" cannot delete session, currently creating");
		return;
	}

	// join session is marked as invalid so we don't stay with it 
	// upon completion
	if(currentTask == CLobbyTaskQueue::eST_Join)	
	{
		DrxLog(" cannot delete session, currently joining");
		m_sessionIsInvalid = true;
		return;
	}

	m_pendingGameJoin.Invalidate();

	if(m_squadHandle != DrxSessionInvalidHandle)	// don't bother trying to add a delete if we're not already in a session
	{	
		m_leavingSession = true;

		CLobbyTaskQueue::ESessionTask firstRequiredTask = CLobbyTaskQueue::eST_Delete;
		if (m_bSessionStarted)
		{
			firstRequiredTask = CLobbyTaskQueue::eST_SessionEnd;
		}

		if (currentTask != firstRequiredTask)
		{
			if (m_currentTaskId != DrxLobbyInvalidTaskID)
			{
				IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
				if (pLobby)
				{
					IDrxLobbyService *pLobbyService = pLobby->GetLobbyService(eCLS_Online);
					if (pLobbyService)
					{
						IDrxMatchMaking *pMatchMaking = pLobbyService->GetMatchMaking();
						if (pMatchMaking)
						{
							pMatchMaking->CancelTask(m_currentTaskId);
							m_currentTaskId = DrxLobbyInvalidTaskID;
						}
					}
				}
			}
			m_taskQueue.Reset();

			if (m_bSessionStarted)
			{
				m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionEnd, false);
			}
			m_taskQueue.AddTask(CLobbyTaskQueue::eST_Delete, false);
		}
		else
		{
			m_taskQueue.ClearNotStartedTasks();
			// We're already doing the first required task, if this task is a eST_SessionEnd then we need
			// to follow it with a eST_Delete
			if (firstRequiredTask == CLobbyTaskQueue::eST_SessionEnd)
			{
				m_taskQueue.AddTask(CLobbyTaskQueue::eST_Delete, false);
			}
		}
	}
}

//-------------------------------------------------------------------------
EDrxLobbyError CSquadUpr::DoStartSession( IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId )
{
	DrxLog("CSquadUpr::DoStartSession()");
	DRX_ASSERT(m_taskQueue.GetCurrentTask() == CLobbyTaskQueue::eST_SessionStart);

	return pMatchMaking->SessionStart(m_squadHandle, &taskId, SessionStartCallback, this);
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionStartCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg )
{
	DrxLog("CSquadUpr::SessionStartCallback()");

	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(pArg);

	if (pSquadUpr->CallbackReceived(taskID, error))
	{
		pSquadUpr->m_bSessionStarted = true;
	}
}

//-------------------------------------------------------------------------
EDrxLobbyError CSquadUpr::DoEndSession( IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId )
{
	DrxLog("CSquadUpr::DoEndSession()");
	DRX_ASSERT(m_taskQueue.GetCurrentTask() == CLobbyTaskQueue::eST_SessionEnd);

	return pMatchMaking->SessionEnd(m_squadHandle, &taskId, SessionEndCallback, this);
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionEndCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg )
{
	DrxLog("CSquadUpr::SessionEndCallback()");

	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(pArg);

	pSquadUpr->CallbackReceived(taskID, error);

	// If we get any result other than a timeout (which will try again), we need to reset the started flag
	if (error != eCLE_TimeOut)
	{
		pSquadUpr->m_bSessionStarted = false;
	}
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionClosedCallback( UDrxLobbyEventData eventData, uk userParam )
{
	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(userParam);
	DRX_ASSERT(pSquadUpr);
	if (pSquadUpr)
	{
		SDrxLobbySessionEventData *pEventData = eventData.pSessionEventData;
		
		if ((pSquadUpr->m_squadHandle == pEventData->session) && (pEventData->session != DrxSessionInvalidHandle))
		{
			DrxLog("CSquadUpr::SessionClosedCallback() received SessionClosed event, leaving session");
			pSquadUpr->RequestLeaveSquad();
		}
	}	
}

//-------------------------------------------------------------------------
void CSquadUpr::ForcedFromRoomCallback(UDrxLobbyEventData eventData, uk pArg)
{
	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(pArg);
	if (pSquadUpr)
	{
		SDrxLobbyForcedFromRoomData *pEventData = eventData.pForcedFromRoomData;

		DrxLog("CSquadUpr::ForcedFromRoomCallback session %d reason %d", (i32)pEventData->m_session, pEventData->m_why);

		if (pEventData->m_session != DrxSessionInvalidHandle && pSquadUpr->m_squadHandle == pEventData->m_session)
		{
			// any of the other reasons will result in sign out, disconnect, thus
			// handled appropriately by the other events out there
			DrxLog("[squad] received eCLSE_ForcedFromRoom event with reason %d, leaving session", pEventData->m_why);
			pSquadUpr->RequestLeaveSquad();
		}
	}
}

//-------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CSquadUpr::OnTerminate( SHostMigrationInfo& hostMigrationInfo, u32& state )
{
	if (m_squadHandle == hostMigrationInfo.m_session)
	{
		DrxLog("CSquadUpr::OnTerminate() host migration failed, leaving session");
		RequestLeaveSquad();
	}	

	return IHostMigrationEventListener::Listener_Done;
}

//-------------------------------------------------------------------------
IHostMigrationEventListener::EHostMigrationReturn CSquadUpr::OnReset(SHostMigrationInfo& hostMigrationInfo, u32& state)
{
	return IHostMigrationEventListener::Listener_Done;
}

//-------------------------------------------------------------------------
bool CSquadUpr::SquadsSupported()
{
	CPlaylistUpr *pPlaylistUpr = g_pGame->GetPlaylistUpr();
	bool result = true;

	if(pPlaylistUpr)
	{
		i32 activeVariant = pPlaylistUpr->GetActiveVariantIndex();
		if(activeVariant >= 0)
		{
			const SGameVariant *pGameVariant = pPlaylistUpr->GetVariant(activeVariant);
			if(pGameVariant)
			{
				result = pGameVariant->m_allowSquads;
			}
		}
	}

	return result;
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionChangeSlotType(ESessionSlotType type)
{
	DrxLog("[squad] SessionChangeSlotType set slots to be %s", (type == eSST_Public) ? "public" : "private");

	m_requestedSlotType = type;
	m_taskQueue.AddTask(CLobbyTaskQueue::eST_SessionUpdateSlotType, false);
}

//-------------------------------------------------------------------------
EDrxLobbyError CSquadUpr::DoSessionChangeSlotType(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	EDrxLobbyError error = eCLE_Success;

	u32 numPublicSlots = (m_requestedSlotType == eSST_Public) ? SQUADMGR_MAX_SQUAD_SIZE	: 0;
	u32 numPrivateSlots = (m_requestedSlotType == eSST_Private) ? SQUADMGR_MAX_SQUAD_SIZE	: 0;

	DrxLog("[squad] DoSessionChangeSlotType numPublicSlots %d numPrivateSlots %d squadHandle %d", numPublicSlots, numPrivateSlots, GetSquadSessionHandle());
	error = pMatchMaking->SessionUpdateSlots(GetSquadSessionHandle(), numPublicSlots, numPrivateSlots, &taskId, CSquadUpr::SessionChangeSlotTypeCallback, this);

	if(error == eCLE_Success)
	{
		m_inProgressSlotType = m_requestedSlotType;
	}

	return error;
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionChangeSlotTypeFinished(DrxLobbyTaskID taskID, EDrxLobbyError error)
{
	if (CallbackReceived(taskID, error))
	{
		DrxLog("[squad] SessionChangeSlotTypeFinished succeeded in changing slot status to %s", (m_inProgressSlotType == eSST_Public) ? "public" : "private");
		
		m_slotType = m_inProgressSlotType;		
	}
	else if (error != eCLE_TimeOut)
	{
		DrxLog("[squad] SessionChangeSlotTypeFinished could not set slot type to %s, failed with error code %d",  (m_inProgressSlotType == eSST_Public) ? "public" : "private", error);
	}
}

//-------------------------------------------------------------------------
EDrxLobbyError CSquadUpr::DoSessionSetLocalFlags(IDrxMatchMaking *pMatchMaking, DrxLobbyTaskID &taskId)
{
	EDrxLobbyError result = pMatchMaking->SessionSetLocalFlags(GetSquadSessionHandle(), DRXSESSION_LOCAL_FLAG_HOST_MIGRATION_CAN_BE_HOST, &taskId, CSquadUpr::SessionSetLocalFlagsCallback, this);

	return result;
}

//-------------------------------------------------------------------------
void CSquadUpr::SessionSetLocalFlagsCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, DrxSessionHandle h, u32 flags, uk pArg )
{
	DrxLog("CSquadUpr::SessionSetLocalFlagsCallback() error=%u", error);

	CSquadUpr *pSquadUpr = static_cast<CSquadUpr*>(pArg);
	pSquadUpr->CallbackReceived(taskID, error);
}

//-------------------------------------------------------------------------
void CSquadUpr::LeftGameSessionInProgress()
{
	DrxLog("CSquadUpr::LeftGameSessionInProgress()");

	if (HaveSquadMates())
	{
		if (InCharge())
		{
			DrxLog("  currently in charge of an active squad, leave it");
			RequestLeaveSquad();
		}
		else
		{
			if (GameNetworkUtils::CompareDrxSessionId(m_requestedGameSessionId, m_currentGameSessionId))
			{
				DrxLog("  currently a member in a squad, leaving game that squad leader is in, leave squad too");
				RequestLeaveSquad();
			}
		}
	}
}

//-------------------------------------------------------------------------
void CSquadUpr::KickPlayer( DrxUserID userId )
{
	if (m_squadLeader)
	{
		if (!(m_squadLeaderId == userId))		// DrxUserID doesn't support !=  :-(
		{
			i32 index = m_nameList.FindByUserId(userId);
			if (index != SSessionNames::k_unableToFind)
			{
				SSessionNames::SSessionName &player = m_nameList.m_sessionNames[index];
				m_pendingKickUserId = player.m_userId;
				DrxLog("CSquadUpr::KickPlayer() requested kick '%s' uid=%u", player.m_name, player.m_conId.m_uid);

				g_pGame->AddGameWarning("KickPlayerFromSquad", player.m_name, this);
			}
		}
	}
}

//-------------------------------------------------------------------------
bool CSquadUpr::AllowedToJoinSession( DrxSessionID sessionId )
{
	IPlayerProfileUpr *pPlayerProfileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if (pPlayerProfileUpr)
	{
		tukk pCurrentUser = pPlayerProfileUpr->GetCurrentUser();
		if (strcmp(pCurrentUser, m_kickedSessionsUsername.c_str()))
		{
			// If the user has changed since our banned list was created, allow the join
			// Note: The list will be reset when we're next kicked, by resetting it here we stop the user
			// just logging out and back in again to bypass the ban
			return true;
		}
	}
	
	i32k numKickedSessions = m_kickedSessions.size();
	for (i32 i = 0; i < numKickedSessions; ++ i)
	{
		if (GameNetworkUtils::CompareDrxSessionId(m_kickedSessions[i].m_sessionId, sessionId))
		{
			CTimeValue now = gEnv->pTimer->GetFrameStartTime();
			float timeDiff = (now - m_kickedSessions[i].m_timeKicked).GetSeconds();

			if (timeDiff < 1800.f)		// 30 mins
			{
				return false;
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------
void CSquadUpr::RemoveFromBannedList(DrxSessionID sessionId)
{
	i32k numKickedSessions = m_kickedSessions.size();
	for (i32 i = 0; i < numKickedSessions; ++ i)
	{
		if (GameNetworkUtils::CompareDrxSessionId(m_kickedSessions[i].m_sessionId, sessionId))
		{
			m_kickedSessions.removeAt(i);
			break;
		}
	}
}

//-------------------------------------------------------------------------
bool CSquadUpr::OnWarningReturn( THUDWarningId id, tukk returnValue )
{
	if (id == g_pGame->GetWarnings()->GetWarningId("KickPlayerFromSquad"))
	{
		if (!stricmp(returnValue, "yes"))
		{
			i32 index = m_nameList.FindByUserId(m_pendingKickUserId);
			if (index != SSessionNames::k_unableToFind)
			{
				SSessionNames::SSessionName &player = m_nameList.m_sessionNames[index];
				DrxLog("CSquadUpr::KickPlayer() kick confirmed for '%s' uid=%u", player.m_name, player.m_conId.m_uid);
				SendSquadPacket(eGUPD_SquadKick, player.m_conId);
			}
		}
		m_pendingKickUserId = DrxUserInvalidID;
	}
	return true;
}

//-------------------------------------------------------------------------
bool CSquadUpr::IsEnabled()
{
	return sm_enable != 0;
}

//-------------------------------------------------------------------------
void CSquadUpr::SquadJoinGame(DrxSessionID sessionID, bool isMatchmakingGame, u32 playlistID, i32 restrictRank, i32 requireRank)
{
	DrxLog("SquadJoinGame isMatchmakingGame %d, playlistID %d, restrictRank %d requireRank %d", isMatchmakingGame, playlistID, restrictRank, requireRank);
	
	m_requestedGameSessionId = sessionID;

	CPlayerProgression *pPlayerProgression = CPlayerProgression::GetInstance();

	i32k  displayRank = (pPlayerProgression ? pPlayerProgression->GetData(EPP_DisplayRank) : 0);
	i32k  reincarnations = (pPlayerProgression ? pPlayerProgression->GetData(EPP_Reincarnate) : 0);
	tukk  pWarning = NULL;  // a NULL warning means allowed to join
	if (displayRank)
	{
	 	pWarning = ((!pWarning && restrictRank && ((displayRank > restrictRank) || (reincarnations > 0))) ? "RankTooHigh" : pWarning);
	 	pWarning = ((!pWarning && requireRank && ((displayRank < requireRank) && (reincarnations == 0))) ? "RankTooLow" : pWarning);
	}

	if (!pWarning)	
	{
		if (isMatchmakingGame)
		{
			g_pGame->GetPlaylistUpr()->ChoosePlaylistById(playlistID);
		}

		JoinGameSession(m_requestedGameSessionId, isMatchmakingGame);

		// TODO: michiel - oldfrontend
	}
	else
	{
		DRX_ASSERT(pWarning);
		DrxLog("  my rank (%d) out of variant rank range (%d to %d), leaving squad with warning \"%s\"", displayRank, requireRank, restrictRank, pWarning);
		RequestLeaveSquad();
		gEnv->pGame->AddGameWarning("SquadUprError", pWarning);
	}
}
