// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntity.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

//////////////////////////////////////////////////////////////////////////
// This is the order in which the proxies are serialized
const static EEntityProxy ProxySerializationOrder[] = {
	ENTITY_PROXY_SCRIPT,
	ENTITY_PROXY_AUDIO,
	ENTITY_PROXY_AREA,
	ENTITY_PROXY_BOIDS,
	ENTITY_PROXY_BOID_OBJECT,
	ENTITY_PROXY_CAMERA,
	ENTITY_PROXY_FLOWGRAPH,
	ENTITY_PROXY_SUBSTITUTION,
	ENTITY_PROXY_TRIGGER,
	ENTITY_PROXY_USER,
	ENTITY_PROXY_ROPE,
	ENTITY_PROXY_ENTITYNODE,
	ENTITY_PROXY_CLIPVOLUME,
	ENTITY_PROXY_DYNAMICRESPONSE,
	ENTITY_PROXY_LAST
};

//////////////////////////////////////////////////////////////////////////
// These headers cannot be replaced with forward references.
// They are needed for correct up casting from IEntityComponent to real proxy class.
#include <drx3D/Entity/RenderProxy.h>
#include <drx3D/Entity/PhysicsProxy.h>
#include <drx3D/Entity/ScriptProxy.h>
#include <drx3D/Entity/SubstitutionProxy.h>
#include <drx3D/Entity/EntityComponentsVector.h>
//////////////////////////////////////////////////////////////////////////

//  предварительные объявления.
struct IEntitySystem;
struct IEntityArchetype;
class CEntitySystem;
struct AIObjectParams;
struct SGridLocation;
struct SProximityElement;

namespace sxema
{
// Forward declare interfaces.
struct IObjectProperties;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IObjectProperties)
}

// (MATT) This should really live in a minimal AI include, which right now we don't have  {2009/04/08}
#ifndef INVALID_AIOBJECTID
typedef u32 tAIObjectID;
	#define INVALID_AIOBJECTID ((tAIObjectID)(0))
#endif

//////////////////////////////////////////////////////////////////////
class CEntity : public IEntity
{
public:
	enum class EInternalFlag : u32
	{
		Initialized         = 1 << 0,
		InActiveList        = 1 << 1,
		MarkedForDeletion   = 1 << 2,
		Hidden              = 1 << 3,
		Invisible           = 1 << 4,
		LoadedFromLevelFile = 1 << 5,
		SelectedInEditor    = 1 << 6,
		HighlightedInEditor = 1 << 7,

		// Start CEntityRender entries
		// Bounding box should not be recalculated
		FixedBounds  = 1 << 8,
		ValidBounds  = 1 << 9,
		HasParticles = 1 << 10,

		// Start CEntityPhysics entries
		FirstPhysicsFlag                   = 1 << 11,
		PhysicsIgnoreTransformEvent        = FirstPhysicsFlag,
		PhysicsDisabled                    = 1 << 12,
		PhysicsSyncCharacter               = 1 << 13,
		PhysicsHasCharacter                = 1 << 14,
		PhysicsAwakeOnRender               = 1 << 15,
		PhysicsAttachClothOnRender         = 1 << 16,
		PhysicsDisableNetworkSerialization = 1 << 17,
		PhysicsRemoved                     = 1 << 18,
		LastPhysicsFlag                    = PhysicsRemoved,
		BlockEvents                        = 1 << 19
	};

	// Entity constructor.
	// Should only be called from Entity System.
	CEntity() = default;
	// Entity destructor.
	// Should only be called from Entity System.
	virtual ~CEntity() = default;

	void PreInit(SEntitySpawnParams& params);

	// Called by entity system to complete initialization of the entity.
	bool Init(SEntitySpawnParams& params);
	// Called by EntitySystem before entity is destroyed.
	void ShutDown();

	//////////////////////////////////////////////////////////////////////////
	// IEntity interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EntityId          GetId() const final   { return m_id; }
	virtual const EntityGUID& GetGuid() const final { return m_guid; }

	//////////////////////////////////////////////////////////////////////////
	virtual IEntityClass*     GetClass() const final     { return m_pClass; }
	virtual IEntityArchetype* GetArchetype() const final { return m_pArchetype; }

	//////////////////////////////////////////////////////////////////////////
	virtual void   SetFlags(u32 flags) final;
	virtual u32 GetFlags() const final                      { return m_flags; }
	virtual void   AddFlags(u32 flagsToAdd) final           { SetFlags(m_flags | flagsToAdd); }
	virtual void   ClearFlags(u32 flagsToClear) final       { SetFlags(m_flags & (~flagsToClear)); }
	virtual bool   CheckFlags(u32 flagsToCheck) const final { return (m_flags & flagsToCheck) == flagsToCheck; }

	virtual void   SetFlagsExtended(u32 flags) final;
	virtual u32 GetFlagsExtended() const final                            { return m_flagsExtended; }

	virtual bool   IsGarbage() const final                                   { return HasInternalFlag(EInternalFlag::MarkedForDeletion); }
	virtual bool   IsLoadedFromLevelFile() const final                       { return HasInternalFlag(EInternalFlag::LoadedFromLevelFile); }
	ILINE void     SetLoadedFromLevelFile(const bool wasLoadedFromLevelFile) { SetInternalFlag(EInternalFlag::LoadedFromLevelFile, wasLoadedFromLevelFile); }

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetName(tukk sName) final;
	virtual tukk GetName() const final { return m_name.c_str(); }
	virtual string      GetEntityTextDescription() const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void     AttachChild(IEntity* pChildEntity, const SChildAttachParams& attachParams) final;
	virtual void     DetachAll(AttachmentFlagsType attachmentFlags = 0) final;
	virtual void     DetachThis(AttachmentFlagsType attachmentFlags = 0, EntityTransformationFlagsType transformReasons = 0) final;
	virtual i32      GetChildCount() const final;
	virtual IEntity* GetChild(i32 nIndex) const final;
	virtual IEntity* GetParent() const final         { return m_hierarchy.pParent; }
	virtual IEntity* GetLocalSimParent() const final { return m_hierarchy.parentBindingType == EBindingType::eBT_LocalSim ? m_hierarchy.pParent : nullptr; }
	virtual Matrix34 GetParentAttachPointWorldTM() const final;
	virtual bool     IsParentAttachmentValid() const final;
	virtual IEntity* GetAdam();

	//////////////////////////////////////////////////////////////////////////
	virtual void SetWorldTM(const Matrix34& tm, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;
	virtual void SetLocalTM(const Matrix34& tm, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;

	// Set position rotation and scale at once.
	virtual void            SetPosRotScale(const Vec3& vPos, const Quat& qRotation, const Vec3& vScale, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;

	virtual Matrix34        GetLocalTM() const final;
	virtual const Matrix34& GetWorldTM() const final { return m_worldTM; }

	virtual void            GetWorldBounds(AABB& bbox) const final;
	virtual void            GetLocalBounds(AABB& bbox) const final;
	virtual void            SetLocalBounds(const AABB& bounds, bool bDoNotRecalculate) final;
	virtual void            InvalidateLocalBounds() final;

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetPos(const Vec3& vPos, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask(), bool bRecalcPhyBounds = false, bool bForce = false) final;
	virtual const Vec3& GetPos() const final { return m_position; }

	virtual void        SetRotation(const Quat& qRotation, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;
	virtual const Quat& GetRotation() const final { return m_rotation; }

	virtual void        SetScale(const Vec3& vScale, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;
	virtual const Vec3& GetScale() const final    { return m_scale; }

	virtual Vec3        GetWorldPos() const final { return m_worldTM.GetTranslation(); }
	virtual Ang3        GetWorldAngles() const final;
	virtual Quat        GetWorldRotation() const final;
	virtual Vec3        GetWorldScale() const final;
	//////////////////////////////////////////////////////////////////////////

	virtual void UpdateComponentEventMask(const IEntityComponent* pComponent) final;
	virtual bool IsActivatedForUpdates() const final { return HasInternalFlag(EInternalFlag::InActiveList); }

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(TSerialize ser, i32 nFlags) final;

	virtual bool SendEvent(const SEntityEvent& event) final;

	virtual void AddEventListener(EEntityEvent event, IEntityEventListener* pListener) final;
	virtual void RemoveEventListener(EEntityEvent event, IEntityEventListener* pListener) final;

	//////////////////////////////////////////////////////////////////////////

	virtual i32   SetTimer(i32 nTimerId, i32 nMilliSeconds) final;
	virtual void  KillTimer(i32 nTimerId) final;

	virtual void  Hide(bool bHide, EEntityHideFlags hideFlags = ENTITY_HIDE_NO_FLAG) final;
	void          SendHideEvent(bool bHide, EEntityHideFlags hideFlags);
	virtual bool  IsHidden() const final { return HasInternalFlag(EInternalFlag::Hidden); }

	virtual void  Invisible(bool bInvisible) final;
	virtual bool  IsInvisible() const final { return HasInternalFlag(EInternalFlag::Invisible); }

	virtual u8 GetComponentChangeState() const final;

	//////////////////////////////////////////////////////////////////////////
	virtual const IAIObject* GetAI() const final                 { return m_aiObjectID ? GetAIObject() : nullptr; }
	virtual IAIObject*       GetAI() final                       { return m_aiObjectID ? GetAIObject() : nullptr; }
	virtual bool             HasAI() const final                 { return m_aiObjectID != 0; }
	virtual tAIObjectID      GetAIObjectID() const final         { return m_aiObjectID; }
	virtual void             SetAIObjectID(tAIObjectID id) final { m_aiObjectID = id; }
	//////////////////////////////////////////////////////////////////////////

	virtual ISerializableInfoPtr GetSerializableNetworkSpawnInfo() const final;

	//////////////////////////////////////////////////////////////////////////
	// Entity Proxies Interfaces access functions.
	//////////////////////////////////////////////////////////////////////////
	virtual IEntityComponent* GetProxy(EEntityProxy proxy) const final;
	virtual IEntityComponent* CreateProxy(EEntityProxy proxy) final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual IEntityComponent* CreateComponentByInterfaceID(const DrxInterfaceID& interfaceId, IEntityComponent::SInitParams* pInitParams) final;
	virtual bool              AddComponent(std::shared_ptr<IEntityComponent> pComponent, IEntityComponent::SInitParams* pInitParams) final;
	virtual void              RemoveComponent(IEntityComponent* pComponent) final;
	virtual void              RemoveAllComponents() final;
	virtual void              ReplaceComponent(IEntityComponent* pExistingComponent, std::shared_ptr<IEntityComponent> pNewComponent) final;
	virtual IEntityComponent* GetComponentByTypeId(const DrxInterfaceID& interfaceID) const final;
	virtual void              GetComponentsByTypeId(const DrxInterfaceID& interfaceID, DynArray<IEntityComponent*>& components) const final;
	virtual IEntityComponent* GetComponentByGUID(const DrxGUID& guid) const final;
	virtual void              QueryComponentsByInterfaceID(const DrxInterfaceID& interfaceID, DynArray<IEntityComponent*>& components) const final;
	virtual IEntityComponent* QueryComponentByInterfaceID(const DrxInterfaceID& interfaceID) const final;
	virtual void              GetComponents(DynArray<IEntityComponent*>& components) const final;
	virtual u32            GetComponentsCount() const final;
	virtual void              VisitComponents(const ComponentsVisitor& visitor) final;

	//////////////////////////////////////////////////////////////////////////
	// Physics.
	//////////////////////////////////////////////////////////////////////////
	virtual void             Physicalize(SEntityPhysicalizeParams& params) final;
	virtual void             AssignPhysicalEntity(IPhysicalEntity* pPhysEntity, i32 nSlot = -1) final;
	virtual void             EnablePhysics(bool enable) final;
	virtual bool             IsPhysicsEnabled() const final;
	virtual IPhysicalEntity* GetPhysics() const final;

	virtual i32              PhysicalizeSlot(i32 slot, SEntityPhysicalizeParams& params) final;
	virtual void             UnphysicalizeSlot(i32 slot) final;
	virtual void             UpdateSlotPhysics(i32 slot) final;

	virtual void             SetPhysicsState(XmlNodeRef& physicsState) final;
	virtual void             GetPhysicsWorldBounds(AABB& bounds) const final;

	virtual void             AddImpulse(i32 ipart, const Vec3& pos, const Vec3& impulse, bool bPos, float fAuxScale, float fPushScale = 1.0f) final;

	virtual bool             PhysicalizeFoliage(i32 iSlot) final;
	virtual void             DephysicalizeFoliage(i32 iSlot) final;

	//! retrieve starting partid for a given slot.
	virtual i32  GetPhysicalEntityPartId0(i32 islot = 0) final;

	virtual void PhysicsNetSerializeEnable(bool enable) final;
	virtual void PhysicsNetSerializeTyped(TSerialize& ser, i32 type, i32 flags) final;
	virtual void PhysicsNetSerialize(TSerialize& ser) final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Custom entity material.
	//////////////////////////////////////////////////////////////////////////
	virtual void       SetMaterial(IMaterial* pMaterial) final;
	virtual IMaterial* GetMaterial() final;

	//////////////////////////////////////////////////////////////////////////
	// Entity Slots interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool                       IsSlotValid(i32 nSlot) const final;
	virtual i32                        AllocateSlot() final;
	virtual void                       FreeSlot(i32 nSlot) final;
	virtual i32                        GetSlotCount() const final;
	virtual void                       ClearSlots() final;
	virtual bool                       GetSlotInfo(i32 nSlot, SEntitySlotInfo& slotInfo) const final;
	virtual Matrix34                   GetSlotWorldTM(i32 nIndex) const final;
	virtual Matrix34                   GetSlotLocalTM(i32 nIndex, bool bRelativeToParent) const final;
	virtual void                       SetSlotLocalTM(i32 nIndex, const Matrix34& localTM, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask()) final;
	virtual void                       SetSlotCameraSpacePos(i32 nIndex, const Vec3& cameraSpacePos) final;
	virtual void                       GetSlotCameraSpacePos(i32 nSlot, Vec3& cameraSpacePos) const final;
	virtual bool                       SetParentSlot(i32 nParentSlot, i32 nChildSlot) final;
	virtual void                       SetSlotMaterial(i32 nSlot, IMaterial* pMaterial) final;
	virtual IMaterial*                 GetSlotMaterial(i32 nSlot) const final;
	virtual IMaterial*                 GetRenderMaterial(i32 nSlot = -1) const final;
	virtual void                       SetSlotFlags(i32 nSlot, u32 nFlags) final;
	virtual u32                     GetSlotFlags(i32 nSlot) const final;
	virtual i32                        SetSlotRenderNode(i32 nSlot, IRenderNode* pRenderNode) final;
	virtual IRenderNode*               GetSlotRenderNode(i32 nSlot) final;
	virtual void                       UpdateSlotForComponent(IEntityComponent* pComponent, bool callOnTransformChanged = true) final;
	virtual bool                       ShouldUpdateCharacter(i32 nSlot) const final;
	virtual ICharacterInstance*        GetCharacter(i32 nSlot) final;
	virtual i32                        SetCharacter(ICharacterInstance* pCharacter, i32 nSlot, bool bUpdatePhysics) final;
	virtual IStatObj*                  GetStatObj(i32 nSlot) final;
	virtual i32                        SetStatObj(IStatObj* pStatObj, i32 nSlot, bool bUpdatePhysics, float mass = -1.0f) final;
	virtual IParticleEmitter*          GetParticleEmitter(i32 nSlot) final;
	virtual IGeomCacheRenderNode*      GetGeomCacheRenderNode(i32 nSlot) final;
	virtual IRenderNode*               GetRenderNode(i32 nSlot = -1) const final;
	virtual bool                       IsRendered() const final;
	virtual void                       PreviewRender(SEntityPreviewContext& context) final;

	virtual void                       MoveSlot(IEntity* targetIEnt, i32 nSlot) final;

	virtual i32                        LoadGeometry(i32 nSlot, tukk sFilename, tukk sGeomName = NULL, i32 nLoadFlags = 0) final;
	virtual i32                        LoadCharacter(i32 nSlot, tukk sFilename, i32 nLoadFlags = 0) final;
#if defined(USE_GEOM_CACHES)
	virtual i32                        LoadGeomCache(i32 nSlot, tukk sFilename) final;
#endif
	virtual i32                        SetParticleEmitter(i32 nSlot, IParticleEmitter* pEmitter, bool bSerialize = false) final;
	virtual i32                        LoadParticleEmitter(i32 nSlot, IParticleEffect* pEffect, SpawnParams const* params = NULL, bool bPrime = false, bool bSerialize = false) final;
	virtual i32                        LoadLight(i32 nSlot, SRenderLight* pLight) final;
	i32                                LoadLightImpl(i32 nSlot, SRenderLight* pLight);

	virtual bool                       UpdateLightClipBounds(SRenderLight& light) final;
	i32                                LoadCloudBlocker(i32 nSlot, const SCloudBlockerProperties& properties);
	virtual i32                        LoadFogVolume(i32 nSlot, const SFogVolumeProperties& properties) override;

	i32                                FadeGlobalDensity(i32 nSlot, float fadeTime, float newGlobalDensity);

	virtual void                       SetSubObjHideMask(i32 nSlot, hidemask nSubObjHideMask) final        { GetEntityRender()->SetSubObjHideMask(nSlot, nSubObjHideMask); };
	virtual hidemask                   GetSubObjHideMask(i32 nSlot) const final                            { return GetEntityRender()->GetSubObjHideMask(nSlot); };

	virtual void                       SetRenderNodeParams(const IEntity::SRenderNodeParams& params) final { GetEntityRender()->SetRenderNodeParams(params); };
	virtual IEntity::SRenderNodeParams GetRenderNodeParams() const final                                   { return GetEntityRender()->GetRenderNodeParams(); };

	virtual void                       InvalidateTM(EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask(), bool bRecalcPhyBounds = false) final;

	// Inline implementation.
	virtual IScriptTable* GetScriptTable() const final;

	// Load/Save entity parameters in XML node.
	virtual void         SerializeXML(XmlNodeRef& node, bool bLoading, bool bIncludeScriptProxy = true, bool bExcludeSchematycProperties = false) final;

	virtual IEntityLink* GetEntityLinks() final;
	virtual IEntityLink* AddEntityLink(tukk sLinkName, EntityId entityId, EntityGUID entityGuid) final;
	virtual void         RenameEntityLink(IEntityLink* pLink, tukk sNewLinkName) final;
	virtual void         RemoveEntityLink(IEntityLink* pLink) final;
	virtual void         RemoveAllEntityLinks() final;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	virtual IEntity* UnmapAttachedChild(i32& partId) final;

	virtual void     GetMemoryUsage(IDrxSizer* pSizer) const final;

	// Get fast access to the slot, only used internally by entity components.
	class CEntitySlot* GetSlot(i32 nSlot) const;

	// Get render proxy object.
	ILINE const CEntityRender* GetEntityRender() const { return static_cast<const CEntityRender*>(&m_render); }
	ILINE CEntityRender*       GetEntityRender()       { return static_cast<CEntityRender*>(&m_render); }

	// Get physics proxy object.
	ILINE const CEntityPhysics* GetPhysicalProxy() const { return static_cast<const CEntityPhysics*>(&m_physics); }
	ILINE CEntityPhysics*       GetPhysicalProxy()       { return static_cast<CEntityPhysics*>(&m_physics); }

	// Get script proxy object.
	ILINE CEntityComponentLuaScript* GetScriptProxy() const { return static_cast<CEntityComponentLuaScript*>(CEntity::GetProxy(ENTITY_PROXY_SCRIPT)); }
	//////////////////////////////////////////////////////////////////////////

	// For internal use.
	CEntitySystem* GetCEntitySystem() const { return g_pIEntitySystem; }

	// Sends the event to all listeners, if there are any
	// Bypasses the main IsGarbage check from SendEvent!
	bool           SendEventInternal(const SEntityEvent& event);
	void           PrepareForDeletion();

	// for ProximityTriggerSystem
	SProximityElement* GetProximityElement()         { return m_pProximityEntity; }

	virtual void       IncKeepAliveCounter() final   { m_keepAliveCounter++; }
	virtual void       DecKeepAliveCounter() final   { assert(m_keepAliveCounter > 0); m_keepAliveCounter--; }
	virtual void       ResetKeepAliveCounter() final { m_keepAliveCounter = 0; }
	virtual bool       IsKeptAlive() const final     { return m_keepAliveCounter > 0; }

	// LiveCreate entity interface
	virtual bool                  HandleVariableChange(tukk szVarName, ukk pVarData) final;

	virtual void                  OnRenderNodeVisibilityChange(bool bBecomeVisible) final;
	virtual float                 GetLastSeenTime() const final;

	virtual INetEntity*           AssignNetEntityLegacy(INetEntity* ptr) final;
	virtual INetEntity*           GetNetEntity() final;

	virtual u32                GetEditorObjectID() const final override;
	virtual void                  GetEditorObjectInfo(bool& bSelected, bool& bHighlighted) const final override;
	virtual void                  SetEditorObjectInfo(bool bSelected, bool bHighlighted) final override;

	virtual sxema::IObject*   GetSchematycObject() const final   { return m_pSchematycObject; };
	virtual void                  OnSchematycObjectDestroyed() final { m_pSchematycObject = nullptr; }

	virtual EEntitySimulationMode GetSimulationMode() const final    { return m_simulationMode; };

	virtual bool IsInitialized() const final { return HasInternalFlag(EInternalFlag::Initialized); }
	//~IEntity

	void                     SetSimulationMode(EEntitySimulationMode mode);
	void                     OnEditorGameModeChanged(bool bEnterGameMode);

	void                     ShutDownComponent(SEntityComponentRecord& componentRecord);

	CEntityComponentsVector<SEntityComponentRecord>& GetComponentsVector() { return m_components; };

	void                     AddSimpleEventListeners(EntityEventMask events, ISimpleEntityEventListener* pListener, IEntityComponent::ComponentEventPriority priority);
	void                     AddSimpleEventListener(EEntityEvent event, ISimpleEntityEventListener* pListener, IEntityComponent::ComponentEventPriority priority);
	void                     RemoveSimpleEventListener(EEntityEvent event, ISimpleEntityEventListener* pListener);
	void                     ClearComponentEventListeners();

	void                     SetInternalFlag(EInternalFlag flag, bool set)
	{
		if (set)
		{
			m_internalFlags |= static_cast<std::underlying_type<EInternalFlag>::type>(flag);
		}
		else
		{
			m_internalFlags &= ~static_cast<std::underlying_type<EInternalFlag>::type>(flag);
		}
	}
	bool HasInternalFlag(EInternalFlag flag) const { return (m_internalFlags & static_cast<std::underlying_type<EInternalFlag>::type>(flag)) != 0; }

protected:
	//////////////////////////////////////////////////////////////////////////
	// Attachment.
	//////////////////////////////////////////////////////////////////////////
	void OnRellocate(EntityTransformationFlagsMask transformReasons);
	void LogEvent(const SEntityEvent& event, CTimeValue dt);
	//////////////////////////////////////////////////////////////////////////

private:
	bool IsScaled(float threshold = 0.0003f) const
	{
		return (fabsf(m_scale.x - 1.0f) + fabsf(m_scale.y - 1.0f) + fabsf(m_scale.z - 1.0f)) >= threshold;
	}
	// Fetch the IA object from the AIObjectID, if any
	IAIObject* GetAIObject() const;

	void       CreateSchematycObject(const SEntitySpawnParams& params);

	void       AddComponentInternal(std::shared_ptr<IEntityComponent> pComponent, const DrxGUID& componentTypeID, IEntityComponent::SInitParams* pInitParams, const CEntityComponentClassDesc* pClassDescription);

	//////////////////////////////////////////////////////////////////////////
	// Component Save/Load
	//////////////////////////////////////////////////////////////////////////
	// Loads a component, but leaves it uninitialized
	void LoadComponent(Serialization::IArchive& archive, u8*& pBuffer);
	void SaveComponent(Serialization::IArchive& archive, IEntityComponent& component);
	// Loads a component with the legacy XML format, but leaves it uninitialized
	bool LoadComponentLegacy(XmlNodeRef entityNode, XmlNodeRef componentNode);
	void SaveComponentLegacy(DrxGUID typeId, XmlNodeRef& entityNode, XmlNodeRef& componentNode, IEntityComponent& component, bool bIncludeScriptProxy);
	//////////////////////////////////////////////////////////////////////////

	void OnComponentMaskChanged(const SEntityComponentRecord& componentRecord, EntityEventMask prevMask);
	void UpdateComponentEventListeners(const SEntityComponentRecord& componentRecord, EntityEventMask prevMask);

private:
	friend class CEntitySystem;
	friend class CPhysicsEventListener; // For faster access to internals.
	friend class CEntitySlot;           // For faster access to internals.
	friend class CEntityRender;         // For faster access to internals.
	friend class CEntityComponentTriggerBounds;
	friend class CEntityPhysics;
	friend class CNetEntity; // CNetEntity iterates all components on serialization.

	enum class EBindingType : u8
	{
		eBT_Pivot,
		eBT_GeomCacheNode,
		eBT_CharacterBone,
		eBT_LocalSim,
	};
	virtual EBindingType GetParentBindingType() const final { return m_hierarchy.parentBindingType; }

	// Childs structure, only allocated when any child entity is attached.
	struct STransformHierarchy
	{
		STransformHierarchy() = default;

		std::vector<CEntity*> children;
		CEntity*              pParent = nullptr;
		EBindingType          parentBindingType = EBindingType::eBT_Pivot;
		i32                   attachId = -1;
		attachMask            childrenAttachIds = 0; // bitmask of used attachIds of the children
	};

private:
	std::underlying_type<EInternalFlag>::type m_internalFlags = 0;

	u8 m_sendEventRecursionCount = 0;
	u16 m_componentChangeState = 0;

	// Name of the entity.
	string m_name;

	// Pointer to the class that describe this entity.
	IEntityClass* m_pClass = nullptr;

	// Pointer to the entity archetype.
	IEntityArchetype* m_pArchetype = nullptr;

	// Pointer to hierarchical binding structure.
	// It contains array of child entities and pointer to the parent entity.
	// Because entity attachments are not used very often most entities do not need it,
	// and space is preserved by keeping it separate structure.
	STransformHierarchy m_hierarchy;

	// Custom entity material.
	_smart_ptr<IMaterial> m_pMaterial;

	// Entity Links.
	IEntityLink* m_pEntityLinks = nullptr;

	// For tracking entity in the partition grid.
	SGridLocation*     m_pGridLocation = nullptr;
	// For tracking entity inside proximity trigger system.
	SProximityElement* m_pProximityEntity = nullptr;

	//! sxema object associated with this entity.
	sxema::IObject* m_pSchematycObject = nullptr;

	//! Public Properties of the associated sxema object
	sxema::IObjectPropertiesPtr m_pSchematycProperties = nullptr;

	struct SExternalEventListener final : public ISimpleEntityEventListener
	{
		SExternalEventListener(IEntityEventListener* pListener, CEntity* pEntity) : m_pListener(pListener), m_pEntity(pEntity) {}

		virtual void ProcessEvent(const SEntityEvent& event) override
		{
			m_pListener->OnEntityEvent(m_pEntity, event);
		}

		IEntityEventListener* m_pListener;
		CEntity*              m_pEntity;
	};

	std::vector<std::unique_ptr<SExternalEventListener>> m_externalEventListeners;
	// Mask of entity events that are present in m_simpleEventListeners, avoiding find operation every time
	uint64 m_eventListenerMask = 0;

	struct SEventListener
	{
		ISimpleEntityEventListener*              pListener;
		IEntityComponent::ComponentEventPriority eventPriority;
	};

	struct SEventListenerSet
	{
		SEventListenerSet(EEntityEvent entityEvent, const SEventListener& firstEventListener)
			: event(entityEvent)
			, firstUnsortedListenerIndex(-1)
			, hasValidElements(1) 
			, listeners({ firstEventListener }) {}

		SEventListenerSet(const SEventListenerSet&) = delete;
		SEventListenerSet& operator=(const SEventListenerSet&) = delete;
		SEventListenerSet(SEventListenerSet&& other)
			: event(other.event)
			, listeners(std::move(other.listeners))
			, firstUnsortedListenerIndex(other.firstUnsortedListenerIndex)
			, hasValidElements(other.hasValidElements) {}
		SEventListenerSet& operator=(SEventListenerSet&&) = default;

		EEntityEvent event;

	protected:
		std::vector<SEventListener> listeners;
		// Index of the listener we pushed back while iterating
		// Allows us to skip going through a lot of possibly unneeded listeners
		// Negative if we are sorted
		i16 firstUnsortedListenerIndex : 15;
		// Whether or not we have valid (non-null) entries in the vector
		// Needed since Remove while we are iterating requires that we invalidate instead of erasing
		i16 hasValidElements : 1;

		void CheckForInvalidatedElements()
		{
			if (hasValidElements)
			{
				hasValidElements = std::find_if(listeners.begin(), listeners.end(), [](const SEventListener& listener) -> bool { return listener.pListener != nullptr; }) != listeners.end();
			}
		}

	public:
		std::vector<SEventListener>::iterator FindListener(ISimpleEntityEventListener* pListener)
		{
			return std::find_if(listeners.begin(), listeners.end(), [pListener](const SEventListener& listener) -> bool { return listener.pListener == pListener; });
		}

		std::vector<SEventListener>::const_iterator GetEnd() const { return listeners.end(); }

		// Whether or not the entire collection of listeners is sorted
		bool IsSorted() const { return isneg(firstUnsortedListenerIndex) != 0; }
		void OnSorted() { firstUnsortedListenerIndex = -1; }

		void SendEvent(const SEntityEvent& event)
		{
			// Numbered iteration as listeners may be invalidated (set to null) during iteration
			// Sorting is not affected while iterating, so this is safe
			for (size_t i = 0; i < listeners.size(); ++i)
			{
				if (listeners[i].pListener != nullptr)
				{
					listeners[i].pListener->ProcessEvent(event);
				}
			}
		}

		void SortedInsert(SEventListener& listener)
		{
			// Attempt to sort existing elements first, if there are any unsorted
			if (!IsSorted())
			{
				SortUnsortedElements();
			}

			// Sorted insertion based on the requested event priority
			auto upperBoundIt = std::upper_bound(listeners.begin(), listeners.end(), listener, [](const SEventListener& a, const SEventListener& b) -> bool { return a.eventPriority < b.eventPriority; });
			listeners.emplace(upperBoundIt, listener);
			hasValidElements = 1;
		}

		void UnsortedInsert(SEventListener& listener)
		{
			if (IsSorted())
			{
				firstUnsortedListenerIndex = static_cast<i16>(listeners.size());
			}

			// Adding of event listener while we are iterating, we cannot sort here so push to back and require sort when iteration is done
			listeners.push_back(listener);
			hasValidElements = 1;
		}

		void RemoveElement(std::vector<SEventListener>::const_iterator listenerIt)
		{
			// See if we need to adjust firstUnsortedListenerIndex due to an element being erased
			if (!IsSorted() && std::distance(static_cast<std::vector<SEventListener>::const_iterator>(listeners.begin()), listenerIt) < firstUnsortedListenerIndex)
			{
				firstUnsortedListenerIndex--;
			}

			listeners.erase(listenerIt);

			// Take the opportunity to sort the elements if we are able
			if (!IsSorted())
			{
				SortUnsortedElements();
			}

			CheckForInvalidatedElements();
		}

		void InvalidateElement(std::vector<SEventListener>::iterator listenerIt)
		{
			DRX_ASSERT(listenerIt->pListener != nullptr);
			listenerIt->pListener = nullptr;

			u16 elementIndex = static_cast<u16>(std::distance(listeners.begin(), listenerIt));

			if (elementIndex < firstUnsortedListenerIndex)
			{
				firstUnsortedListenerIndex = elementIndex;
			}

			CheckForInvalidatedElements();
		}

		bool HasValidElements() const
		{
			return hasValidElements != 0;
		}

		// Sort any potentially unsorted elements, does not affect the rest of the container as it is assumed sorted
		void SortUnsortedElements()
		{
			DRX_ASSERT(!IsSorted());
			u16 numUnsortedListeners = static_cast<u16>(listeners.size() - firstUnsortedListenerIndex);

			while (numUnsortedListeners > 0 && listeners.size() != 0)
			{
				// Temporarily remove item for sorting
				auto listenerIt = --listeners.end();
				numUnsortedListeners--;

				if (listenerIt->pListener != nullptr)
				{
					SEventListener eventListener = *listenerIt;
					listeners.erase(listenerIt);

					// Sorted insertion based on the requested event priority
					auto upperBoundIt = std::upper_bound(listeners.begin(), listeners.end() - numUnsortedListeners, eventListener, [](const SEventListener& a, const SEventListener& b) -> bool { return a.eventPriority < b.eventPriority; });
					listeners.insert(upperBoundIt, eventListener);
				}
				else
				{
					listeners.erase(listenerIt);
				}
			}

			OnSorted();
		}
	};

	std::vector<std::unique_ptr<SEventListenerSet>> m_simpleEventListeners;

	// NetEntity stores the network serialization configuration.
	std::unique_ptr<INetEntity> m_pNetEntity;

	CEntityRender               m_render;
	CEntityPhysics              m_physics;

	// Optional entity guid.
	EntityGUID m_guid;

	// Unique ID of the entity.
	EntityId m_id;

	// The representation of this entity in the AI system.
	tAIObjectID m_aiObjectID = INVALID_AIOBJECTID;

	// Entity flags. any combination of EEntityFlags values.
	std::underlying_type<EEntityFlags>::type m_flags = 0;

	// Entity extended flags. any combination of EEntityFlagsExtended values.
	std::underlying_type<EEntityFlagsExtended>::type m_flagsExtended = 0;

	//! Simulation mode of this entity, (ex Preview, In Game, In Editor)
	EEntitySimulationMode m_simulationMode = EEntitySimulationMode::Idle;

	// Position of the entity in local space.
	Vec3             m_position = ZERO;
	// Rotation of the entity in local space.
	Quat             m_rotation = IDENTITY;
	// Scale of the entity in local space.
	Vec3             m_scale = Vec3(1.f);
	// World transformation matrix of this entity.
	mutable Matrix34 m_worldTM = IDENTITY;

	// counter to prevent deletion if entity is processed deferred by for example physics events
	u16 m_keepAliveCounter = 0;

	using TComponentsRecord = CEntityComponentsVector<SEntityComponentRecord>;
	// Array of components, linear search in a small array is almost always faster then a more complicated set or map containers.
	TComponentsRecord m_components;
};
