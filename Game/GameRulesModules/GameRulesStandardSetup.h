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

#ifndef _GameRulesStandardSetup_h_
#define _GameRulesStandardSetup_h_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesModules/IGameRulesPlayerSetupModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPickupListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamChangedListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRevivedListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>
#include <drx3D/Game/GameRulesTypes.h>

struct IInventory;

class CGameRulesStandardSetup : public IGameRulesPlayerSetupModule,
								public IGameRulesPickupListener,
								public IGameRulesRoundsListener,
								public IGameRulesTeamChangedListener,
								public IGameRulesRevivedListener,
								public IGameRulesClientConnectionListener
{
protected:
	typedef DrxFixedStringT<32> TFixedString;

public:
	CGameRulesStandardSetup();
	virtual ~CGameRulesStandardSetup();

	// IGameRulesPlayerSetupModule
	virtual void Init(XmlNodeRef xml);
	virtual void PostInit();

	virtual void OnClientConnect(i32 channelId, bool isReset, tukk playerName, bool isSpectator);
	virtual void OnPlayerRevived(EntityId playerId);
	virtual void SetupPlayerTeamSpecifics();
	virtual void SetupRemotePlayerTeamSpecifics(EntityId playerId);
	virtual void SetupAllRemotePlayerTeamSpecifics();

	virtual void SvOnStartNewRound(bool isReset);
	virtual void OnActorJoinedFromSpectate(IActor* pActor, i32 channelId);
	// ~IGameRulesPlayerSetupModule

	// IGameRulesPickupListener
	virtual void OnItemPickedUp(EntityId itemId, EntityId actorId);
	virtual void OnItemDropped(EntityId itemId, EntityId actorId);
	virtual void OnPickupEntityAttached(EntityId entityId, EntityId actorId) {};
	virtual void OnPickupEntityDetached(EntityId entityId, EntityId actorId, bool isOnRemove) {};
	// ~IGameRulesPickupListener

	// IGameRulesRoundsListener
	virtual void OnRoundStart() {}
	virtual void OnRoundEnd() {}
	virtual void OnSuddenDeath() {}
	virtual void ClRoundsNetSerializeReadState(i32 newState, i32 curState);
	virtual void OnRoundAboutToStart() {}
	// ~IGameRulesRoundsListener

	// IGameRulesTeamChangedListener
	virtual void OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId);
	// ~IGameRulesTeamChangedListener

	// IGameRulesRevivedListener
	virtual void EntityRevived(EntityId entityId);
	// ~IGameRulesRevivedListener

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) {};
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId) {};
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) {};
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	void AssignActorToTeam(IActor* pActor, i32 channelId);

protected:
	void SetAmmoCapacity(IInventory *pInventory, tukk pAmmoClass, i32 amount);
	bool IsInIgnoreItemTypeList(const IItem* pItem) const;
	void CallLuaFunc(TFixedString* funcName);
	void CallLuaFunc1e(TFixedString* funcName, EntityId e);

	static i32k MAX_IGNORE_REMOVE_ITEM_CLASSES = 5;
	const IEntityClass *m_itemRemoveIgnoreClasses[MAX_IGNORE_REMOVE_ITEM_CLASSES];

	TFixedString  m_luaSetupPlayerTeamSpecificsFunc;
	TFixedString  m_luaSetupRemotePlayerTeamSpecificsFunc;
	TFixedString  m_luaResetPlayerTeamSpecificsFunc;
	TFixedString  m_luaEquipTeamSpecificsFunc;

	CGameRules*  m_pGameRules;

	i32 m_numIgnoreItems;

	bool m_usesTeamSpecifics;
};

#endif // _GameRulesStandardSetup_h_