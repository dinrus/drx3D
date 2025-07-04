// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Colin Gulliver

*************************************************************************/

#ifndef _GameRulesStandardState_h_
#define _GameRulesStandardState_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesTypes.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>

class CGameRules;
class CPlayer;

class CGameRulesStandardState : public IGameRulesStateModule
{
private:
  typedef std::vector<IGameRulesStateListener*> TGameRulesStateListenersVec;
	typedef DrxFixedStringT<32> TFixedString;

public:
	CGameRulesStandardState();
	virtual ~CGameRulesStandardState();

	// IGameRulesStateModule
	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();

	virtual void OnGameRestart();
	virtual void OnGameReset();
	virtual void OnGameEnd();

	virtual EGR_GameState GetGameState() const;

	virtual void Update(float frameTime);

	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual EPostGameState GetPostGameState() { return m_postGameState; }
	
	virtual bool IsInitialChannelId(i32 channelId) const;
	virtual void OwnClientEnteredGame(const CPlayer& rPlayer);

	virtual bool IsOkToStartRound() const;
	virtual void OnIntroCompleted();
	// ~IGameRulesStateModule

	// IGameRulesStateListener
	void OnGameStart_NotifyListeners();
	void OnGameEnd_NotifyListeners();
	void OnIntroStart_NotifyListeners();
	void OnStateEntered_NotifyListeners(); 
	virtual void AddListener(IGameRulesStateListener * pListener);
	virtual void RemoveListener(IGameRulesStateListener * pListener);

	void StartCountdown(bool start)	{ m_isStarting = start; }
	bool IsInCountdown() const			{ return m_isStarting;  }

protected:
	typedef DrxFixedArray<i32, MAX_PLAYER_LIMIT> TChannelIdArray;

	void ChangeState(EGR_GameState newState);
	void EnterPostGameState(EPostGameState state);
	void Add3DWinningTeamMember();
	bool CheckInitialChannelPlayers();
	
	void CleanUpAbortedIntro();

	static void CmdSetState(IConsoleCmdArgs *pArgs);

	TChannelIdArray m_initialChannelIds;

	TFixedString m_startMatchString;
	TGameRulesStateListenersVec m_listeners;

	CGameRules *m_pGameRules;

	float m_timeInPostGame;
	float m_startTimerOverrideWait;
	float m_timeInCurrentPostGameState;

	EGR_GameState m_state;
	EGR_GameState m_lastReceivedServerState;
	EPostGameState m_postGameState;

	bool m_isStarting;
	bool m_introMessageShown;

	bool m_isWaitingForOverrideTimer;
	bool m_bHaveNotifiedIntroListeners; 
	bool m_bHasShownHighlightReel;
};

#endif // _GameRulesStandardState_h_
