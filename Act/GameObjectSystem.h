// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GAMEOBJECTSYSTEM_H__
#define __GAMEOBJECTSYSTEM_H__

#pragma once

// FIXME: Cell SDK GCC bug workaround.
#ifndef __IGAMEOBJECTSYSTEM_H__
	#include "IGameObjectSystem.h"
#endif

#include "GameObjectDispatch.h"
#include <vector>
#include <map>

class CGameObjectSystem : public IGameObjectSystem
{
public:
	bool                                               Init();

	IGameObjectSystem::ExtensionID                     GetID(tukk name) override;
	tukk                                        GetName(IGameObjectSystem::ExtensionID id) override;
	u32                                             GetExtensionSerializationPriority(IGameObjectSystem::ExtensionID id) override;
	IGameObjectExtension*                              Instantiate(IGameObjectSystem::ExtensionID id, IGameObject* pObject) override;
	virtual void                                       RegisterExtension(tukk szName, IGameObjectExtensionCreatorBase* pCreator, IEntityClassRegistry::SEntityClassDesc* pClsDesc) override;
	virtual void                                       RegisterSchedulingProfile(tukk szEntityClassName, tukk szNormalPolicy, tukk szOwnedPolicy) override;
	virtual void                                       DefineProtocol(bool server, IProtocolBuilder* pBuilder) override;
	virtual void                                       BroadcastEvent(const SGameObjectEvent& evt) override;

	virtual void                                       RegisterEvent(u32 id, tukk name) override;
	virtual u32                                     GetEventID(tukk name) override;
	virtual tukk                                GetEventName(u32 id) override;

	virtual IGameObject*                               CreateGameObjectForEntity(EntityId entityId) override;
	virtual IEntityComponent*                          CreateGameObjectEntityProxy(IEntity& entity, IGameObject** pGameObject = NULL) override;

	virtual void                                       PostUpdate(float frameTime) override;
	virtual void                                       SetPostUpdate(IGameObject* pGameObject, bool enable) override;

	virtual void                                       Reset() override;

	const SEntitySchedulingProfiles*                   GetEntitySchedulerProfiles(IEntity* pEnt);

	void                                               RegisterFactories(IGameFramework* pFW);

	IEntity*                                           CreatePlayerProximityTrigger();
	ILINE IEntityClass*                                GetPlayerProximityTriggerClass() { return m_pClassPlayerProximityTrigger; }
	ILINE std::vector<IGameObjectSystem::ExtensionID>* GetActivatedExtensionsTop()      { return &m_activatedExtensions_top; }

	void                                               GetMemoryUsage(IDrxSizer* s) const;

	virtual void                                       AddSink(IGameObjectSystemSink* pSink) override;
	virtual void                                       RemoveSink(IGameObjectSystemSink* pSink) override;
private:
	void                                               LoadSerializationOrderFile();
	
	std::map<string, ExtensionID> m_nameToID;

	struct SExtensionInfo
	{
		string                           name;
		u32                           serializationPriority; // lower values is higher priority
		IGameObjectExtensionCreatorBase* pFactory;

		void                             GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
		}
	};
	std::vector<SExtensionInfo> m_extensionInfo;

	static IEntityComponent* CreateGameObjectWithPreactivatedExtension(
	  IEntity* pEntity, SEntitySpawnParams& params, uk pUserData);

	CGameObjectDispatch       m_dispatch;

	std::vector<IGameObject*> m_postUpdateObjects;
	bool                      m_isPostUpdating = false;

	IEntityClass*             m_pClassPlayerProximityTrigger;

	typedef std::map<string, SEntitySchedulingProfiles> TSchedulingProfiles;
	TSchedulingProfiles       m_schedulingParams;
	SEntitySchedulingProfiles m_defaultProfiles;

	// event ID management
	std::map<string, u32> m_eventNameToID;
	std::map<u32, string> m_eventIDToName;
	//
	typedef std::list<IGameObjectSystemSink*> SinkList;
	SinkList                                    m_lstSinks; // registered sinks get callbacks

	std::vector<IGameObjectSystem::ExtensionID> m_activatedExtensions_top;
	std::vector<string>                         m_serializationOrderList; // defines serialization order for extensions
};

#endif
