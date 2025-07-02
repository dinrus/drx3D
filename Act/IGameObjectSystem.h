// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IGAMEOBJECTSYSTEM_H__
#define __IGAMEOBJECTSYSTEM_H__

#pragma once

#include <drx3D/Entity/IEntityClass.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Network/INetwork.h>

enum EGameObjectEventFlags
{
	eGOEF_ToScriptSystem     = 0x0001,
	eGOEF_ToGameObject       = 0x0002,
	eGOEF_ToExtensions       = 0x0004,
	eGOEF_LoggedPhysicsEvent = 0x0008, // was this a logged or immediate physics event

	eGOEF_ToAll              = eGOEF_ToScriptSystem | eGOEF_ToGameObject | eGOEF_ToExtensions
};

struct IGameObject;
struct SGameObjectEvent;

struct IGameObjectBoxListener
{
	virtual ~IGameObjectBoxListener(){}
	virtual void OnEnter(i32 id, EntityId entity) = 0;
	virtual void OnLeave(i32 id, EntityId entity) = 0;
	virtual void OnRemoveParent() = 0;
};

// Описание:
//		A callback interface for a class that wants to be aware when new game objects are being spawned. A class that implements
//		this interface will be called every time a new game object is spawned.
struct IGameObjectSystemSink
{
	// This callback is called after this game object is initialized.
	virtual void OnAfterInit(IGameObject* object) = 0;
	virtual ~IGameObjectSystemSink(){}
};

struct IGameObjectSystem
{
	virtual ~IGameObjectSystem(){}
	// If this is set as the user data for a GameObject with Preactivated Extension
	// spawn, then it will be called back to provide additional initialization.
	struct SEntitySpawnParamsForGameObjectWithPreactivatedExtension
	{
		// If the user wants to extend this spawn parameters using this as a base class,
		// make sure to override 'm_type' member with your own typeID starting at 'eSpawnParamsType_Custom'
		enum EType
		{
			eSpawnParamsType_Default = 0,
			eSpawnParamsType_Custom,
		};

		bool  (* hookFunction)(IEntity* pEntity, IGameObject*, uk pUserData);
		uk pUserData;

		SEntitySpawnParamsForGameObjectWithPreactivatedExtension()
			: m_type(eSpawnParamsType_Default)
			, pUserData(nullptr)
			, hookFunction(nullptr)
		{
		}

		u32 IsOfType(u32k type) const { return (m_type == type); };

	protected:
		u32 m_type;
	};

	typedef u16                  ExtensionID;
	static const ExtensionID InvalidExtensionID = ~ExtensionID(0);
	typedef IGameObjectExtension*(* GameObjectExtensionFactory)();

	virtual IGameObjectSystem::ExtensionID GetID(tukk name) = 0;
	virtual tukk                    GetName(IGameObjectSystem::ExtensionID id) = 0;
	virtual u32                         GetExtensionSerializationPriority(IGameObjectSystem::ExtensionID id) = 0;
	virtual IGameObjectExtension*          Instantiate(IGameObjectSystem::ExtensionID id, IGameObject* pObject) = 0;
	virtual void                           BroadcastEvent(const SGameObjectEvent& evt) = 0;

	static u32k                    InvalidEventID = ~u32(0);
	virtual void              RegisterEvent(u32 id, tukk name) = 0;
	virtual u32            GetEventID(tukk name) = 0;
	virtual tukk       GetEventName(u32 id) = 0;

	virtual IGameObject*      CreateGameObjectForEntity(EntityId entityId) = 0;
	virtual IEntityComponent* CreateGameObjectEntityProxy(IEntity& entity, IGameObject** ppGameObject = NULL) = 0;

	virtual void            RegisterExtension(tukk szName, IGameObjectExtensionCreatorBase* pCreator, IEntityClassRegistry::SEntityClassDesc* pEntityCls) = 0;
	virtual void            RegisterSchedulingProfile(tukk szEntityClassName, tukk szNormalPolicy, tukk szOwnedPolicy) = 0;
	virtual void            DefineProtocol(bool server, IProtocolBuilder* pBuilder) = 0;

	virtual void              PostUpdate(float frameTime) = 0;
	virtual void              SetPostUpdate(IGameObject* pGameObject, bool enable) = 0;

	virtual void              Reset() = 0;

	virtual void              AddSink(IGameObjectSystemSink* pSink) = 0;
	virtual void              RemoveSink(IGameObjectSystemSink* pSink) = 0;
};

// Summary
//   Structure used to define a game object event
struct SGameObjectEvent
{
	SGameObjectEvent(u32 event, u16 flags, IGameObjectSystem::ExtensionID target = IGameObjectSystem::InvalidExtensionID, uk _param = 0)
	{
		this->event = event;
		this->target = target;
		this->flags = flags;
		this->ptr = 0;
		this->param = _param;
	}
	u32                         event;
	IGameObjectSystem::ExtensionID target;
	u16                         flags;
	uk                          ptr;
	// optional parameter of event (ugly)
	union
	{
		uk param;
		bool  paramAsBool;
	};
};

#endif
