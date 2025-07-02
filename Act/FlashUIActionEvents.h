// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUIActionEvents.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUIActionEvents_H__
#define __FlashUIActionEvents_H__

#include <drx3D/Sys/IFlashUI.h>
#include <drx3D/Act/ILevelSystem.h>

class CFlashUIActionEvents
{
public:
	CFlashUIActionEvents();
	~CFlashUIActionEvents();

	// ui events
	void OnSystemStart();
	void OnSystemShutdown();
	void OnLoadingStart(ILevelInfo* pLevel);
	void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount);
	void OnLoadingComplete();
	void OnLoadingError(ILevelInfo* pLevel, tukk error);
	void OnGameplayStarted();
	void OnGameplayEnded();
	void OnLevelUnload();
	void OnUnloadComplete();
	void OnConnect(tukk server);
	void OnDisconnect(tukk error);
	void OnGameplayPaused();
	void OnGameplayResumed();
	void OnReload();

	// ui functions
	void OnUnloadAllElements(const SUIArguments& elements);

private:
	enum EUIEvent
	{
		eUIE_OnSystemStarted,
		eUIE_OnSystemShutdown,

		eUIE_OnLoadingStart,
		eUIE_OnLoadingProgress,
		eUIE_OnLoadingComplete,
		eUIE_OnLoadingError,

		eUIE_OnGameplayStarted,
		eUIE_OnGameplayEnded,

		eUIE_OnUnloadStart,
		eUIE_OnUnloadComplete,

		eUIE_OnConnect,
		eUIE_OnDisconnect,

		eUIE_OnGamePause,
		eUIE_OnGameResume,

		eUIE_OnReload,
	};

	SUIEventReceiverDispatcher<CFlashUIActionEvents> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent>               m_eventSender;

	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	IGameFramework* m_pGameFramework;
	ILevelSystem*   m_pLevelSystem;
};

#endif // #ifndef __FlashUIActionEvents_H__
