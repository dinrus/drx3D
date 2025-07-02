// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// Описание: Container Entity to be placed in the Editor that gets
// automatically registered with the Entity Container Upr
// - 10/02/2016 Created by Dario Sancho

#pragma once

#include <drx3D/Act/IGameObject.h>

class CEntityContainerObject : public CGameObjectExtensionHelper<CEntityContainerObject, IGameObjectExtension>
{
public:

	CEntityContainerObject() {};
	virtual ~CEntityContainerObject(){}

	// IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject);
	virtual void                 InitClient(i32 channelId)                                                       {};
	virtual void                 PostInit(IGameObject* pGameObject);
	virtual void                 PostInitClient(i32 channelId)                                                   {};
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) {}
	virtual bool                 GetEntityPoolSignature(TSerialize signature);
	virtual void                 Release();
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags) { return true; }
	virtual void                 FullSerialize(TSerialize ser)                                                  {};
	virtual void                 PostSerialize()                                                                {}
	virtual void                 SerializeSpawnInfo(TSerialize ser)                                             {}
	virtual ISerializableInfoPtr GetSpawnInfo()                                                                 { return 0; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32 updateSlot)                              {}
	virtual void                 PostUpdate(float frameTime)                                                    {};
	virtual void                 PostRemoteSpawn()                                                              {};
	virtual void                 HandleEvent(const SGameObjectEvent&)                                           {};
	virtual void                 ProcessEvent(const SEntityEvent&);
	virtual uint64               GetEventMask() const;
	virtual void                 SetChannelId(u16 id)                                                        {};
	virtual void                 SetAuthority(bool auth)                                                        {};
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const                                        { pSizer->Add(*this); }
	//~IGameObjectExtension

private:
	bool ReadEditorProperties();
};
