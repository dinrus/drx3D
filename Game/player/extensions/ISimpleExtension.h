// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <drx3D/Act/IGameObject.h>

struct ISimpleExtension : public IGameObjectExtension
{
	//IGameObjectExtension
	virtual bool                   Init(IGameObject* pGameObject) override                                                  { SetGameObject(pGameObject); return true; }
	virtual void                   PostInit(IGameObject* pGameObject) override                                              {}
	virtual void                   HandleEvent(const SGameObjectEvent& event) override                                      {}
	virtual void                   ProcessEvent(SEntityEvent& event) override                                               {}
	virtual void                   InitClient(i32 channelId) override                                                       {}
	virtual void                   PostInitClient(i32 channelId) override                                                   {}
	virtual bool                   ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override     { return true; }
	virtual void                   PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override {}
	virtual bool                   GetEntityPoolSignature(TSerialize signature) override                                    { return false; }
	virtual void                   Release() override                                                                       { delete this; }
	virtual void                   FullSerialize(TSerialize ser) override                                                   {}
	virtual bool                   NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags) override  { return true; }
	virtual void                   PostSerialize() override                                                                 {}
	virtual void                   SerializeSpawnInfo(TSerialize ser) override                                              {}
	virtual ISerializableInfoPtr   GetSpawnInfo() override                                                                  { return nullptr; }
	virtual void                   Update(SEntityUpdateContext& ctx, i32 updateSlot) override                               {}
	virtual void                   SetChannelId(u16 id) override                                                         {}
	virtual void                   SetAuthority(bool auth) override                                                         {}
	virtual void                   PostUpdate(float frameTime) override                                                     {}
	virtual void                   PostRemoteSpawn() override                                                               {}
	virtual void                   GetMemoryUsage(ICrySizer* pSizer) const override                                         {}
	virtual ComponentEventPriority GetEventPriority(i32k eventID) const override                                       { return EEntityEventPriority_GameObject; }
	//~IGameObjectExtension

	ISimpleExtension() {}
	virtual ~ISimpleExtension() {}
};
