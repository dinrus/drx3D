// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   UIMultiPlayer.h
//  Version:     v1.00
//  Created:     26/8/2011 by Paul Reindell.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIMultiPlayer_H__
#define __UIMultiPlayer_H__

#include <drx3D/Game/IUIGameEventSystem.h>
#include <drx3D/Sys/Scaleform/IFlashUI.h>
#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>

class CUIMultiPlayer 
	: public IUIGameEventSystem
	, public IUIModule
	, public IGameRulesKillListener
{
public:
	CUIMultiPlayer();

	// IUIGameEventSystem
	UIEVENTSYSTEM( "UIMultiPlayer" );
	virtual void InitEventSystem() override;
	virtual void UnloadEventSystem() override;
	virtual void LoadProfile( IPlayerProfile* pProfile );
	virtual void SaveProfile( IPlayerProfile* pProfile ) const;

	// IUIModule
	virtual void Reset() override;
	// ~IUIModule

	// IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo) override {}
	virtual void OnEntityKilled(const HitInfo &hitInfo) override;
	// ~IGameRulesKillListener

	// UI functions
	void EnteredGame();
	void PlayerJoined(EntityId playerid, const string& name);
	void UpdateScoreBoardItem(EntityId playerid, const string& name, i32 kills, i32 deaths, const string& team);
	void PlayerLeft(EntityId playerid, const string& name);
	void PlayerKilled(EntityId playerid, EntityId shooterid);
	void PlayerRenamed(EntityId playerid, const string& newName);
	void OnChatReceived(EntityId senderId, i32 teamFaction, tukk message);

private:
	// UI events
	void RequestPlayers();
	void GetPlayerName();
	void RequestUpdatedScores();
	void SetPlayerName( const string& newname );
	void ConnectToServer( const string& server );
	void GetServerName();
	void OnSendChatMessage( const string& message );


	void SubmitNewName();
	string GetPlayerNameById( EntityId playerid );
	string GetPlayerTeamById( EntityId playerid );

private:
	enum EUIEvent
	{
		eUIE_EnteredGame,
		eUIE_PlayerJoined,
		eUIE_ScoreBoardItemWasUpdated,
		eUIE_PlayerLeft,
		eUIE_PlayerKilled,
		eUIE_PlayerRenamed,
		eUIE_SendName,
		eUIE_SendServer,
		eUIE_ChatMsgReceived,
	};

	SUIEventReceiverDispatcher<CUIMultiPlayer> m_eventDispatcher;
	SUIEventSenderDispatcher<EUIEvent> m_eventSender;
	IUIEventSystem* m_pUIEvents;
	IUIEventSystem* m_pUIFunctions;

	struct SPlayerInfo
	{
		SPlayerInfo() : name("<UNDEFINED>") {}

		string name;
	};
	typedef std::map<EntityId, SPlayerInfo> TPlayers;
	TPlayers m_Players;

	string m_LocalPlayerName;
	string m_ServerName;
};


#endif // __UIMultiPlayer_H__
