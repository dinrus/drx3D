// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_GameRules.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/GameClientChannel.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/ActionGame.h>

#if defined(GAME_CHANNEL_SYNC_CLIENT_SERVER_TIME)
	#include <drx3D/Act/GameClientChannel.h>
	#include <drx3D/Act/GameClientNub.h>
#endif
/*
 * Create Game Rules
 */

class CCET_CreateGameRules : public CCET_Base
{
public:
	tukk                 GetName() { return "CreateGameRules"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if (CDrxAction::GetDrxAction()->GetGameContext()->GetRequestedGameRules().empty())
		{
			GameWarning("CreateGameRules: No game rules set");
			return eCETR_Failed;
		}
		return CDrxAction::GetDrxAction()->GetIGameRulesSystem()->CreateGameRules(CDrxAction::GetDrxAction()->GetGameContext()->GetRequestedGameRules()) ? eCETR_Ok : eCETR_Failed;
	}
};

void AddGameRulesCreation(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_CreateGameRules);
}

/*
 * reset game rules
 */

class CCET_GameRulesReset : public CCET_Base
{
public:
	tukk                 GetName() { return "GameRulesReset"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		SEntityEvent event(ENTITY_EVENT_RESET);
		event.nParam[0] = 1;

		CDrxAction::GetDrxAction()->GetIGameRulesSystem()->GetCurrentGameRulesEntity()->SendEvent(event);
		return eCETR_Ok;
	}
};

void AddGameRulesReset(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_GameRulesReset);
}

/*
 * Send Game Type
 */

class CCET_SendGameType : public CCET_Base
{
public:
	CCET_SendGameType(CClassRegistryReplicator* pRep, std::vector<SSendableHandle>* pWaitFor) : m_pRep(pRep), m_pWaitFor(pWaitFor) {}

	tukk                 GetName() { return "SendGameType"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		// check our game rules have been registered
		if (CDrxAction::GetDrxAction()->GetGameContext()->GetRequestedGameRules().empty())
		{
			GameWarning("SendGameRules: No game rules set");
			return eCETR_Failed;
		}
		if (CDrxAction::GetDrxAction()->GetGameContext()->GetLevelName().empty())
		{
			GameWarning("SendGameRules: no level name set");
			return eCETR_Failed;
		}

		u16 id = ~u16(0);

		// game rules can be set via an alias, if we pass the alias into ClassIdFromName it will fail and disconnect the client, so we lookup the proper name here
		IGameRulesSystem* pGameRulesSystem = CDrxAction::GetDrxAction()->GetIGameRulesSystem();
		tukk gameRulesName = pGameRulesSystem->GetGameRulesName(CDrxAction::GetDrxAction()->GetGameContext()->GetRequestedGameRules());

		if ((!gameRulesName || !m_pRep->ClassIdFromName(id, string(gameRulesName)) || id == (u16)(~u16(0)))
			&& !CDrxAction::GetDrxAction()->GetGameContext()->HasContextFlag(eGSF_NoGameRules))
		{
			GameWarning("Cannot find rules %s in network class registry", CDrxAction::GetDrxAction()->GetGameContext()->GetRequestedGameRules().c_str());
		}

		// called on the server, we should send any information about the
		// game type that we need to

		SSendableHandle* pWaitFor = 0;
		i32 nWaitFor = 0;
		if (m_pWaitFor && !m_pWaitFor->empty())
		{
			pWaitFor = &*m_pWaitFor->begin();
			nWaitFor = m_pWaitFor->size();
		}

		state.pSender->AddSendable(
		  new CSimpleNetMessage<SGameTypeParams>(SGameTypeParams(id, CDrxAction::GetDrxAction()->GetGameContext()->GetLevelName(), CDrxAction::GetDrxAction()->GetGameContext()->HasContextFlag(eGSF_ImmersiveMultiplayer)), CGameClientChannel::SetGameType),
		  nWaitFor, pWaitFor, NULL);

		return eCETR_Ok;
	}

private:
	CClassRegistryReplicator*     m_pRep;
	std::vector<SSendableHandle>* m_pWaitFor;
};

void AddSendGameType(IContextEstablisher* pEst, EContextViewState state, CClassRegistryReplicator* pRep, std::vector<SSendableHandle>* pWaitFor)
{
	pEst->AddTask(state, new CCET_SendGameType(pRep, pWaitFor));
}

/*
 * Send reset map
 */

class CCET_SendResetMap : public CCET_Base
{
public:
	tukk                 GetName() { return "SendResetMap"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CGameClientChannel::SendResetMapWith(SNoParams(), state.pSender);
		return eCETR_Ok;
	}
};

void AddSendResetMap(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_SendResetMap());
}

/*
 * Init game immersiveness parameters
 */

class CCET_InitImmersiveness : public CCET_Base
{
public:
	tukk                 GetName() { return "InitImmersiveness"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CActionGame::Get()->InitImmersiveness();
		return eCETR_Ok;
	}
};

void AddInitImmersiveness(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_InitImmersiveness());
}

class CCET_ClearOnHold : public CCET_Base
{
public:
	CCET_ClearOnHold() : m_pServerChan(nullptr) {}

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if (INetChannel* pNC = state.pSender)
			if (CGameServerChannel* pSC = (CGameServerChannel*) pNC->GetGameChannel())
				pSC->SetOnHold(false);
		return eCETR_Ok;
	}

	tukk GetName() { return "ClearOnHold"; };

private:
	CGameServerChannel* m_pServerChan;
};

void AddClearOnHold(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_ClearOnHold());
}

/*
 * pause/unpause game
 */

class CCET_PauseGame : public CCET_Base
{
public:
	CCET_PauseGame(bool pause, bool forcePause) : m_pause(pause), m_forcePause(forcePause) {}

	tukk                 GetName() { return m_pause ? "PauseGame" : "ResumeGame"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CDrxAction::GetDrxAction()->PauseGame(m_pause, m_forcePause);
		return eCETR_Ok;
	}

private:
	bool m_pause;
	bool m_forcePause;
};

void AddPauseGame(IContextEstablisher* pEst, EContextViewState state, bool pause, bool forcePause)
{
	pEst->AddTask(state, new CCET_PauseGame(pause, forcePause));
}

/*
 * wait for precaching to finish before starting the game
 */

class CCET_WaitForPreCachingToFinish : public CCET_Base
{
public:
	CCET_WaitForPreCachingToFinish(bool* pGameStart) : m_pGameStart(pGameStart) {}

	tukk                 GetName() { return "WaitForPrecachingToFinish"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		if(gEnv->IsEditor() || (gEnv->pSystem->GetSystemGlobalState() >= ESYSTEM_GLOBAL_STATE_LEVEL_LOAD_END) || gEnv->bMultiplayer)
		{
			*m_pGameStart = true;
			return eCETR_Ok;
		}

		return eCETR_Wait;
	}

private:
	bool* m_pGameStart;
};

void AddWaitForPrecachingToFinish(IContextEstablisher* pEst, EContextViewState state, bool* pGameStart)
{
	pEst->AddTask(state, new CCET_WaitForPreCachingToFinish(pGameStart));
}

/*
 * save game for reset
 */

class CCET_InitialSaveGame : public CCET_Base
{
public:
	CCET_InitialSaveGame() {}

	tukk                 GetName() { return "InitialSaveGame"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		bool loadingSaveGame = CDrxAction::GetDrxAction()->IsLoadingSaveGame();
		if (!loadingSaveGame && !GetISystem()->IsSerializingFile())
		{
			CDrxAction::GetDrxAction()->SaveGame(CDrxAction::GetDrxAction()->GetStartLevelSaveGameName(), true, true, eSGR_LevelStart);
		}

		return eCETR_Ok;
	}
};

void AddInitialSaveGame(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_InitialSaveGame());
}

#if defined(GAME_CHANNEL_SYNC_CLIENT_SERVER_TIME)

/*
 * Attempt to sync the clocks
 */

class CCET_SyncClientToServerClock : public CCET_Base
{
public:
	CCET_SyncClientToServerClock() {}

	tukk                 GetName() { return "SyncClientToServerClock"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CDrxAction* pFramework = CDrxAction::GetDrxAction();
		if (CGameClientNub* pClientNub = pFramework->GetGameClientNub())
		{
			CGameClientChannel* pChannel = pClientNub->GetGameClientChannel();
			if (pChannel)
			{
				switch (pChannel->GetClock().GetSyncState())
				{
				case CClientClock::eSS_NotDone:
					{
						pChannel->GetClock().StartSync();
					}
					break;
				case CClientClock::eSS_Done:
					{
						return eCETR_Ok;
					}
					break;
				}
			}
		}
		return eCETR_Wait;
	}
};

void AddClientTimeSync(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_SyncClientToServerClock());
}

#else

void AddClientTimeSync(IContextEstablisher* pEst, EContextViewState state)
{
}

#endif
