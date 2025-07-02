// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Class for managing a series of SimpleEntityObjectives based on xml
		parameterisation

	-------------------------------------------------------------------------
	История:
	- 20:10:2009  : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRulesSimpleEntityBasedObjective.h>
#include <drx3D/Sys/XML/IXml.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Utility/DrxWatch.h>

#include <drx3D/Game/GameRulesModules/GameRulesCombiCaptureObjective.h>
#include <drx3D/Game/GameRulesModules/GameRulesKingOfTheHillObjective.h>
#include <drx3D/Game/GameRulesModules/IGameRulesStateModule.h>
#include <drx3D/Game/GameRulesModules/GameRulesSpawningBase.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>
#include <drx3D/Game/ActorUpr.h>

#define SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY		0x80
#define SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_REMOVE								0x40
#define SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_NEW										0x20

#ifndef _RELEASE
CGameRulesSimpleEntityBasedObjective *CGameRulesSimpleEntityBasedObjective::s_instance = NULL;
#endif

//------------------------------------------------------------------------
CGameRulesSimpleEntityBasedObjective::CGameRulesSimpleEntityBasedObjective()
{
	m_pObjective = NULL;
	m_moduleRMIIndex = 0;
	g_pGame->GetIGameFramework()->RegisterListener(this, "simpleentitybasedobjective", FRAMEWORKLISTENERPRIORITY_GAME);

#ifndef _RELEASE
	// Make sure only one instance is saved, this is not a singleton class
	if (s_instance == NULL)
	{
		s_instance = this;

		REGISTER_COMMAND("g_selectNewRandomObjectiveEntities", CmdNewRandomEntity, VF_NULL, "");
	}
#endif
}

//------------------------------------------------------------------------
CGameRulesSimpleEntityBasedObjective::~CGameRulesSimpleEntityBasedObjective()
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (pGameRules)
	{
		pGameRules->UnRegisterModuleRMIListener(m_moduleRMIIndex);
		pGameRules->UnRegisterClientConnectionListener(this);
	}
	gEnv->pEntitySystem->RemoveSink(this);
	SAFE_DELETE(m_pObjective);
	g_pGame->GetIGameFramework()->UnregisterListener(this);

#ifndef _RELEASE
	// Only clear if we're the registered instance, this is not a singleton class
	if (s_instance == this)
	{
		s_instance = NULL;
		gEnv->pConsole->RemoveCommand("g_selectNewRandomObjectiveEntities");
	}
#endif
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::LoadRandomEntitySelectData(XmlNodeRef xmlChild, SEntityDetails& entityDetails) const
{
	if (xmlChild->getAttr("time", entityDetails.m_randomChangeTimeLength))
	{
		entityDetails.m_randomChangeTimeLength = MAX(entityDetails.m_randomChangeTimeLength, 1.f);

		entityDetails.m_useRandomChangeTimer = true;
		entityDetails.m_timeToRandomChange = entityDetails.m_randomChangeTimeLength;

		xmlChild->getAttr("timeBetweenLocations", entityDetails.m_timeBetweenSites);
	}

	xmlChild->getAttr("count", entityDetails.m_numRandomEntities);
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::Init(XmlNodeRef xml)
{
	i32 numChildren = xml->getChildCount();
	for (i32 childIndex = 0; childIndex < numChildren; ++ childIndex)
	{
		XmlNodeRef xmlChild = xml->getChild(childIndex);

		tukk pChildTag = xmlChild->getTag();
		if (!stricmp(pChildTag, "Entity"))
		{
			SEntityDetails entityDetails;

			tukk pEntityClass = 0;
			tukk pSelectType = 0;
			if (xmlChild->getAttr("class", &pEntityClass))
			{
				entityDetails.m_pEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pEntityClass);
				DrxLog("CGameRulesSimpleEntityBasedObjective::Init, Entity class='%s'", pEntityClass);
			}
			else
			{
				DrxLog("CGameRulesSimpleEntityBasedObjective::Init, expected 'class' attribute within 'Entity' node");
				DRX_ASSERT_MESSAGE(false, "We require 'class' attribute within 'Entity' node");
			}

			if (xmlChild->getAttr("select", &pSelectType))
			{
				if (!stricmp(pSelectType, "All"))
				{
					entityDetails.m_entitySelectType = EEST_All;
					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, Selection type='All'");
				}
				else if (!stricmp(pSelectType, "Random"))
				{
					entityDetails.m_entitySelectType = EEST_Random;

					LoadRandomEntitySelectData(xmlChild, entityDetails);

					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, Selection type='Random', count='%i'", entityDetails.m_numRandomEntities);
				}
				else if (!stricmp(pSelectType, "Random_Weighted"))
				{
					entityDetails.m_entitySelectType = EEST_Random_Weighted;

					LoadRandomEntitySelectData(xmlChild, entityDetails);

					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, Selection type='Random_Weighted', count='%i'", entityDetails.m_numRandomEntities);
				}
				else
				{
					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, unknown 'select' attribute within 'Entity' node");
					DRX_ASSERT_MESSAGE(false, "Unknown entity select type");
				}
			}

			i32 scan = 0;
			if (xmlChild->getAttr("scan", scan))
			{
				entityDetails.m_scan = (scan != 0);
			}

			tukk pUpdateType = 0;
			if (xmlChild->getAttr("update", &pUpdateType))
			{
				if (!stricmp(pUpdateType, "all"))
				{
					entityDetails.m_updateType = SEntityDetails::eUT_All;
				}
				else if (!stricmp(pUpdateType, "active"))
				{
					entityDetails.m_updateType = SEntityDetails::eUT_Active;
				}
			}

			// Only add the entity if it is a new class
			bool addEntity = true;
			i32 numEntityTypes = m_entityDetails.size();
			for (i32 i = 0; i < numEntityTypes; ++ i)
			{
				if (m_entityDetails[i].m_pEntityClass == entityDetails.m_pEntityClass)
				{
					DRX_ASSERT_MESSAGE(false, "Duplicate entity class detected!");
					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, ERROR: duplicate entity class detected, entity definitions must be unique");
					addEntity = false;
				}
			}

			if (addEntity)
			{
				m_entityDetails.push_back(entityDetails);
			}
		}
		else if (!stricmp(pChildTag, "Implementation"))
		{
			tukk pImplementationType = 0;
			if (xmlChild->getAttr("type", &pImplementationType))
			{
				if (!stricmp(pImplementationType, "CombiCapture"))
				{
					m_pObjective = new CGameRulesCombiCaptureObjective();
				}
				else if (!stricmp(pImplementationType, "KingOfTheHill"))
				{
					m_pObjective = new CGameRulesKingOfTheHillObjective();
				}
				else
				{
					DrxLog("CGameRulesSimpleEntityBasedObjective::Init, unknown implementation type given ('%s')", pImplementationType);
					DRX_ASSERT_MESSAGE(false, "Unknown implementation type given");
				}
				if (m_pObjective)
				{
					m_pObjective->Init(xmlChild);
				}
			}
			else
			{
				DrxLog("CGameRulesSimpleEntityBasedObjective::Init, implementation type not given");
				DRX_ASSERT_MESSAGE(false, "Implementation type not given");
			}
		}
	}

	CGameRules *pGameRules = g_pGame->GetGameRules();
	m_moduleRMIIndex = pGameRules->RegisterModuleRMIListener(this);
	pGameRules->RegisterClientConnectionListener(this);

	gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnSpawn | IEntitySystem::OnRemove, 0);

	DRX_ASSERT_MESSAGE(m_pObjective, "Sub-objective not created, this will crash!");
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::Update(float frameTime)
{
	// for PC pre-game handling do nothing till we're actually in game
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules->HasGameActuallyStarted() && !pGameRules->IsPrematchCountDown())
	{
		return;
	}

#if DRX_WATCH_ENABLED
	if (g_pGameCVars->g_SimpleEntityBasedObjective_watchLvl > 0)
	{
		DrxWatch("[CGameRulesSimpleEntityBasedObjective::Update()]");
		DrxWatch(" Carry entity details:");
		SEntityDetails*  pEntityDetails = &m_entityDetails[0];
		i32  numSlots = pEntityDetails->m_allEntities.size();
		for (i32 i=0; i<numSlots; i++)
		{
			IEntity*  e = gEnv->pEntitySystem->GetEntity(pEntityDetails->m_allEntities[i]);
			DrxWatch("  %d: ent %d '%s'", i, pEntityDetails->m_allEntities[i], (e?e->GetName():"?"));
		}
	}
#endif

	i32 numEntityTypes = m_entityDetails.size();
	if (gEnv->bServer)
	{
		for (i32 entityType = 0; entityType < numEntityTypes; ++ entityType)
		{
			SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

			i32 remainingEntities = 0;

			i32 numEntities = pEntityDetails->m_currentEntities.size();
			for (i32 i = 0; i < numEntities; ++ i)
			{
				if (pEntityDetails->m_currentEntities[i])
				{
					if (m_pObjective->IsEntityFinished(entityType, i))
					{
						SvRemoveEntity(entityType, i, true);
					}
					else
					{
						++ remainingEntities;
					}
				}
			}

			if (!remainingEntities)
			{
				// No remaining entities, jump straight to 'between sites' state
				if (pEntityDetails->m_timeToRandomChange > pEntityDetails->m_timeBetweenSites)
				{
					pEntityDetails->m_timeToRandomChange = pEntityDetails->m_timeBetweenSites;
				}
			}

			if (pEntityDetails->m_useRandomChangeTimer)
			{
				float oldValue = pEntityDetails->m_timeToRandomChange;
				pEntityDetails->m_timeToRandomChange -= frameTime;

				const float percLifeGone = 1.f - (pEntityDetails->m_timeToRandomChange / pEntityDetails->m_randomChangeTimeLength);
				m_pObjective->OnTimeTillRandomChangeUpdated(entityType, percLifeGone);

				if (oldValue >= pEntityDetails->m_timeBetweenSites && pEntityDetails->m_timeToRandomChange < pEntityDetails->m_timeBetweenSites)
				{
					bool canRemoveEntities = true;

					// Check if we can remove current entities
					for (i32 i = 0; i < numEntities; ++ i)
					{
						if (pEntityDetails->m_currentEntities[i])
						{
							canRemoveEntities &= m_pObjective->CanRemoveEntity(entityType, i);
						}
					}

					if (canRemoveEntities)
					{
						for (i32 i = 0; i < numEntities; ++ i)
						{
							if (pEntityDetails->m_currentEntities[i])
							{
								SvRemoveEntity(entityType, i, true);
							}
						}
					}
					else
					{
						pEntityDetails->m_timeToRandomChange = oldValue;
					}
				}

				IGameRulesStateModule::EGR_GameState state = IGameRulesStateModule::EGRS_InGame;
				bool bCountDownStarted = false;
				if (IGameRulesStateModule *pStateModule = pGameRules->GetStateModule())
				{
					state							= pStateModule->GetGameState();
					bCountDownStarted = pStateModule->IsInCountdown();
				}

				if ((pEntityDetails->m_timeToRandomChange < 0.f) && 
					((state == IGameRulesStateModule::EGRS_InGame) ||
					 (state == IGameRulesStateModule::EGRS_PreGame && bCountDownStarted && pGameRules->GetRemainingStartTimer() < g_pGameCVars->g_HoldObjective_secondsBeforeStartForSpawn)))
				{
					// Add new site
					SvDoRandomTypeSelection(entityType);
				}
			}
		}
	}
	else
	{
		for (i32 entityType = 0; entityType < numEntityTypes; ++ entityType)
		{
			// Keep random site change timer vaguely up to date for host migration purposes
			SEntityDetails *pEntityDetails = &m_entityDetails[entityType];
			if (pEntityDetails->m_useRandomChangeTimer)
			{
				pEntityDetails->m_timeToRandomChange -= frameTime;

				const float percLifeGone = 1.f - (pEntityDetails->m_timeToRandomChange / pEntityDetails->m_randomChangeTimeLength);
				m_pObjective->OnTimeTillRandomChangeUpdated(entityType, percLifeGone);
			}
		}
	}

	for (i32 entityType = 0; entityType < numEntityTypes; ++ entityType)
	{
		SEntityDetails *pEntityDetails = &m_entityDetails[entityType];
		if (pEntityDetails->m_updateType == SEntityDetails::eUT_All)
		{
			CallScriptUpdateFunction(pEntityDetails->m_allEntities, frameTime);
		}
		else if (pEntityDetails->m_updateType == SEntityDetails::eUT_Active)
		{
			CallScriptUpdateFunction(pEntityDetails->m_currentEntities, frameTime);
		}
	}

	m_pObjective->Update(frameTime);
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::CallScriptUpdateFunction( TEntityIdVec &entitiesVec, float frameTime )
{
	IScriptSystem *pScriptSystem = gEnv->pScriptSystem;

	i32 numEntities = entitiesVec.size();
	for (i32 i = 0; i < numEntities; ++ i)
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entitiesVec[i]);
		if (pEntity)
		{
			IScriptTable *pScript = pEntity->GetScriptTable();
			if (pScript != NULL && pScript->GetValueType("Update") == svtFunction)
			{
				pScriptSystem->BeginCall(pScript, "Update");
				pScriptSystem->PushFuncParam(pScript);
				pScriptSystem->PushFuncParam(frameTime);
				pScriptSystem->EndCall();
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnStartGame()
{
	CGameRules *pGameRules = g_pGame->GetGameRules();

	if (gEnv->bServer)
	{
		i32 numEntityTypes = m_entityDetails.size();
		for (i32 entityType = 0; entityType < numEntityTypes; ++ entityType)
		{
			SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

			pEntityDetails->m_allEntities.clear();
			pEntityDetails->m_currentEntities.clear();

			IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
			IEntity *pEntity = 0;

			pIt->MoveFirst();

			while(pEntity = pIt->Next())
			{
				if (pEntity->GetClass() == pEntityDetails->m_pEntityClass)
				{
					EntityId entId = pEntity->GetId();
					i32 index = pEntityDetails->m_allEntities.size();

					pEntityDetails->m_allEntities.push_back(entId);

					CGameRules::SModuleRMIEntityParams params(m_moduleRMIIndex, entId, index | SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY);
					g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMISingleEntity(), params, eRMI_ToRemoteClients, entId);
				}
			}

			pEntityDetails->m_currentEntities.reserve(pEntityDetails->m_allEntities.size());

			m_pObjective->ClearEntities(entityType);
			switch (pEntityDetails->m_entitySelectType)
			{
			case EEST_All:
				{
					i32 numEntities = pEntityDetails->m_allEntities.size();
					for (i32 i = 0; i < numEntities; ++ i)
					{
						EntityId entId = pEntityDetails->m_allEntities[i];
						SvAddEntity(entityType, entId, i);
					}
				}
				break;
			case EEST_Random:
			case EEST_Random_Weighted:	//Deliberate fall-through
				{
					// All existing entities are available for selection
					pEntityDetails->m_availableEntities = pEntityDetails->m_allEntities;
					pEntityDetails->m_timeToRandomChange = -1.f;
				}
				break;
			}
		}
	}

	m_pObjective->OnStartGame();
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvDoRandomTypeSelection(i32 entityType)
{
	DRX_ASSERT(entityType < (i32)m_entityDetails.size());
	SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

	if(pEntityDetails->m_entitySelectType == EEST_Random)
	{
		SvDoTrueRandomSelection(entityType, pEntityDetails);
	}
	else if (pEntityDetails->m_entitySelectType == EEST_Random_Weighted)
	{
		SvDoRandomWeightedSelection(entityType, pEntityDetails);
	}

	pEntityDetails->m_timeToRandomChange = pEntityDetails->m_randomChangeTimeLength;
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvDoTrueRandomSelection(i32 entityType, SEntityDetails *pEntityDetails)
{
	i32k numEntitiesToChoose = pEntityDetails->m_numRandomEntities;
	i32 numAvailable = pEntityDetails->m_availableEntities.size();

	for (i32 i = 0; i < numEntitiesToChoose; ++ i)
	{
		if (numAvailable)
		{
			numAvailable += SvResetAvailableEntityList(entityType);
		}
		if (numAvailable)
		{
			i32 rndIndex = g_pGame->GetRandomNumber() % numAvailable;
			EntityId entId = pEntityDetails->m_availableEntities[rndIndex];
			SvAddEntity(entityType, entId, i);
			stl::find_and_erase(pEntityDetails->m_availableEntities, entId);
			-- numAvailable;
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvDoRandomWeightedSelection(i32 entityType, SEntityDetails *pEntityDetails)
{
#ifndef _RELEASE
	if(pEntityDetails->m_numRandomEntities > 1)
		DrxFatalError("Entity selection type Random_Weighted only supports 1 entity per cycle");
#endif

	if (pEntityDetails->m_availableEntities.empty())
	{
		SvResetAvailableEntityList(entityType);
	}

	CGameRules * pGameRules = g_pGame->GetGameRules();
	IGameRulesStateModule			*pStateModule = pGameRules->GetStateModule();
	IGameRulesStateModule::EGR_GameState gameState = pStateModule->GetGameState();
	const CGameRulesSpawningBase * pSpawningModule = static_cast<const CGameRulesSpawningBase*>(g_pGame->GetGameRules()->GetSpawningModule());
	if ( (gameState == IGameRulesStateModule::EGRS_PreGame || gameState == IGameRulesStateModule::EGRS_Intro || pGameRules->IsPrematchCountDown()) && pSpawningModule->HasInitialSpawns())
	{
		//We're in the state where players will be assigned initial spawns. Avoid selecting any points that are near initial spawns.
		DrxLog("SvDoRandomWeightedSelection(), avoiding initial spawns as gamestate is %d, prematch countdown is '%s' and initial spawns are <%s>", gameState, pGameRules->IsPrematchCountDown() ? "happening" : "not happening", pSpawningModule->HasInitialSpawns() ? "present" : "not present");
		SvDoRandomSelectionAvoidingInitialSpawns(entityType, pEntityDetails, pSpawningModule);
	}
	else
	{
		DrxLog("SvDoRandomWeightedSelection(), avoiding actors as gamestate is %d, prematch countdown is '%s' and initial spawns are <%s>", gameState, pGameRules->IsPrematchCountDown() ? "happening" : "not happening", pSpawningModule->HasInitialSpawns() ? "present" : "not present");
		//We're in game. Avoid spawning next to any existing players. They'll move, mind, so this won't be perfect.
		SvDoRandomSelectionAvoidingActors(entityType, pEntityDetails);
	}
}


//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvDoRandomSelectionAvoidingInitialSpawns(i32 entityType, SEntityDetails * pEntityDetails, const CGameRulesSpawningBase * pSpawningModule)
{
	i32 numAvailable = pEntityDetails->m_availableEntities.size();

	const TEntityIdVec& initialSpawns = pSpawningModule->GetInitialSpawns();

	i32k numInitialSpawns = initialSpawns.size();

	DrxLog("SvDoRandomSelectionAvoidingInitialSpawns(), %d objectives available, %d initial spawns", numAvailable, numInitialSpawns);

	struct SObjectiveSelectionInfo
	{
		SObjectiveSelectionInfo() : idx(-1), fDistSqToClosestSpawn(0.0f) {}
		i32 idx;
		float fDistSqToClosestSpawn;
	};

	float fWorstDistSqToSpawn = -1.0f;
	i32		nWorstIdx						= 0;

	i32k kNumObjectivesToSelectFrom = 3;
	SObjectiveSelectionInfo objectiveSelectionInfo[kNumObjectivesToSelectFrom];


	//As we're going to be looping over the spawn positions multiple times, we want to cache to locations to
	//	avoid lots of virtual calls
	std::vector<Vec3> spawnPositions;
	spawnPositions.reserve(numInitialSpawns);

	for (i32 i = 0; i < numInitialSpawns; i++)
	{
		IEntity * pSpawnEntity = gEnv->pEntitySystem->GetEntity(initialSpawns[i]);

		if(pSpawnEntity)
		{
			spawnPositions.push_back(pSpawnEntity->GetWorldPos());
		}
	}


	//Now make a list of the objectives that are furthest away from the spawns to randomly select from
	i32k numSpawnPositions = spawnPositions.size();

	DRX_ASSERT(numSpawnPositions == numInitialSpawns);

	for (i32 i = 0; i < numAvailable; i++)
	{
		EntityId entId = pEntityDetails->m_availableEntities[i];

		if(IEntity * pEntity = gEnv->pEntitySystem->GetEntity(entId))
		{
			Vec3 entityPos = pEntity->GetWorldPos();

			float fDistSq = FLT_MAX;

			//Find the distance of closest spawn to the objective
			for (i32 j = 0; j < numSpawnPositions; j++)
			{
				fDistSq = min(fDistSq, spawnPositions[j].GetSquaredDistance(entityPos));
			}

			//If this is further away than the closest of the selection we currently have stored, it's better
			if(fDistSq > fWorstDistSqToSpawn)
			{
				//Replace the closest objective position that we have stored
				objectiveSelectionInfo[nWorstIdx].fDistSqToClosestSpawn = fDistSq;
				objectiveSelectionInfo[nWorstIdx].idx										= i;

				//Update the closest 
				float fNewWorstDistSqToSpawn = FLT_MAX;
				i32		newWorstIdx = 0;
				for(i32 j = 0; j < kNumObjectivesToSelectFrom; j++)
				{
					if(objectiveSelectionInfo[j].fDistSqToClosestSpawn < fNewWorstDistSqToSpawn)
					{
						fNewWorstDistSqToSpawn = objectiveSelectionInfo[j].fDistSqToClosestSpawn;
						newWorstIdx = j;
					}
				}

				nWorstIdx = newWorstIdx;

				fWorstDistSqToSpawn = objectiveSelectionInfo[nWorstIdx].fDistSqToClosestSpawn;
			}
		}
	}

#ifndef _RELEASE
	for( i32 i = 0; i < kNumObjectivesToSelectFrom; i++ )
	{
		if(objectiveSelectionInfo[i].idx != -1)
			DrxLog(" option '%s' as closest actor is %f away", gEnv->pEntitySystem->GetEntity(pEntityDetails->m_availableEntities[objectiveSelectionInfo[i].idx])->GetName(), sqrt(objectiveSelectionInfo[i].fDistSqToClosestSpawn));
	}
#endif

	//We now have a list of kNumObjectivesToSelectFrom objective positions to randomly select from.
	//We also need to handle the potential for the level to have fewer than kNumObjectivesToSelectFrom positions
	i32 randomIdx = drx_random(0, kNumObjectivesToSelectFrom - 1);
	
	while(objectiveSelectionInfo[randomIdx].idx == -1)
		randomIdx = drx_random(0, kNumObjectivesToSelectFrom - 1);

	EntityId chosenObjectiveId = pEntityDetails->m_availableEntities[objectiveSelectionInfo[randomIdx].idx];

#ifndef _RELEASE
	DrxLog(" CHOSEN: '%s'", gEnv->pEntitySystem->GetEntity(chosenObjectiveId)->GetName());
#endif

	SvAddEntity(entityType, chosenObjectiveId, objectiveSelectionInfo[randomIdx].idx);

	stl::find_and_erase(pEntityDetails->m_availableEntities, chosenObjectiveId);
}


//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvDoRandomSelectionAvoidingActors(i32 entityType, SEntityDetails * pEntityDetails)
{
	CActorUpr * pActorUpr = CActorUpr::GetActorUpr();
	
	pActorUpr->PrepareForIteration();

	float fMinDistSq = -1.0f;
	i32 furthestIdx = -1;
	i32 numAvailable = pEntityDetails->m_availableEntities.size();

	for (i32 j = 0; j < numAvailable; j++)
	{
		EntityId entId = pEntityDetails->m_availableEntities[j];

		if(IEntity * pEntity = gEnv->pEntitySystem->GetEntity(entId))
		{
			float fDistSq = pActorUpr->GetDistSqToClosestActor(pEntity->GetWorldPos());

			if(fDistSq > fMinDistSq)
			{
				fMinDistSq = fDistSq;
				furthestIdx = j;
			}
		}
	}

	DrxLog("SvDoRandomSelectionAvoidingActors() selecting '%s' as closest actor is %f away", gEnv->pEntitySystem->GetEntity(pEntityDetails->m_availableEntities[furthestIdx])->GetName(), sqrt(fMinDistSq));

	EntityId chosenObjectiveId = pEntityDetails->m_availableEntities[furthestIdx];

	SvAddEntity(entityType, chosenObjectiveId, furthestIdx);

	stl::find_and_erase(pEntityDetails->m_availableEntities, chosenObjectiveId);
}

//------------------------------------------------------------------------
i32 CGameRulesSimpleEntityBasedObjective::SvResetAvailableEntityList(i32 entityType)
{
	DRX_ASSERT(entityType < (i32)m_entityDetails.size());
	SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

	i32 entitiesMadeAvailable = 0;
	i32 numEntities = pEntityDetails->m_allEntities.size();
	for (i32 i = 0; i < numEntities; ++ i)
	{
		// Copy all entities that aren't currently active into the available entities list
		if (!stl::find(pEntityDetails->m_currentEntities, pEntityDetails->m_allEntities[i]))
		{
			pEntityDetails->m_availableEntities.push_back(pEntityDetails->m_allEntities[i]);
			++ entitiesMadeAvailable;
		}
	}
	return entitiesMadeAvailable;
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvAddEntity( i32 entityType, EntityId id, i32 index )
{
	DRX_ASSERT(entityType < (i32)m_entityDetails.size());
	SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

	if ((i32)pEntityDetails->m_currentEntities.size() <= index)
	{
		pEntityDetails->m_currentEntities.resize(index + 1);
	}
	pEntityDetails->m_currentEntities[index] = id;

	i32 flags = 0;
	bool countAsNewEntity = false;
	if (pEntityDetails->m_useRandomChangeTimer)
	{
		flags = SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_NEW;
		countAsNewEntity = true;
	}
	CGameRules *pGameRules = g_pGame->GetGameRules();
	CTimeValue now(pGameRules->GetServerTime() * 0.001f);
	CGameRules::SModuleRMIEntityTimeParams params(m_moduleRMIIndex, id, index | flags, now);
	pGameRules->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMIEntityWithTime(), params, eRMI_ToRemoteClients, id);

	m_pObjective->AddEntityId(entityType, id, index, countAsNewEntity, now);
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::SvRemoveEntity(i32 entityType, i32 index, bool sendRMI)
{
	DRX_ASSERT(entityType < (i32)m_entityDetails.size());
	SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

	EntityId id = pEntityDetails->m_currentEntities[index];
	pEntityDetails->m_currentEntities[index] = 0;

	if (sendRMI)
	{
		CGameRules::SModuleRMIEntityParams params(m_moduleRMIIndex, id, SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_REMOVE);
		g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMISingleEntity(), params, eRMI_ToRemoteClients, id);
	}

	m_pObjective->RemoveEntityId(entityType, id);
}

//------------------------------------------------------------------------
bool CGameRulesSimpleEntityBasedObjective::NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags )
{
	return m_pObjective->NetSerialize(ser, aspect, profile, flags);
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnClientEnteredGame( i32 channelId, bool isReset, EntityId playerId )
{
	CGameRules *pGameRules = g_pGame->GetGameRules();

	i32 numEntityTypes = m_entityDetails.size();
	for (i32 entityType = 0; entityType < numEntityTypes; ++ entityType)
	{
		SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

		i32k numAllEntities = pEntityDetails->m_allEntities.size();
		for (i32 index = 0; index < numAllEntities; ++ index)
		{
			EntityId entId = pEntityDetails->m_allEntities[index];

			if (entId)
			{
				CGameRules::SModuleRMIEntityParams params(m_moduleRMIIndex, entId, index | SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY);
				g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMISingleEntity(), params, eRMI_ToClientChannel|eRMI_NoLocalCalls, entId, channelId);
			}
		}

		i32 flags = 0;
		float timeSinceAdded = 0.f;
		if (pEntityDetails->m_useRandomChangeTimer)
		{
			timeSinceAdded = pEntityDetails->m_randomChangeTimeLength - pEntityDetails->m_timeToRandomChange;
			flags |= SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_NEW;
		}

		i32 numEntities = pEntityDetails->m_currentEntities.size();
		for (i32 i = 0; i < numEntities; ++ i)
		{
			EntityId entId = pEntityDetails->m_currentEntities[i];

			if (entId)
			{
				float currentTime = pGameRules->GetServerTime() * 0.001f;
				float fTimeStarted = currentTime - timeSinceAdded;

				CTimeValue timeStarted(fTimeStarted);

				DrxLog("CGameRulesSimpleEntityBasedObjective::OnClientEnteredGame() timeSinceAdded=%f, timeStarted=%f, currentTime=%f", timeSinceAdded, fTimeStarted, currentTime);

				CGameRules::SModuleRMIEntityTimeParams params(m_moduleRMIIndex, entId, i | flags, timeStarted);
				pGameRules->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMIEntityWithTime(), params, eRMI_ToClientChannel|eRMI_NoLocalCalls, entId, channelId);
			}
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnSingleEntityRMI( CGameRules::SModuleRMIEntityParams params )
{
	// Determine which entity type we're dealing with
	i32 entityType = -1;
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(params.m_entityId);
	if (pEntity)
	{
		SEntityDetails *pEntityDetails = 0;
		i32 numEntityTypes = m_entityDetails.size();
		for (i32 i = 0; i < numEntityTypes; ++ i)
		{
			pEntityDetails = &m_entityDetails[i];

			if (pEntityDetails->m_pEntityClass == pEntity->GetClass())
			{
				entityType = i;
				break;
			}
		}
		if (entityType == -1)
		{
			DRX_ASSERT_MESSAGE(false, "Entity type not found");
			return;
		}

		// Work out if we're adding or removing
		i32 data = params.m_data;
		if (data & SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_REMOVE)
		{
			m_pObjective->RemoveEntityId(entityType, params.m_entityId);
			stl::find_and_erase(pEntityDetails->m_currentEntities, params.m_entityId);
		}
		else if (data & SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY)
		{
			i32 index = data & ~SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY;
			if (index >= (i32)pEntityDetails->m_allEntities.size())
			{
				pEntityDetails->m_allEntities.resize(index + 1);
			}
			DrxLog("CGameRulesSimpleEntityBasedObjective::OnSingleEntityRMI() index=%d, id=%u", index, params.m_entityId);
			pEntityDetails->m_allEntities[index] = params.m_entityId;
		}
	}
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnEntityWithTimeRMI( CGameRules::SModuleRMIEntityTimeParams params )
{
	// Determine which entity type we're dealing with
	i32 entityType = -1;
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(params.m_entityId);
	if (pEntity)
	{
		SEntityDetails *pEntityDetails = 0;
		i32 numEntityTypes = m_entityDetails.size();
		for (i32 i = 0; i < numEntityTypes; ++ i)
		{
			pEntityDetails = &m_entityDetails[i];

			if (pEntityDetails->m_pEntityClass == pEntity->GetClass())
			{
				entityType = i;
				break;
			}
		}
		if (entityType == -1)
		{
			DRX_ASSERT_MESSAGE(false, "Entity type not found");
			return;
		}

		i32 data = params.m_data;
		bool isNewEntity = (0 != (data & SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_NEW));
		data &= ~SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_NEW;
		if ((i32)pEntityDetails->m_currentEntities.size() <= data)
		{
			pEntityDetails->m_currentEntities.resize(data + 1);
		}
		pEntityDetails->m_currentEntities[data] = params.m_entityId;
		pEntityDetails->m_timeToRandomChange = pEntityDetails->m_randomChangeTimeLength;

		m_pObjective->AddEntityId(entityType, params.m_entityId, data, isNewEntity, params.m_time);
	}
}

//------------------------------------------------------------------------
bool CGameRulesSimpleEntityBasedObjective::HasCompleted(i32 teamId)
{
	return m_pObjective->IsComplete(teamId);
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnSpawn( IEntity *pEntity,SEntitySpawnParams &params )
{
	if (gEnv->bServer)
	{
		i32 numTypes = m_entityDetails.size();
		for (i32 entityType = 0; entityType < numTypes; ++ entityType)
		{
			SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

			if (pEntityDetails->m_scan && pEntity->GetClass() == pEntityDetails->m_pEntityClass)
			{
				EntityId entId = pEntity->GetId();

				DrxLog("CGameRulesSimpleEntityBasedObjective::OnSpawn, new entity %u '%s' detected", entId, pEntity->GetName());

				i32 index = -1;
				i32 numSlots = pEntityDetails->m_allEntities.size();
				for (i32 i = 0; i < numSlots; ++ i)
				{
					DrxLog("  index=%d, id=%u", i, pEntityDetails->m_allEntities[i]);
					if (pEntityDetails->m_allEntities[i] == 0)
					{
						pEntityDetails->m_allEntities[i] = entId;
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					index = pEntityDetails->m_allEntities.size();
					pEntityDetails->m_allEntities.push_back(entId);
				}

				CGameRules::SModuleRMIEntityParams entparams(m_moduleRMIIndex, entId, index | SIMPLE_ENTITY_BASED_OBJECTIVE_FLAGS_ADD_EXISTING_ENTITY);
				g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClModuleRMISingleEntity(), entparams, eRMI_ToRemoteClients, entId);

				if (pEntityDetails->m_entitySelectType == EEST_All)
				{
					SvAddEntity(entityType, entId, index);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
bool CGameRulesSimpleEntityBasedObjective::OnRemove( IEntity *pEntity )
{
	i32 numTypes = m_entityDetails.size();
	for (i32 entityType = 0; entityType < numTypes; ++ entityType)
	{
		SEntityDetails *pEntityDetails = &m_entityDetails[entityType];

		if (pEntity->GetClass() == pEntityDetails->m_pEntityClass)
		{
			EntityId entId = pEntity->GetId();

#if 0
			DrxLog("[tlh][6] entity type %d, size %d, ERASING!",entityType,pEntityDetails->m_allEntities.size());
			stl::find_and_erase(pEntityDetails->m_allEntities, entId);
#endif
#if 1
			DrxLog("[tlh][6] entity type %d, size %" PRISIZE_T ", ZEROING!",entityType,pEntityDetails->m_allEntities.size());
			i32  numAll = pEntityDetails->m_allEntities.size();
			for (i32 i=0; i<numAll; i++)
			{
				if (pEntityDetails->m_allEntities[i] == entId)
				{
					pEntityDetails->m_allEntities[i] = 0;
				}
			}
#endif

			if (gEnv->bServer)
			{
				i32 numCurrentEntities = pEntityDetails->m_currentEntities.size();
				for (i32 index = 0; index < numCurrentEntities; ++ index)
				{
					if (pEntityDetails->m_currentEntities[index] == entId)
					{
						SvRemoveEntity(entityType, index, false);		// Don't need to tell clients, they will know because the entity has just been deleted
					}
				}
			}
			else
			{
				// Find and remove the entity
				i32 numEntities = pEntityDetails->m_currentEntities.size();
				for (i32 entIdx = 0; entIdx < numEntities; ++ entIdx)
				{
					EntityId id = pEntityDetails->m_currentEntities[entIdx];
					if (id == pEntity->GetId())
					{
						pEntityDetails->m_currentEntities[entIdx] = 0;
						m_pObjective->RemoveEntityId(entityType, id);
					}
				}
			}
		}
	}
	return true;
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnReused( IEntity *pEntity, SEntitySpawnParams &params )
{
	DRX_ASSERT_MESSAGE(false, "CGameRulesSimpleEntityBasedObjective::OnReused needs implementing");
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnActionEvent( const SActionEvent& event )
{
	switch(event.m_event)
	{
	case eAE_resetBegin:
		{
			i32 numDetails = m_entityDetails.size();
			for (i32 i = 0; i < numDetails; ++ i)
			{
				SEntityDetails *pEntityDetails = &m_entityDetails[i];
				pEntityDetails->m_allEntities.clear();
				pEntityDetails->m_availableEntities.clear();
				pEntityDetails->m_currentEntities.clear();
			}
		}
		break;
	}
}

//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::OnHostMigration( bool becomeServer )
{
	m_pObjective->OnHostMigration(becomeServer);
}

//------------------------------------------------------------------------
bool CGameRulesSimpleEntityBasedObjective::CanPlayerRegenerate(EntityId playerId) const
{
	if (CGameRules *pGameRules = g_pGame->GetGameRules())
	{
		if(pGameRules->GetGameMode() == eGM_Assault)
		{
			i32k  attackingTeam = pGameRules->GetRoundsModule()->GetPrimaryTeam();
			i32k  actorTeam = pGameRules->GetTeam(playerId);
			return actorTeam == attackingTeam;
		}
	}

	return true;
}

#ifndef _RELEASE
//------------------------------------------------------------------------
void CGameRulesSimpleEntityBasedObjective::CmdNewRandomEntity( IConsoleCmdArgs* )
{
	if (s_instance && gEnv->bServer)
	{
		i32k numEntityTypes = s_instance->m_entityDetails.size();
		for (i32 entType = 0; entType < numEntityTypes; ++ entType)
		{
			// For each entity type that uses a random change timer, remove all current entities and set the new entities timer to 0
			SEntityDetails *pEntityDetails = &s_instance->m_entityDetails[entType];
			if (pEntityDetails->m_useRandomChangeTimer)
			{
				i32k numEntities = pEntityDetails->m_currentEntities.size();
				for (i32 i = 0; i < numEntities; ++ i)
				{
					if (pEntityDetails->m_currentEntities[i])
					{
						s_instance->SvRemoveEntity(entType, i, true);
					}
				}

				pEntityDetails->m_timeToRandomChange = 0.f;
			}
		}
	}
	else
	{
		DrxLog("Server only command");
	}
}
#endif
