// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Extension/IDrxUnknown.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Network/SerializeFwd.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/Serializer.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/BitMask.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/Network/INetEntity.h>

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/EnumFlags.h>

#include <drx3D/CoreX/Math/Rotation.h>
#include <drx3D/CoreX/Math/Transform.h>

#include <drx3D/Entity/IEntityBasicTypes.h>

// Forward declarations
struct SEntitySpawnParams;
struct SEntityEvent;
struct SEntityUpdateContext;
struct IShaderPublicParams;
struct IFlowGraph;
struct IEntityEventListener;
struct IPhysicalEntity;
struct SSGHandle;
struct a2DPoint;
struct IRenderMesh;
struct IClipVolume;
struct IBSPTree3D;
struct IMaterial;
struct IScriptTable;
struct AABB;
struct IRenderNode;
struct IEntity;
struct INetworkSpawnParams;
struct IEntityScript;
struct SEntityPreviewContext;

namespace sxema
{
class CObject;
}

// Forward declaration of Sandbox's entity object, should be removed when IEntityComponent::Run is gone
class CEntityObject;

//! Derive from this interface to expose custom entity properties in the editor using the serialization framework.
//! Each entity component can contain one property group, each component will be separated by label in the entity property view
struct IEntityPropertyGroup
{
	virtual ~IEntityPropertyGroup() {}

	virtual tukk GetLabel() const = 0;
	virtual void        SerializeProperties(Serialization::IArchive& archive) = 0;
};

//////////////////////////////////////////////////////////////////////////
//! Compatible to the DRXINTERFACE_DECLARE
#define DRX_ENTITY_COMPONENT_INTERFACE(iname, iidHigh, iidLow)                           DRX_PP_ERROR("Deprecated macro: Use DRX_ENTITY_COMPONENT_INTERFACE_GUID instead. Please refer to the Migrating Guide from DRXENGINE 5.3 to DRXENGINE 5.4 for more details.")
#define DRX_ENTITY_COMPONENT_INTERFACE_GUID(iname, iguid)                                DRXINTERFACE_DECLARE_GUID(iname, iguid)

#define DRX_ENTITY_COMPONENT_CLASS(implclassname, interfaceName, cname, iidHigh, iidLow) DRX_PP_ERROR("Deprecated macro: Use DRX_ENTITY_COMPONENT_CLASS_GUID instead. Please refer to the Migrating Guide from DRXENGINE 5.3 to DRXENGINE 5.4 for more details.")

#define DRX_ENTITY_COMPONENT_CLASS_GUID(implclassname, interfaceName, cname, cguid) \
  DRXINTERFACE_BEGIN()                                                              \
  DRXINTERFACE_ADD(IEntityComponent)                                                \
  DRXINTERFACE_ADD(interfaceName)                                                   \
  DRXINTERFACE_END()                                                                \
  DRXGENERATE_CLASS_GUID(implclassname, cname, cguid)

#define DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS(implclassname, cname, cguid) DRX_PP_ERROR("Deprecated macro: Use DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS_GUID instead. Please refer to the Migrating Guide from DRXENGINE 5.3 to DRXENGINE 5.4 for more details.")

#define DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS_GUID(implclassname, cname, cguid) \
  DRX_ENTITY_COMPONENT_INTERFACE_GUID(implclassname, cguid)                        \
  DRX_ENTITY_COMPONENT_CLASS_GUID(implclassname, implclassname, cname, cguid)

#include <drx3D/Schema/Component.h>

enum class EEntityComponentFlags : u32
{
	None              = 0,
	Singleton         = BIT(0),  //!< Allow only of one instance of this component per class/object.
	Legacy            = BIT(1),  //!< Legacy component, only for backward computability should not be accessible for creation in the UI. (Will also enable saving with LegacySerializeXML)
	Transform         = BIT(2),  //!< Component has transform.
	Socket            = BIT(3),  //!< Other components can be attached to socket of this component.
	Attach            = BIT(4),  //!< This component can be attached to socket of other components.
	sxema         = BIT(5),  //!< Component was created and owned by the sxema.
	SchematycEditable = BIT(6),  //!< sxema components where properties of it can be edited per each Entity instance.
	SchematycModified = BIT(7),  //!< Only in combination with the SchematycEditable component to indicate that some parameters where modified from sxema defaults by the user.
	UserAdded         = BIT(8),  //!< This component was added in the Editor by the user
	NoSave            = BIT(9),  //!< Not save this component under entity components list when saving/loading
	NetNotReplicate   = BIT(10), //!< This component should be not be network replicated.
	HideFromInspector = BIT(11), //!< This component can not be added from the Inspector, instead requiring use in sxema or C++.
	ServerOnly        = BIT(12), //!< This component can only be loaded when we are running as local or dedicated server
	ClientOnly        = BIT(13), //!< This component can only be loaded when we are running as a client, never on a dedicated server
	HiddenFromUser    = BIT(14), //!< This component will not be shown to the user
	NoCreationOffset  = BIT(15), //!< Disables the creation offset in sandbox. The sandbox uses the bounding box radius as an offset to place new entities so they don't clip into the terrain.
};
typedef CEnumFlags<EEntityComponentFlags> EntityComponentFlags;

//! Structure that describes how one entity component
//! interacts with another entity component.
struct SEntityComponentRequirements
{
	enum class EType : u32
	{
		Incompatibility,  //!< These components are incompatible and cannot be used together
		SoftDependency,   //!< Dependency must be initialized before component.
		HardDependency    //!< Dependency must exist and be initialized before component.
	};

	inline SEntityComponentRequirements(EType _type, const DrxGUID& _guid)
		: type(_type)
		, guid(_guid)
	{}

	EType   type;
	DrxGUID guid;
};

//! Interface used by the editor to Preview Render of the entity component
//! This can be used to draw helpers or preview elements in the sandbox
//! \par Example
//! \include DinrusXEntitySys/Examples/PreviewComponent.cpp
struct IEntityComponentPreviewer
{
	virtual ~IEntityComponentPreviewer() {}

	//! Override this method to Edit UI properties for previewer of the component
	virtual void SerializeProperties(Serialization::IArchive& archive) = 0;

	//! Override this method to Render a preview of the Entity Component
	//! This method is not used when entity is normally rendered
	//! But only used for previewing the entity in the Sandbox Editor
	//! \param entity Entity which gets drawn
	//! \param component Component which is gets drawn
	//! \param context PreviewContext contains information and settings for the rendering
	//! \see IEntity::SEntityPreviewContext
	virtual void Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const = 0;
};

//! \cond INTERNAL
//! A class that describe and reflect members of the entity component
//! Properties of the component are reflected using run-time class
//! reflection system, and stored in here.
class CEntityComponentClassDesc : public sxema::CClassDesc
{
public:
	inline void SetComponentFlags(const EntityComponentFlags& flags)
	{
		m_flags = flags;
	}

	EntityComponentFlags GetComponentFlags() const
	{
		return m_flags;
	}

	inline  void AddComponentInteraction(SEntityComponentRequirements::EType type, const DrxGUID& guid)
	{
		m_interactions.emplace_back(type, guid);
	}

	template<class T>
	inline void AddComponentInteraction(SEntityComponentRequirements::EType type)
	{
		m_interactions.emplace_back(type, sxema::GetTypeGUID<T>());
	}

	inline bool IsCompatibleWith(const DrxGUID& guid) const
	{
		for (const SEntityComponentRequirements& dependency : m_interactions)
		{
			if (dependency.type == SEntityComponentRequirements::EType::Incompatibility && dependency.guid == guid)
			{
				return false;
			}
		}

		return true;
	}

	inline bool DependsOn(const DrxGUID& guid) const
	{
		for (const SEntityComponentRequirements& dependency : m_interactions)
		{
			if ((dependency.type == SEntityComponentRequirements::EType::SoftDependency || dependency.type == SEntityComponentRequirements::EType::HardDependency)
			    && dependency.guid == guid)
			{
				return true;
			}
		}

		return false;
	}

	const DynArray<SEntityComponentRequirements>& GetComponentInteractions() const { return m_interactions; }

private:
	EntityComponentFlags                   m_flags;
	DynArray<SEntityComponentRequirements> m_interactions;
};
//! \endcond

namespace sxema
{
struct SObjectSignal;
//////////////////////////////////////////////////////////////////////////
// All classes derived from IEntityComponent will be using
// CEntityComponentClassDesc
//////////////////////////////////////////////////////////////////////////
namespace Helpers
{
template<typename TYPE> struct SIsCustomClass<TYPE, typename std::enable_if<std::is_convertible<TYPE, IEntityComponent>::value>::type>
{
	static const bool value = true;
};
}
template<typename TYPE>
class CTypeDesc<TYPE, typename std::enable_if<std::is_convertible<TYPE, IEntityComponent>::value>::type>
	: public CClassDescInterface<TYPE, CEntityComponentClassDesc>
{
};

} // sxema

//! \brief Represents a component that can be attached to an individual entity instance
//! Allows for populating entities with custom game logic
//! \anchor MinimalEntityComponent
//! \par Example
//! \include DinrusXEntitySys/Examples/MinimalEntityComponent.cpp
struct IEntityComponent : public IDrxUnknown, ISimpleEntityEventListener
{
	// Helper to serialize both legacy and sxema properties of an entity
	struct SPropertySerializer
	{
		void Serialize(Serialization::IArchive& archive)
		{
			// Start with the legacy properties
			if (IEntityPropertyGroup* pPropertyGroup = pComponent->GetPropertyGroup())
			{
				struct SSerializeWrapper
				{
					void Serialize(Serialization::IArchive& archive)
					{
						pGroup->SerializeProperties(archive);
					}

					IEntityPropertyGroup* pGroup;
				};

				archive(SSerializeWrapper { pPropertyGroup }, "legacy", "legacy");
			}

			// Serialize sxema properties
			sxema::Utils::SerializeClass(archive, pComponent->GetClassDesc(), pComponent, "schematyc", "schematyc");
		}

		IEntityComponent* pComponent;
	};

	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityComponent, "6a6ffe9a-a3d4-4cd6-9ef1-fc42ee649776"_drx_guid)

	typedef i32                   ComponentEventPriority;

	typedef EEntityComponentFlags EFlags;
	typedef EntityComponentFlags  ComponentFlags;

	static constexpr i32 EmptySlotId = -1;

	//! SInitParams is only used from sxema to call PreInit to initialize sxema Entity Component
	struct SInitParams
	{
		inline SInitParams(
		  IEntity* pEntity_,
		  const DrxGUID& guid_,
		  const string& name_,
		  const CEntityComponentClassDesc* classDesc_,
		  EntityComponentFlags flags_,
		  IEntityComponent* pParent_,
		  const DrxTransform::CTransformPtr& transform_
		  )
			: pEntity(pEntity_)
			, guid(guid_)
			, name(name_)
			, classDesc(classDesc_)
			, flags(flags_)
			, pParent(pParent_)
			, transform(transform_)
		{}

		IEntity*                           pEntity;
		const DrxGUID                      guid;
		const string&                      name;
		const CEntityComponentClassDesc*   classDesc;
		const DrxTransform::CTransformPtr& transform;
		IEntityComponent*                  pParent = nullptr;
		INetworkSpawnParams*               pNetworkSpawnParams = nullptr;
		EntityComponentFlags               flags;
	};

public:
	//~IDrxUnknown
	virtual IDrxFactory* GetFactory() const { return nullptr; };

protected:
	virtual uk QueryInterface(const DrxInterfaceID& iid) const { return nullptr; };
	virtual uk QueryComposite(tukk name) const          { return nullptr; };
	//~IDrxUnknown

public:
	// Return Host entity pointer
	ILINE IEntity* GetEntity() const { return m_pEntity; };
	ILINE EntityId GetEntityId() const;

public:
	IEntityComponent() {}
	IEntityComponent(IEntityComponent&& other)
		: m_pEntity(other.m_pEntity)
		, m_componentFlags(other.m_componentFlags)
		, m_guid(other.m_guid)
		, m_name(other.m_name.c_str())
		, m_pTransform(std::move(other.m_pTransform))
		, m_pParent(other.m_pParent)
		, m_pClassDesc(other.m_pClassDesc)
		, m_entitySlotId(other.m_entitySlotId)
	{
		other.m_pTransform.reset();
		other.m_entitySlotId = EmptySlotId;
	}
	IEntityComponent(const IEntityComponent&) = default;
	IEntityComponent& operator=(const IEntityComponent&) = default;
	IEntityComponent& operator=(IEntityComponent&&) = default;
	virtual ~IEntityComponent() {}

	//! Return ClassDesc for this component
	//! Class description is storing runtime C++ class reflection information
	//! It contain information about member variables of the component and how to serialize them,
	//! information how to create an instance of class and all relevant additional information to handle this class.
	const CEntityComponentClassDesc& GetClassDesc() const
	{
		if (m_pClassDesc == nullptr)
		{
			static CEntityComponentClassDesc nullClassDesc;
			return nullClassDesc;
		}
		return *m_pClassDesc;
	}

	//////////////////////////////////////////////////////////////////////////
	// BEGIN IEntityComponent virtual interface
	// Derived classes mostly interested in overriding these virtual methods
	//////////////////////////////////////////////////////////////////////////

protected:
	//! Only called by system classes to initalize component.
	//! Users must not call this method directly
	virtual void PreInit(const SInitParams& params)
	{
		m_guid = params.guid;
		m_name = params.name;
		m_pClassDesc = params.classDesc;
		if (m_pClassDesc)
		{
			m_componentFlags.Add(m_pClassDesc->GetComponentFlags());
		}
		m_componentFlags.Add(params.flags);
		m_pTransform = params.transform;
		//m_pPreviewer = params.pPreviewer;
		m_pParent = params.pParent;
	}

	//! Called at the very first initialization of the component, at component creation time.
	virtual void Initialize() {}

	//! Called on all Entity components right before all of the Entity Components are destructed.
	virtual void OnShutDown() {};

	//! Called when the transformation of the component is changed
	virtual void OnTransformChanged() {}

	//! By overriding this function component will be able to handle events sent from the host Entity.
	//! Requires returning the desired event flag in GetEventMask.
	//! \param event Event structure, contains event id and parameters.
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentEvents.cpp
	virtual void ProcessEvent(const SEntityEvent& event) {}

public:
	//! Return bit mask of the EEntityEvent flags that we want to receive in ProcessEvent
	//! (ex: ENTITY_EVENT_BIT(ENTITY_EVENT_HIDE) | ENTITY_EVENT_BIT(ENTITY_EVENT_UNHIDE))
	//! Only events matching the returned bit mask will be sent to the ProcessEvent method
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentEvents.cpp
	virtual Drx::Entity::EntityEventMask GetEventMask() const { return 0; }

	//! Determines the order in which this component will receive entity events (including update). Lower number indicates a higher priority.
	virtual ComponentEventPriority GetEventPriority() const { return (ComponentEventPriority)GetProxyType(); }

	//! \brief Network serialization. Override to provide a mask of active network aspects
	//! used by this component. Called once during binding to network.
	//! \warning Sending entity events or querying other components is prohibited from within this function!
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentNetSerialize.cpp
	virtual NetworkAspectType GetNetSerializeAspectMask() const { return 0; }

	//! \brief Network serialization. Will be called for each active aspect for both reading and writing.
	//! @param[in,out] ser Serializer for reading/writing values.
	//! @param[in] aspect The number of the aspect being serialized.
	//! @param[in] profile Can be ignored, used by DrxPhysics only.
	//! @param[in] flags Can be ignored, used by DrxPhysics only.
	//! \see ISerialize::Value()
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentNetSerialize.cpp
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) { return true; };

	//! Used to match an entity's state when it is replicated onto a remote machine.
	//! This is called once when spawning an entity, in order to serialize its data - and once again on the remote client to deserialize the state.
	//! Deserialization will always occur *before* IEntityComponent::Initialize is called.
	//! @param[in,out] ser Serializer for reading / writing values.
	//! \warning This is not called from the Main thread, keep thread safety in mind - and in the best case only serialize local values, without invoking complex logic.
	//! \see ISerialize::Value()
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentNetReplicate.cpp
	virtual void NetReplicateSerialize(TSerialize ser) {}

	//! \brief Call this to trigger aspect synchronization over the network. A shortcut.
	//! \see INetEntity::MarkAspectsDirty()
	//! \par Example
	//! \include DinrusXEntitySys/Examples/ComponentNetSerialize.cpp
	virtual void NetMarkAspectsDirty(const NetworkAspectType aspects); // The definition is in IEntity.h

	//! \brief Override this to return preview render interface for the component.
	//! Multiple component instances can usually share the same previewer class instance.
	//! \see IEntityComponentPreviewer
	virtual IEntityComponentPreviewer* GetPreviewer() { return nullptr; }

	//////////////////////////////////////////////////////////////////////////
	//! END IEntityComponent virtual interface
	//////////////////////////////////////////////////////////////////////////

public:
	//! Set flags for this component
	void SetComponentFlags(ComponentFlags flags) { m_componentFlags = flags; };

	//! Return flags for this component
	const ComponentFlags& GetComponentFlags() const { return m_componentFlags; };
	ComponentFlags&       GetComponentFlags()       { return m_componentFlags; };

	//! Return GUID of this component.
	//! This GUID is only guaranteed to be unique within the host entity, different entities can have components with equal GUIDs.
	//! Each component in the entity have type guid (GetClassDesc().GetGUID()) (ex: identify EntityLightComponent class)
	//! and a unique instance guid IEntityComponent::GetGUID() (ex: identify Light01,Light02,etc.. component)
	const DrxGUID& GetGUID() const { return m_guid; }

	//! Return Parent component, only used by sxema components
	//! Initialized by the PreInit call
	IEntityComponent* GetParent() const { return m_pParent; };

	//! Return Transformation of the entity component relative to the owning entity or parent component
	const DrxTransform::CTransformPtr& GetTransform() const { return m_pTransform; }

	//! Sets the transformation form a matrix. If the component doesn't have a transformation yet the function will add one.
	void SetTransformMatrix(const Matrix34& transform);

	//! Sets the transformation from another transformation.  If the component doesn't have a transformation yet the function will add one.
	void SetTransformMatrix(const DrxTransform::CTransformPtr& transform);

	//! Return Transformation of the entity component relative to the world
	Matrix34 GetWorldTransformMatrix() const;

	//! Return Calculated Transformation Matrix for current component transform
	Matrix34 GetTransformMatrix() const { return (m_componentFlags.Check(EEntityComponentFlags::Transform) && m_pTransform) ? m_pTransform->ToMatrix34() : IDENTITY; }

	//! Get name of this individual component, usually only sxema components will have names
	tukk GetName() const { return m_name.c_str(); };

	//! Set a new name for this component
	//! Names of the components must not be unique
	void SetName(tukk szName) { m_name = szName; };

	//////////////////////////////////////////////////////////////////////////
	// HELPER METHODS FOR WORKING WITH ENTITY SLOTS
	//////////////////////////////////////////////////////////////////////////

	//! Return optional EntitySlot id used by this Component
	i32 GetEntitySlotId() const { return m_entitySlotId; }

	//! Return optional EntitySlot id used by this Component
	//! If slot id is not allocated, new slotid will be allocated and returned
	i32 GetOrMakeEntitySlotId();

	//! Stores Entity slot id used by this component.
	void SetEntitySlotId(i32 slotId) { m_entitySlotId = slotId; }

	//! Frees entity slot used by this component
	void FreeEntitySlot();
	//////////////////////////////////////////////////////////////////////////

	//! Return Current simulation mode of the host Entity
	EEntitySimulationMode GetEntitySimulationMode() const;

	//! Send event to this specific component, first checking if the component is interested in the event
	//! \param event description
	//! \param receiving component
	inline void SendEvent(const SEntityEvent& event)
	{
		if ((GetEventMask() & ENTITY_EVENT_BIT(event.event)) != 0)
		{
			ProcessEvent(event);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// SCHEMATYC SIGNALS HELPERS
	//////////////////////////////////////////////////////////////////////////
	//void ProcessSignal( const sxema::SObjectSignal &signal );

public:
	//////////////////////////////////////////////////////////////////////////
	// BEGIN Deprecated Methods
	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const {};

	//! SaveGame serialization. Override to specify what to serialize in a saved game.
	//! \param ser Serializing stream. Use IsReading() to decide read/write phase. Use Value() to read/write a property.
	virtual void GameSerialize(TSerialize ser) {}
	//! SaveGame serialization. Override to enable serialization for the component.
	//! \return true If component needs to be serialized to/from a saved game.
	virtual bool NeedGameSerialize() { return false; };

	//! Optionally serialize component to/from XML.
	//! For user-facing properties, see GetProperties.
	virtual void LegacySerializeXML(XmlNodeRef& entityNode, XmlNodeRef& componentNode, bool bLoading) {}

	//! Optionally serialize component to/from XML.
	//! For user-facing properties, see GetProperties.
	virtual void Serialize(Serialization::IArchive& archive) {}

	//! Only for backward compatibility to Release 5.3.0 for loading
	virtual struct IEntityPropertyGroup* GetPropertyGroup() { return nullptr; }

	//! Legacy, used for old entity proxies
	virtual EEntityProxy GetProxyType() const { return ENTITY_PROXY_LAST; };
	//////////////////////////////////////////////////////////////////////////
	// ~END Deprecated Methods
	//////////////////////////////////////////////////////////////////////////

protected:
	friend IEntity;
	friend class CEntity;
	friend class CEntitySystem;
	// Needs access to OnShutDown to maintain legacy game object extension shutdown behavior
	friend class CGameObject;
	// Needs access to Initialize
	friend sxema::CObject;

	// Host Entity pointer
	IEntity*       m_pEntity = nullptr;

	ComponentFlags m_componentFlags;

	//! Unique GUID of the instance of this component
	DrxGUID m_guid;

	//! name of this component
	string m_name;

	//! Optional transformation setting for the component within the Entity object
	DrxTransform::CTransformPtr m_pTransform;

	//! Optional pointer to our parent component
	IEntityComponent* m_pParent = nullptr;

	//! Reflected type description for this component
	//! Contain description of the reflected member variables
	const CEntityComponentClassDesc* m_pClassDesc = nullptr;

	//! Optional Entity SlotId for storing component data like geometry of character
	i32 m_entitySlotId = EmptySlotId;
};

//! Lua Script component interface.
struct IEntityScriptComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityScriptComponent, "bd6403cf-3b49-f39e-9540-3fd1c6d4f755"_drx_guid)

	virtual void          SetScriptUpdateRate(float fUpdateEveryNSeconds) = 0;
	virtual IScriptTable* GetScriptTable() = 0;
	virtual void          CallEvent(tukk sEvent) = 0;
	virtual void          CallEvent(tukk sEvent, float fValue) = 0;
	virtual void          CallEvent(tukk sEvent, bool bValue) = 0;
	virtual void          CallEvent(tukk sEvent, tukk sValue) = 0;
	virtual void          CallEvent(tukk sEvent, const Vec3& vValue) = 0;
	virtual void          CallEvent(tukk sEvent, EntityId nEntityId) = 0;

	//! Change current state of the entity script.
	//! \return If state was successfully set.
	virtual bool GotoState(tukk sStateName) = 0;

	//! Change current state of the entity script.
	//! \return If state was successfully set.
	virtual bool GotoStateId(i32 nStateId) = 0;

	//! Check if entity is in specified state.
	//! \param sStateName Name of state table within entity script (case sensitive).
	//! \return If entity script is in specified state.
	virtual bool IsInState(tukk sStateName) = 0;

	//! Retrieves name of the currently active entity script state.
	//! \return Name of current state.
	virtual tukk GetState() = 0;

	//! Retrieves the id of the currently active entity script state.
	//! \return Index of current state.
	virtual i32 GetStateId() = 0;

	//! Fires an event in the entity script.
	//! This will call OnEvent(id,param) Lua function in entity script, so that script can handle this event.
	virtual void SendScriptEvent(i32 Event, IScriptTable* pParamters, bool* pRet = NULL) = 0;
	virtual void SendScriptEvent(i32 Event, tukk str, bool* pRet = NULL) = 0;
	virtual void SendScriptEvent(i32 Event, i32 nParam, bool* pRet = NULL) = 0;

	//! Change the Entity Script used by the Script Component.
	//! Caller is responsible for making sure new script is initialised and script bound as required
	//! \param pScript an entity script object that has already been loaded with the new script.
	//! \param params parameters used to set the properties table if required.
	virtual void ChangeScript(IEntityScript* pScript, SEntitySpawnParams* params) = 0;

	//! Sets physics parameters from an existing script table
	//! \param type - one of PHYSICPARAM_... values
	//! \param params script table containing the values to set
	virtual void SetPhysParams(i32 type, IScriptTable* params) = 0;

	//! Determines whether or not the script should receive update callbacks
	//! Replaces IEntity::Activate for legacy projects
	virtual void EnableScriptUpdate(bool bEnable) = 0;
};

//! Interface to the trigger component, exposing support for tracking enter / leave events for other entities entering a predefined trigger box
//! An in-game example that uses this functionality is the ProximityTrigger entity
//! \par Example
//! \include DinrusXEntitySys/Examples/TriggerComponent.cpp
struct IEntityTriggerComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityTriggerComponent, "de73851b-7e35-419f-a509-51d204f555de"_drx_guid)

	//! Creates a trigger bounding box.
	//! When physics will detect collision with this bounding box it will send an events to the entity.
	//! If entity have script OnEnterArea and OnLeaveArea events will be called.
	//! \param bbox Axis aligned bounding box of the trigger in entity local space (Rotation and scale of the entity is ignored). Set empty bounding box to disable trigger.
	virtual void SetTriggerBounds(const AABB& bbox) = 0;

	//! Retrieve trigger bounding box in local space.
	//! \return Axis aligned bounding box of the trigger in the local space.
	virtual void GetTriggerBounds(AABB& bbox) = 0;

	//! Forward enter/leave events to this entity
	virtual void ForwardEventsTo(EntityId id) = 0;

	//! Invalidate the trigger, so it gets recalculated and catches things which are already inside when it gets enabled.
	virtual void InvalidateTrigger() = 0;
};

//! Helper component for playing back audio on the current world-space position of an entity.
//! Wraps low-level DrxAudio logic.
struct IEntityAudioComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityAudioComponent, "9824845c-fe37-7889-b172-4a63f331d8a2"_drx_guid)

	virtual void                    SetFadeDistance(float const fadeDistance) = 0;
	virtual float                   GetFadeDistance() const = 0;
	virtual void                    SetEnvironmentFadeDistance(float const environmentFadeDistance) = 0;
	virtual float                   GetEnvironmentFadeDistance() const = 0;
	virtual float                   GetGreatestFadeDistance() const = 0;
	virtual void                    SetEnvironmentId(DrxAudio::EnvironmentId const environmentId) = 0;
	virtual DrxAudio::EnvironmentId GetEnvironmentId() const = 0;
	//! Creates an additional audio object managed by this component, allowing individual handling of effects
	//! IEntityAudioComponent will always create an audio object by default where audio will be played unless otherwise specified.
	virtual DrxAudio::AuxObjectId   CreateAudioAuxObject() = 0;
	virtual bool                    RemoveAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId) = 0;
	virtual void                    SetAudioAuxObjectOffset(Matrix34 const& offset, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual Matrix34 const&         GetAudioAuxObjectOffset(DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual bool                    PlayFile(DrxAudio::SPlayFileInfo const& playbackInfo, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) = 0;
	virtual void                    StopFile(char const* const szFile, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	//! Executes the specified trigger on the entity
	//! \param audioTriggerId The trigger we want to execute
	//! \param audioAuxObjectId Audio object within the component that we want to set, see IEntityAudioComponent::CreateAudioAuxObject. If not provided it is played on the default object.
	//! \par Example
	//! \include DinrusXEntitySys/Examples/Audio/ExecuteTrigger.cpp
	virtual bool                    ExecuteTrigger(DrxAudio::ControlId const audioTriggerId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) = 0;
	virtual void                    StopTrigger(DrxAudio::ControlId const audioTriggerId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) = 0;
	//! Sets the current state of a switch in the entity
	//! \param audioSwitchId Identifier of the switch whose state we want to change
	//! \param audioStateId Identifier of the switch state we want to set to
	//! \param audioAuxObjectId Audio object within the component that we want to set, see IEntityAudioComponent::CreateAudioAuxObject. If not provided it is played on the default object.
	//! \par Example
	//! \include DinrusXEntitySys/Examples/Audio/SetSwitchState.cpp
	virtual void                    SetSwitchState(DrxAudio::ControlId const audioSwitchId, DrxAudio::SwitchStateId const audioStateId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	//! Sets the value of the specified parameter on the entity
	//! \param parameterId Identifier of the parameter we want to modify the value of
	//! \param audioAuxObjectId Audio object within the component that we want to set, see IEntityAudioComponent::CreateAudioAuxObject. If not provided it is played on the default object.
	//! \par Example
	//! \include DinrusXEntitySys/Examples/Audio/SetParameterValue.cpp
	virtual void                    SetParameter(DrxAudio::ControlId const parameterId, float const value, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual void                    SetObstructionCalcType(DrxAudio::EOcclusionType const occlusionType, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual void                    SetEnvironmentAmount(DrxAudio::EnvironmentId const audioEnvironmentId, float const amount, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual void                    SetCurrentEnvironments(DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) = 0;
	virtual void                    AudioAuxObjectsMoveWithEntity(bool const bCanMoveWithEntity) = 0;
	virtual void                    AddAsListenerToAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId, void (* func)(DrxAudio::SRequestInfo const* const), DrxAudio::ESystemEvents const eventMask) = 0;
	virtual void                    RemoveAsListenerFromAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId, void (* func)(DrxAudio::SRequestInfo const* const)) = 0;
	virtual DrxAudio::AuxObjectId   GetAuxObjectIdFromAudioObject(DrxAudio::IObject* pObject) = 0;
};

//! Type of an area managed by IEntityAreaComponent.
enum EEntityAreaType
{
	ENTITY_AREA_TYPE_SHAPE,          //!< Area type is a closed set of points forming shape.
	ENTITY_AREA_TYPE_BOX,            //!< Area type is a oriented bounding box.
	ENTITY_AREA_TYPE_SPHERE,         //!< Area type is a sphere.
	ENTITY_AREA_TYPE_GRAVITYVOLUME,  //!< Area type is a volume around a bezier curve.
	ENTITY_AREA_TYPE_SOLID           //!< Area type is a solid which can have any geometry figure.
};

//! Area component allow for entity to host an area trigger.
//! Area can be shape, box or sphere, when marked entities cross this area border,
//! it will send ENTITY_EVENT_ENTERAREA, ENTITY_EVENT_LEAVEAREA, and ENTITY_EVENT_AREAFADE
//! events to the target entities.
struct IEntityAreaComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityAreaComponent, "98eda61f-de8b-e2b1-a1ca-2a88e4eede66"_drx_guid)

	enum EAreaComponentFlags
	{
		FLAG_NOT_UPDATE_AREA = BIT(1), //!< When set points in the area will not be updated.
		FLAG_NOT_SERIALIZE   = BIT(2)  //!< Areas with this flag will not be serialized.
	};

	//! Area flags.
	virtual void SetFlags(i32 nAreaComponentFlags) = 0;

	//! Area flags.
	virtual i32 GetFlags() = 0;

	//! Retrieve area type.
	//! \return One of EEntityAreaType enumerated types.
	virtual EEntityAreaType GetAreaType() const = 0;

	//! Get the actual area referenced by this component.
	virtual struct IArea* GetArea() const = 0;

	//! Sets area to be a shape, and assign points to this shape.
	//! Points are specified in local entity space, shape will always be constructed in XY plane,
	//! lowest Z of specified points will be used as a base Z plane.
	//! If fHeight parameter is 0, shape will be considered 2D shape, and during intersection Z will be ignored
	//! If fHeight is not zero shape will be considered 3D and will accept intersection within vertical range from baseZ to baseZ+fHeight.
	//! \param pPoints                   Array of 3D vectors defining shape vertices.
	//! \param pSoundObstructionSegments Array of corresponding booleans that indicate sound obstruction.
	//! \param numLocalPoints            Number of vertices in vPoints array.
	//! \param height                    Height of the shape.
	virtual void SetPoints(Vec3 const* const pPoints, bool const* const pSoundObstructionSegments, size_t const numLocalPoints, bool const bClosed, float const height) = 0;

	//! Sets area to be a Box, min and max must be in local entity space.
	//! Host entity orientation will define the actual world position and orientation of area box.
	virtual void SetBox(const Vec3& min, const Vec3& max, const bool* const pabSoundObstructionSides, size_t const nSideCount) = 0;

	//! Sets area to be a Sphere, center and radius must be specified in local entity space.
	//! Host entity world position will define the actual world position of the area sphere.
	virtual void SetSphere(const Vec3& vCenter, float fRadius) = 0;

	//! This function need to be called before setting convex hulls for a AreaSolid.
	//! Then AddConvexHullSolid() function is called as the number of convexhulls consisting of a geometry.
	//! \see AddConvexHullToSolid, EndSettingSolid
	virtual void BeginSettingSolid(const Matrix34& worldTM) = 0;

	//! Add a convex hull to a solid. This function have to be called after calling BeginSettingSolid()
	//! \see BeginSettingSolid, EndSettingSolid
	virtual void AddConvexHullToSolid(const Vec3* verticesOfConvexHull, bool bObstruction, i32 numberOfVertices) = 0;

	//! Finish setting a solid geometry. Generally the BSPTree based on convex hulls which is set before is created in this function.
	//!\see BeginSettingSolid, AddConvexHullToSolid
	virtual void EndSettingSolid() = 0;

	//! Retrieve number of points for shape area, return 0 if not area type is not shape.
	virtual i32 GetPointsCount() = 0;

	//! Retrieve array of points for shape area, will return NULL for all other area types.
	virtual const Vec3* GetPoints() = 0;

	//! Set shape area height, if height is 0 area is 2D.
	virtual void SetHeight(float const value) = 0;

	//! Retrieve shape area height, if height is 0 area is 2D.
	virtual float GetHeight() const = 0;

	//! Retrieve min and max in local space of area box.
	virtual void GetBox(Vec3& min, Vec3& max) = 0;

	//! Retrieve center and radius of the sphere area in local space.
	virtual void GetSphere(Vec3& vCenter, float& fRadius) = 0;

	virtual void SetGravityVolume(const Vec3* pPoints, i32 nNumPoints, float fRadius, float fGravity, bool bDontDisableInvisible, float fFalloff, float fDamping) = 0;

	//! Set area ID, this id will be provided to the script callback OnEnterArea, OnLeaveArea.
	virtual void SetID(i32k id) = 0;

	//! Retrieve area ID.
	virtual i32 GetID() const = 0;

	//! Set area group id, areas with same group id act as an exclusive areas.
	//! If 2 areas with same group id overlap, entity will be considered in the most internal area (closest to entity).
	virtual void SetGroup(i32k id) = 0;

	//! Retrieve area group id.
	virtual i32 GetGroup() const = 0;

	//! Set priority defines the individual priority of an area,
	//! Area with same group id will depend on which has the higher priority
	virtual void SetPriority(i32k nPriority) = 0;

	//! Retrieve area priority.
	virtual i32 GetPriority() const = 0;

	//! Sets sound obstruction depending on area type
	virtual void SetSoundObstructionOnAreaFace(size_t const index, bool const bObstructs) = 0;

	//! Add target entity to the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void AddEntity(EntityId id) = 0;

	//! Add target entity to the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void AddEntity(EntityGUID guid) = 0;

	//! Remove target entity from the area.
	//! When someone enters/leaves an area, it will send ENTERAREA, LEAVEAREA, AREAFADE, events to these target entities.
	virtual void RemoveEntity(EntityId const id) = 0;

	//! Remove target entity from the area.
	//! When someone enters/leaves an area, it will send ENTERAREA ,LEAVEAREA, AREAFADE, events to these target entities.
	virtual void RemoveEntity(EntityGUID const guid) = 0;

	//! Removes all added target entities.
	virtual void RemoveEntities() = 0;

	//! Set area proximity region near the border.
	//! When someone is moving within this proximity region from the area outside border
	//! Area will generate ENTITY_EVENT_AREAFADE event to the target entity, with a fade ratio from 0, to 1.
	//! Where 0 will be at the area outside border, and 1 inside the area in distance fProximity from the outside area border.
	virtual void SetProximity(float fProximity) = 0;

	//! Retrieves area proximity.
	virtual float GetProximity() = 0;

	//! Compute and return squared distance to a point which is outside
	//! OnHull3d is the closest point on the hull of the area
	virtual float CalcPointNearDistSq(EntityId const nEntityID, Vec3 const& Point3d, Vec3& OnHull3d) = 0;

	//! Computes and returns squared distance from a point to the hull of the area
	//! OnHull3d is the closest point on the hull of the area
	//! This function is not sensitive of if the point is inside or outside the area
	virtual float ClosestPointOnHullDistSq(EntityId const nEntityID, Vec3 const& Point3d, Vec3& OnHull3d) = 0;

	//! Checks if a given point is inside the area.
	//! \note Ignoring the height speeds up the check.
	virtual bool CalcPointWithin(EntityId const nEntityID, Vec3 const& Point3d, bool const bIgnoreHeight = false) const = 0;

	//! get number of entities in area
	virtual size_t GetNumberOfEntitiesInArea() const = 0;

	//! get entity in area by index
	virtual EntityId GetEntityInAreaByIdx(size_t const index) const = 0;

	//! Retrieve inner fade distance of this area.
	virtual float GetInnerFadeDistance() const = 0;

	//! Set this area's inner fade distance.
	virtual void SetInnerFadeDistance(float const distance) = 0;
};

struct IClipVolumeComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IClipVolumeComponent, "92bc520e-aaa2-b3f0-9095-087aee67d9ff"_drx_guid)

	virtual void         SetGeometryFilename(tukk sFilename) = 0;
	virtual void         UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) = 0;
	virtual IClipVolume* GetClipVolume() const = 0;
	virtual IBSPTree3D*  GetBspTree() const = 0;
	virtual void         SetProperties(bool bIgnoresOutdoorAO, u8 viewDistRatio) = 0;
};

//! Flow Graph component allows entity to host reference to the flow graph.
struct IEntityFlowGraphComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityFlowGraphComponent, "17e5eba7-57e4-4662-a1c2-1f41de946cda"_drx_guid)

	virtual void        SetFlowGraph(IFlowGraph* pFlowGraph) = 0;
	virtual IFlowGraph* GetFlowGraph() = 0;
};

//! Substitution component remembers IRenderNode this entity substitutes and unhides it upon deletion
struct IEntitySubstitutionComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntitySubstitutionComponent, "429b0bce-2947-49d9-a458-df6faee6830c"_drx_guid)

	virtual void         SetSubstitute(IRenderNode* pSubstitute) = 0;
	virtual IRenderNode* GetSubstitute() = 0;
};

//! Represents entity camera.
struct IEntityCameraComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityCameraComponent, "9da92df2-37d7-4d2f-b64f-b827fcecfdd3"_drx_guid)

	virtual void     SetCamera(CCamera& cam) = 0;
	virtual CCamera& GetCamera() = 0;
};

//! Interface to the rope component, providing support for creating a rendered and physical rope on the entity position
//! \par Example
//! \include DinrusXEntitySys/Examples/RopeComponent.cpp
struct IEntityRopeComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityRopeComponent, "368e5dcd-0d95-4101-b1f9-da514945f40c"_drx_guid)

	virtual struct IRopeRenderNode* GetRopeRenderNode() = 0;
};

namespace DRS
{
struct IResponseActor;
struct IVariableCollection;
typedef std::shared_ptr<IVariableCollection> IVariableCollectionSharedPtr;
}

//! Component for dynamic response system actors.
struct IEntityDynamicResponseComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityDynamicResponseComponent, "67994647-83dd-41b8-a098-e26b4b2c95fd"_drx_guid)

	virtual void                      ReInit(tukk szName, tukk szGlobalVariableCollectionToUse) = 0;
	virtual DRS::IResponseActor*      GetResponseActor() const = 0;
	virtual DRS::IVariableCollection* GetLocalVariableCollection() const = 0;
};

//! Component interface for GeomEntity to work in the CreateObject panel
struct IGeometryEntityComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IGeometryEntityComponent, "54b2c130-8e27-4e07-8bf0-f1fe89228d14"_drx_guid)

	virtual void SetGeometry(tukk szFilePath) = 0;
};

//! Component interface for ParticleEntity to work in the CreateObject panel
struct IParticleEntityComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IParticleEntityComponent, "68e3655d-ddd3-4390-aad5-448264e74461"_drx_guid)

	virtual void SetParticleEffectName(tukk szEffectName) = 0;
};
