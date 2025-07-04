// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 

	-------------------------------------------------------------------------
	История:
	- 03:09:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesStandardSetup.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/GameRulesModules/IGameRulesTeamsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesSpectatorModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesPlayerStatsModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesObjectivesModule.h>
#include <drx3D/Game/EquipmentLoadout.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>

//------------------------------------------------------------------------
CGameRulesStandardSetup::CGameRulesStandardSetup()
{
	m_pGameRules = nullptr;
	m_usesTeamSpecifics = false;
	m_numIgnoreItems = 0;
	memset(m_itemRemoveIgnoreClasses, 0, sizeof(IEntityClass*) * MAX_IGNORE_REMOVE_ITEM_CLASSES);
}

//------------------------------------------------------------------------
CGameRulesStandardSetup::~CGameRulesStandardSetup()
{
	m_pGameRules->UnRegisterPickupListener(this);

	if (m_usesTeamSpecifics)
	{
		if (gEnv->IsClient())
		{
			CallLuaFunc(&m_luaResetPlayerTeamSpecificsFunc);

			m_pGameRules->UnRegisterRoundsListener(this);
			m_pGameRules->UnRegisterTeamChangedListener(this);
			m_pGameRules->UnRegisterRevivedListener(this);
		}
	}

	if (gEnv->IsClient())
	{
		m_pGameRules->UnRegisterClientConnectionListener(this);
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::Init( XmlNodeRef xml )
{
	m_pGameRules = g_pGame->GetGameRules();
	DRX_ASSERT(m_pGameRules);

	m_usesTeamSpecifics = false;

	i32 numChildren = xml->getChildCount();
	for (i32 i = 0; i < numChildren; ++ i)
	{
		XmlNodeRef xmlChild = xml->getChild(i);

		if (!stricmp(xmlChild->getTag(), "RemoveItemsOnDrop"))
		{
			xmlChild->getAttr("time", g_pGameCVars->g_droppedItemVanishTimer);

			m_numIgnoreItems = xmlChild->getChildCount();
			if (m_numIgnoreItems >= MAX_IGNORE_REMOVE_ITEM_CLASSES)
			{
				DRX_ASSERT_MESSAGE(false, "Too many ignore item classes");
				m_numIgnoreItems = MAX_IGNORE_REMOVE_ITEM_CLASSES - 1;
			}

			for (i32 j = 0; j < m_numIgnoreItems; ++ j)
			{
				XmlNodeRef xmlIgnore = xmlChild->getChild(j);
				if (!stricmp(xmlIgnore->getTag(), "IgnoreEntity"))
				{
					tukk pClassName = 0;
					if (xmlIgnore->getAttr("class", &pClassName))
					{
						const IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pClassName);
						DRX_ASSERT(pClass);
						m_itemRemoveIgnoreClasses[j] = pClass;
					}
				}
			}
		}
		else if (!stricmp(xmlChild->getTag(), "TeamSpecifics"))
		{
			m_usesTeamSpecifics = true;

			m_luaSetupPlayerTeamSpecificsFunc.Format("%s", xmlChild->getAttr("luaSetupPlayerTeamSpecificsFunc"));
			m_luaSetupRemotePlayerTeamSpecificsFunc.Format("%s", xmlChild->getAttr("luaSetupRemotePlayerTeamSpecificsFunc"));
			m_luaResetPlayerTeamSpecificsFunc.Format("%s", xmlChild->getAttr("luaResetPlayerTeamSpecificsFunc"));
			m_luaEquipTeamSpecificsFunc.Format("%s", xmlChild->getAttr("luaEquipTeamSpecificsFunc"));
		}
	}

	m_pGameRules->RegisterPickupListener(this);
	if (m_usesTeamSpecifics)
	{
		if (gEnv->IsClient())
		{
			m_pGameRules->RegisterRoundsListener(this);
			m_pGameRules->RegisterTeamChangedListener(this);
			m_pGameRules->RegisterRevivedListener(this);
		}
	}

	if (gEnv->IsClient())
	{
		m_pGameRules->RegisterClientConnectionListener(this);
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::PostInit()
{

}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::OnClientConnect(i32 channelId, bool isReset, tukk playerName, bool isSpectator)
{
	Vec3 pos(1.f, 1.f, 1.f);		// Default to 1,1,1 because rendering doesn't start until the camera leaves 0,0,0
	Ang3 angles(0.f, 0.f, 0.f);

	IGameRulesSpectatorModule*  specmod = m_pGameRules->GetSpectatorModule();

	IActor *pActor = m_pGameRules->SpawnPlayer(channelId, playerName, "Player", pos, angles);
	if (pActor)
	{
		bool bIntro = false;

		IGameRulesStateModule *pStateModule = m_pGameRules->GetStateModule();
		if (pStateModule)
		{
			bIntro = (pStateModule->GetGameState() == IGameRulesStateModule::EGRS_Intro) && m_pGameRules->IsIntroSequenceRegistered();
		}		

		static_cast<CPlayer*>(pActor)->SetPlayIntro(bIntro);

		if(isSpectator)
		{
			if( specmod )
			{
				specmod->ChangeSpectatorMode( pActor, CActor::eASM_Follow, 0, false );
			}
			static_cast<CActor*>(pActor)->SetSpectatorState(CActor::eASS_SpectatorMode);
		}
		else if (specmod && !bIntro)
		{
			EntityId  spectatorPointId = specmod->GetInterestingSpectatorLocation();
			if (spectatorPointId)
			{
				specmod->ChangeSpectatorMode(pActor, CActor::eASM_Fixed, spectatorPointId, false);
			}
			else
			{
				specmod->ChangeSpectatorMode(pActor, CActor::eASM_Free, 0, false);
			}
		}
#if !defined(_RELEASE)
		else if (bIntro)
		{
			DrxLog("[SPEC] ChangeSpectatorMode() not called as client %s is in intro and not a spectator", playerName);
		}
#endif

		if (!isReset)
		{
			AssignActorToTeam(pActor, channelId);
		}

		// Always give player's a no weapon by default
		if(g_pGameCVars->g_IntroSequencesEnabled && bIntro)
		{
			IItemSystem *pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
			pItemSystem->GiveItem(pActor, "NoWeapon", false, true, false);
		}
	}
	else
	{
		DrxLogAlways("CGameRulesStandardSetup::OnClientConnect(), failed to create player for channel %i (isReset=%i)", channelId, (isReset ? 1 : 0));
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::OnPlayerRevived(EntityId playerId)
{
	CActor *pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(playerId));
	if (pActor)
	{
		DRX_ASSERT_MESSAGE(pActor->IsPlayer(), "Actor that is being spawned is not a player!");
		CPlayer *pPlayer = static_cast<CPlayer*>(pActor);

		bool  luaEquip = (m_usesTeamSpecifics && !m_luaEquipTeamSpecificsFunc.empty());

		static IEntityClass *pNoWeaponClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("NoWeapon");
		static IEntityClass *pPickAndThrowClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("PickAndThrowWeapon");

		EntityId noWeaponId = 0; 
		EntityId PickAndThrowId = 0;

		IInventory *pInventory = pActor->GetInventory();
		if(pInventory)
		{
			noWeaponId = pInventory->GetItemByClass(pNoWeaponClass);
			PickAndThrowId = pInventory->GetItemByClass(pPickAndThrowClass);
		}

		IItemSystem * pItemSystem = g_pGame->GetIGameFramework()->GetIItemSystem();
		if(!noWeaponId)
			 noWeaponId = pItemSystem->GiveItem(pActor, "NoWeapon", false, false, false);

		if(!PickAndThrowId && !g_pGameCVars->g_mpNoEnvironmentalWeapons)
			PickAndThrowId = pItemSystem->GiveItem(pActor, "PickAndThrowWeapon", false, false, false);

		DRX_ASSERT_MESSAGE(pItemSystem->GetItem(noWeaponId), "Failed to find CItem for NoWeapon");
		DRX_ASSERT_MESSAGE(pItemSystem->GetItem(PickAndThrowId) || g_pGameCVars->g_mpNoEnvironmentalWeapons, "Failed to find CItem for PickAndThrowWeapon");

		CEquipmentLoadout *pEquipmentLoadout = g_pGame->GetEquipmentLoadout();
		if (pEquipmentLoadout != NULL && pEquipmentLoadout->SvHasClientEquipmentLoadout(pActor->GetChannelId()))
		{
			pEquipmentLoadout->SvAssignClientEquipmentLoadout(pActor->GetChannelId(), playerId);
		}
		else if (!luaEquip)	// Give a default as no loadout is set yet, and it's not gonna be set by lua either
		{
			pItemSystem->GiveItem(pActor, "SmokeGrenades", false, true, true);
			pItemSystem->GiveItem(pActor, "FragGrenades", false, true, true);
			pItemSystem->GiveItem(pActor, "Pistol", false, true, true);

			EntityId weaponId = pItemSystem->GiveItem(pActor, "Rifle", false, true, true);

			DRX_ASSERT_MESSAGE(pItemSystem->GetItem(weaponId), "Failed to find CItem for Rifle");
		}

		if (luaEquip)
		{
			CallLuaFunc1e(&m_luaEquipTeamSpecificsFunc, playerId);
			//pPlayer->EquipAttachmentsOnCarriedWeapons();
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::SetupPlayerTeamSpecifics()
{
	DRX_ASSERT(gEnv->IsClient());
	DRX_ASSERT(m_usesTeamSpecifics);
	if(const EntityId earlyGameSafeLocalActorId = g_pGame->GetIGameFramework()->GetClientActorId())
	{
		CallLuaFunc1e(&m_luaSetupPlayerTeamSpecificsFunc, earlyGameSafeLocalActorId);
	}

	CHUDEventDispatcher::CallEvent(SHUDEvent(eHUDEvent_SetupPlayerTeamSpecifics));
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::SetupRemotePlayerTeamSpecifics(EntityId playerId)
{
	DRX_ASSERT(m_usesTeamSpecifics);
	DRX_ASSERT(playerId);
	DRX_ASSERT(playerId != g_pGame->GetIGameFramework()->GetClientActorId());

	CallLuaFunc1e(&m_luaSetupRemotePlayerTeamSpecificsFunc, playerId);
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::SetupAllRemotePlayerTeamSpecifics()
{
	const EntityId  clientId = g_pGame->GetIGameFramework()->GetClientActorId();
	IGameRulesPlayerStatsModule*  pPlayStatsMo = m_pGameRules->GetPlayerStatsModule();
	DRX_ASSERT(pPlayStatsMo);
	i32k  numStats = pPlayStatsMo->GetNumPlayerStats();
	for (i32 i=0; i<numStats; i++)
	{
		const SGameRulesPlayerStat*  s = pPlayStatsMo->GetNthPlayerStats(i);
		if (s->playerId != clientId)
		{
			SetupRemotePlayerTeamSpecifics(s->playerId);
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::SetAmmoCapacity( IInventory *pInventory, tukk pAmmoClass, i32 amount )
{
	IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pAmmoClass);
	if (pClass)
	{
		pInventory->SetAmmoCapacity(pClass, amount);
	}
	else
	{
		DrxLog("CGameRulesStandardSetup::SetAmmoCapacity(), failed to find ammo class '%s'", pAmmoClass);
	}
}

//------------------------------------------------------------------------
bool CGameRulesStandardSetup::IsInIgnoreItemTypeList(const IItem* pItem) const
{
	bool found = false;

	if (pItem)
	{
		const IEntityClass *pItemClass = pItem->GetEntity()->GetClass();
		for (i32 i = 0; i < m_numIgnoreItems; ++ i)
		{
			if (m_itemRemoveIgnoreClasses[i] == pItemClass)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::OnItemPickedUp( EntityId itemId, EntityId actorId )
{
	const IItem *pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
	if (!IsInIgnoreItemTypeList(pItem))
	{
		m_pGameRules->AbortEntityRemoval(itemId);
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::OnItemDropped( EntityId itemId, EntityId actorId )
{
	const IItem *pItem = g_pGame->GetIGameFramework()->GetIItemSystem()->GetItem(itemId);
	if (!IsInIgnoreItemTypeList(pItem) && !static_cast<const CItem*>(pItem)->GetStats().brandnew)
	{
		if(!static_cast<const CItem*>(pItem)->IsHeavyWeapon())
		{
			m_pGameRules->ScheduleEntityRemoval(itemId, g_pGameCVars->g_droppedItemVanishTimer, false);
		}
		else
		{
			m_pGameRules->ScheduleEntityRemoval(itemId, g_pGameCVars->g_droppedHeavyWeaponVanishTimer, false);
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::SvOnStartNewRound(bool isReset)
{
	DrxLog("[tlh] @ CGameRulesStandardSetup::SvOnStartNewRound(isReset=%d)", isReset);
	DRX_ASSERT(gEnv->bServer);
	if (gEnv->IsClient())
	{
		if (m_usesTeamSpecifics)
		{
			if (g_pGame->GetIGameFramework()->GetClientActorId())
			{
				SetupPlayerTeamSpecifics();
			}

			SetupAllRemotePlayerTeamSpecifics();
		}
	}
}

//------------------------------------------------------------------------
// IGameRulesRoundsListener
void CGameRulesStandardSetup::ClRoundsNetSerializeReadState(i32 newState, i32 curState)
{
	DrxLog("[tlh] @ CGameRulesStandardSetup::ClRoundsNetSerializeReadState(newState=%d, curState=%d)", newState, curState);
	DRX_ASSERT(!gEnv->bServer);
	DRX_ASSERT(gEnv->IsClient());
	if (m_usesTeamSpecifics)
	{
		DRX_ASSERT(g_pGame->GetIGameFramework()->GetClientActorId());
		IGameRulesRoundsModule*  pRoundsModule = m_pGameRules->GetRoundsModule();
		DRX_ASSERT(pRoundsModule);
		if (pRoundsModule->IsRestartingRound(curState))
		{
			SetupPlayerTeamSpecifics();
			SetupAllRemotePlayerTeamSpecifics();
		}
	}
}

//------------------------------------------------------------------------
// IGameRulesTeamChangedListener
void CGameRulesStandardSetup::OnChangedTeam(EntityId entityId, i32 oldTeamId, i32 newTeamId)
{
	DrxLog("CGameRulesStandardSetup::OnChangedTeam(entityId=%d, oldTeamId=%d, newTeamId=%d)", entityId, oldTeamId, newTeamId);
	DRX_ASSERT(gEnv->IsClient());
	if (m_usesTeamSpecifics)
	{
		if (entityId == g_pGame->GetIGameFramework()->GetClientActorId())
		{
			SetupPlayerTeamSpecifics();
		}
		else
		{
			SetupRemotePlayerTeamSpecifics(entityId);
		}
	}
}

//------------------------------------------------------------------------
// IGameRulesRevivedListener
// [tlh] NOTE this doesn't seem to get called for server player on game start if server player not already joined game (left spectator mode) when starts
void CGameRulesStandardSetup::EntityRevived(EntityId entityId)
{
	DrxLog("[tlh] @ CGameRulesStandardSetup::EntityRevived(entityId=%d)", entityId);
	DRX_ASSERT(gEnv->IsClient());
	if (m_usesTeamSpecifics)
	{
		if (entityId == g_pGame->GetIGameFramework()->GetClientActorId())
		{
			bool  setup = true;
			if (gEnv->bServer)
			{
				IGameRulesRoundsModule*  pRoundsModule = m_pGameRules->GetRoundsModule();
				setup = (!pRoundsModule || !pRoundsModule->IsRestarting());  // ie. don't do this on server if round restarting, because for the server's client the lua func will get called after the restart anyway
			}
			if (setup)
			{
				SetupPlayerTeamSpecifics();
			}
		}
		else
		{
			SetupRemotePlayerTeamSpecifics(entityId);
		}
	}
}

//------------------------------------------------------------------------
// IGameRulesClientConnectionListener
void CGameRulesStandardSetup::OnOwnClientEnteredGame()
{
	DrxLog("[tlh] @ CGameRulesStandardSetup::OnOwnClientEnteredGame()");
	DRX_ASSERT(gEnv->IsClient());
	DRX_ASSERT(g_pGame->GetIGameFramework()->GetClientActorId());

	if (gEnv->bServer)
	{
		if (m_usesTeamSpecifics)
		{
			SetupPlayerTeamSpecifics();
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::CallLuaFunc(TFixedString* funcName)
{
	if (funcName && !funcName->empty())
	{
		IScriptTable*  pTable = m_pGameRules->GetEntity()->GetScriptTable();
		HSCRIPTFUNCTION  func;
		if (pTable != NULL && pTable->GetValue(funcName->c_str(), func))
		{
			IScriptSystem*  pScriptSystem = gEnv->pScriptSystem;
			Script::Call(pScriptSystem, func, pTable);
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesStandardSetup::CallLuaFunc1e(TFixedString* funcName, EntityId e)
{
	if (funcName && !funcName->empty())
	{
		IScriptTable*  pTable = m_pGameRules->GetEntity()->GetScriptTable();
		HSCRIPTFUNCTION  func;
		if (pTable != NULL && pTable->GetValue(funcName->c_str(), func))
		{
			IScriptSystem*  pScriptSystem = gEnv->pScriptSystem;
			Script::Call(pScriptSystem, func, pTable, ScriptHandle(e));
		}
	}
}

void CGameRulesStandardSetup::AssignActorToTeam( IActor* pActor, i32 channelId )
{
	IGameRulesTeamsModule *teamModule = m_pGameRules->GetTeamsModule();
	if (teamModule)
	{
		EntityId playerEntityId = pActor->GetEntityId();
		// We're in a team game, attempt to get team from the lobby first, if this fails then auto assign from the teams module
		i32 teamId = 0;
		if ((g_pGameCVars->g_autoAssignTeams!=0) || (!g_pGame->GetGameLobby()->UseLobbyTeamBalancing()))
		{
			bool isSpectator = static_cast<CActor*>(pActor)->GetSpectatorState() == CActor::eASS_SpectatorMode;
			
			if (!isSpectator)
			{
				CGameLobby *pGameLobby = g_pGame->GetGameLobby();
				if (pGameLobby)
				{
					teamId = pGameLobby->GetTeamByChannelId(channelId);
				}

				if(teamId == 0)
				{
					teamId = teamModule->GetAutoAssignTeamId(playerEntityId);
				}

				if (teamId == 0)
				{
					IGameRulesObjectivesModule *pObjectivesModule = m_pGameRules->GetObjectivesModule();
					if (pObjectivesModule)
					{
						teamId = pObjectivesModule->GetAutoAssignTeamId(channelId);
					}
				}

				m_pGameRules->SetTeam(teamId, playerEntityId);
			}
		}
	}
}

void CGameRulesStandardSetup::OnActorJoinedFromSpectate( IActor* pActor, i32 channelId )
{
	AssignActorToTeam(pActor, channelId);
}
