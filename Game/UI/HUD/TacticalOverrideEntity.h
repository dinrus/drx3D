// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: HUD Tactical override entity
  
 -------------------------------------------------------------------------
  История:
  - 13:12:2012: Created by Dean Claassen

*************************************************************************/

#ifndef __HUD_TACTICALOVERRIDE_ENTITY_H__
#define __HUD_TACTICALOVERRIDE_ENTITY_H__

// CTacticalOverrideEntity

class CTacticalOverrideEntity : public CGameObjectExtensionHelper<CTacticalOverrideEntity, IGameObjectExtension>
{
public:
	CTacticalOverrideEntity();
	virtual ~CTacticalOverrideEntity();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId);
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(i32 channelId) {}
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext &ctx, i32 updateSlot) {}
	virtual void PostUpdate(float frameTime) {}
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent(const SGameObjectEvent& details) {}
	virtual void ProcessEvent(SEntityEvent& details);
	virtual void SetChannelId(u16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	//~IGameObjectExtension

protected:
	bool m_bMappedToParent;
};

#endif // __HUD_TACTICALOVERRIDE_ENTITY_H__

