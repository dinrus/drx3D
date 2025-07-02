// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UIUpr.cpp
//  Version:     v1.00
//  Created:     08/8/2011 by Paul Reindell.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/UIUpr.h>
#include <drx3D/Game/UIMenuEvents.h>

#include <drx3D/CoreX/Game/IGame.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Sys/Scaleform/IFlashUI.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/WarningsUpr.h>
#include <drx3D/Game/ProfileOptions.h>
#include <drx3D/Game/UICVars.h>
#include <drx3D/Game/Utils/ScreenLayoutUpr.h>
#include <drx3D/Game/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/HUD/HUDSilhouettes.h>
#include <drx3D/Game/HUD/HUDMissionObjectiveSystem.h>
#include <drx3D/Game/Graphics/2DRenderUtils.h>
#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/UIInput.h>

IUIEventSystemFactory* IUIEventSystemFactory::s_pFirst = NULL;
IUIEventSystemFactory* IUIEventSystemFactory::s_pLast;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// CTor/DTor ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CUIUpr::CUIUpr()
	: m_bRegistered(false)
	, m_controlSchemeListeners(1)
	, m_pWarningUpr(NULL)
	, m_pOptions(NULL)
	, m_p2DRendUtils(NULL)
	, m_pScreenLayoutMan(NULL)
	, m_pHudSilhouettes(NULL)
	, m_pCVars(NULL)
	, m_pMOSystem(NULL)
	, m_soundListener(INVALID_ENTITYID)
	, m_curControlScheme(eControlScheme_NotSpecified)
{
}

/////////////////////////////////////////////////////////////////////////////////////
CUIUpr::~CUIUpr()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::Init()
{
	CHUDEventDispatcher::SetUpEventListener();
	SHUDEvent::InitDataStack();

	m_pWarningUpr = new CWarningsUpr();
	m_pOptions = new CProfileOptions();
	m_pScreenLayoutMan = new ScreenLayoutUpr();
	m_p2DRendUtils = new C2DRenderUtils(m_pScreenLayoutMan);
	m_pHudSilhouettes = new CHUDSilhouettes();
	m_pCVars = new CUICVars();
	m_pMOSystem = new CHUDMissionObjectiveSystem();

	m_pCVars->RegisterConsoleCommandsAndVars();
	
	IUIEventSystemFactory* pFactory = IUIEventSystemFactory::GetFirst();
	while (pFactory)
	{
		TUIEventSystemPtr pGameEvent = pFactory->Create();
		DRX_ASSERT_MESSAGE(pGameEvent != NULL, "Invalid IUIEventSystemFactory!");
		tukk name = pGameEvent->GetTypeName();
		TUIEventSystems::const_iterator it = m_EventSystems.find(name);
		if(it == m_EventSystems.end())
		{
			m_EventSystems[name] = pGameEvent;
		}
		else
		{
			string str;
			str.Format("IUIGameEventSystem \"%s\" already exists!", name);
			DRX_ASSERT_MESSAGE(false, str.c_str());
		}
		pFactory = pFactory->GetNext();
	}

	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->InitEventSystem();
	}

	InitSound();

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener( this );
	g_pGame->GetIGameFramework()->RegisterListener(this, "CUIUpr", FRAMEWORKLISTENERPRIORITY_HUD);
	m_bRegistered = true;
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::Shutdown()
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
		it->second->UnloadEventSystem();
	m_EventSystems.clear();

	if (m_bRegistered)
	{
		if (gEnv->pSystem && gEnv->pSystem->GetISystemEventDispatcher())
			gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener( this );
		if (g_pGame && g_pGame->GetIGameFramework())
			g_pGame->GetIGameFramework()->UnregisterListener(this);
	}

	SAFE_DELETE(m_pWarningUpr);
	SAFE_DELETE(m_pOptions);
	SAFE_DELETE(m_p2DRendUtils);
	SAFE_DELETE(m_pScreenLayoutMan);
	SAFE_DELETE(m_pHudSilhouettes);
	SAFE_DELETE(m_pCVars);
	SAFE_DELETE(m_pMOSystem);
	
	m_bRegistered = false;	

	CUIInput::Shutdown();
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::GetMemoryUsage( IDrxSizer *pSizer ) const
{
	SIZER_SUBCOMPONENT_NAME(pSizer, "CUIUpr");
	pSizer->AddObject(this, sizeof(*this));

	pSizer->Add( *m_pWarningUpr );
	pSizer->Add( *m_pOptions );
	pSizer->Add( *m_p2DRendUtils );
	pSizer->AddObject( m_pScreenLayoutMan );
	pSizer->AddObject( m_pHudSilhouettes );
	pSizer->AddObject( m_pCVars );
	pSizer->AddObject( m_pMOSystem );
	
	// TODO
// 	TUIEventSystems::const_iterator it = m_EventSystems.begin();
// 	TUIEventSystems::const_iterator end = m_EventSystems.end();
// 	for (;it != end; ++it)
// 		it->second->GetMemoryUsage(pSizer);

}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::InitGameType(bool multiplayer, bool fromInit)
{
	// TODO?
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::PostSerialize()
{
	// TODO?
}

/////////////////////////////////////////////////////////////////////////////////////
IUIGameEventSystem* CUIUpr::GetUIEventSystem(tukk type) const
{
	TUIEventSystems::const_iterator it = m_EventSystems.find(type);
	assert(it != m_EventSystems.end());
	return it != m_EventSystems.end() ? it->second.get() : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::OnPostUpdate(float fDeltaTime)
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->OnUpdate(fDeltaTime);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ProcessViewParams(const SViewParams &viewParams)
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->UpdateView(viewParams);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
bool CUIUpr::RegisterControlSchemeListener(IUIControlSchemeListener* pListener)
{
	return m_controlSchemeListeners.Add(pListener);
}

/////////////////////////////////////////////////////////////////////////////////////
bool CUIUpr::UnregisterControlSchemeListener(IUIControlSchemeListener* pListener)
{
	if (m_controlSchemeListeners.Contains(pListener))
	{
		m_controlSchemeListeners.Remove(pListener);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ClearControlSchemeListeners()
{
	m_controlSchemeListeners.Clear();
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::SetDefaultControlScheme()
{
	EControlScheme defaultControlScheme;

#if DRX_PLATFORM_ORBIS
	defaultControlScheme = eControlScheme_PS4Controller;
#elif DRX_PLATFORM_DURANGO
	defaultControlScheme = eControlScheme_XBoxOneController;
#else
	defaultControlScheme = eControlScheme_Keyboard;
#endif

	SetCurControlScheme(defaultControlScheme);
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::SetCurControlScheme( const EControlScheme controlScheme )
{
	if (GetCurControlScheme() == controlScheme)
		return;

	const EControlScheme prevControlScheme = m_curControlScheme;
	m_curControlScheme = controlScheme;

	SHUDEvent hudEvent(eHUDEvent_OnControlSchemeSwitch);
	hudEvent.AddData(SHUDEventData((i32)controlScheme));
	hudEvent.AddData(SHUDEventData((i32)prevControlScheme));
	CHUDEventDispatcher::CallEvent(hudEvent);

	// Notify listeners (Msg3D entities use this currently)
	for (TUIControlSchemeListeners::Notifier notifier(m_controlSchemeListeners); notifier.IsValid(); notifier.Next())
	{
		bool bHandled = notifier->OnControlSchemeChanged(controlScheme);
		if (bHandled) // Allow blocking
		{
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::OnSystemEvent( ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam )
{

	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
		InitSound();
		break;
	case ESYSTEM_EVENT_LEVEL_LOAD_PREPARE:
		ShutdownSound();
		break;
	case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
	case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
		{
			if (event == ESYSTEM_EVENT_LEVEL_GAMEPLAY_START || wparam == 1)
			{
				ILevelInfo* pLevel = gEnv->pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel();
				if(pLevel)
				{
					m_pMOSystem->LoadLevelObjectives(pLevel->GetPath(), true);
				}
			}
			if (event == ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED)
			{
				// Make sure the menu is ready to be called
				CUIMenuEvents* pMenuEvents = UIEvents::Get<CUIMenuEvents>();
				if (gEnv && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->IsGameStarted() && pMenuEvents && pMenuEvents->IsIngameMenuStarted())
					pMenuEvents->DisplayIngameMenu(false);
			}
		}
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::InitSound()
{
	REINST("UI listener");
	if (m_soundListener == INVALID_ENTITYID)
	{
		//m_soundListener = gEnv->pSoundSystem->CreateListener();
	}
	
	if (m_soundListener != INVALID_ENTITYID)
	{
		/*IAudioListener* const pListener = gEnv->pSoundSystem->GetListener(m_soundListener);

		if (pListener)
		{
			pListener->SetRecordLevel(1.0f);
			pListener->SetActive(true);
		}*/
	}
}

void CUIUpr::ShutdownSound()
{
	/*if (m_soundListener != INVALID_ENTITYID)
	{
	gEnv->pSoundSystem->RemoveListener(m_soundListener);
	m_soundListener = INVALID_ENTITYID;
	}*/
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ActivateState(tukk state)
{

}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ActivateStateImmediate(tukk state)
{

}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ActivateDefaultState()
{

}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ActivateDefaultStateImmediate()
{

}

/////////////////////////////////////////////////////////////////////////////////////
bool CUIUpr::IsLoading()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
bool CUIUpr::IsInMenu()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
bool CUIUpr::IsPreGameDone()
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIUpr::ForceCompletePreGame()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
EGameRulesMissionObjectives SGameRulesMissionObjectiveInfo::GetIconId(tukk nameOrNumber)
{
	i32k nameAsNumber = atoi(nameOrNumber);
	if( INT_MAX != nameAsNumber && INT_MIN != nameAsNumber && 0 != nameAsNumber )
	{// we have a number
		return static_cast<EGameRulesMissionObjectives>(nameAsNumber);
	}	

	return EGRMO_Unknown;
}

