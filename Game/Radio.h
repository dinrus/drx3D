// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RADIO_H__
#define __RADIO_H__

#pragma once

#include <drx3D/Game/GameActions.h>
#include <drx3D/Input/IInput.h>

class CGameRules;

class CRadio:public IInputEventListener
{
public:
	CRadio(CGameRules*);
	~CRadio();
	bool OnAction(const ActionId& actionId, i32 activationMode, float value);

	void Update();

	static i32k RADIO_MESSAGE_NUM;

	//from IInputEventListener
	virtual bool	OnInputEvent( const SInputEvent &event );
	void			OnRadioMessage(i32 id, EntityId fromId);
	void			CancelRadio();
	void			SetTeam(const string& name);
	void			GetMemoryStatistics(IDrxSizer * s);
private:
	CGameRules	*m_pGameRules;
	i32			m_currentGroup;
	string		m_TeamName;
	float		m_lastMessageTime;
	float		m_menuOpenTime;		// used to close the menu if no input within n seconds.

	bool	m_keyState[10];
	bool	m_keyIgnored[10];
	i32		m_requestedGroup;
	bool	m_inputEventConsumedKey;
	bool	m_waitForInputEvents;
	bool		UpdatePendingGroup();

	void		CloseRadioMenu();
};

#endif