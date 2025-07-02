// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 15:9:2004   10:30 : Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameObjectSystem.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/GameRulesSystem.h>
#include <drx3D/Act/GameServerNub.h>

#include <list>

#define GAMERULES_GLOBAL_VARIABLE   ("g_gameRules")
#define GAMERULESID_GLOBAL_VARIABLE ("g_gameRulesId")

//------------------------------------------------------------------------
CGameRulesSystem::CGameRulesSystem(ISystem* pSystem, IGameFramework* pGameFW)
	: m_pGameFW(pGameFW),
	m_pGameRules(0),
	m_currentGameRules(0)
{
	pGameFW->AddNetworkedClientListener(*this);
}

//------------------------------------------------------------------------
CGameRulesSystem::~CGameRulesSystem()
{
	m_pGameFW->RemoveNetworkedClientListener(*this);
}

//------------------------------------------------------------------------
bool CGameRulesSystem::RegisterGameRules(tukk rulesName, tukk extensionName, bool bUseScript)
{
	IEntityClassRegistry::SEntityClassDesc ruleClass;

	char scriptName[1024];
	if (bUseScript)
	{
		drx_sprintf(scriptName, "Scripts/GameRules/%s.lua", rulesName);
		ruleClass.sScriptFile = scriptName;
	}

	ruleClass.sName = rulesName;
	ruleClass.pUserProxyCreateFunc = CreateGameObject;
	ruleClass.pUserProxyData = this;
	ruleClass.flags |= ECLF_INVISIBLE;

	gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(ruleClass);

	std::pair<TGameRulesMap::iterator, bool> rit = m_GameRules.insert(TGameRulesMap::value_type(rulesName, SGameRulesDef()));
	rit.first->second.extension = extensionName;

	// Automatically register scheduling profile
	gEnv->pGameFramework->GetIGameObjectSystem()->RegisterSchedulingProfile(ruleClass.sName, "rule", nullptr);

	return true;
}

//------------------------------------------------------------------------
bool CGameRulesSystem::CreateGameRules(tukk rulesName)
{
	tukk name = GetGameRulesName(rulesName);
	TGameRulesMap::iterator it = m_GameRules.find(name);
	if (it == m_GameRules.end())
		return false;

	// If a rule is currently being used, ask the entity system to remove it
	DestroyGameRules();

	SEntitySpawnParams params;

	params.sName = "GameRules";
	params.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);
	params.nFlags |= ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_UNREMOVABLE;
	params.id = 1;

	IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(params);
	DRX_ASSERT(pEntity);

	if (pEntity == NULL)
		return false;

	// Make sure game rules is activated
	IGameObject* pGameObject = gEnv->pGameFramework->GetGameObject(pEntity->GetId());
	pGameObject->ForceUpdate(true);

	if (pEntity->GetScriptTable())
	{
		IScriptSystem* pSS = gEnv->pScriptSystem;

		pSS->SetGlobalValue(GAMERULES_GLOBAL_VARIABLE, pEntity->GetScriptTable());
		pSS->SetGlobalValue(GAMERULESID_GLOBAL_VARIABLE, ScriptHandle((UINT_PTR)m_currentGameRules));
	}

	//since we re-instantiating game rules, let's get rid of everything related to previous match

	if (IGameplayRecorder* pGameplayRecorder = CDrxAction::GetDrxAction()->GetIGameplayRecorder())
		pGameplayRecorder->Event(pEntity, GameplayEvent(eGE_GameReset));

	if (gEnv->bServer)
	{
		if (CGameServerNub* pGameServerNub = CDrxAction::GetDrxAction()->GetGameServerNub())
			pGameServerNub->ResetOnHoldChannels();
	}

	return true;
}

//------------------------------------------------------------------------
bool CGameRulesSystem::DestroyGameRules()
{
	// If a rule is currently being used, ask the entity system to remove it
	if (m_currentGameRules)
	{
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_currentGameRules);
		if (pEntity)
			pEntity->ClearFlags(ENTITY_FLAG_UNREMOVABLE);

		gEnv->pEntitySystem->RemoveEntity(m_currentGameRules, true);
		SetCurrentGameRules(0);

		IScriptSystem* pSS = gEnv->pScriptSystem;

		pSS->SetGlobalToNull(GAMERULES_GLOBAL_VARIABLE);
		pSS->SetGlobalToNull(GAMERULESID_GLOBAL_VARIABLE);
	}

	return true;
}

//------------------------------------------------------------------------
bool CGameRulesSystem::HaveGameRules(tukk rulesName)
{
	tukk name = GetGameRulesName(rulesName);
	if (!name || !gEnv->pEntitySystem->GetClassRegistry()->FindClass(name))
		return false;

	if (m_GameRules.find(name) == m_GameRules.end())
		return false;

	return true;
}

//------------------------------------------------------------------------
void CGameRulesSystem::AddGameRulesAlias(tukk gamerules, tukk alias)
{
	if (SGameRulesDef* def = GetGameRulesDef(gamerules))
		def->aliases.push_back(alias);
}

//------------------------------------------------------------------------
void CGameRulesSystem::AddGameRulesLevelLocation(tukk gamerules, tukk mapLocation)
{
	if (SGameRulesDef* def = GetGameRulesDef(gamerules))
		def->maplocs.push_back(mapLocation);
}

//------------------------------------------------------------------------
tukk CGameRulesSystem::GetGameRulesLevelLocation(tukk gamerules, i32 i)
{
	if (SGameRulesDef* def = GetGameRulesDef(GetGameRulesName(gamerules)))
	{
		if (i >= 0 && i < def->maplocs.size())
			return def->maplocs[i].c_str();
	}
	return 0;
}

//------------------------------------------------------------------------
void CGameRulesSystem::SetCurrentGameRules(IGameRules* pGameRules)
{
	m_pGameRules = pGameRules;

	m_currentGameRules = m_pGameRules ? m_pGameRules->GetEntityId() : 0;
}

//------------------------------------------------------------------------
IGameRules* CGameRulesSystem::GetCurrentGameRules() const
{
	return m_pGameRules;
}

//------------------------------------------------------------------------
tukk CGameRulesSystem::GetGameRulesName(tukk alias) const
{
	for (TGameRulesMap::const_iterator it = m_GameRules.begin(); it != m_GameRules.end(); ++it)
	{
		if (!stricmp(it->first.c_str(), alias))
			return it->first.c_str();

		for (std::vector<string>::const_iterator ait = it->second.aliases.begin(); ait != it->second.aliases.end(); ++ait)
		{
			if (!stricmp(ait->c_str(), alias))
				return it->first.c_str();
		}
	}

	return 0;
}

//------------------------------------------------------------------------
/*
   string& CGameRulesSystem::GetCurrentGameRules()
   {
   return
   }
 */

CGameRulesSystem::SGameRulesDef* CGameRulesSystem::GetGameRulesDef(tukk name)
{
	TGameRulesMap::iterator it = m_GameRules.find(name);
	if (it == m_GameRules.end())
		return 0;

	return &it->second;
}

//------------------------------------------------------------------------
IEntityComponent* CGameRulesSystem::CreateGameObject(IEntity* pEntity, SEntitySpawnParams& params, uk pUserData)
{
	CGameRulesSystem* pThis = static_cast<CGameRulesSystem*>(pUserData);
	DRX_ASSERT(pThis);
	TGameRulesMap::iterator it = pThis->m_GameRules.find(params.pClass->GetName());
	DRX_ASSERT(it != pThis->m_GameRules.end());

	auto pGameObject = pEntity->GetOrCreateComponentClass<CGameObject>();

	if (!it->second.extension.empty())
	{
		if (!pGameObject->ActivateExtension(it->second.extension.c_str()))
		{
			pEntity->RemoveComponent(pGameObject);
			return nullptr;
		}
	}

	return pGameObject;
}

void CGameRulesSystem::GetMemoryStatistics(IDrxSizer* s)
{
	s->Add(*this);
	s->AddContainer(m_GameRules);
}

void CGameRulesSystem::OnLocalClientDisconnected(EDisconnectionCause cause, tukk description)
{
	if (m_pGameRules != nullptr)
	{
		m_pGameRules->OnDisconnect(cause, description);
	}
}

bool CGameRulesSystem::OnClientConnectionReceived(i32 channelId, bool bIsReset)
{
	if (m_pGameRules != nullptr)
	{
		m_pGameRules->OnClientConnect(channelId, bIsReset);
	}

	return true;
}

bool CGameRulesSystem::OnClientReadyForGameplay(i32 channelId, bool bIsReset)
{
	if (m_pGameRules != nullptr)
	{
		m_pGameRules->OnClientEnteredGame(channelId, bIsReset);
	}

	return true;
}

void CGameRulesSystem::OnClientDisconnected(i32 channelId, EDisconnectionCause cause, tukk description, bool bKeepClient)
{
	if (m_pGameRules != nullptr)
	{
		m_pGameRules->OnClientDisconnect(channelId, cause, description, bKeepClient);
	}
}

bool CGameRulesSystem::OnClientTimingOut(i32 channelId, EDisconnectionCause cause, tukk description)
{
	if (m_pGameRules != nullptr && m_pGameRules->ShouldKeepClient(channelId, cause, description))
	{
		return false;
	}

	return true;
}