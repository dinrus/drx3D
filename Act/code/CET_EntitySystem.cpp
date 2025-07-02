// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CET_EntitySystem.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Network/NetHelpers.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/CoreX/Audio/Dialog/IDialogSystem.h>
#include <drx3D/Act/IMaterialEffects.h>
#include <drx3D/Act/ActionGame.h>

/*
 * Reset entity system
 */

class CCET_EntitySystemReset final : public CCET_Base
{
public:
	CCET_EntitySystemReset(bool skipPlayers, bool skipGameRules) : m_skipPlayers(skipPlayers), m_skipGameRules(skipGameRules) {}

	tukk                 GetName() { return "EntitySystemReset"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CScopedRemoveObjectUnlock unlockRemovals(CDrxAction::GetDrxAction()->GetGameContext());
		if (m_skipGameRules || m_skipPlayers)
		{
			IEntityItPtr i = gEnv->pEntitySystem->GetEntityIterator();

			while (!i->IsEnd())
			{
				IEntity* pEnt = i->Next();

				// skip gamerules
				if (m_skipGameRules)
					if (pEnt->GetId() == CDrxAction::GetDrxAction()->GetIGameRulesSystem()->GetCurrentGameRulesEntity()->GetId())
						continue;

				// skip players
				if (m_skipPlayers)
				{
					IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(pEnt->GetId());
					if (pActor && pActor->IsPlayer())
						continue;
				}

				pEnt->ClearFlags(ENTITY_FLAG_UNREMOVABLE);

				// force remove all other entities
				gEnv->pEntitySystem->RemoveEntity(pEnt->GetId(), true);
			}
		}
		else
		{
			if (!gEnv->pSystem->IsSerializingFile())
				gEnv->pEntitySystem->Reset();
		}

		return eCETR_Ok;
	}

private:
	bool m_skipPlayers;
	bool m_skipGameRules;
};

void AddEntitySystemReset(IContextEstablisher* pEst, EContextViewState state, bool skipPlayers, bool skipGamerules)
{
	pEst->AddTask(state, new CCET_EntitySystemReset(skipPlayers, skipGamerules));
}

/*
 * Random system reset
 */

class CCET_RandomSystemReset final : public CCET_Base
{
public:
	CCET_RandomSystemReset(bool loadingNewLevel) : m_loadingNewLevel(loadingNewLevel) {}

	tukk                 GetName() { return "RandomSystemReset"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		// reset a bunch of subsystems
		gEnv->p3DEngine->ResetParticlesAndDecals();
		gEnv->pGameFramework->ResetBrokenGameObjects();
		gEnv->pPhysicalWorld->ResetDynamicEntities();
		gEnv->pFlowSystem->Reset(false);
		gEnv->pGameFramework->GetIItemSystem()->Reset();
		gEnv->pDialogSystem->Reset(false);
		gEnv->pGameFramework->GetIMaterialEffects()->Reset(false);

		if (gEnv->pAISystem)
		{
			ILevelInfo* pLevel = gEnv->pGameFramework->GetILevelSystem()->GetCurrentLevel();
			if (pLevel)
			{
				gEnv->pAISystem->FlushSystem();

				// Don't waste time reloading the current level's navigation data
				// if we're about to load a new level and throw this data away
				if (!m_loadingNewLevel)
				{
					const ILevelInfo::TGameTypeInfo* pGameTypeInfo = pLevel->GetDefaultGameType();
					tukk const szGameTypeName = pGameTypeInfo ? pGameTypeInfo->name.c_str() : "";
					gEnv->pAISystem->LoadLevelData(pLevel->GetPath(), szGameTypeName);
				}
			}
		}

		//m_pPersistantDebug->Reset();
		return eCETR_Ok;
	}

	bool m_loadingNewLevel;
};

void AddRandomSystemReset(IContextEstablisher* pEst, EContextViewState state, bool loadingNewLevel)
{
	pEst->AddTask(state, new CCET_RandomSystemReset(loadingNewLevel));
}

/*
 * Fake some spawns
 */

class CCET_FakeSpawns final : public CCET_Base
{
public:
	CCET_FakeSpawns(unsigned what) : m_what(what) {}

	tukk                 GetName() { return "FakeSpawns"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		bool allowPlayers = (m_what & eFS_Players) != 0;
		bool allowGameRules = (m_what & eFS_GameRules) != 0;
		bool allowOthers = (m_what & eFS_Others) != 0;
		EntityId gameRulesId = 0;
		if (IEntity* pGameRules = CDrxAction::GetDrxAction()->GetIGameRulesSystem()->GetCurrentGameRulesEntity())
			gameRulesId = pGameRules->GetId();

		// we are in the editor, and that means that there have been entities spawned already
		// that are not bound to the network context... so lets bind them!
		IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
		while (IEntity* pEntity = pIt->Next())
		{
			bool isOther = true;

			bool isPlayer = false;
			IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(pEntity->GetId());
			if (pActor && pActor->IsPlayer())
			{
				isPlayer = true;
				isOther = false;
			}
			if (isPlayer && !allowPlayers)
				continue;

			bool isGameRules = false;
			if (pEntity->GetId() == gameRulesId)
			{
				isGameRules = true;
				isOther = false;
			}
			if (isGameRules && !allowGameRules)
				continue;

			if (isOther && !allowOthers)
				continue;

			CGameObject* pGO = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER);
			if (pGO)
			{
				if (pGO->IsBoundToNetwork())
					pGO->BindToNetwork(eBTNM_Force); // force rebinding
			}

			SEntitySpawnParams fakeParams;
			fakeParams.id = pEntity->GetId();
			fakeParams.nFlags = pEntity->GetFlags();
			fakeParams.pClass = pEntity->GetClass();
			fakeParams.qRotation = pEntity->GetRotation();
			fakeParams.sName = pEntity->GetName();
			fakeParams.vPosition = pEntity->GetPos();
			fakeParams.vScale = pEntity->GetScale();
			CDrxAction::GetDrxAction()->GetGameContext()->OnSpawn(pEntity, fakeParams);
		}
		return eCETR_Ok;
	}

private:
	unsigned m_what;
};

void AddFakeSpawn(IContextEstablisher* pEst, EContextViewState state, unsigned what)
{
	pEst->AddTask(state, new CCET_FakeSpawns(what));
}

/*
 * load entities from the mission file
 */

class CCET_LoadLevelEntities final : public CCET_Base
{
public:
	tukk                 GetName() { return "LoadLevelEntities"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		ILevelInfo* pLevel = CDrxAction::GetDrxAction()->GetILevelSystem()->GetCurrentLevel();
		if (!pLevel)
		{
			GameWarning("levelInfo is null");
			return eCETR_Failed;
		}
		tukk levelName = CDrxAction::GetDrxAction()->GetLevelName();
		if (!levelName)
		{
			GameWarning("levelName is null");
			return eCETR_Failed;
		}

		// delete any pending entities before reserving EntityIds in LoadEntities()
		gEnv->pEntitySystem->DeletePendingEntities();

		string missionXml = pLevel->GetDefaultGameType()->xmlFile;
		string xmlFile = string(pLevel->GetPath()) + "/" + missionXml;

		XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(xmlFile.c_str());

		if (rootNode)
		{
			tukk script = rootNode->getAttr("Script");

			if (script && script[0])
				gEnv->pScriptSystem->ExecuteFile(script, true, true);

			XmlNodeRef objectsNode = rootNode->findChild("Objects");

			if (objectsNode)
				gEnv->pEntitySystem->LoadEntities(objectsNode, false);
		}
		else
			return eCETR_Failed;

		gEnv->pEntitySystem->OnLevelLoaded();

		return eCETR_Ok;
	}
};

void AddLoadLevelEntities(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_LoadLevelEntities);
}

/*
 * send an event
 */

class CCET_EntitySystemEvent final : public CCET_Base, private SEntityEvent
{
public:
	CCET_EntitySystemEvent(const SEntityEvent& evt) : SEntityEvent(evt) {}

	tukk                 GetName() { return "EntitySystemEvent"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		gEnv->pEntitySystem->SendEventToAll(*this);
		return eCETR_Ok;
	}
};

void AddEntitySystemEvent(IContextEstablisher* pEst, EContextViewState state, const SEntityEvent& evt)
{
	pEst->AddTask(state, new CCET_EntitySystemEvent(evt));
}

// Notify gameplay start
class CCET_EntitySystemGameplayStart final : public CCET_Base
{
public:
	tukk                 GetName() { return "EntitySystemGameplayStart"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		gEnv->pEntitySystem->OnLevelGameplayStart();
		return eCETR_Ok;
	}
};

void AddEntitySystemGameplayStart(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_EntitySystemGameplayStart());
}
