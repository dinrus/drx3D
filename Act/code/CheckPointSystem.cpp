// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Checkpoint Save/Load system for Game04

   -------------------------------------------------------------------------
   История:
   - 10:07:2008 : Created By Jan Müller
   - 05:02:2009 : Refactored and moved By Kevin Kirst

*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CheckPointSystem.h>

//engine interfaces
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/CoreX/String/Path.h>

//statics
FixedCheckpointString CCheckpointSystem::g_lastSavedCheckpoint;
std::list<ICheckpointListener*> CCheckpointSystem::g_vCheckpointSystemListeners;

const static i32 CHECKPOINT_VERSION_NUMBER = 0;
const static tuk FILENAME_EXTENSION = ".jmc";
const static tuk CONSOLE_SAVEGAME_DIRECTORY = "%USER%/SaveGames";

//section flags
const static i32 CHECKPOINT_DATA_SIZE = 1024000;
const static tuk ACTOR_FLAGS_SECTION = "ActorFlags";
const static tuk ACTIVATED_ACTORS_SECTION = "ActivatedActors";
const static tuk META_DATA_SECTION = "MetaData";
const static tuk GAMETOKEN_SECTION = "Gametokens";
const static tuk EXTERNAL_ENTITIES_SECTION = "ExternalEntities";

//checkpoint data sanity check, usually triggered by changed entity Id's
static bool CHECKPOINT_RESAVE_NECESSARY = false;

//opened XML node for writing
static XmlNodeRef CHECKPOINT_SAVE_XML_NODE = NULL;
//opened XML node for reading
static XmlNodeRef CHECKPOINT_LOAD_XML_NODE = NULL;

//////////////////////////////////////////////////////////////////////////
CCheckpointSystem::CCheckpointSystem() : m_pGameHandler(NULL)
{

}

//////////////////////////////////////////////////////////////////////////
CCheckpointSystem::~CCheckpointSystem()
{

}

//////////////////////////////////////////////////////////////////////////
CCheckpointSystem* CCheckpointSystem::GetInstance()
{
	//singleton instance
	static CCheckpointSystem* g_pSLS = NULL;
	if (g_pSLS == NULL)
	{
		g_pSLS = new CCheckpointSystem();
		g_lastSavedCheckpoint.clear();
	}
	return g_pSLS;
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::SaveXMLNode(XmlNodeRef node, tukk identifier)
{
	if (!identifier || !node)
		return false;
	//check whether a checkpoint is currently open
	if (!CHECKPOINT_SAVE_XML_NODE)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Tried writing checkpoint section %s while checkpoint was not open.", identifier);
		return false;
	}

	//flag section as being external and save name
	node->setAttr("external", identifier);

	//add section to opened file
	CHECKPOINT_SAVE_XML_NODE->addChild(node);
	return true;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CCheckpointSystem::LoadXMLNode(tukk identifier)
{
	if (!identifier)
		return NULL;
	//check whether a checkpoint is currently open
	if (!CHECKPOINT_LOAD_XML_NODE)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Tried reading checkpoint section %s while checkpoint was not open.", identifier);
		return NULL;
	}

	i32 numChildren = CHECKPOINT_LOAD_XML_NODE->getChildCount();
	for (i32 i = 0; i < numChildren; ++i)
	{
		XmlNodeRef child = CHECKPOINT_LOAD_XML_NODE->getChild(i);
		//return external section if name matches
		tukk key = "external";
		tukk attribName = child->getAttr(key);
		if (attribName)
		{
			//check name
			if (!stricmp(identifier, attribName))
				return child;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::SaveExternalEntity(EntityId id)
{
	//this function allows external logic (flowgraph) to save specific entities
	if (!CHECKPOINT_SAVE_XML_NODE)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Tried writing external entity %i while savegame was not open.", (i32)id);
		return false;
	}

	//find entity and access external section
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (pEntity)
	{
		XmlNodeRef externalEntities = CHECKPOINT_SAVE_XML_NODE->findChild(EXTERNAL_ENTITIES_SECTION);
		if (!externalEntities)
		{
			externalEntities = GetISystem()->CreateXmlNode(EXTERNAL_ENTITIES_SECTION);
			CHECKPOINT_SAVE_XML_NODE->addChild(externalEntities);
		}

		IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(pEntity->GetId());
		if (pActor)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "The actor %s is additionally saved as external entity.", pEntity->GetName());
		}

		//create entity data
		char entityId[16];
		drx_sprintf(entityId, "%s%u", "id", id);
		XmlNodeRef nextEntity = GetISystem()->CreateXmlNode(entityId);
		if (nextEntity)
		{
			nextEntity->setAttr("id", pEntity->GetId());
			nextEntity->setAttr("name", pEntity->GetName());
			nextEntity->setAttr("hidden", pEntity->IsHidden());
			//save translation and rotation (complete tm matrix for simplicity)
			SerializeWorldTM(pEntity, nextEntity, true);
			//add new entity to checkpoint
			externalEntities->addChild(nextEntity);

			return true;
		}

		return false;
	}

	return false;
}

//SAVING *********************************

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::SaveGame(EntityId checkpointId, tukk fileName)
{
	DRX_ASSERT(fileName);
	if (!fileName || CHECKPOINT_SAVE_XML_NODE || CHECKPOINT_LOAD_XML_NODE)
		return false;

	//set extension
	FixedCheckpointString file(fileName);
	SetFilenameExtension(file);

	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "Saving checkpoint %s", file.c_str());

	CHECKPOINT_SAVE_XML_NODE = GetISystem()->CreateXmlNode("Checkpoint");

	//write checkpoint data
	SCheckpointData metaData;
	WriteMetaData(checkpointId, CHECKPOINT_SAVE_XML_NODE, metaData);

	//actor data
	// TODO For now, not saving actor info (AI) - If this happens later, support needs to be added for entity pools
	//WriteActorData(CHECKPOINT_SAVE_XML_NODE);

	//vehicle data
	WriteVehicleData(CHECKPOINT_SAVE_XML_NODE);

	//write game tokens
	WriteGameTokens(CHECKPOINT_SAVE_XML_NODE);

	//let game write
	if (m_pGameHandler)
	{
		m_pGameHandler->OnWriteData(CHECKPOINT_SAVE_XML_NODE);
	}

	//inform listeners
	UpdateListener(metaData, true);

	//write to file
	WriteXML(CHECKPOINT_SAVE_XML_NODE, file.c_str());

	//draw text message on screen
	//static const ColorF color (0.0f, 0.85f, 0.2f, 1.0f);
	//g_pGame->GetIGameFramework()->GetIPersistantDebug()->Add2DText("Checkpoint saved", 2.5f, color, 2.0f);

	CHECKPOINT_SAVE_XML_NODE = NULL;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::WriteActorData(XmlNodeRef parentNode)
{
	XmlNodeRef node = GetISystem()->CreateXmlNode(ACTOR_FLAGS_SECTION);
	//write only data for non-active or hidden actors
	XmlNodeRef activatedActors = GetISystem()->CreateXmlNode(ACTIVATED_ACTORS_SECTION);
	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();
	IActorIteratorPtr it = pActorSystem->CreateActorIterator();
	while (IActor* pActor = it->Next())
	{
		IEntity* pEntity = pActor->GetEntity();
		if (!pEntity->IsHidden() && pEntity->IsActivatedForUpdates())
		{
			EntityId id = pEntity->GetId();
			tukk name = pEntity->GetName(); //we have to rely on names, since Id's change on level reexport
			char buffer[100];
			drx_sprintf(buffer, "%s%u", "id", id);
			activatedActors->setAttr(buffer, name);
		}
	}
	node->addChild(activatedActors);

	parentNode->addChild(node);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::WriteVehicleData(XmlNodeRef parentNode)
{
	IVehicleSystem* pVehicleSystem = CDrxAction::GetDrxAction()->GetIVehicleSystem();
	IVehicleIteratorPtr pVehIt = pVehicleSystem->CreateVehicleIterator();
	while (IVehicle* pVehicle = pVehIt->Next())
	{
		SaveExternalEntity(pVehicle->GetEntityId());
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::WriteMetaData(EntityId checkpointId, XmlNodeRef parentNode, SCheckpointData& outMetaData)
{
	XmlNodeRef node = GetISystem()->CreateXmlNode(META_DATA_SECTION);

	tukk levelName = CDrxAction::GetDrxAction()->GetLevelName();
	DRX_ASSERT(levelName);
	DrxFixedStringT<32> curlevelName = levelName;
	RepairLevelName(curlevelName);

	node->setAttr("Version", CHECKPOINT_VERSION_NUMBER);
	node->setAttr("LevelName", curlevelName.c_str());
	node->setAttr("CheckpointId", checkpointId);

	//write checkpoint name to be independent of entityId
	IEntity* pCheckpointEntity = gEnv->pEntitySystem->GetEntity(checkpointId);
	if (pCheckpointEntity)
		node->setAttr("CheckpointName", pCheckpointEntity->GetName());
	else
		node->setAttr("CheckpointName", "none");

	string timeString;
	GameUtils::timeToString(time(NULL), timeString);
	node->setAttr("Timestamp", timeString.c_str());

	parentNode->addChild(node);

	//write back metadata for listeners
	outMetaData.m_checkPointId = checkpointId;
	outMetaData.m_levelName = levelName;
	outMetaData.m_saveTime = timeString.c_str();
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::WriteGameTokens(XmlNodeRef parentNode)
{
	//create serialization writer
	XmlNodeRef node = GetISystem()->CreateXmlNode(GAMETOKEN_SECTION);
	IXmlSerializer* pSerializer = GetISystem()->GetXmlUtils()->CreateXmlSerializer();
	ISerialize* pWriter = pSerializer->GetWriter(node);

	//get serialization data
	TSerialize ser = TSerialize(pWriter);
	IGameTokenSystem* pTokenSystem = CDrxAction::GetDrxAction()->GetIGameTokenSystem();
	pTokenSystem->Serialize(ser);

	//add to parent node
	parentNode->addChild(node);

	pSerializer->Release();
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::WriteXML(XmlNodeRef data, tukk fileName)
{
	IPlayerProfileUpr* pPlayerProfMan = CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	;

	string path;
	if (!pPlayerProfMan)
	{
		path = CONSOLE_SAVEGAME_DIRECTORY;
	}
	else
	{
		tukk sharedSaveGameFolder = pPlayerProfMan->GetSharedSaveGameFolder();
		path = sharedSaveGameFolder;
	}

	path = PathUtil::AddSlash(path);
	path.append(fileName);
	if (data)
	{
		//write checkpoint data to xml file with given name
		const string xmlHeader("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");

		bool bSuccess = data->saveToFile(path.c_str(), 32767 / 2, NULL);
		if (bSuccess)
		{
			//remember last saved checkpoint for "quickload"
			g_lastSavedCheckpoint = fileName;
		}
		else
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed writing checkpoint file at %s", path.c_str());
	}
}

//LOADING ********************************

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::LoadLastCheckpoint()
{
	if (!g_lastSavedCheckpoint.empty())
		return LoadGame(g_lastSavedCheckpoint.c_str());
	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Can't load latest checkpoint : no recent checkpoint found!");
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::LoadGame(tukk fileName)
{
	//make sure not not save/load recursively or multiple times at once
	if (CHECKPOINT_SAVE_XML_NODE || CHECKPOINT_LOAD_XML_NODE)
		return false;

	//set extension
	FixedCheckpointString file(fileName);
	SetFilenameExtension(file);

	CHECKPOINT_LOAD_XML_NODE = ReadXML(file.c_str());
	if (!CHECKPOINT_LOAD_XML_NODE)
		return false;

	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "Loading checkpoint %s", file.c_str());

	//check for EntityId errors
	CHECKPOINT_RESAVE_NECESSARY = false;

	//process meta data
	SCheckpointData metaData;
	if (!ReadMetaData(CHECKPOINT_LOAD_XML_NODE, metaData))
		return false;

	//check version number
	if (metaData.m_versionNumber != CHECKPOINT_VERSION_NUMBER)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Checkpoint version number (%i) does not match current version (%i). Please reexport all checkpoints of this level to prevent errors!", metaData.m_versionNumber, CHECKPOINT_VERSION_NUMBER);
	}

	//check for level mismatch
	DrxFixedStringT<32> curlevelName = CDrxAction::GetDrxAction()->GetLevelName();
	RepairLevelName(curlevelName);
	if (curlevelName.empty() || stricmp(metaData.m_levelName.c_str(), curlevelName.c_str()))
	{
		if (!LoadCheckpointMap(metaData, curlevelName))
			return false;
	}
	else
	{
		//reset the dynamic parts of the engine
		ResetEngine();
	}

	//read actor data and respawn AI
	// TODO For now, not restoring actor info (AI) - If this happens later, support needs to be added for entity pools
	//RespawnAI(CHECKPOINT_LOAD_XML_NODE);

	//load gametokens again
	ReadGameTokens(CHECKPOINT_LOAD_XML_NODE);

	//let game read
	if (m_pGameHandler)
	{
		m_pGameHandler->OnReadData(CHECKPOINT_LOAD_XML_NODE);
	}

	//resets some gameplay data like action filters etc.
	RestartGameplay();

	//load external entities, that are controlled by flowgraph
	LoadExternalEntities(CHECKPOINT_LOAD_XML_NODE);

	//inform listeners
	UpdateListener(metaData, false);

	//trigger flowgraph node
	OnCheckpointLoaded(metaData);

	//draw text message on screen
	//static const ColorF color (0.0f, 0.85f, 0.2f, 1.0f);
	//g_pGame->GetIGameFramework()->GetIPersistantDebug()->Add2DText("Checkpoint loaded", 2.5f, color, 2.0f);

	//checkpoint file sanity check
	if (CHECKPOINT_RESAVE_NECESSARY)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Checkpoint file contained obsolete or wrong data, trying to re-save checkpoint.");
		//resave checkpoint to fix changed entity Ids
		SaveGame(metaData.m_checkPointId, fileName);
		//make sure the script entity is aware of the activity
		OnCheckpointLoaded(metaData);
	}

	CHECKPOINT_LOAD_XML_NODE = NULL;

	//when a checkpoint was loaded, it becomes the most recent checkpoint
	g_lastSavedCheckpoint = fileName;

	//make sure the scripts are clean
	//CXP : this caused a crash after some reloads, which hints to problems in the gamerules script
	//gEnv->pScriptSystem->ForceGarbageCollection();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::LoadCheckpointMap(const SCheckpointData& metaData, DrxFixedStringT<32>& curLevelName)
{
	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_COMMENT, "Checkpoint loads new level :  %s", metaData.m_levelName.c_str());

	//load map
	if (gEnv->IsEditor())
		return false; //can't do that
	DrxFixedStringT<64> mapCmd("map ");
	mapCmd += metaData.m_levelName.c_str();
	gEnv->pConsole->ExecuteString(mapCmd.c_str());

	curLevelName = CDrxAction::GetDrxAction()->GetLevelName();
	RepairLevelName(curLevelName);
	if (stricmp(metaData.m_levelName.c_str(), curLevelName.c_str()))
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed loaded level %s from checkpoint.", metaData.m_levelName.c_str());
		return false; //still wrong level
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::LoadExternalEntities(XmlNodeRef parentNode)
{
	XmlNodeRef data = parentNode->findChild(EXTERNAL_ENTITIES_SECTION);
	if (!data)
		return;

	i32 numEntities = data->getChildCount();
	for (i32 i = 0; i < numEntities; ++i)
	{
		XmlNodeRef nextEntity = data->getChild(i);
		if (nextEntity)
		{
			EntityId id = 0;
			nextEntity->getAttr("id", id);
			tukk name = nextEntity->getAttr("name");

			//fix entityId if broken
			if (RepairEntityId(id, name))
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
				//setup entity
				bool bHidden = false;
				nextEntity->getAttr("hidden", bHidden);
				pEntity->Hide(bHidden);
				//load matrix
				SerializeWorldTM(pEntity, nextEntity, false);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//clear everything out before loading a checkpoint
void CCheckpointSystem::ResetEngine()
{
	//let game do initial resetting
	if (m_pGameHandler)
	{
		m_pGameHandler->OnPreResetEngine();
	}

	//turn off physics
	gEnv->pSystem->SetThreadState(ESubsys_Physics, false);

	//reset engine similar to editor when leaving game mode
	//broken geometry and dynamics
	CDrxAction::GetDrxAction()->FlushBreakableObjects();
	DeleteDynamicEntities();
	CDrxAction::GetDrxAction()->ResetBrokenGameObjects();

	//this should just reset the velocity of all moving things, but instead vehicle doors fall off ...
	//CXP : add this back in after CXP, Anton has to fix it
	//gEnv->pPhysicalWorld->ResetDynamicEntities();

	//particle and post process effects
	gEnv->p3DEngine->ResetPostEffects();
	gEnv->p3DEngine->ResetParticlesAndDecals();

	REINST("notify the audio system?");

	//AI System
	if (gEnv->pAISystem)
	{
		gEnv->pAISystem->Reset(IAISystem::RESET_EXIT_GAME);
		gEnv->pAISystem->Reset(IAISystem::RESET_ENTER_GAME);
	}

	//flow system

	//reset trackview
	gEnv->pMovieSystem->Reset(true, true);

	//entity system
	SEntityEvent event;
	event.event = ENTITY_EVENT_RESET;
	event.nParam[0] = 0;
	gEnv->pEntitySystem->SendEventToAll(event);

	//Vehicle System
	IVehicleSystem* pVehicleSystem = CDrxAction::GetDrxAction()->GetIVehicleSystem();
	IVehicleIteratorPtr pVehIt = pVehicleSystem->CreateVehicleIterator();
	while (IVehicle* pVehicle = pVehIt->Next())
	{
		pVehicle->Reset(true);
	}

	//make sure the scripts are clean
	gEnv->pScriptSystem->ForceGarbageCollection();

	//turn on physics again
	gEnv->pSystem->SetThreadState(ESubsys_Physics, true);

	//let game do final resetting
	if (m_pGameHandler)
	{
		m_pGameHandler->OnPostResetEngine();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::ReadMetaData(XmlNodeRef parentNode, SCheckpointData& metaData, bool bRepairId /*=true*/)
{
	XmlNodeRef data = parentNode->findChild(META_DATA_SECTION);
	if (!data)
		return false;

	metaData.m_versionNumber = 0;
	metaData.m_levelName.clear();
	metaData.m_saveTime.clear();
	metaData.m_checkPointId = 0;

	//read meta data
	i32 numAttribs = data->getNumAttributes();
	tukk key, * value;
	tukk checkpointName = NULL;
	for (i32 i = 0; i < numAttribs; ++i)
	{
		data->getAttributeByIndex(i, &key, &value);

		if (!stricmp("Version", key))
		{
			metaData.m_versionNumber = atoi(value);
		}
		else if (!stricmp("CheckpointId", key))
		{
			metaData.m_checkPointId = EntityId(atoi(value));
		}
		else if (!stricmp("CheckpointName", key))
		{
			checkpointName = value;
		}
		else if (!stricmp("LevelName", key))
		{
			metaData.m_levelName = value;
		}
		else if (!stricmp("Timestamp", key))
		{
			metaData.m_saveTime = value;
		}
	}

	//EntityId's may change on level export -> fix id
	if (checkpointName && bRepairId)
	{
		if (!RepairEntityId(metaData.m_checkPointId, checkpointName))
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed finding checkpoint entity during loading, flowgraph might be broken.");
	}

	//check all values have been read
	DRX_ASSERT(metaData.m_levelName.size() > 0);
	DRX_ASSERT(metaData.m_saveTime.size() > 0);
	//DRX_ASSERT(metaData.m_checkPointId);

	return true;
}

//////////////////////////////////////////////////////////////////////////
//this respawns the active AI at their spawn locations
void CCheckpointSystem::RespawnAI(XmlNodeRef data)
{
	if (!data)
		return;

	XmlNodeRef actorData = data->findChild(ACTOR_FLAGS_SECTION);
	if (!actorData)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed reading actor data from checkpoint, actors won't be respawned");
		return;
	}

	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();

	//first run through all actors and hide/deactivate them
	IActorIteratorPtr it = pActorSystem->CreateActorIterator();
	while (IActor* pActor = it->Next())
	{
		IEntity* pEntity = pActor->GetEntity();
		//deactivate all actors
		pEntity->Hide(true);
	}

	//load actorflags for active actors
	XmlNodeRef activatedActors = actorData->findChild(ACTIVATED_ACTORS_SECTION);
	if (activatedActors)
	{
		i32 actorFlags = activatedActors->getNumAttributes();
		tukk key;
		tukk value;
		for (i32 i = 0; i < actorFlags; ++i)
		{
			activatedActors->getAttributeByIndex(i, &key, &value);
			//format is "idXXX"
			DRX_ASSERT(strlen(key) > 2);
			EntityId id = (EntityId)(atoi(&key[2]));
			bool foundEntity = RepairEntityId(id, value);
			if (foundEntity)
			{
				IActor* pActor = pActorSystem->GetActor(id);
				if (pActor)
				{
					pActor->GetEntity()->Hide(false);
				}
				else
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed finding actor %i from checkpoint.", (i32)id);
			}
			else
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed finding actor %s from checkpoint, actor is not setup correctly.", value);
		}
	}
	else
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Deactivated actors section was missing in checkpoint.");

	it = pActorSystem->CreateActorIterator();
	//iterate all actors and respawn if active
	while (IActor* pActor = it->Next())
	{
		IEntity* pEntity = pActor->GetEntity();
		if (pEntity->GetId() == LOCAL_PLAYER_ENTITY_ID) //don't respawn player
			continue;

		//we don't respawn deactivated actors
		if (!pEntity->IsHidden() && pEntity->IsActivatedForUpdates())
		{
			pActor->SetHealth(0);
			pActor->Respawn();
		}
		else //but we still reset their position
		{
			pActor->ResetToSpawnLocation();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::ReadGameTokens(XmlNodeRef parentNode)
{
	//get source node
	XmlNodeRef node = parentNode->findChild(GAMETOKEN_SECTION);
	if (!node)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Couldn't find Gametoken section while reading checkpoint.");
		return;
	}

	//create serialization reader
	IXmlSerializer* pSerializer = GetISystem()->GetXmlUtils()->CreateXmlSerializer();
	ISerialize* pReader = pSerializer->GetReader(node);

	//read serialization data
	TSerialize ser = TSerialize(pReader);
	IGameTokenSystem* pTokenSystem = CDrxAction::GetDrxAction()->GetIGameTokenSystem();
	pTokenSystem->Serialize(ser);

	pSerializer->Release();
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CCheckpointSystem::ReadXML(tukk fileName)
{
	IPlayerProfileUpr* pPlayerProfMan = CDrxAction::GetDrxAction()->GetIPlayerProfileUpr();
	;

	string path;
	if (!pPlayerProfMan)
	{
		//on consoles there is no profile manager
		path = CONSOLE_SAVEGAME_DIRECTORY;
	}
	else
	{
		tukk sharedSaveGameFolder = pPlayerProfMan->GetSharedSaveGameFolder();
		path = sharedSaveGameFolder;
	}

	path = PathUtil::AddSlash(path);
	path.append(fileName);

	//read XML data from given checkpoint file
	_smart_ptr<IXmlParser> xmlParser;
	xmlParser.reset(GetISystem()->GetXmlUtils()->CreateXmlParser());
	XmlNodeRef data = xmlParser->ParseFile(path.c_str(), true);

	if (!data)
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed reading checkpoint at %s", path.c_str());

	return data;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::RestartGameplay()
{
	//if paused - start game
	gEnv->pGameFramework->PauseGame(false, true);

	//let game restart
	if (m_pGameHandler)
	{
		m_pGameHandler->OnRestartGameplay();
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::OnCheckpointLoaded(SCheckpointData metaData)
{
	IEntity* pCheckpoint = gEnv->pEntitySystem->GetEntity(metaData.m_checkPointId);
	if (pCheckpoint)
	{
		//Trigger OnLoad
		IScriptTable* pScript = pCheckpoint->GetScriptTable();
		if (pScript)
		{
			HSCRIPTFUNCTION hScriptFunc(NULL);
			pScript->GetValue("Event_OnLoadCheckpoint", hScriptFunc);

			if (hScriptFunc) //this will trigger the flowgraph output
			{
				IScriptSystem* pIScriptSystem = gEnv->pScriptSystem;
				Script::Call(pIScriptSystem, hScriptFunc, pScript);
				pIScriptSystem->ReleaseFunc(hScriptFunc);
			}
		}
	}
}

//SHARED ********************************

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::GetMetaData(tukk fileName, SCheckpointData& metaData, bool bRepairId /*=true*/)
{
	XmlNodeRef data = ReadXML(fileName);
	if (!data)
		return false;

	//process meta data
	return ReadMetaData(data, metaData, bRepairId);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::SetFilenameExtension(FixedCheckpointString& fileName)
{
	i32 fileNameLen = fileName.length();

	//check for empty string
	if (fileNameLen == 0)
	{
		fileName = "placeholder";
		fileName.append(FILENAME_EXTENSION);
		return;
	}

	//look up extension
	size_t pos = fileName.find(FILENAME_EXTENSION);
	if (pos != FixedCheckpointString::npos && pos == (fileNameLen - strlen(FILENAME_EXTENSION)))
		return;

	//look up and remove wrong extension
	pos = fileName.rfind('.');
	if (pos != FixedCheckpointString::npos)
	{
		fileName = fileName.substr(0, pos);
	}

	//add extension
	fileName.append(FILENAME_EXTENSION);
}

//////////////////////////////////////////////////////////////////////////
bool CCheckpointSystem::RepairEntityId(EntityId& id, tukk pEntityName)
{
	//EntityId's may change on level export -> fix id
	if (pEntityName)
	{
		//test the original entity id
		IEntity* pOriginalEntity = gEnv->pEntitySystem->GetEntity(id);
		if (pOriginalEntity && !stricmp(pOriginalEntity->GetName(), pEntityName))
			return true; //seems correct

		IEntity* pNewEntity = gEnv->pEntitySystem->FindEntityByName(pEntityName);
		if (!pNewEntity)
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Entity %s in loaded checkpoint could not be found. This means the checkpoint file is not compatible with this level version.", pEntityName);
		else if (!stricmp(pNewEntity->GetName(), pEntityName))
		{
			//if the level was re-exported, the entity id might differ
			CHECKPOINT_RESAVE_NECESSARY = true;
			//this is a weakness of our entity system/editor and might be fixed in future
			id = pNewEntity->GetId();
			return true;
		}
		return false;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::RepairLevelName(DrxFixedStringT<32>& levelName)
{
	if (levelName.empty())
		return;
	i32 posSlash = levelName.rfind('/');
	if (posSlash != levelName.npos)  //This is a work-around for the editor/fullgame incompatible level name convention
		levelName = levelName.substr(posSlash + 1, levelName.size() - posSlash);
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::SetGameHandler(ICheckpointGameHandler* pHandler)
{
	DRX_ASSERT(pHandler);
	m_pGameHandler = pHandler;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::ClearGameHandler()
{
	m_pGameHandler = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::RegisterListener(ICheckpointListener* pListener)
{
	DRX_ASSERT(pListener);

	//check for existing listener
	if (std::find(g_vCheckpointSystemListeners.begin(), g_vCheckpointSystemListeners.end(), pListener) == g_vCheckpointSystemListeners.end())
	{
		//add listener
		g_vCheckpointSystemListeners.push_back(pListener);
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::RemoveListener(ICheckpointListener* pListener)
{
	DRX_ASSERT(pListener);

	//look up listener
	std::list<ICheckpointListener*>::iterator it = std::find(g_vCheckpointSystemListeners.begin(), g_vCheckpointSystemListeners.end(), pListener);
	if (it != g_vCheckpointSystemListeners.end())
	{
		//remove listener
		g_vCheckpointSystemListeners.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::UpdateListener(SCheckpointData data, bool wasSaving)
{
	//external listeners may also read/write checkpoint data now
	std::list<ICheckpointListener*>::iterator it = g_vCheckpointSystemListeners.begin();
	std::list<ICheckpointListener*>::iterator end = g_vCheckpointSystemListeners.end();
	for (; it != end; ++it)
	{
		//send update - the global checkpoint flownode also is set here, but is not triggered until the flowgraph updates
		if (wasSaving)
			(*it)->OnSave(&data, GetInstance());
		else
			(*it)->OnLoad(&data, GetInstance());
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::SerializeWorldTM(IEntity* pEntity, XmlNodeRef data, bool writing)
{
	if (!pEntity || !data)
		return;

	if (writing)
	{
		//write all TM columns to checkpoint node
		const Matrix34& tm = pEntity->GetWorldTM();
		data->setAttr("TMcol0", tm.GetColumn0());
		data->setAttr("TMcol1", tm.GetColumn1());
		data->setAttr("TMcol2", tm.GetColumn2());
		data->setAttr("TMcol3", tm.GetColumn3());
	}
	else
	{
		//read and set TM columns from node
		Matrix34 tm;
		Vec3 temp = Vec3(0, 0, 0);
		bool foundData = data->getAttr("TMcol0", temp);
		tm.SetColumn(0, temp);
		foundData &= data->getAttr("TMcol1", temp);
		tm.SetColumn(1, temp);
		foundData &= data->getAttr("TMcol2", temp);
		tm.SetColumn(2, temp);
		foundData &= data->getAttr("TMcol3", temp);
		tm.SetColumn(3, temp);
		DRX_ASSERT(foundData);
		//set matrix to entity
		pEntity->SetWorldTM(tm);
	}
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::DeleteDynamicEntities()
{
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;
	IEntityItPtr pIt = pEntitySystem->GetEntityIterator();
	//////////////////////////////////////////////////////////////////////////
	pIt->MoveFirst();
	while (!pIt->IsEnd())
	{
		IEntity* pEntity = pIt->Next();
		u32 nEntityFlags = pEntity->GetFlags();

		// Local player must not be deleted.
		if (nEntityFlags & ENTITY_FLAG_LOCAL_PLAYER)
			continue;

		if (nEntityFlags & ENTITY_FLAG_SPAWNED)
			pEntitySystem->RemoveEntity(pEntity->GetId());
	}
	// Force deletion of removed entities.
	pEntitySystem->DeletePendingEntities();
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::GetCurrentLevelCheckpoints(std::vector<string>& saveNames)
{
	//get current level
	tukk levelName = CDrxAction::GetDrxAction()->GetLevelName();
	DrxFixedStringT<32> curlevelName = levelName;
	if (levelName)
		CCheckpointSystem::GetInstance()->RepairLevelName(curlevelName);

	//parse savegames fitting current level
	ParseSavedFiles(saveNames, curlevelName.c_str());
}

//////////////////////////////////////////////////////////////////////////
void CCheckpointSystem::ParseSavedFiles(std::vector<string>& saveNames, tukk pLevelName)
{
	//create search index
	string search = string(CONSOLE_SAVEGAME_DIRECTORY) + "/*" + string(FILENAME_EXTENSION);

	//scan savegame directory
	_finddata_t fd;
	intptr_t handle = gEnv->pDrxPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			//if levelname is provided, check metaData
			if (pLevelName)
			{
				//try to get meta data for save file
				SCheckpointData metaData;
				if (!CCheckpointSystem::GetInstance()->GetMetaData(fd.name, metaData, false))
				{
					DRX_ASSERT(0); //shouldn't happen
					continue;
				}

				if (stricmp(metaData.m_levelName.c_str(), pLevelName))
					continue;
			}

			//add filename to list
			saveNames.push_back(fd.name);
		}
		while (gEnv->pDrxPak->FindNext(handle, &fd) >= 0);

		gEnv->pDrxPak->FindClose(handle);
	}
}

//~CCheckpointSystem
