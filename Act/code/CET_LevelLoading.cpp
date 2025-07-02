// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_LevelLoading.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/GameClientChannel.h>
#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

/*
 * Prepare level
 */

class CCET_PrepareLevel : public CCET_Base
{
public:
	tukk                 GetName() { return "PrepareLevel"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CGameContext* pGameContext = CDrxAction::GetDrxAction()->GetGameContext();
		string levelName = pGameContext ? pGameContext->GetLevelName() : "";
		if (levelName.empty())
		{
			//GameWarning("No level name set");
			return eCETR_Wait;
		}
		if (gEnv->IsClient() && !gEnv->bServer)
		{
			DrxLogAlways("============================ PrepareLevel %s ============================", levelName.c_str());

			gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_PREPARE, 0, 0);

			CDrxAction::GetDrxAction()->GetILevelSystem()->PrepareNextLevel(levelName);
		}
		return eCETR_Ok;
	}
};

void AddPrepareLevelLoad(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_PrepareLevel());
}

/*
 * Load a level
 */

class CCET_LoadLevel : public CCET_Base
{
public:
	CCET_LoadLevel() : m_bStarted(false) {}

	tukk                 GetName() { return "LoadLevel"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		ILevelInfo* pILevel = NULL;
		string levelName = CDrxAction::GetDrxAction()->GetGameContext()->GetLevelName();
		if (levelName.empty())
		{
			GameWarning("No level name set");
			return eCETR_Failed;
		}

		CDrxAction* pAction = CDrxAction::GetDrxAction();

		pAction->StartNetworkStallTicker(true);
		GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_START, (UINT_PTR)(levelName.c_str()), 0);
		pILevel = pAction->GetILevelSystem()->LoadLevel(levelName);
		GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_END, 0, 0);
		pAction->StopNetworkStallTicker();

		if (pILevel == NULL)
		{
			GameWarning("Failed to load level: %s", levelName.c_str());
			return eCETR_Failed;
		}
		m_bStarted = true;
		return eCETR_Ok;
	}

	bool m_bStarted;
};

class CLevelLoadingThread : public IThread
{
public:
	enum EState
	{
		eState_Working,
		eState_Failed,
		eState_Succeeded,
	};

	CLevelLoadingThread(ILevelSystem* pLevelSystem, tukk szLevelName) 
		: m_pLevelSystem(pLevelSystem), m_state(eState_Working), m_levelName(szLevelName) 
	{
	}

	virtual void ThreadEntry() override
	{
		const threadID levelLoadingThreadId = DrxGetCurrentThreadId();

		if (gEnv->pRenderer) //Renderer may be unavailable when turned off
		{
			gEnv->pRenderer->SetLevelLoadingThreadId(levelLoadingThreadId);
		}

		const ILevelInfo* pLoadedLevelInfo = m_pLevelSystem->LoadLevel(m_levelName.c_str());
		const bool bResult = (pLoadedLevelInfo != NULL);

		if (gEnv->pRenderer) //Renderer may be unavailable when turned off
		{
			gEnv->pRenderer->SetLevelLoadingThreadId(0);
		}

		m_state = bResult ? eState_Succeeded : eState_Failed;
	}

	EState GetState() const
	{
		return m_state;
	}

private:
	ILevelSystem* m_pLevelSystem;
	 EState m_state;
	const string m_levelName;
};

class CCET_LoadLevelAsync : public CCET_Base
{
public:
	CCET_LoadLevelAsync() : m_bStarted(false), m_pLevelLoadingThread(nullptr) {}

	virtual tukk GetName() override 
	{ 
		return "LoadLevelAsync"; 
	}

	virtual EContextEstablishTaskResult OnStep( SContextEstablishState& state ) override
	{
		CDrxAction* pAction = CDrxAction::GetDrxAction();
		const string levelName = pAction->GetGameContext()->GetLevelName();
		if (!m_bStarted)
		{
			if (levelName.empty())
			{
				GameWarning("No level name set");
				return eCETR_Failed;
			} 

			pAction->StartNetworkStallTicker(true);
			GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_START, (UINT_PTR)(levelName.c_str()), 0);

			ILevelSystem* pLevelSystem = pAction->GetILevelSystem();
			m_pLevelLoadingThread = new CLevelLoadingThread(pLevelSystem, levelName.c_str());
			if (!gEnv->pThreadUpr->SpawnThread(m_pLevelLoadingThread, "LevelLoadingThread"))
			{
				DrxFatalError("Error spawning LevelLoadingThread thread.");
			}

			m_bStarted = true;
		}

		const CLevelLoadingThread::EState threadState = m_pLevelLoadingThread->GetState();
		if(threadState == CLevelLoadingThread::eState_Working)
			return eCETR_Wait;

		gEnv->pThreadUpr->JoinThread(m_pLevelLoadingThread, eJM_Join);
		delete m_pLevelLoadingThread;
		m_pLevelLoadingThread = nullptr;

		GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_END, 0, 0);
		pAction->StopNetworkStallTicker();
		if(threadState == CLevelLoadingThread::eState_Succeeded)
		{
			return eCETR_Ok;
		}
		else
		{
			GameWarning("Failed to load level: %s", levelName.c_str());
			return eCETR_Failed;
		}
	}

	bool m_bStarted;
	CLevelLoadingThread* m_pLevelLoadingThread;
};

void AddLoadLevel( IContextEstablisher * pEst, EContextViewState state, bool ** ppStarted )
{
	const bool bIsEditor = gEnv->IsEditor();
	const bool bIsDedicated = gEnv->IsDedicated();
	const ICVar* pAsyncLoad = gEnv->pConsole->GetCVar("g_asynclevelload");
	const bool bAsyncLoad = pAsyncLoad && pAsyncLoad->GetIVal() > 0;

	if(!bIsEditor && !bIsDedicated && bAsyncLoad)
	{
		CCET_LoadLevelAsync* pLL = new CCET_LoadLevelAsync;
		pEst->AddTask( state, pLL );
		*ppStarted = &pLL->m_bStarted;
	}
	else
	{
		CCET_LoadLevel* pLL = new CCET_LoadLevel;
		pEst->AddTask( state, pLL );
		*ppStarted = &pLL->m_bStarted;
	}
}

/*
 * Loading complete
 */

class CCET_LoadingComplete : public CCET_Base
{
public:
	CCET_LoadingComplete(bool* pStarted) : m_pStarted(pStarted) {}

	tukk                 GetName() { return "LoadingComplete"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		ILevelSystem* pLS = CDrxAction::GetDrxAction()->GetILevelSystem();
		if (pLS->GetCurrentLevel())
		{
			pLS->OnLoadingComplete(pLS->GetCurrentLevel());
			return eCETR_Ok;
		}
		return eCETR_Failed;
	}
	virtual void OnFailLoading(bool hasEntered)
	{
		ILevelSystem* pLS = CDrxAction::GetDrxAction()->GetILevelSystem();
		if (m_pStarted && *m_pStarted && !hasEntered)
			pLS->OnLoadingError(NULL, "context establisher failed");
	}

	bool* m_pStarted;
};

void AddLoadingComplete(IContextEstablisher* pEst, EContextViewState state, bool* pStarted)
{
	pEst->AddTask(state, new CCET_LoadingComplete(pStarted));
}

/*
 * Reset Areas
 */
class CCET_ResetAreas : public CCET_Base
{
public:
	tukk                 GetName() { return "ResetAreas"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		gEnv->pEntitySystem->ResetAreas();
		return eCETR_Ok;
	}
};

void AddResetAreas(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_ResetAreas);
}

/*
 * Action events
 */
class CCET_ActionEvent : public CCET_Base
{
public:
	CCET_ActionEvent(const SActionEvent& evt) : m_evt(evt) {}

	tukk                 GetName() { return "ActionEvent"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CDrxAction::GetDrxAction()->OnActionEvent(m_evt);
		return eCETR_Ok;
	}

private:
	SActionEvent m_evt;
};

void AddActionEvent(IContextEstablisher* pEst, EContextViewState state, const SActionEvent& evt)
{
	pEst->AddTask(state, new CCET_ActionEvent(evt));
}
