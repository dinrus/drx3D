// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameSerialize.h>

#include <drx3D/Act/GameSerializeHelpers.h>

#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/DrxActionCVars.h>

#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Act/DialogSystem.h>
#include <drx3D/Act/MaterialEffects.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/GameServerNub.h>
#include <drx3D/Act/ColorGradientUpr.h>

#ifdef USE_COPYPROTECTION
	#include <drx3D/Act/CopyProtection.h>
#endif

#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Phys/IDeferredCollisionEvent.h>

#include <set>
#include <time.h>
#include <drx3D/Sys/ITimer.h>

//#define EXCESSIVE_ENTITY_DEBUG 1
//#undef  EXCESSIVE_ENTITY_DEBUG

static tukk SAVEGAME_GAMESTATE_SECTION = "GameState";
static tukk SAVEGAME_AISTATE_SECTION = "AIState";
static tukk SAVEGAME_AIOBJECTID_SECTION = "AIObjectReservedIDs";
static tukk SAVEGAME_GAMETOKEN_SECTION = "GameTokens";
static tukk SAVEGAME_TERRAINSTATE_SECTION = "TerrainState";
static tukk SAVEGAME_TIMER_SECTION = "Timer";
static tukk SAVEGAME_FLOWSYSTEM_SECTION = "FlowSystem";
static tukk SAVEGAME_COLORGRADIANTMANAGER_SECTION = "ColorGradientUpr";
static tukk SAVEGAME_DIALOGSYSTEM_SECTION = "DialogSystem";
static tukk SAVEGAME_AUDIOSYSTEM_SECTION = "AudioSystem";
static tukk SAVEGAME_LTLINVENTORY_SECTION = "LTLInventory";
static tukk SAVEGAME_VIEWSYSTEM_SECTION = "ViewSystem";
static tukk SAVEGAME_MATERIALEFFECTS_SECTION = "MatFX";
static tukk SAVEGAME_BUILD_TAG = "build";
static tukk SAVEGAME_TIME_TAG = "saveTime";
static tukk SAVEGAME_VERSION_TAG = "version";
static tukk SAVEGAME_LEVEL_TAG = "level";
static tukk SAVEGAME_GAMERULES_TAG = "gameRules";
static tukk SAVEGAME_CHECKPOINT_TAG = "checkPointName";

//static tukk  SAVEGAME_DEFINITIONFILE_VALUE = "definitionFile";

static i32k SAVEGAME_VERSION_VALUE = 32;

#ifndef _RELEASE
struct CGameSerialize::DebugSaveParts
{
	DebugSaveParts(const TEntitiesToSerialize& entities, const CSaveGameHolder& saveGameHolder)
		: m_entities(entities)
		, m_saveGameHolder(saveGameHolder)
	{
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddContainer(m_entities);
		pSizer->AddObject(m_saveGameHolder.Get());
	}

	const TEntitiesToSerialize& m_entities;
	const CSaveGameHolder&      m_saveGameHolder;
};
#endif

#define CHECKPOINT_OUTPUT true

ILevelInfo* GetLevelInfo()
{
	ILevelSystem* pLevelSystem = CDrxAction::GetDrxAction()->GetILevelSystem();
	assert(pLevelSystem != 0);
	if (pLevelSystem)
	{
		return pLevelSystem->GetCurrentLevel();
	}

	return NULL;
}

bool VerifyEntities(const TBasicEntityDatas& basicEntityData)
{
	DRX_ASSERT(gEnv);
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	DRX_ASSERT(pEntitySystem);

	TBasicEntityDatas::const_iterator itEnd = basicEntityData.end();
	for (TBasicEntityDatas::const_iterator it = basicEntityData.begin(); it != itEnd; ++it)
	{
		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(it->id))
		{
			if (!pEntity->IsGarbage() && !(pEntity->GetFlags() & ENTITY_FLAG_UNREMOVABLE) && (0 != strcmp(it->className, pEntity->GetClass()->GetName())))
			{
				GameWarning("[LoadGame] Entity ID=%d '%s', class mismatch, should be '%s'", pEntity->GetId(), pEntity->GetEntityTextDescription().c_str(), it->className.c_str());
				return false;
			}
		}
	}

	return true;
}

bool SSaveEnvironment::InitSave(tukk name, ESaveGameReason reason)
{
	// initialize serialization method
	if (!m_pSaveGame.PointerOk())
	{
		GameWarning("Invalid serialization method: %s", m_saveMethod);
		return false;
	}
	// TODO: figure correct path?
	if (!m_pSaveGame->Init(name))
	{
		GameWarning("Unable to save to %s", name);
		return false;
	}

	m_pSaveGame->SetSaveGameReason(reason);

	return true;
}

bool SLoadEnvironment::InitLoad(bool requireQuickLoad, tukk path)
{
	if (!m_pLoadGame.PointerOk())
	{
		GameWarning("Failed to initialize loadgame.");
		return false;
	}
	// TODO: figure correct path?
	if (!m_pLoadGame->Init(path))
	{
		GameWarning("Unable to Load to %s", path);
		return false;
	}

	m_checkpoint.Check("Loading&Uncompress");

	// editor always requires quickload
	requireQuickLoad |= gEnv->IsEditor();

	// sanity checks - version, node names, etc
	/*
	   if (0 != strcmp(rootNode->getTag(), SAVEGAME_ROOT_SECTION))
	   {
	   GameWarning( "Not a save game file: %s", path );
	   return false;
	   }
	 */

	i32 version = -1;
	if (!m_pLoadGame->GetMetadata(SAVEGAME_VERSION_TAG, version) || version != SAVEGAME_VERSION_VALUE)
	{
		GameWarning("Invalid save game version in %s; expecting %d got %d", path, SAVEGAME_VERSION_VALUE, version);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
CGameSerialize::CGameSerialize()
{
	if (!gEnv->IsEditor())
	{
		gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnSpawn | IEntitySystem::OnRemove);

		if (ILevelSystem* pLS = CDrxAction::GetDrxAction()->GetILevelSystem())
			pLS->AddListener(this);
	}
}

CGameSerialize::~CGameSerialize()
{
	if (!gEnv->IsEditor())
	{
		gEnv->pEntitySystem->RemoveSink(this);

		if (ILevelSystem* pLS = CDrxAction::GetDrxAction()->GetILevelSystem())
			pLS->RemoveListener(this);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::OnSpawn(IEntity* pEntity, SEntitySpawnParams&)
{
	assert(pEntity);

	if (!gEnv->bMultiplayer && !(pEntity->GetFlags() & ENTITY_FLAG_UNREMOVABLE))
	{
		bool bSerializeEntity = gEnv->pEntitySystem->ShouldSerializedEntity(pEntity);

		if (bSerializeEntity)
		{
			m_dynamicEntities.insert(pEntity->GetId());
		}
	}
}

bool CGameSerialize::OnRemove(IEntity* pEntity)
{
	if (!gEnv->bMultiplayer)
		stl::find_and_erase(m_dynamicEntities, pEntity->GetId());

	// true means allow removal of entity
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::OnLoadingStart(ILevelInfo* pLevelInfo)
{
	assert(pLevelInfo);

	// load the list of entities to serialize from the level pak.
	//	This creates a list of ids which will be fully serialized.

	m_dynamicEntities.clear();
	m_serializeEntities.clear();

	if (pLevelInfo)
	{
		string levelPath = pLevelInfo->GetPath();
		string file = PathUtil::Make(levelPath.c_str(), "Serialize.xml");
		XmlNodeRef node = gEnv->pSystem->LoadXmlFromFile(file.c_str());
		if (node)
		{
			i32 count = node->getChildCount();
			m_serializeEntities.reserve(count);
			for (i32 i = 0; i < count; ++i)
			{
				XmlNodeRef child = node->getChild(i);
				EntityId id = 0;
				child->getAttr("id", id);

				m_serializeEntities.push_back(id);
			}
		}
	}
}

void CGameSerialize::OnUnloadComplete(ILevelInfo* pLevel)
{
	stl::free_container(m_serializeEntities);
	stl::free_container(m_dynamicEntities);
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::RegisterSaveGameFactory(tukk name, SaveGameFactory factory)
{
	m_saveGameFactories[name] = factory;
}

void CGameSerialize::RegisterLoadGameFactory(tukk name, LoadGameFactory factory)
{
	m_loadGameFactories[name] = factory;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::Clean()
{
	gEnv->pEntitySystem->DeletePendingEntities();
}

bool CGameSerialize::IsUserSignedIn(CDrxAction* pDrxAction) const
{
	u32 user;
	return pDrxAction->GetISystem()->GetPlatformOS()->UserIsSignedIn(pDrxAction->GetIPlayerProfileUpr()->GetCurrentUser(), user);
}

#ifndef _RELEASE
void CGameSerialize::DebugPrintMemoryUsage(const DebugSaveParts& debugSaveParts) const
{
	if (CDrxActionCVars::Get().g_debugSaveLoadMemory)
	{
		IDrxSizer* pSizer = GetISystem()->CreateSizer();

		pSizer->AddObject(debugSaveParts);

		const size_t bytes = pSizer->GetTotalSize();
		const size_t kb = bytes / 1024;

		DrxLogAlways("[SaveGame]: Spiked more than %" PRISIZE_T "KB while saving", kb);

		pSizer->Release();
	}
}
#endif

bool CGameSerialize::SaveGame(CDrxAction* pDrxAction, tukk method, tukk name, ESaveGameReason reason, tukk checkPointName)
{
	struct SSaveGameAutoCleanup
	{
		SSaveGameAutoCleanup()
		{
			gEnv->pSystem->SetThreadState(ESubsys_Physics, false);
		}
		~SSaveGameAutoCleanup()
		{
			gEnv->pSystem->SetThreadState(ESubsys_Physics, true);
		}
	};

	// [AlexMcC|31.05.10] Wrap this whole giant scope so we can measure memory usage after freeing local variables
	Checkpoint total(CHECKPOINT_OUTPUT, "TotalTime");
	Checkpoint checkpoint(CHECKPOINT_OUTPUT);

	bool bResult = false;

	if (reason == eSGR_FlowGraph)
	{
		if (pDrxAction->IsInTimeDemo())
			return true; // Ignore checkpoint saving when running time demo
	}

	if (gEnv->IsEditor())
	{
		if (CDrxActionCVars::Get().g_allowSaveLoadInEditor == 0)
		{
			tukk const szSafeName = name ? name : "<UNKNOWN>";
			tukk const szSafeCheckpointName = checkPointName ? checkPointName : "<UNKNOWN>";
			GameWarning("Ignoring editor savegame \"%s\" at checkpoint \"%s\"", szSafeName, szSafeCheckpointName);
			return false;
		}
	}
	else if (!IsUserSignedIn(pDrxAction))
	{
		return false;
	}

	Vec3 camPos = gEnv->pSystem->GetViewCamera().GetPosition();
	MEMSTAT_LABEL_FMT("Savegame_start (pos=%4.2f,%4.2f,%4.2f)", camPos.x, camPos.y, camPos.z);

	Clean();
	checkpoint.Check("Clean");

	checkpoint.Check("Init");

	SSaveEnvironment saveEnvironment(pDrxAction, checkPointName, method, checkpoint, m_saveGameFactories);
	if (!saveEnvironment.InitSave(name, reason))
		return false;

	assert(gEnv->pSystem);
	SSaveGameAutoCleanup autoCleanup;

	//savegame header
	if (!SaveMetaData(saveEnvironment))
	{
		return false;
	}

	// engine and systems
	SaveEngineSystems(saveEnvironment);

	// now the primary game stuff
	if (!SaveEntities(saveEnvironment))
	{
		return false;
	}

	// AI final serialize (needs to be after both the AI and entity serialization)
	if (gEnv->pAISystem)
		gEnv->pAISystem->SerializeObjectIDs(saveEnvironment.m_pSaveGame->AddSection(SAVEGAME_AIOBJECTID_SECTION));
	checkpoint.Check("AI_FinalSerialize");

	Clean();
	checkpoint.Check("Clean2");

	// final serialization
	MEMSTAT_LABEL("Savegame_flush");
	bResult = saveEnvironment.m_pSaveGame.Complete();
	checkpoint.Check("Writing");

	MEMSTAT_LABEL("Savegame_end");
	checkpoint.Check("Flushed memory");
	return bResult;
}

// helper function to position/rotate/etc.. all entities that currently
// exist in the entity system based on current game state
bool CGameSerialize::RepositionEntities(const TBasicEntityDatas& basicEntityData, bool insistOnAllEntitiesBeingThere)
{
	bool bErrors = false;
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	IEntityClass* pClothEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Cloth");

	TBasicEntityDatas::const_iterator iter = basicEntityData.begin();
	TBasicEntityDatas::const_iterator end = basicEntityData.end();
	for (; iter != end; ++iter)
	{
		if (iter->ignorePosRotScl || iter->iPhysType == PE_SOFT)
			continue;

		IEntity* pEntity = pEntitySystem->GetEntity(iter->id);
		if (pEntity)
		{
			// TODO: find a more generic way to do this. The basic problem is that we can not call SetPosRotScale atwill for clothes, because then the transformation is stacked for the verts.
			//       the iPhysType==PE_SOFT check does not always prevent this from happening, because cloth entities destroy their physical entity when hiden.
			//       so, if a cloth was hiden when the savegame was created, iPhysType will be NONE, and then SetPos will be called here on loading, and the cloth will look wrong (usually with accumulated rotation)
			if (pEntity->GetClass() == pClothEntityClass)
				continue;

			if (iter->parentEntity)
			{
				IEntity* pParentEntity = pEntitySystem->GetEntity(iter->parentEntity);
				if (!pParentEntity && insistOnAllEntitiesBeingThere)
				{
					GameWarning("[LoadGame] Missing Entity with ID=%d, probably was removed during Serialization.", iter->parentEntity);
					bErrors = true;
					continue;
				}
				else if (pParentEntity)
				{
					IEntity* pCurrentParentEntity = pEntity->GetParent();
					if (pCurrentParentEntity != pParentEntity)
					{
						if (pCurrentParentEntity)
							pEntity->DetachThis();
						pParentEntity->AttachChild(pEntity);
					}
				}
			}
			else
			{
				if (pEntity->GetParent())
					pEntity->DetachThis();
			}
			pEntity->SetPosRotScale(iter->pos, iter->rot, iter->scale);
		}
		else if (insistOnAllEntitiesBeingThere)
		{
			GameWarning("[LoadGame] Missing Entity ID=%d", iter->id);
			bErrors = true;
		}
	}

	return !bErrors;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::ReserveEntityIds(const TBasicEntityDatas& basicEntityData)
{
	//////////////////////////////////////////////////////////////////////////
	TBasicEntityDatas::const_iterator iter = basicEntityData.begin();
	TBasicEntityDatas::const_iterator end = basicEntityData.end();
	for (; iter != end; ++iter)
	{
		const IEntity* pEntity = gEnv->pEntitySystem->GetEntity(iter->id);
		if (!pEntity)
		{
			// for existing entities this will set the salt to 0!
			gEnv->pEntitySystem->ReserveEntityId(iter->id);
#ifdef EXCESSIVE_ENTITY_DEBUG
			EntityId id = iter->id;
			DrxLogAlways("> ReserveEntityId: ID=%d HexID=%X", id, id);
#endif
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::FlushActivatableGameObjectExtensions()
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();

	pIt->MoveFirst();
	while (!pIt->IsEnd())
	{
		const IEntity* pEntity = pIt->Next();
		CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER);
		if (pGameObject)
			pGameObject->FlushActivatableExtensions();
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::DeleteDynamicEntities(const TBasicEntityDatas& basicEntityData)
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

#ifdef EXCESSIVE_ENTITY_DEBUG
	// dump entities which can potentially be re-used (if class matches)
	DrxLogAlways("*QL -----------------------------------------");
	DrxLogAlways("*QL DeleteDynamicEntities");
#endif

	IEntityItPtr pIt = pEntitySystem->GetEntityIterator();
	SBasicEntityData tempSearchEntity;
	//////////////////////////////////////////////////////////////////////////
	// Delete all entities except the unremovable ones and the local player.
	pIt->MoveFirst();
	while (!pIt->IsEnd())
	{
		IEntity* pEntity = pIt->Next();
		u32 nEntityFlags = pEntity->GetFlags();

		// Local player must not be deleted.
		if (nEntityFlags & ENTITY_FLAG_LOCAL_PLAYER)
			continue;

		if (nEntityFlags & ENTITY_FLAG_UNREMOVABLE)
		{
#ifdef EXCESSIVE_ENTITY_DEBUG
			DrxLogAlways(">Unremovable Entity ID=%d Name='%s'", pEntity->GetId(), pEntity->GetEntityTextDescription().c_str());
#endif

			tempSearchEntity.id = pEntity->GetId();
			if (pEntity->IsGarbage() && (stl::binary_find(basicEntityData.begin(), basicEntityData.end(), tempSearchEntity) != basicEntityData.end()))
			{
				// Restore disabled, un-removable entity.
				SEntityEvent event;
				event.event = ENTITY_EVENT_INIT;
				pEntity->SendEvent(event);
			}
		}
		else
		{
#ifdef EXCESSIVE_ENTITY_DEBUG
			DrxLogAlways(">Removing Entity ID=%d Name='%s'", pEntity->GetId(), pEntity->GetEntityTextDescription().c_str());
#endif

			pEntity->ResetKeepAliveCounter();
			pEntitySystem->RemoveEntity(pEntity->GetId());
		}
	}
	// Force deletion of removed entities.
	pEntitySystem->DeletePendingEntities();
	//////////////////////////////////////////////////////////////////////////

#ifdef EXCESSIVE_ENTITY_DEBUG
	// dump entities which can potentially be re-used (if class matches)
	DrxLogAlways("*QL DeleteDynamicEntities Done");
	DrxLogAlways("*QL -----------------------------------------");
#endif
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::DumpEntities()
{
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	pIt->MoveFirst();
	while (!pIt->IsEnd())
	{
		IEntity* pEntity = pIt->Next();
		if (pEntity)
		{
			DrxLogAlways("ID=%u Name='%s'", pEntity->GetId(), pEntity->GetEntityTextDescription().c_str());
		}
		else
		{
			DrxLogAlways("Invalid NULL entity encountered.");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// TODO: split maybe into some separate classes...
ELoadGameResult CGameSerialize::LoadGame(CDrxAction* pDrxAction, tukk method, tukk path, SGameStartParams& startParams, bool requireQuickLoad)
{
	struct SLoadGameAutoCleanup
	{
		SLoadGameAutoCleanup(bool _pauseStreaming)
			: pauseStreaming(_pauseStreaming)
		{
			if (pauseStreaming)
			{
				// Pause streaming engine for anything but sound, video and flash.
				u32 nMask = (1 << eStreamTaskTypeFlash) | (1 << eStreamTaskTypeVideo) | STREAM_TASK_TYPE_AUDIO_ALL; // Unblock specified streams
				nMask = ~nMask;                                                                                        // Invert mask, bit set means blocking type.
				GetISystem()->GetStreamEngine()->PauseStreaming(true, nMask);
			}

			gEnv->pSystem->SetThreadState(ESubsys_Physics, false);
		}
		~SLoadGameAutoCleanup()
		{
			if (pauseStreaming)
			{
				GetISystem()->GetStreamEngine()->PauseStreaming(false, -1);
			}

			gEnv->pSystem->SetThreadState(ESubsys_Physics, true);
		}
		bool pauseStreaming;
	} autoCleanup(requireQuickLoad);

	if (gEnv->IsEditor())
	{
		if (CDrxActionCVars::Get().g_allowSaveLoadInEditor == 0)
		{
			tukk const szSafeName = path ? path : "<UNKNOWN>";
			GameWarning("Ignoring editor loadgame \"%s\"", szSafeName);
			return eLGR_Failed;
		}
	}
	else if (!IsUserSignedIn(pDrxAction))
	{
		return eLGR_Failed;
	}

	DrxGetIMemReplay()->AddLabel("Loadgame_start");
	Checkpoint total(CHECKPOINT_OUTPUT, "TotalTime");
	Checkpoint checkpoint(CHECKPOINT_OUTPUT);

	// stop all deferred physics events in the fly
	if (gEnv->p3DEngine)
	{
		IDeferredPhysicsEventUpr* pDeferredPhysicsEventUpr = gEnv->p3DEngine->GetDeferredPhysicsEventUpr();
		if (pDeferredPhysicsEventUpr)
			pDeferredPhysicsEventUpr->ClearDeferredEvents();
	}

	Clean();

	// need to leave the player entity in the list, else it won't get
	//	saved after this point (it isn't deleted/respawned)
	m_dynamicEntities.clear();
	m_dynamicEntities.insert(CDrxAction::GetDrxAction()->GetClientActorId());
	checkpoint.Check("Clean");

	//////////////////////////////////////////////////////////////////////////
	// Lock geometries to not be released during game loading.
	STempAutoResourcesLock autoResourcesLock;
	//////////////////////////////////////////////////////////////////////////

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	//create load environment
	SLoadEnvironment loadEnvironment(pDrxAction, checkpoint, method, m_loadGameFactories);
	if (!loadEnvironment.InitLoad(requireQuickLoad, path)) // the savegame file is actually loaded into memory in this call.
	{
		return eLGR_Failed;
	}

	tukk levelName = loadEnvironment.m_pLoadGame->GetMetadata(SAVEGAME_LEVEL_TAG);
	pDrxAction->NotifySavegameFileLoadedToListeners(levelName);
	TBasicEntityDatas& basicEntityData = loadEnvironment.m_basicEntityData;

	// compare the build numbers; output a warning if different to notify that there might be
	//	issues (if level or code have changed).
	//	Ideally we'd refuse to load saves from another build but that won't work for post-release patching.
	tukk buildName = loadEnvironment.m_pLoadGame->GetMetadata(SAVEGAME_BUILD_TAG);
	const SFileVersion& fileVersion = GetISystem()->GetFileVersion();
	char tmpbuf[128];
	fileVersion.ToString(tmpbuf);
	if (strcmp(tmpbuf, buildName))
	{
		GameWarning("Loading a save from a different build, may result in errors. Save = %s; Current = %s", buildName, tmpbuf);
	}

	// also output the checkpoint name if available - helps with Resume bugs
	tukk checkpointName = loadEnvironment.m_pLoadGame->GetMetadata(SAVEGAME_CHECKPOINT_TAG);
	if (checkpointName && checkpointName[0])
	{
		DrxLog("Loading checkpoint %s", checkpointName);
	}

	tukk sCurrentLevel = pDrxAction->GetLevelName();
	const bool bIsQuickLoading = sCurrentLevel != 0 && (stricmp(sCurrentLevel, levelName) == 0);
	if (requireQuickLoad && !bIsQuickLoading)
	{
		GameWarning("Unable to quick load: different level names (%s != %s)", sCurrentLevel, levelName);
		return eLGR_CantQuick_NeedFullLoad;
	}

	// basic game state loading... need to do it early to verify and reserve entity id's
	std::unique_ptr<TSerialize> pGameStateSer(loadEnvironment.m_pLoadGame->GetSection(SAVEGAME_GAMESTATE_SECTION));
	if (!pGameStateSer.get())
	{
		return eLGR_Failed;
	}

	{
		pGameStateSer->BeginGroup("BasicEntityData");
		i32 size = 0;
		pGameStateSer->Value("BasicEntityDataSize", size);
		basicEntityData.resize(size);
		for (i32 i = 0; i < size; ++i)
		{
			pGameStateSer->BeginGroup("BasicEntity");
			basicEntityData[i].Serialize(*pGameStateSer);
			pGameStateSer->EndGroup();
		}
		pGameStateSer->EndGroup();
	}

	//this early out doesn't work anymore, find better ways to identify corrupt savegames
	/*
	   if (bIsQuickLoading && !VerifyEntities(basicEntityData))
	   {
	    GameWarning("[LoadGame] Corrupt savegame detected, aborting load");
	    return eLGR_Failed;
	   }*/

	//point of no return, if loading fails now, the game can't recover
	//this has to be fixed in Drxsis 3

	//reset all entities if using layer-based serialization
	SEntityEvent resetEvent(ENTITY_EVENT_RESET);
	ICVar* pLayerSaveLoad = gEnv->pConsole->GetCVar("es_layerSaveLoadSerialization");
	//we only need this reset for non-serialized stuff on deactivated layers
	//but it probably doesn't hurt to always use it ..
	if (pLayerSaveLoad && pLayerSaveLoad->GetIVal() == 1)
		gEnv->pEntitySystem->SendEventToAll(resetEvent);

	//tell existing entities that serialization starts
	IEntityItPtr pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	SEntityEvent serializeEvent(ENTITY_EVENT_PRE_SERIALIZE);
	while (IEntity* pEntity = pEntityIterator->Next())
	{
		if (gEnv->pEntitySystem->ShouldSerializedEntity(pEntity))
		{
			pEntity->SendEvent(serializeEvent);
		}
	}

	loadEnvironment.m_checkpoint.Check("PreSerialize Event");

	// load timer data
	loadEnvironment.m_pSer = loadEnvironment.m_pLoadGame->GetSection(SAVEGAME_TIMER_SECTION);
	if (loadEnvironment.m_pSer.get())
		gEnv->pTimer->Serialize(*loadEnvironment.m_pSer);
	else
	{
		GameWarning("Unable to find timer");
		return eLGR_Failed;
	}

	// call DinrusAction's Framework listeners
	pDrxAction->NotifyGameFrameworkListeners(loadEnvironment.m_pLoadGame.Get());

	checkpoint.Check("FrameWork Listeners");

	// reset movie system, don't play any sequences
	if (gEnv->pMovieSystem)
		gEnv->pMovieSystem->Reset(false, false);

	checkpoint.Check("MovieSystem");

	// reset areas (who entered who caches get flushed)
	pEntitySystem->ResetAreas();

	// Prepare next level?
	if (!bIsQuickLoading)
	{
		gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_LOAD_PREPARE, 0, 0);

		pEntitySystem->Reset();

		// delete any left-over entities
		pEntitySystem->DeletePendingEntities();
		FlushActivatableGameObjectExtensions();
		pDrxAction->FlushBreakableObjects();

		// Clean all entities that should not be in system before loading entities from savegame.
		DeleteDynamicEntities(loadEnvironment.m_basicEntityData); //must happen before reserving entities

		// end the game context
		autoResourcesLock.Unlock();
		pDrxAction->EndGameContext();
		autoResourcesLock.Lock();

		if (ILevelSystem* pLevelSystem = CDrxAction::GetDrxAction()->GetILevelSystem())
		{
			pLevelSystem->PrepareNextLevel(levelName);
		}

		// For the non-quickload path, the flush/reload nav happens after
		//	the level load. This ensures AI objects created during level load
		//	are not present when the entity pool etc are serialized.
	}
	else
	{
		// ai system data
		// let's reset first
		if (gEnv->pAISystem)
			gEnv->pAISystem->FlushSystem();

		// Flushing the system clears the navagation data, so reload it.
		const ILevelInfo* pLevelInfo = GetLevelInfo();
		DRX_ASSERT_MESSAGE(CDrxAction::GetDrxAction()->StartedGameContext() == false || (pLevelInfo != 0), "Can't find level info: This might break AI");
		if (pLevelInfo)
		{
			const ILevelInfo::TGameTypeInfo* pGameTypeInfo = pLevelInfo->GetDefaultGameType();
			tukk const szGameTypeName = pGameTypeInfo ? pGameTypeInfo->name.c_str() : "";
			if (gEnv->pAISystem)
			{
				const EAILoadDataFlags loadDataFlags = eAILoadDataFlag_AllSystems | (bIsQuickLoading ? eAILoadDataFlag_QuickLoad : eAILoadDataFlag_None);
				gEnv->pAISystem->LoadLevelData(pLevelInfo->GetPath(), szGameTypeName, loadDataFlags);
			}
		}

		checkpoint.Check("AIFlush");
	}

	// Entity ids vector must always be sorted, binary search is used on it.
	std::sort(basicEntityData.begin(), basicEntityData.end());

	checkpoint.Check("EntityIDSort");

	// load the level
	if (!LoadLevel(loadEnvironment, startParams, autoResourcesLock, bIsQuickLoading, requireQuickLoad))
	{
		return loadEnvironment.m_failure;
	}

	// AI object ID serialization (needs to be before the AI and entity serialization, but after the AI flush)
	{
		loadEnvironment.m_pSer = loadEnvironment.m_pLoadGame->GetSection(SAVEGAME_AIOBJECTID_SECTION);

		if (loadEnvironment.m_pSer.get())
		{
			if (gEnv->pAISystem)
				gEnv->pAISystem->SerializeObjectIDs(*loadEnvironment.m_pSer);
		}
		else
		{
			GameWarning("Unable to open section %s", SAVEGAME_AIOBJECTID_SECTION);
		}
		checkpoint.Check("AI_ObjectIDs");
	}

	// load engine systems
	LoadEngineSystems(loadEnvironment);

	// load entities
	if (!LoadEntities(loadEnvironment, std::move(pGameStateSer)))
	{
		return loadEnvironment.m_failure;
	}
	assert(!loadEnvironment.m_bLoadingErrors);

	// load game data and proxies
	LoadGameData(loadEnvironment);
	//assert(!loadEnvironment.m_bLoadingErrors);

	//unlock entity system
	pEntitySystem->LockSpawning(false);

#ifdef _DEBUG
	//look for additionally spawned entities (that are not in the save file)
	{
		IEntityItPtr pIt = pEntitySystem->GetEntityIterator();
		pIt->MoveFirst();
		SBasicEntityData tempSearchEntity;
		while (!pIt->IsEnd())
		{
			IEntity* pNextEntity = pIt->Next();
			u32 flags = pNextEntity->GetFlags();
			if (!pNextEntity->IsGarbage() && !(flags & ENTITY_FLAG_UNREMOVABLE && flags & ENTITY_FLAG_NO_SAVE))
			{
				tempSearchEntity.id = pNextEntity->GetId();
				if ((stl::binary_find(loadEnvironment.m_basicEntityData.begin(), loadEnvironment.m_basicEntityData.end(), tempSearchEntity) == loadEnvironment.m_basicEntityData.end()))
					GameWarning("[LoadGame] Entities were spawned that are not in the save file! : %s with ID=%d", pNextEntity->GetEntityTextDescription().c_str(), pNextEntity->GetId());
			}
		}
	}
#endif

	loadEnvironment.m_pSer = loadEnvironment.m_pLoadGame->GetSection(SAVEGAME_AISTATE_SECTION);

	if (!loadEnvironment.m_pSer.get())
	{
		GameWarning("No AI section in save game");
		//return failure;
		loadEnvironment.m_bLoadingErrors = true;
	}
	else
	{
		if (gEnv->pAISystem)
			gEnv->pAISystem->Serialize(*loadEnvironment.m_pSer);
	}
	checkpoint.Check("AI System");

	//clear old entities from the systems [could become obsolete]
	//gEnv->pGameFramework->GetIItemSystem()->Reset(); //this respawns ammo, moved before serialization
	gEnv->pGameFramework->GetIActorSystem()->Reset();
	gEnv->pGameFramework->GetIVehicleSystem()->Reset();
	//We need to reset particles to get rid of certain effects from before the load
	gEnv->pParticleUpr->Reset();
	checkpoint.Check("ResetSubSystems");

	//when loading a save game from scratch this is hopefully singleplayer; let network know ...
	pDrxAction->GetGameServerNub()->SetMaxPlayers(1);

	if (GetISystem()->IsSerializingFile() == 1)    //general quickload fix-ups
	{
		//clear keys
		gEnv->pInput->ClearKeyState();
	}

	if (gEnv->p3DEngine)
	{
		gEnv->p3DEngine->PostSerialize(true);
		checkpoint.Check("3DPostSerialize");
	}

#ifdef EXCESSIVE_ENTITY_DEBUG
	DrxLogAlways("Dumping Entities after serialization");
	DumpEntities();
#endif

	//inform all entities that serialization is over
	SEntityEvent event2(ENTITY_EVENT_POST_SERIALIZE);
	pEntitySystem->SendEventToAll(event2);

	if (auto* pGame = CDrxAction::GetDrxAction()->GetIGame())
		pGame->PostSerialize();

	gEnv->pGameFramework->GetIViewSystem()->PostSerialize();

	//return failure;
	if (loadEnvironment.m_bLoadingErrors)
	{
		//TODO [AlexMcC|10.05.10]: return failure here! We can't do this right now
		//because there's a crash on level unload :( http://jira/browse/DRXII-4903
		GameWarning("Errors during Game Loading");
	}
	checkpoint.Check("EntityPostSerialize");

	if (gEnv->pScriptSystem)
		gEnv->pScriptSystem->ForceGarbageCollection();

	checkpoint.Check("Lua GC Cycle");

	Clean();
	checkpoint.Check("PostClean");

	// Make sure player entity is not hidden.
	if (IActor* pClientActor = CDrxAction::GetDrxAction()->GetClientActor())
	{
		if (IEntity* pClientEntity = pClientActor->GetEntity())
		{
			if (pClientEntity->IsHidden())
			{
				GameWarning("Player entity was hidden in save game");

				pClientEntity->Hide(false);
			}
		}
	}

	if (gEnv->pFlowSystem != NULL)
	{
		// this guarantees that the values sent by the scripts in OnPostLoad
		// through their FG nodes get delivered to all of the connected nodes
		gEnv->pFlowSystem->Update();
	}

	DrxGetIMemReplay()->AddLabel("Loadgame_end");

	return eLGR_Ok;
}

//////////////////////////////////////////////////////////////////////////
bool CGameSerialize::SaveMetaData(SSaveEnvironment& savEnv)
{
	// verify that there's a game active
	tukk levelName = savEnv.m_pDrxAction->GetLevelName();
	if (!levelName)
	{
		GameWarning("No game active - cannot save");
		return false;
	}

	// save serialization version
	savEnv.m_pSaveGame->AddMetadata(SAVEGAME_VERSION_TAG, SAVEGAME_VERSION_VALUE);
	// save current level and game rules
	savEnv.m_pSaveGame->AddMetadata(SAVEGAME_LEVEL_TAG, levelName);
	savEnv.m_pSaveGame->AddMetadata(SAVEGAME_GAMERULES_TAG, savEnv.m_pDrxAction->GetIGameRulesSystem()->GetCurrentGameRulesEntity()->GetClass()->GetName());
	// save some useful information for debugging - should not be relied upon in loading
	const SFileVersion& fileVersion = GetISystem()->GetFileVersion();
	char tmpbuf[128];
	fileVersion.ToString(tmpbuf);
	savEnv.m_pSaveGame->AddMetadata(SAVEGAME_BUILD_TAG, tmpbuf);
	i32 bitSize = sizeof(tuk) * 8;
	savEnv.m_pSaveGame->AddMetadata("Bit", bitSize);

	//add savEnv.m_checkpoint name if available
	if (savEnv.m_checkPointName)
		savEnv.m_pSaveGame->AddMetadata(SAVEGAME_CHECKPOINT_TAG, savEnv.m_checkPointName);
	else
		savEnv.m_pSaveGame->AddMetadata(SAVEGAME_CHECKPOINT_TAG, "");

	string timeString;
	GameUtils::timeToString(time(NULL), timeString);
	savEnv.m_pSaveGame->AddMetadata(SAVEGAME_TIME_TAG, timeString.c_str());

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::SaveEngineSystems(SSaveEnvironment& savEnv)
{
	// call DinrusAction's Framework listeners
	savEnv.m_pDrxAction->NotifyGameFrameworkListeners(savEnv.m_pSaveGame.Get());

	savEnv.m_checkpoint.Start();

	// timer data
	gEnv->pTimer->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_TIMER_SECTION));
	savEnv.m_checkpoint.Check("Timer");

	// terrain modifications (e.g. heightmap changes)
	gEnv->p3DEngine->SerializeState(savEnv.m_pSaveGame->AddSection(SAVEGAME_TERRAINSTATE_SECTION));
	savEnv.m_checkpoint.Check("3DEngine");
	{
		// game tokens
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Game token serialization");
		savEnv.m_pDrxAction->GetIGameTokenSystem()->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_GAMETOKEN_SECTION));
		savEnv.m_checkpoint.Check("GameToken");
	}
	// view system
	savEnv.m_pDrxAction->GetIViewSystem()->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_VIEWSYSTEM_SECTION));
	savEnv.m_checkpoint.Check("ViewSystem");
	// ai system data
	if (gEnv->pAISystem)
		gEnv->pAISystem->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_AISTATE_SECTION));
	savEnv.m_checkpoint.Check("AISystem");

	//itemsystem - LTL inventory only
	if (savEnv.m_pDrxAction->GetIItemSystem())
		savEnv.m_pDrxAction->GetIItemSystem()->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_LTLINVENTORY_SECTION));
	savEnv.m_checkpoint.Check("Inventory");

	//FlowSystem data
	if (savEnv.m_pDrxAction->GetIFlowSystem())
		savEnv.m_pDrxAction->GetIFlowSystem()->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_FLOWSYSTEM_SECTION));
	savEnv.m_checkpoint.Check("FlowSystem");

	//ColorGradientUpr data
	if (savEnv.m_pDrxAction->GetColorGradientUpr())
		savEnv.m_pDrxAction->GetColorGradientUpr()->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_COLORGRADIANTMANAGER_SECTION));
	savEnv.m_checkpoint.Check("ColorGradientUpr");

	CMaterialEffects* pMatFX = static_cast<CMaterialEffects*>(savEnv.m_pDrxAction->GetIMaterialEffects());
	if (pMatFX)
		pMatFX->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_MATERIALEFFECTS_SECTION));
	savEnv.m_checkpoint.Check("MaterialFX");

	CDialogSystem* pDS = savEnv.m_pDrxAction->GetDialogSystem();
	if (pDS)
		pDS->Serialize(savEnv.m_pSaveGame->AddSection(SAVEGAME_DIALOGSYSTEM_SECTION));
	savEnv.m_checkpoint.Check("DialogSystem");
}

//////////////////////////////////////////////////////////////////////////
bool CGameSerialize::SaveEntities(SSaveEnvironment& savEnv)
{
	TSerialize gameState = savEnv.m_pSaveGame->AddSection(SAVEGAME_GAMESTATE_SECTION);

	ICVar* pUseNoSaveFlag = gEnv->pConsole->GetCVar("es_SaveLoadUseLUANoSaveFlag");
	bool bUseNoSaveFlag = (pUseNoSaveFlag && pUseNoSaveFlag->GetIVal() != 0);

	TEntitiesToSerialize entities;
	entities.reserve(gEnv->pEntitySystem->GetNumEntities());

	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Basic entity data serialization");

		gameState.BeginGroup("BasicEntityData");
		i32 nSavedEntityCount = 0;

		IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
		while (IEntity* pEntity = pIt->Next())
		{
			u32 flags = pEntity->GetFlags();
			if (flags & ENTITY_FLAG_NO_SAVE)
			{
				// GameWarning("Skipping Entity %d '%s' '%s' NoSave", pEntity->GetId(), pEntity->GetName(), pEntity->GetClass()->GetName());
				continue;
			}

			{
				MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Basic entity data serialization");

				// basic entity data
				SBasicEntityData bed;
				bed.pEntity = pEntity;
				bed.id = pEntity->GetId();

				// if we're not going to serialize these, don't bother fetching them
				if (!(flags & ENTITY_FLAG_UNREMOVABLE) || !CDrxActionCVars::Get().g_saveLoadBasicEntityOptimization)
				{
					bed.name = pEntity->GetName();
					bed.className = pEntity->GetClass()->GetName();
					if (pEntity->GetArchetype())
						bed.archetype = pEntity->GetArchetype()->GetName();
				}

				bed.ignorePosRotScl = false;
				//lua flag
				if (bUseNoSaveFlag)
				{
					IScriptTable* pEntityScript = pEntity->GetScriptTable();
					SmartScriptTable props;
					if (pEntityScript && pEntityScript->GetValue("Properties", props))
					{
						bool bSerialize = true;
						if (props->GetValue("bSerialize", bSerialize) && (bSerialize == false))
							bed.ignorePosRotScl = true;
					}
				}

				bed.aiObjectId = pEntity->GetAIObjectID();

				IEntity* pParentEntity = pEntity->GetParent();
				IPhysicalEntity* pPhysEnt;
				pe_status_awake sa;
				pe_status_pos sp;
				// for active rigid bodies, query the most recent position from the physics, since due to multithreading
				// CEntity position might be lagging behind
				if (!pParentEntity && (pPhysEnt = pEntity->GetPhysics()) && pPhysEnt->GetType() == PE_RIGID && pPhysEnt->GetStatus(&sa) && pPhysEnt->GetStatus(&sp))
				{
					bed.pos = sp.pos;
					bed.rot = sp.q;
				}
				else
				{
					bed.pos = pEntity->GetPos();
					bed.rot = pEntity->GetRotation();
				}
				bed.scale = pEntity->GetScale();
				bed.flags = flags;
				bed.isHidden = pEntity->IsHidden();
				bed.isInvisible = pEntity->IsInvisible();

				bed.isPhysicsEnabled = pEntity->IsPhysicsEnabled();

				bed.parentEntity = pParentEntity ? pParentEntity->GetId() : 0;
				bed.iPhysType = pEntity->GetPhysics() ? pEntity->GetPhysics()->GetType() : PE_NONE;

				entities.push_back(pEntity);

				gameState.BeginGroup("BasicEntity");
				bed.Serialize(gameState);
				gameState.EndGroup();
				++nSavedEntityCount;
			}
		}

		gameState.Value("BasicEntityDataSize", nSavedEntityCount);
		gameState.EndGroup();

		savEnv.m_checkpoint.Check("EntityPrep");
	}

	{
		SSerializeScopedBeginGroup entity_group(gameState, "NormalEntityData");
		TEntitiesToSerialize::const_iterator itEnd = entities.end();
		for (TEntitiesToSerialize::const_iterator it = entities.begin(); it != itEnd; ++it)
		{
			IEntity* pEntity = *it;
			assert(pEntity);
			//that's all that is left from entity properties (using early out to reduce save size)
			i32 iEntityFlags = pEntity->GetFlags();
			if ((iEntityFlags & ENTITY_SERIALIZE_PROPERTIES) && !(iEntityFlags & ENTITY_FLAG_UNREMOVABLE))
			{
				SSerializeScopedBeginGroup entity_group2(gameState, "EntityProps");
				pEntity->Serialize(gameState, ENTITY_SERIALIZE_PROPERTIES);
			}
		}
	}

	savEnv.m_checkpoint.Check("EntitySystem2");

	// Must be serialized after basic entity data.
	savEnv.m_pDrxAction->Serialize(gameState); //breakable objects ..

	savEnv.m_checkpoint.Check("Breakables");

#ifndef _RELEASE
	{
		DebugSaveParts debugSaveParts(entities, savEnv.m_pSaveGame);
		DebugPrintMemoryUsage(debugSaveParts);
		savEnv.m_checkpoint.Check("Debug memory usage");
	}
#endif

	return SaveGameData(savEnv, gameState, entities);
}

//////////////////////////////////////////////////////////////////////////
bool CGameSerialize::SaveGameData(SSaveEnvironment& savEnv, TSerialize& gameState, TEntitiesToSerialize& entities)
{
	//EntitySystem
	gEnv->pEntitySystem->Serialize(gameState);

	savEnv.m_checkpoint.Check("EntitySystem");

	// this was previously done 32x for every game object serialize ... once here should be enough
	gEnv->pNetwork->SyncWithGame(eNGS_FrameEnd);

	gameState.BeginGroup("ExtraEntityData");
	u32 saveCount = 0;

	for (i32 pass = 0; pass < 2; pass++) // save (and thus load) ropes after other entities (during pass 1) to make sure attachments are ready during loading
		// fall back on the previous save code if no list is present
		if (m_serializeEntities.empty() || CDrxActionCVars::Get().g_saveLoadUseExportedEntityList == 0)
		{
			for (TEntitiesToSerialize::const_iterator iter = entities.begin(), end = entities.end(); iter != end; ++iter)
			{
				IEntity* pEntity = *iter;
				DRX_ASSERT(pEntity);

				bool bSerializeEntity = gEnv->pEntitySystem->ShouldSerializedEntity(pEntity);
				//step over entities in hidden layers
				if (!bSerializeEntity)
				{
					//DrxLogAlways("Saved Entity On Deactivated Layer : %i, %s", iter->id, iter->name);
					//gameState.EndGroup();
					continue;
				}
				if ((pEntity->GetPhysics() && pEntity->GetPhysics()->GetType() == PE_ROPE) == !pass)
					continue;

				// c++ entity data
				gameState.BeginGroup("Entity");

				EntityId temp = pEntity->GetId();
				gameState.Value("id", temp);
				++saveCount;

				if (pEntity->GetFlags() & ENTITY_FLAG_MODIFIED_BY_PHYSICS)
				{
					gameState.BeginGroup("EntityGeometry");
					pEntity->Serialize(gameState, ENTITY_SERIALIZE_GEOMETRIES);
					gameState.EndGroup();
				}
				pEntity->Serialize(gameState, ENTITY_SERIALIZE_PROXIES);

				gameState.EndGroup();//Entity
			}
		}
		else
		{
			for (TEntityVector::iterator iter = m_serializeEntities.begin(), end = m_serializeEntities.end(); iter != end; ++iter)
			{
				EntityId id = *iter;
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);

				if (pEntity)
				{
					if ((pEntity->GetPhysics() && pEntity->GetPhysics()->GetType() == PE_ROPE) == !pass)
						continue;
					DRX_ASSERT_TRACE(!(pEntity->GetFlags() & ENTITY_FLAG_LOCAL_PLAYER), ("%s has ENTITY_FLAG_LOCAL_PLAYER - local player should not be in m_serializeEntities!", pEntity->GetEntityTextDescription().c_str()));

					// c++ entity data
					gameState.BeginGroup("Entity");

					EntityId temp = id;
					gameState.Value("id", temp);
					++saveCount;

					if (pEntity->GetFlags() & ENTITY_FLAG_MODIFIED_BY_PHYSICS)
					{
						gameState.BeginGroup("EntityGeometry");
						pEntity->Serialize(gameState, ENTITY_SERIALIZE_GEOMETRIES);
						gameState.EndGroup();
					}
					pEntity->Serialize(gameState, ENTITY_SERIALIZE_PROXIES);

					gameState.EndGroup();//Entity
				}
			}

			// also save all dynamic entities
			assert(stl::find(m_dynamicEntities, CDrxAction::GetDrxAction()->GetClientActorId()));
			for (TEntitySet::iterator iter = m_dynamicEntities.begin(), end = m_dynamicEntities.end(); iter != end; ++iter)
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(*iter);
				DRX_ASSERT(pEntity);
				if ((pEntity->GetPhysics() && pEntity->GetPhysics()->GetType() == PE_ROPE) == !pass)
					continue;

				// c++ entity data
				gameState.BeginGroup("Entity");

				EntityId temp = *iter;
				gameState.Value("id", temp);
				++saveCount;

				if (pEntity)
				{
					if (pEntity->GetFlags() & ENTITY_FLAG_MODIFIED_BY_PHYSICS)
					{
						gameState.BeginGroup("EntityGeometry");
						pEntity->Serialize(gameState, ENTITY_SERIALIZE_GEOMETRIES);
						gameState.EndGroup();
					}
					pEntity->Serialize(gameState, ENTITY_SERIALIZE_PROXIES);
				}

				gameState.EndGroup();//Entity
			}
		}

	gameState.Value("savedEntityCount", saveCount);

	gameState.EndGroup();//ExtraEntityData

	savEnv.m_checkpoint.Check("ExtraEntity");

	if (auto* pGame = CDrxAction::GetDrxAction()->GetIGame())
		pGame->FullSerialize(gameState);

	// 3DEngine
	gEnv->p3DEngine->PostSerialize(false);

	savEnv.m_checkpoint.Check("3DEnginePost");

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::LoadEngineSystems(SLoadEnvironment& loadEnv)
{
	// Reset the flowsystem here (sending eFE_Initialize) to all FGs
	// also makes sure, that nodes present in the level.pak but not in the
	// savegame loaded afterwards get initialized correctly
	gEnv->pFlowSystem->Reset(false);

	loadEnv.m_checkpoint.Check("DestroyedState");

	loadEnv.m_failure = eLGR_FailedAndDestroyedState;

	// timer serialization (after potential context switch, e.g. when loading a savegame for which Level has not been loaded yet)
	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_TIMER_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("Unable to open timer %s", SAVEGAME_TIMER_SECTION);
	else
		gEnv->pTimer->Serialize(*loadEnv.m_pSer);

	// Now Pause the Game timer if not already done!
	// so no time passes until serialization is over
	SPauseGameTimer pauseGameTimer;

	// terrain modifications (e.g. heightmap changes)
	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_TERRAINSTATE_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("Unable to open section %s", SAVEGAME_TERRAINSTATE_SECTION);
	else
		gEnv->p3DEngine->SerializeState(*loadEnv.m_pSer);

	loadEnv.m_checkpoint.Check("3DEngine");

	// game tokens
	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_GAMETOKEN_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("No game token data in save game");
	else
	{
		IGameTokenSystem* pGTS = CDrxAction::GetDrxAction()->GetIGameTokenSystem();
		if (pGTS)
		{
			if (gEnv->IsEditor())
			{
				tuk sLevelName;
				tuk levelFolder;
				loadEnv.m_pDrxAction->GetEditorLevel(&sLevelName, &levelFolder);
				string tokenPath = levelFolder;
				tokenPath += "/GameTokens/*.xml";
				pGTS->Reset();
				pGTS->LoadLibs(tokenPath);
				pGTS->Serialize(*loadEnv.m_pSer);
			}
			else
			{
				// no need to reload token libraries
				pGTS->Serialize(*loadEnv.m_pSer);
			}
		}
	}

	loadEnv.m_checkpoint.Check("GameTokens");

	CMaterialEffects* pMatFX = static_cast<CMaterialEffects*>(loadEnv.m_pDrxAction->GetIMaterialEffects());
	if (pMatFX)
		pMatFX->Reset(false);

	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_MATERIALEFFECTS_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("Unable to open section %s", SAVEGAME_MATERIALEFFECTS_SECTION);
	else
	{
		if (pMatFX)
		{
			pMatFX->Serialize(*loadEnv.m_pSer);
		}
	}
	loadEnv.m_checkpoint.Check("MaterialFX");

	// ViewSystem Serialization
	IViewSystem* pViewSystem = loadEnv.m_pDrxAction->GetIViewSystem();

	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_VIEWSYSTEM_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("Unable to open section %s", SAVEGAME_VIEWSYSTEM_SECTION);
	else
	{
		if (pViewSystem)
		{
			pViewSystem->Serialize(*loadEnv.m_pSer);
		}
	}

	loadEnv.m_checkpoint.Check("ViewSystem");

	if (loadEnv.m_pDrxAction->GetIItemSystem())
	{
		loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_LTLINVENTORY_SECTION);
		if (loadEnv.m_pSer.get())
			loadEnv.m_pDrxAction->GetIItemSystem()->Serialize(*loadEnv.m_pSer);
		else
			GameWarning("Unable to open section %s", SAVEGAME_LTLINVENTORY_SECTION);
	}
	loadEnv.m_checkpoint.Check("ItemSystem");

	// load flowsystem data
	if (loadEnv.m_pDrxAction->GetIFlowSystem())
	{
		loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_FLOWSYSTEM_SECTION);
		if (loadEnv.m_pSer.get())
			loadEnv.m_pDrxAction->GetIFlowSystem()->Serialize(*loadEnv.m_pSer);
		else
			GameWarning("Unable to open section %s", SAVEGAME_FLOWSYSTEM_SECTION);
	}
	loadEnv.m_checkpoint.Check("FlowSystem");

	// Load ColorGradientUpr data
	if (loadEnv.m_pDrxAction->GetColorGradientUpr())
	{
		loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_COLORGRADIANTMANAGER_SECTION);
		if (loadEnv.m_pSer)
			loadEnv.m_pDrxAction->GetColorGradientUpr()->Serialize(*loadEnv.m_pSer);
		else
			GameWarning("Unable to open section %s", SAVEGAME_COLORGRADIANTMANAGER_SECTION);
	}
	loadEnv.m_checkpoint.Check("ColorGradientUpr");

	// Dialog System Reset & Serialization
	CDialogSystem* pDS = loadEnv.m_pDrxAction->GetDialogSystem();
	if (pDS)
		pDS->Reset(false);

	// Dialog System First Pass [recreates DialogSessions]
	loadEnv.m_pSer = loadEnv.m_pLoadGame->GetSection(SAVEGAME_DIALOGSYSTEM_SECTION);
	if (!loadEnv.m_pSer.get())
		GameWarning("Unable to open section %s", SAVEGAME_DIALOGSYSTEM_SECTION);
	else
	{
		if (pDS)
		{
			pDS->Serialize(*loadEnv.m_pSer);
		}
	}
	loadEnv.m_checkpoint.Check("DialogSystem");
}

//////////////////////////////////////////////////////////////////////////
bool CGameSerialize::LoadLevel(SLoadEnvironment& loadEnv, SGameStartParams& startParams, STempAutoResourcesLock& autoResourcesLock, bool bIsQuickLoading, bool requireQuickLoad)
{
	tukk sCurrentLevel = loadEnv.m_pDrxAction->GetLevelName();

	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	SGameContextParams ctx;

	if (!loadEnv.m_pLoadGame->HaveMetadata(SAVEGAME_LEVEL_TAG) || !loadEnv.m_pLoadGame->HaveMetadata(SAVEGAME_GAMERULES_TAG))
	{
		return false;
	}

	ctx.levelName = loadEnv.m_pLoadGame->GetMetadata(SAVEGAME_LEVEL_TAG);
	ctx.gameRules = loadEnv.m_pLoadGame->GetMetadata(SAVEGAME_GAMERULES_TAG);

	if (bIsQuickLoading)
	{
		// can quick-load
		if (!gEnv->p3DEngine->RestoreTerrainFromDisk())
			return false;

		if (gEnv->p3DEngine)
		{
			gEnv->p3DEngine->ResetPostEffects();
		}

		// [AlexMcC|18.03.10] Taken from CEntitySytem::Reset:
		// This fixes physics crashes caused by old update events
		// Flush the physics linetest and events queue, but don't pump events (on Anton's recommendation)
		gEnv->pPhysicalWorld->TracePendingRays(0);
		gEnv->pPhysicalWorld->ClearLoggedEvents();

		pEntitySystem->DeletePendingEntities();
		loadEnv.m_failure = eLGR_FailedAndDestroyedState;

		gEnv->pNetwork->SyncWithGame(eNGS_Shutdown);
		gEnv->pNetwork->SyncWithGame(eNGS_Shutdown_Clear);

		// no level loading, so imm start the precaching phase
		gEnv->pSystem->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_LEVEL_PRECACHE_START, 0, 0);

		return true;
	}
	else if (requireQuickLoad)
	{
		GameWarning("Unable to quick load: different level names (%s != %s)", sCurrentLevel, ctx.levelName);
		return false;
	}
	else if (gEnv->IsEditor())
	{
		GameWarning("Can only quick-load in editor");
		return false;
	}

	//NON-QUICKLOAD PATH
	startParams.pContextParams = &ctx;
	startParams.flags |= eGSF_BlockingClientConnect | eGSF_LocalOnly;

	loadEnv.m_failure = eLGR_FailedAndDestroyedState;

	// Note: About to load the level this save file is from, so unlock resources during the level load duration
	autoResourcesLock.Unlock();
	if (!loadEnv.m_pDrxAction->StartGameContext(&startParams))    // this is the call that actually loads the level data
	{
		GameWarning("Could not start game context.");
		return false;
	}
	autoResourcesLock.Lock();
	loadEnv.m_pDrxAction->GetGameContext()->SetLoadingSaveGame();

	// Delete any new dynamic entities (eg. items, accessories) that were created from starting the game context
	DeleteDynamicEntities(loadEnv.m_basicEntityData);

	// Remove all AI objects created during the level load too (to avoid collisions with ones in the save file).
	//	For quickloads this flush/reload happens earlier.
	assert(!bIsQuickLoading);
	if (gEnv->pAISystem)
		gEnv->pAISystem->FlushSystem();

	// Flushing the system clears the navagation data, so reload it.
	const ILevelInfo* pLevelInfo = GetLevelInfo();
	DRX_ASSERT_MESSAGE(CDrxAction::GetDrxAction()->StartedGameContext() == false || (pLevelInfo != 0), "Can't find level info: This might break AI");
	if (pLevelInfo)
	{
		const ILevelInfo::TGameTypeInfo* pGameTypeInfo = pLevelInfo->GetDefaultGameType();
		tukk const szGameTypeName = pGameTypeInfo ? pGameTypeInfo->name.c_str() : "";
		if (gEnv->pAISystem)
		{
			const EAILoadDataFlags loadDataFlags = eAILoadDataFlag_AllSystems | (bIsQuickLoading ? eAILoadDataFlag_QuickLoad : eAILoadDataFlag_None);
			gEnv->pAISystem->LoadLevelData(pLevelInfo->GetPath(), szGameTypeName, loadDataFlags);
		}
	}

	loadEnv.m_bHaveReserved = true;
	ReserveEntityIds(loadEnv.m_basicEntityData);

	IEntityItPtr pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	SEntityEvent serializeEvent(ENTITY_EVENT_PRE_SERIALIZE);
	while (IEntity* pEntity = pEntityIterator->Next())
	{
		if (gEnv->pEntitySystem->ShouldSerializedEntity(pEntity))
		{
			pEntity->SendEvent(serializeEvent);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameSerialize::LoadEntities(SLoadEnvironment& loadEnv, std::unique_ptr<TSerialize> pGameStateSer)
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	// entity creation/deletion/repositioning
	loadEnv.m_checkpoint.Check("DeleteDynamicEntities");

#ifdef EXCESSIVE_ENTITY_DEBUG
	// dump entities which can potentially be re-used (if class matches)
	DrxLogAlways("*QL -----------------------------------------");
	DrxLogAlways("*QL Dumping Entities after deletion of unused");
	DumpEntities();
	DrxLogAlways("*QL Done Dumping Entities after deletion of unused");
	DrxLogAlways("*QL -----------------------------------------");
#endif

	// we reserve the Entities AFTER we deleted any left-overs
	// = entities which are in mem, but not in savegame.
	// this could cause collisions with SaltHandles because a left-over and a new one can point to the same
	// handle (with different salts). if we reserve before deletion, we would actually delete our reserved handle

	{
		FlushActivatableGameObjectExtensions();
		loadEnv.m_checkpoint.Check("FlushExtensions");

		loadEnv.m_pDrxAction->FlushBreakableObjects();
		loadEnv.m_checkpoint.Check("FlushBreakables");

		// Clean all entities that should not be in system before loading entties from savegame.
		// delete any left-over entities
		DeleteDynamicEntities(loadEnv.m_basicEntityData); //must happen before reserving entities
		loadEnv.m_checkpoint.Check("DeletePending");

		ReserveEntityIds(loadEnv.m_basicEntityData);
		loadEnv.m_bHaveReserved = true;
	}

	// serialize breakable objects AFTER reservation
	// this will quite likely spawn new entities
	DRX_ASSERT(pGameStateSer.get());
	loadEnv.m_pSer = std::move(pGameStateSer);
	if (!loadEnv.m_pSer.get())
		return false;

	loadEnv.m_pDrxAction->Serialize(*loadEnv.m_pSer); //breakable object
	loadEnv.m_checkpoint.Check("SerializeBreakables");

	//reset item system, used to be after entity serialization
	gEnv->pGameFramework->GetIItemSystem()->Reset();

	//lock entity system
	pEntitySystem->LockSpawning(true);

	//fix breakables forced ids
	pEntitySystem->SetNextSpawnId(0);

	// basic entity data
	LoadBasicEntityData(loadEnv);

	if (!VerifyEntities(loadEnv.m_basicEntityData))
		return false;

	loadEnv.m_checkpoint.Check("AllEntities");

	// reposition any entities (once again for sanities sake)
	if (!RepositionEntities(loadEnv.m_basicEntityData, true))
	{
		//return loadEnv.m_failure;
		loadEnv.m_bLoadingErrors = true;
	}

	loadEnv.m_checkpoint.Check("Reposition");

	//~ entity creation/deletion/repositioning

	FlushActivatableGameObjectExtensions();
	loadEnv.m_checkpoint.Check("FlushExtensions2");

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::LoadBasicEntityData(SLoadEnvironment& loadEnv)
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	loadEnv.m_pSer->BeginGroup("NormalEntityData");

	// create any entities that are not in the level, but are in the save game
	// or that have the wrong class
	TBasicEntityDatas::const_iterator itEnd = loadEnv.m_basicEntityData.end();
	for (TBasicEntityDatas::const_iterator it = loadEnv.m_basicEntityData.begin(); it != itEnd; ++it)
	{
		const SBasicEntityData& bed = *it;

		IEntity* pEntity = pEntitySystem->GetEntity(bed.id);
		if (!pEntity)
		{
			IEntityClass* pClass = pEntitySystem->GetClassRegistry()->FindClass(bed.className.c_str());
			if (!pClass)
			{
				GameWarning("[LoadGame] Failed Spawning Entity ID=%d, Class '%s' not found", bed.id, bed.className.c_str());
				loadEnv.m_bLoadingErrors = true;
				continue;
				//return loadEnv.m_failure;
			}

			SEntitySpawnParams params;
			params.id = bed.id;
			assert(!bed.ignorePosRotScl);
			//this will also be set in RepositionEntities
			params.vPosition = bed.pos;
			params.qRotation = bed.rot;
			params.vScale = bed.scale;
			params.sName = bed.name.c_str();
			params.nFlags = bed.flags;
			params.pClass = pClass;
			params.bIgnoreLock = true;
			if (!bed.archetype.empty())
			{
				params.pArchetype = pEntitySystem->LoadEntityArchetype(bed.archetype);
			}
			pEntity = pEntitySystem->SpawnEntity(params, false);
			if (!pEntity)
			{
				GameWarning("[LoadGame] Failed Spawning Entity ID=%d, %s (%s)", params.id, bed.name.c_str(), bed.className.c_str());
				//return loadEnv.m_failure;
				loadEnv.m_bLoadingErrors = true;
				continue;
			}
			else if (pEntity->GetId() != bed.id)
			{
				GameWarning("[LoadGame] Spawned Entity ID=%d, %s (%s), with wrong ID=%d", bed.id, bed.name.c_str(), bed.className.c_str(), pEntity->GetId());
				//return loadEnv.m_failure;
				loadEnv.m_bLoadingErrors = true;
				continue;
			}

			// Serialize script properties, this must happen before entity Init
			if ((bed.flags & ENTITY_SERIALIZE_PROPERTIES) && !(bed.flags & ENTITY_FLAG_UNREMOVABLE))
			{
				SSerializeScopedBeginGroup entity_group(*loadEnv.m_pSer, "EntityProps");
				pEntity->Serialize(*loadEnv.m_pSer, ENTITY_SERIALIZE_PROPERTIES);
			}

			// Initialize entity after properties have been loaded.
			if (!pEntitySystem->InitEntity(pEntity, params))
			{
				GameWarning("[LoadGame] Failed Initializing Entity ID=%d, %s (%s)", params.id, bed.name.c_str(), bed.className.c_str());
				loadEnv.m_bLoadingErrors = true;
				continue;
			}
		}
		if (!pEntitySystem->GetEntity(bed.id))
		{
			GameWarning("[LoadGame] Failed Loading Entity ID=%d, %s (%s)", bed.id, bed.name.c_str(), bed.className.c_str());
			//return loadEnv.m_failure;
			loadEnv.m_bLoadingErrors = true;
			continue;
		}
	}
	loadEnv.m_pSer->EndGroup(); // loadEnv.m_pSer->BeginGroup( "NormalEntityData" );
}

//////////////////////////////////////////////////////////////////////////
void CGameSerialize::LoadGameData(SLoadEnvironment& loadEnv)
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	//serialize timers etc. in entity system, also layers have to be serialized before other entities
	pEntitySystem->Serialize(*loadEnv.m_pSer);
	loadEnv.m_checkpoint.Check("EntitySystem");

	// entity state restoration
	loadEnv.m_pSer->BeginGroup("ExtraEntityData");

	u32 saveCount = 0;
	loadEnv.m_pSer->Value("savedEntityCount", saveCount);

	// set the basic entity data
	for (TBasicEntityDatas::const_iterator iter = loadEnv.m_basicEntityData.begin(); iter != loadEnv.m_basicEntityData.end(); ++iter)
	{
		IEntity* pEntity = pEntitySystem->GetEntity(iter->id);
		if (!pEntity)
		{
			GameWarning("[LoadGame] Missing Entity ID %d", iter->id);
			//return loadEnv.m_failure;
			loadEnv.m_bLoadingErrors = true;
			continue;
		}

		pEntity->SetFlags(iter->flags);

		pEntity->EnablePhysics(true);

		pEntity->Hide(false);
		pEntity->Invisible(false);

		// Warning: since the AI system serialize hasn't happened yet, the AI object won't exist yet (previous ones were
		//  removed by the AI flush). Essentially, between this point and the AI serialize
		//	it is not safe to call GetAI() on the entity.
		// Fixing this properly would probably mean making the entity serialize it's own AI object as it does with other proxies.
		pEntity->SetAIObjectID(iter->aiObjectId);

		// extra sanity check for matching class
		if (!(pEntity->GetFlags() & ENTITY_FLAG_UNREMOVABLE) && iter->className != pEntity->GetClass()->GetName())
		{
			GameWarning("[LoadGame] Entity class mismatch ID=%d %s, should have class '%s'", pEntity->GetId(), pEntity->GetEntityTextDescription().c_str(), iter->className.c_str());
			loadEnv.m_bLoadingErrors = true;
			continue;
		}

		if (pEntity->CheckFlags(ENTITY_FLAG_SPAWNED | ENTITY_FLAG_MODIFIED_BY_PHYSICS) && iter->iPhysType != PE_NONE)
		{
			SEntityPhysicalizeParams epp;
			epp.type = iter->iPhysType;
			pEntity->Physicalize(epp);
		}
		if ((iter->flags & (ENTITY_FLAG_MODIFIED_BY_PHYSICS | ENTITY_FLAG_SPAWNED)) == ENTITY_FLAG_MODIFIED_BY_PHYSICS)
			pEntity->AddFlags(iter->flags & ENTITY_FLAG_MODIFIED_BY_PHYSICS);
	}
	// now the extra entity data:
	for (u32 index = 0; index < saveCount; ++index)
	{
		// c++ state
		loadEnv.m_pSer->BeginGroup("Entity");

		EntityId id = 0;
		loadEnv.m_pSer->Value("id", id);

		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
		if (!pEntity)
		{
			loadEnv.m_pSer->EndGroup();
			GameWarning("[LoadGame] Expected Entity ID=%d: Entity not found.", id);
			loadEnv.m_bLoadingErrors = true;
			continue;
		}

		i32 nEntityFlags = pEntity->GetFlags();

		if ((nEntityFlags & ENTITY_FLAG_MODIFIED_BY_PHYSICS) != 0)
		{
			loadEnv.m_pSer->BeginGroup("EntityGeometry");
			pEntity->Serialize(*loadEnv.m_pSer, ENTITY_SERIALIZE_GEOMETRIES);
			loadEnv.m_pSer->EndGroup();
		}

		pEntity->Serialize(*loadEnv.m_pSer, ENTITY_SERIALIZE_PROXIES);

		loadEnv.m_pSer->EndGroup();
	}

	loadEnv.m_pSer->EndGroup();

	// set the hidden/active state again for all entities
	for (i32 pass = 0; pass < 2; pass++) // hide ropes before other entities to prevent detachment
		for (TBasicEntityDatas::const_iterator iter = loadEnv.m_basicEntityData.begin(); iter != loadEnv.m_basicEntityData.end(); ++iter)
		{
			IEntity* pEntity = pEntitySystem->GetEntity(iter->id);
			if (pEntity)
			{
				if ((pEntity->GetPhysics() && pEntity->GetPhysics()->GetType() == PE_ROPE) != !pass)
					continue;
				// moved to after serialize so physicalization works as expected
				pEntity->Hide(iter->isHidden);
				pEntity->Invisible(iter->isInvisible);

				{
					pEntity->EnablePhysics(iter->isPhysicsEnabled);
				}
			}
		}

	loadEnv.m_checkpoint.Check("ExtraEntityData");

	if (auto* pGame = CDrxAction::GetDrxAction()->GetIGame())
		pGame->FullSerialize(*loadEnv.m_pSer);
}
