// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

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

#ifndef _GAME_RULES_SIMPLE_ENTITY_BASED_OBJECTIVE_H_
#define _GAME_RULES_SIMPLE_ENTITY_BASED_OBJECTIVE_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/GameRulesModules/IGameRulesObjectivesModule.h>
#include <drx3D/Game/GameRulesModules/IGameRulesEntityObjective.h>
#include <drx3D/Game/GameRulesModules/IGameRulesModuleRMIListener.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>
#include <drx3D/Entity/IEntitySystem.h>

class CGameRulesSpawningBase;

class CGameRulesSimpleEntityBasedObjective :	public IGameRulesObjectivesModule,
												public IGameRulesModuleRMIListener,
												public IGameRulesClientConnectionListener,
												public IEntitySystemSink,
												public IGameFrameworkListener
{
public:
	CGameRulesSimpleEntityBasedObjective();
	virtual ~CGameRulesSimpleEntityBasedObjective();

	// IGameRulesObjectivesModule
	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual void OnStartGame();
	virtual void OnStartGamePost() {}
	virtual void OnGameReset() { OnStartGame(); }

	virtual void PostInitClient(i32 channelId) {};
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );

	virtual bool CanPlayerRegenerate(EntityId playerId) const;

	virtual bool HasCompleted(i32 teamId);

	virtual void OnHostMigration(bool becomeServer);

	virtual bool AreSuddenDeathConditionsValid() const { return false; }
	virtual void CheckForInteractionWithEntity(EntityId interactId, EntityId playerId, SInteractionInfo& interactionInfo) {};
	virtual bool CheckIsPlayerEntityUsingObjective(EntityId playerId) { return false; }

	virtual ESpawnEntityState GetObjectiveEntityState(EntityId entityId) { return eSES_Unknown; }

	virtual i32 GetAutoAssignTeamId(i32 channelId) { return 0; }
	virtual void OnEntitySignal(EntityId entityId, i32 signal) { } 
	// ~IGameRulesObjectivesModule
	
	// IGameRulesModuleRMIListener
	virtual void OnSingleEntityRMI(CGameRules::SModuleRMIEntityParams params);
	virtual void OnDoubleEntityRMI(CGameRules::SModuleRMITwoEntityParams params) {};
	virtual void OnEntityWithTimeRMI(CGameRules::SModuleRMIEntityTimeParams params);
	virtual void OnSvClientActionRMI(CGameRules::SModuleRMISvClientActionParams params, EntityId fromEid) {};
	// ~IGameRulesModuleRMIListener

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) {};
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId) {};
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId);
	virtual void OnOwnClientEnteredGame() {};
	// ~IGameRulesClientConnectionListener

	// IEntitySystemSink
	virtual bool OnBeforeSpawn( SEntitySpawnParams &params ) { return true; };
	virtual void OnSpawn( IEntity *pEntity,SEntitySpawnParams &params );
	virtual bool OnRemove( IEntity *pEntity );
	virtual void OnReused( IEntity *pEntity, SEntitySpawnParams &params );
	virtual void OnEvent( IEntity *pEntity, SEntityEvent &event ) {};
	// ~IEntitySystemSink

	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime) {}
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(tukk pNextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener

	void GetMemoryUsage(class IDrxSizer *pSizer) const 
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddContainer(m_entityDetails);		
	}
protected:
	typedef std::vector<EntityId> TEntityIdVec;

	void SvAddEntity(i32 type, EntityId id, i32 index);
	void SvRemoveEntity(i32 type, i32 index, bool sendRMI);
	void SvDoRandomTypeSelection(i32 type);
	i32 SvResetAvailableEntityList(i32 type);

	void CallScriptUpdateFunction(TEntityIdVec &entitiesVec, float frameTime);

	enum EEntitySelectType
	{
		EEST_Default,
		EEST_All = EEST_Default,
		EEST_Random,
		EEST_Random_Weighted,
	};

	struct SEntityDetails
	{
		enum EUpdateType
		{
			eUT_None,
			eUT_All,
			eUT_Active,
		};

		SEntityDetails()
		{
			m_pEntityClass = NULL;
			m_numRandomEntities = 0;
			m_randomChangeTimeLength = 0.f;
			m_timeToRandomChange = 0.f;
			m_timeBetweenSites = 0.f;
			m_entitySelectType = EEST_Default;
			m_updateType = eUT_None;
			m_useRandomChangeTimer = false;
			m_scan = false;
		}

		void GetMemoryUsage(class IDrxSizer *pSizer) const 
		{
			pSizer->AddContainer(m_allEntities);
			pSizer->AddContainer(m_currentEntities);
			pSizer->AddContainer(m_availableEntities);
		}
		TEntityIdVec m_allEntities;				// All entities of class 'm_pEntityClass' in the level
		TEntityIdVec m_currentEntities;			// List of entities from inside 'm_allEntities' that are currently set as active objectives
		TEntityIdVec m_availableEntities;		// List of entities from 'm_allEntities' that have not been made active (used in random select type only)
		const IEntityClass *m_pEntityClass;

		static i32k MAX_ENTITY_SELECTS = 10;

		i32 m_numRandomEntities; // Num of entities to pick when doing a random selection

		float m_randomChangeTimeLength;			// Time before a site is disabled (timer length)
		float m_timeToRandomChange;					// Time before a site is disabled (actual timer)
		float m_timeBetweenSites;						// Time between a site being disabled and a new one being activated

		EEntitySelectType m_entitySelectType;
		EUpdateType m_updateType;

		bool m_useRandomChangeTimer;
		bool m_scan;
	};

	typedef std::vector<SEntityDetails> TEntityDetailsVec;

	TEntityDetailsVec m_entityDetails;

	IGameRulesEntityObjective *m_pObjective;

	i32 m_moduleRMIIndex;

private:
	void LoadRandomEntitySelectData(XmlNodeRef xmlChild, SEntityDetails& entityDetails) const;

	void SvDoTrueRandomSelection(i32 entityType, SEntityDetails *pEntityDetails);
	void SvDoRandomWeightedSelection(i32 entityType, SEntityDetails *pEntityDetails);
	void SvDoRandomSelectionAvoidingActors(i32 entityType, SEntityDetails * pEntityDetails);
	void SvDoRandomSelectionAvoidingInitialSpawns(i32 entityType, SEntityDetails * pEntityDetails, const CGameRulesSpawningBase * pSpawningModule);

#ifndef _RELEASE
private:
	static void CmdNewRandomEntity(IConsoleCmdArgs*);

	static CGameRulesSimpleEntityBasedObjective *s_instance;
#endif
};

#endif // _GAME_RULES_SIMPLE_ENTITY_BASED_OBJECTIVE_H_
