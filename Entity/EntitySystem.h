// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Entity/EntityComponentsVector.h>
#include <drx3D/Entity/SaltBufferArray.h>          // SaltBufferArray<>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
#include <drx3D/CoreX/Memory/STLGlobalAllocator.h>
#include <array>

//////////////////////////////////////////////////////////////////////////
//  предварительные объявления.
//////////////////////////////////////////////////////////////////////////
class CEntity;
struct ICVar;
struct IPhysicalEntity;
struct IEntityComponent;
class CEntityClassRegistry;
class CScriptBind_Entity;
class CPhysicsEventListener;
class CAreaUpr;
class CBreakableUpr;
class CEntityArchetypeUpr;
class CPartitionGrid;
class CProximityTriggerSystem;
class CEntityLayer;
class CEntityLoadUpr;
struct SEntityLayerGarbage;
class CGeomCacheAttachmentUpr;
class CCharacterBoneAttachmentUpr;
class CEntitiesComponentPropertyCache;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef std::unordered_map<EntityId, CEntity*, stl::hash_uint32> EntityMap;
typedef std::vector<EntityId>                                    EntityIdVector;
typedef std::vector<EntityGUID>                                  EntityGuidVector;

//////////////////////////////////////////////////////////////////////////
struct SEntityTimerEvent
{
	EntityId entityId;
	i32      nTimerId;
	i32      nMilliSeconds;
};

//////////////////////////////////////////////////////////////////////////
struct SEntityAttachment
{
	EntityId   child;
	EntityId   parent;
	EntityGUID parentGuid;
	Vec3       pos;
	Quat       rot;
	Vec3       scale;
	i32        flags;
	string     target;

	SEntityAttachment() : child(0), parent(0), pos(ZERO), rot(ZERO), scale(ZERO), flags(0) {}
};

//////////////////////////////////////////////////////////////////////////
// Structure for extended information about loading an entity
//	Supports reusing an entity container if specified
struct SEntityLoadParams
{
	SEntitySpawnParams spawnParams;
	size_t             allocationSize;

	SEntityLoadParams();
	~SEntityLoadParams();

	SEntityLoadParams& operator=(const SEntityLoadParams& other) = delete;
	SEntityLoadParams(const SEntityLoadParams& other) = delete;
	SEntityLoadParams& operator=(SEntityLoadParams&& other) = default;
	SEntityLoadParams(SEntityLoadParams&& other) = default;

private:
	void        AddRef();
	void        RemoveRef();

	static bool CloneXmlNode(const XmlNodeRef sourceNode, XmlNodeRef destNode);
};
typedef std::vector<SEntityLoadParams>              TEntityLoadParamsContainer;

//////////////////////////////////////////////////////////////////////
class CEntitySystem final : public IEntitySystem
{
public:
	explicit CEntitySystem(ISystem* pSystem);
	~CEntitySystem();

	bool Init(ISystem* pSystem);

	// interface IEntitySystem ------------------------------------------------------
	virtual void                              Release() final;
	virtual void                              Update() final;
	virtual void                              DeletePendingEntities() final;
	virtual void                              PrePhysicsUpdate() final;
	virtual void                              Reset() final;
	virtual void                              Unload() final;
	virtual void                              PurgeHeaps() final;
	virtual IEntityClassRegistry*             GetClassRegistry() final;
	virtual IEntity*                          SpawnEntity(SEntitySpawnParams& params, bool bAutoInit = true) final;
	virtual bool                              InitEntity(IEntity* pEntity, SEntitySpawnParams& params) final;
	virtual IEntity*                          GetEntity(EntityId id) const final;
	virtual IEntity*                          FindEntityByName(tukk sEntityName) const final;
	virtual void                              ReserveEntityId(const EntityId id) final;
	virtual EntityId                          ReserveUnknownEntityId() final;
	virtual void                              RemoveEntity(EntityId entity, bool bForceRemoveNow = false) final;
	virtual u32                            GetNumEntities() const final;
	virtual IEntityItPtr                      GetEntityIterator() final;
	virtual void                              SendEventToAll(SEntityEvent& event) final;
	virtual void                              OnEditorSimulationModeChanged(EEditorSimulationMode mode) final;
	virtual void                              OnLevelLoaded() final;
	virtual void                              OnLevelGameplayStart() final;
	virtual i32                               QueryProximity(SEntityProximityQuery& query) final;
	virtual void                              ResizeProximityGrid(i32 nWidth, i32 nHeight) final;
	virtual i32                               GetPhysicalEntitiesInBox(const Vec3& origin, float radius, IPhysicalEntity**& pList, i32 physFlags) const final;
	virtual IEntity*                          GetEntityFromPhysics(IPhysicalEntity* pPhysEntity) const final;
	virtual void                              AddSink(IEntitySystemSink* pSink, std::underlying_type<SinkEventSubscriptions>::type subscriptions) final;
	virtual void                              RemoveSink(IEntitySystemSink* pSink) final;
	virtual void                              PauseTimers(bool bPause, bool bResume = false) final;
	virtual bool                              IsIDUsed(EntityId nID) const final;
	virtual void                              GetMemoryStatistics(IDrxSizer* pSizer) const final;
	virtual ISystem*                          GetSystem() const final { return m_pISystem; };
	virtual void                              SetNextSpawnId(EntityId id) final;
	virtual void                              ResetAreas() final;
	virtual void                              UnloadAreas() final;

	virtual void                              AddEntityLayerListener(tukk szLayerName, IEntityLayerListener* pListener, const bool bCaseSensitive = true) final;
	virtual void                              RemoveEntityLayerListener(tukk szLayerName, IEntityLayerListener* pListener, const bool bCaseSensitive = true) final;

	virtual EntityId                          FindEntityByGuid(const EntityGUID& guid) const final;

	virtual IEntityArchetype*                 LoadEntityArchetype(XmlNodeRef oArchetype) final;
	virtual IEntityArchetype*                 LoadEntityArchetype(tukk sArchetype) final;
	virtual void                              UnloadEntityArchetype(tukk sArchetype) final;
	virtual IEntityArchetype*                 CreateEntityArchetype(IEntityClass* pClass, tukk sArchetype) final;
	virtual void                              RefreshEntityArchetypesInRegistry() final;
	virtual void                              SetEntityArchetypeUprExtension(IEntityArchetypeUprExtension* pEntityArchetypeUprExtension) final;
	virtual IEntityArchetypeUprExtension* GetEntityArchetypeUprExtension() const final;

	virtual void                              Serialize(TSerialize ser) final;

	virtual void                              DumpEntities() final;

	virtual void                              ResumePhysicsForSuppressedEntities(bool bWakeUp) final;
	virtual void                              SaveInternalState(struct IDataWriteStream& writer) const final;
	virtual void                              LoadInternalState(struct IDataReadStream& reader) final;

	virtual i32                               GetLayerId(tukk szLayerName) const final;
	virtual tukk                       GetLayerName(i32 layerId) const final;
	virtual i32                               GetLayerChildCount(tukk szLayerName) const final;
	virtual tukk                       GetLayerChild(tukk szLayerName, i32 childIdx) const final;

	virtual i32                               GetVisibleLayerIDs(u8* pLayerMask, u32k maxCount) const final;

	virtual void                              ToggleLayerVisibility(tukk layer, bool isEnabled, bool includeParent = true) final;

	virtual void                              ToggleLayersBySubstring(tukk pSearchSubstring, tukk pExceptionSubstring, bool isEnable) final;

	virtual void                              LockSpawning(bool lock) final { m_bLocked = lock; }

	virtual bool                              OnLoadLevel(tukk szLevelPath) final;

	virtual IEntityLayer*                     AddLayer(tukk szName, tukk szParent, u16 id, bool bHasPhysics, i32 specs, bool bDefaultLoaded) final;
	virtual void                              LoadLayers(tukk dataFile) final;
	virtual void                              LinkLayerChildren() final;
	virtual void                              AddEntityToLayer(tukk layer, EntityId id) final;
	virtual void                              RemoveEntityFromLayers(EntityId id) final;
	virtual void                              ClearLayers() final;
	virtual void                              EnableDefaultLayers(bool isSerialized = true) final;
	virtual void                              EnableLayer(tukk layer, bool isEnable, bool isSerialized = true) final;
	virtual void                              EnableLayerSet(tukk const* pLayers, size_t layerCount, bool bIsSerialized = true, IEntityLayerSetUpdateListener* pListener = nullptr) final;
	// bCaseSensitive is set to false because it's not possible in Sandbox to create 2 layers with the same name but differs in casing
	// The issue was: when switching game data files and folders to different case, FlowGraph would still reference old layer names.
	virtual IEntityLayer*                     FindLayer(tukk szLayerName, const bool bCaseSensitive = false) const final;
	virtual bool                              IsLayerEnabled(tukk layer, bool bMustBeLoaded, bool bCaseSensitive = true) const final;
	virtual bool                              ShouldSerializedEntity(IEntity* pEntity) final;
	virtual void                              RegisterPhysicCallbacks() final;
	virtual void                              UnregisterPhysicCallbacks() final;

	CEntityLayer*                             GetLayerForEntity(EntityId id);
	void                                      EnableLayer(IEntityLayer* pLayer, bool bIsEnable, bool bIsSerialized, bool bAffectsChildren);

	bool                                      OnBeforeSpawn(SEntitySpawnParams& params);
	void                                      OnEntityReused(IEntity* pEntity, SEntitySpawnParams& params);

	// Sets new entity timer event.
	void AddTimerEvent(SEntityTimerEvent& event, CTimeValue startTime = gEnv->pTimer->GetFrameStartTime());

	//////////////////////////////////////////////////////////////////////////
	// Load entities from XML.
	void         LoadEntities(XmlNodeRef& objectsNode, bool bIsLoadingLevelFile) final;
	void         LoadEntities(XmlNodeRef& objectsNode, bool bIsLoadingLevelFile, const Vec3& segmentOffset) final;

	virtual bool ExtractArcheTypeLoadParams(XmlNodeRef& entityNode, SEntitySpawnParams& spawnParams) const final;
	virtual bool ExtractEntityLoadParams(XmlNodeRef& entityNode, SEntitySpawnParams& spawnParams) const final;
	virtual void BeginCreateEntities(i32 nAmtToCreate) final;
	virtual bool CreateEntity(XmlNodeRef& entityNode, SEntitySpawnParams& pParams, EntityId& outUsingId) final;
	virtual void EndCreateEntities() final;

	IEntity*     SpawnPreallocatedEntity(CEntity* pPrecreatedEntity, SEntitySpawnParams& params, bool bAutoInit);

	//////////////////////////////////////////////////////////////////////////
	// Called from CEntity implementation.
	//////////////////////////////////////////////////////////////////////////
	void RemoveTimerEvent(EntityId id, i32 nTimerId);
	bool HasTimerEvent(EntityId id, i32 nTimerId);

	// Access to class that binds script to entity functions.
	// Used by Script proxy.
	CScriptBind_Entity* GetScriptBindEntity() { return m_pEntityScriptBinding; };

	// Access to area manager.
	IAreaUpr* GetAreaUpr() const final { return (IAreaUpr*)(m_pAreaUpr); }

	// Access to breakable manager.
	virtual IBreakableUpr*       GetBreakableUpr() const final         { return m_pBreakableUpr; };

	CEntityLoadUpr*              GetEntityLoadUpr() const              { return m_pEntityLoadUpr; }

	CGeomCacheAttachmentUpr*     GetGeomCacheAttachmentUpr() const     { return m_pGeomCacheAttachmentUpr; }
	CCharacterBoneAttachmentUpr* GetCharacterBoneAttachmentUpr() const { return m_pCharacterBoneAttachmentUpr; }

	static EntityIndex         IdToIndex(const EntityId id) { return CSaltHandle::GetHandleFromId(id).GetIndex(); }
	static CSaltHandle         IdToHandle(const EntityId id) { return CSaltHandle::GetHandleFromId(id); }
	static EntityId            HandleToId(const CSaltHandle id) { return id.GetId(); }

	EntityId                         GenerateEntityId(bool bStaticId);

	void                             RegisterEntityGuid(const EntityGUID& guid, EntityId id);
	void                             UnregisterEntityGuid(const EntityGUID& guid);

	CPartitionGrid*                  GetPartitionGrid()          { return m_pPartitionGrid; }
	CProximityTriggerSystem*         GetProximityTriggerSystem() { return m_pProximityTriggerSystem; }

	void                             ChangeEntityName(CEntity* pEntity, tukk sNewName);

	CEntity*                         GetEntityFromID(EntityId id) const;
	ILINE bool                       HasEntity(EntityId id) const { return GetEntityFromID(id) != 0; };

	virtual void                     PurgeDeferredCollisionEvents(bool bForce = false) final;

	virtual IBSPTree3D*              CreateBSPTree3D(const IBSPTree3D::FaceList& faceList) final;
	virtual void                     ReleaseBSPTree3D(IBSPTree3D*& pTree) final;

	void                             RemoveEntity(CEntity* pEntity, bool forceRemoveImmediately = false, bool ignoreSinks = false);
	// Restore an entity that was marked for deletion (CEntity::IsGarbage)
	void                             ResurrectGarbageEntity(CEntity* pEntity);
	
	void                             EnableComponentUpdates(IEntityComponent* pComponent, bool bEnable);
	void                             EnableComponentPrePhysicsUpdates(IEntityComponent* pComponent, bool bEnable);

private:
	bool ValidateSpawnParameters(SEntitySpawnParams& params);

	void UpdateEntityComponents(float fFrameTime);

	void DeleteEntity(CEntity* pEntity);
	void UpdateTimers();
	void DebugDraw(const CEntity* const pEntity, float fUpdateTime);

	void DebugDrawEntityUsage();
	void DebugDrawLayerInfo();

	void ClearEntityArray();

	void DumpEntity(CEntity* pEntity);

	// slow - to find specific problems
	void CheckInternalConsistency() const;

	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	struct OnEventSink
	{
		OnEventSink(uint64 _subscriptions, IEntitySystemSink* _pSink)
			: subscriptions(_subscriptions)
			, pSink(_pSink)
		{
		}

		uint64             subscriptions;
		IEntitySystemSink* pSink;
	};

	typedef std::vector<OnEventSink, stl::STLGlobalAllocator<OnEventSink>>                                                                                                                              EntitySystemOnEventSinks;
	typedef std::vector<CEntity*>                                                                                                                                                                       DeletedEntities;
	typedef std::multimap<CTimeValue, SEntityTimerEvent, std::less<CTimeValue>, stl::STLPoolAllocator<std::pair<const CTimeValue, SEntityTimerEvent>, stl::PoolAllocatorSynchronizationSinglethreaded>> EntityTimersMap;
	typedef std::multimap<tukk , EntityId, stl::less_stricmp<tukk >>                                                                                                                        EntityNamesMap;
	typedef std::vector<SEntityTimerEvent>                                                                                                                                                              EntityTimersVector;

	std::array<std::vector<IEntitySystemSink*>, (size_t)SinkEventSubscriptions::Count> m_sinks;

	ISystem*              m_pISystem;
	std::array<CEntity*, CSaltBufferArray::GetTSize()> m_EntityArray;                    // [id.GetIndex()]=CEntity
	DeletedEntities       m_deletedEntities;
	std::vector<CEntity*> m_deferredUsedEntities;

	EntityNamesMap        m_mapEntityNames;            // Map entity name to entity ID.

	CSaltBufferArray    m_EntitySaltBuffer;               // used to create new entity ids (with uniqueid=salt)
	//////////////////////////////////////////////////////////////////////////

	CEntityComponentsVector<SMinimalEntityComponentRecord> m_updatedEntityComponents;
	CEntityComponentsVector<SMinimalEntityComponentRecord> m_prePhysicsUpdatedEntityComponents;

	// Entity timers.
	EntityTimersMap    m_timersMap;
	EntityTimersVector m_currentTimers;
	bool               m_bTimersPause;
	CTimeValue         m_nStartPause;

	// Binding entity.
	CScriptBind_Entity* m_pEntityScriptBinding;

	// Entity class registry.
	CEntityClassRegistry*  m_pClassRegistry;
	CPhysicsEventListener* m_pPhysicsEventListener;

	CAreaUpr*          m_pAreaUpr;

	CEntityLoadUpr*    m_pEntityLoadUpr;

	typedef std::unordered_map<EntityGUID, EntityId> EntityGuidMap;
	EntityGuidMap                    m_guidMap;
	EntityGuidMap                    m_genIdMap;

	IBreakableUpr*               m_pBreakableUpr;
	CEntityArchetypeUpr*         m_pEntityArchetypeUpr;
	CGeomCacheAttachmentUpr*     m_pGeomCacheAttachmentUpr;
	CCharacterBoneAttachmentUpr* m_pCharacterBoneAttachmentUpr;

	// Partition grid used by the entity system
	CPartitionGrid*          m_pPartitionGrid;
	CProximityTriggerSystem* m_pProximityTriggerSystem;

	EntityId                 m_idForced;

	//don't spawn any entities without being forced to
	bool m_bLocked;
	bool m_bSupportLegacy64bitGuids = false;

	friend class CEntityItMap;

	typedef std::map<string, CEntityLayer*>  TLayers;
	typedef std::vector<SEntityLayerGarbage> THeaps;

	TLayers m_layers;
	THeaps  m_garbageLayerHeaps;

	std::unique_ptr<CEntitiesComponentPropertyCache> m_entitiesPropertyCache;

public:
	std::unique_ptr<class CEntityObjectDebugger> m_pEntityObjectDebugger;

#ifdef ENABLE_PROFILING_CODE
public:
	struct SLayerProfile
	{
		float         fTimeMS;
		float         fTimeOn;
		bool          isEnable;
		CEntityLayer* pLayer;
	};

	std::vector<SLayerProfile> m_layerProfiles;
#endif //ENABLE_PROFILING_CODE
};

//////////////////////////////////////////////////////////////////////////
// Precache resources mode state.
//////////////////////////////////////////////////////////////////////////
extern bool gPrecacheResourcesMode;
