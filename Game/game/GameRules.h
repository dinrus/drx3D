// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <drx3D/Act/IGameRulesSystem.h>

class CGameRules : public CGameObjectExtensionHelper<CGameRules, IGameRules, 1>
{
public:
	CGameRules();
	virtual ~CGameRules();

	//IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject) override;
	virtual void                 PostInit(IGameObject* pGameObject) override                                              {}
	virtual void                 InitClient(i32 channelId) override                                                       {}
	virtual void                 PostInitClient(i32 channelId) override                                                   {}
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override     { return false; }
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override {}
	virtual bool                 GetEntityPoolSignature(TSerialize signature) override                                    { return false; }
	virtual void                 Release() override                                                                       {};
	virtual void                 FullSerialize(TSerialize ser) override                                                   {}
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) override   { return true; }
	virtual void                 PostSerialize() override                                                                 {}
	virtual void                 SerializeSpawnInfo(TSerialize ser) override                                              {}
	virtual ISerializableInfoPtr GetSpawnInfo() override                                                                  { return nullptr; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32 updateSlot) override                               {}
	virtual void                 HandleEvent(const SGameObjectEvent&) override                                            {}
	virtual void                 ProcessEvent(SEntityEvent&) override                                                     {}
	virtual void                 SetChannelId(u16 id) override                                                         {}
	virtual void                 SetAuthority(bool auth) override                                                         {}
	virtual void                 PostUpdate(float frameTime) override                                                     {}
	virtual void                 PostRemoteSpawn() override                                                               {}
	virtual void                 GetMemoryUsage(ICrySizer* s) const override;
	//~IGameObjectExtension

	//IGameRules
	virtual bool        ShouldKeepClient(i32 channelId, EDisconnectionCause cause, tukk desc) const override                                                                                                         { return true; }
	virtual void        PrecacheLevel() override                                                                                                                                                                            {}
	virtual void        PrecacheLevelResource(tukk resourceName, EGameResourceType resourceType) override                                                                                                            {}
	virtual XmlNodeRef  FindPrecachedXmlFile(tukk sFilename) override                                                                                                                                                { return 0; }
	virtual void        OnConnect(struct INetChannel* pNetChannel) override                                                                                                                                                 {}
	virtual void        OnDisconnect(EDisconnectionCause cause, tukk desc) override                                                                                                                                  {}
	virtual bool        OnClientConnect(i32 channelId, bool isReset) override;
	virtual void        OnClientDisconnect(i32 channelId, EDisconnectionCause cause, tukk desc, bool keepClient) override                                                                                            {}
	virtual bool        OnClientEnteredGame(i32 channelId, bool isReset) override                                                                                                                                           { return true; }
	virtual void        OnEntitySpawn(IEntity* pEntity) override                                                                                                                                                            {}
	virtual void        OnEntityRemoved(IEntity* pEntity) override                                                                                                                                                          {}
	virtual void        SendTextMessage(ETextMessageType type, tukk msg, u32 to = eRMI_ToAllClients, i32 channelId = -1, tukk p0 = 0, tukk p1 = 0, tukk p2 = 0, tukk p3 = 0) override {}
	virtual void        SendChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId, tukk msg) override                                                                                              {}
	virtual void        ForbiddenAreaWarning(bool active, i32 timer, EntityId targetId)                                                                                                                                     {}
	virtual float       GetRemainingGameTime() const override                                                                                                                                                               { return 0.0f; }
	virtual void        SetRemainingGameTime(float seconds) override                                                                                                                                                        {}
	virtual void        ClearAllMigratingPlayers(void) override                                                                                                                                                             {}
	virtual EntityId    SetChannelForMigratingPlayer(tukk name, u16 channelID) override                                                                                                                           { return INVALID_ENTITYID; }
	virtual void        StoreMigratingPlayer(IActor* pActor) override                                                                                                                                                       {}
	virtual bool        IsTimeLimited() const override                                                                                                                                                                      { return false; }
	virtual bool        OnCollision(const SGameCollision& event) override                                                                                                                                                   { return false; }
	virtual void        OnEntityReused(IEntity* pEntity, SEntitySpawnParams& params, EntityId prevId) override                                                                                                              {}
	virtual void        ClientHit(const HitInfo& hitInfo) override                                                                                                                                                          {}
	virtual void        ServerHit(const HitInfo& hitInfo) override                                                                                                                                                          {}
	virtual i32         GetHitTypeId(u32k crc) const override                                                                                                                                                       { return 0; }
	virtual i32         GetHitTypeId(tukk type) const override                                                                                                                                                       { return 0; }
	virtual tukk GetHitType(i32 id) const override                                                                                                                                                                   { return nullptr; }
	virtual void        OnVehicleDestroyed(EntityId id) override                                                                                                                                                            {}
	virtual void        OnVehicleSubmerged(EntityId id, float ratio) override                                                                                                                                               {}
	virtual void        CreateEntityRespawnData(EntityId entityId) override                                                                                                                                                 {}
	virtual bool        HasEntityRespawnData(EntityId entityId) const override                                                                                                                                              { return false; }
	virtual void        ScheduleEntityRespawn(EntityId entityId, bool unique, float timer) override                                                                                                                         {}
	virtual void        AbortEntityRespawn(EntityId entityId, bool destroyData) override                                                                                                                                    {}
	virtual void        ScheduleEntityRemoval(EntityId entityId, float timer, bool visibility) override                                                                                                                     {}
	virtual void        AbortEntityRemoval(EntityId entityId) override                                                                                                                                                      {}
	virtual void        AddHitListener(IHitListener* pHitListener) override                                                                                                                                                 {}
	virtual void        RemoveHitListener(IHitListener* pHitListener) override                                                                                                                                              {}
	virtual void        ShowStatus() override                                                                                                                                                                               {}
	virtual void        OnCollision_NotifyAI(const EventPhys* pEvent) override                                                                                                                                              {}
	virtual bool        CanEnterVehicle(EntityId playerId) override                                                                                                                                                         { return true; }
	virtual tukk GetTeamName(i32 teamId) const override                                                                                                                                                              { return nullptr; }
	//~IGameRules
};
