// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UIDialogs.h
//  Version:     v1.00
//  Created:     22/6/2012 by Paul Reindell.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIDialogs_H__
#define __UIDialogs_H__

#include <drx3D/Game/IUIGameEventSystem.h>
#include <drx3D/Sys/Scaleform/IFlashUI.h>
#include <drx3D/CoreX/Game/IGameFramework.h>

enum EDialogType
{
	eDT_DialogWait = 0,	// displays a wait dialog (no user action)
	eDT_Warning,				// displays warning string
	eDT_Error,					// displays error string
	eDT_AcceptDecline,	// displayes accept/decline dialog
	eDT_Confirm,				// displays confirmation dialog
	eDT_Okay,						// displays ok dialog
	eDT_Input,					// displays input string dialog
};

enum EDialogResponse
{
	eDR_Yes = 0,
	eDR_No,
	eDR_Canceled,
};

struct IDialogCallback
{
	virtual ~IDialogCallback() {}
	virtual void DialogCallback(u32 dialogId, EDialogResponse response, tukk param) = 0;
};

class CUIDialogs
	: public IUIGameEventSystem
{
public:
	CUIDialogs();

	// IUIGameEventSystem
	UIEVENTSYSTEM( "UIDialogs" );
	virtual void InitEventSystem() override;
	virtual void UnloadEventSystem() override;

	// displays dialog
	u32 DisplayDialog(EDialogType type, tukk title, tukk message, tukk paramMessage, IDialogCallback* pListener = NULL);
	void CancelDialog(u32 dialogId);
	void CancelDialogs();

private:
	// ui events
	void OnDialogResult( i32 dialogid, i32 result, tukk message );

private:
	enum EUIEvent
	{
		eUIE_DisplayDialogAsset,
		eUIE_HideDialogAsset,

		eUIE_AddDialogWait,
		eUIE_AddDialogWarning,
		eUIE_AddDialogError,
		eUIE_AddDialogAcceptDecline,
		eUIE_AddDialogConfirm,
		eUIE_AddDialogOkay,
		eUIE_AddDialogInput,

		eUIE_RemoveDialog
	};

	SUIEventReceiverDispatcher<CUIDialogs> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent> m_eventSender;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;
	typedef std::map<u32, IDialogCallback*> TDialogs;
	TDialogs m_dialogs;

	inline u32 GetNextFreeId() const
	{
		u32 id = 0;
		for (TDialogs::const_iterator it = m_dialogs.begin(), end = m_dialogs.end(); it != end && id == it->first; ++it, ++id);
		return id;
	}
};


#endif // __UISettings_H__