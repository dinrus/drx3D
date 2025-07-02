// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef WATER_PUDDLE_H
#define WATER_PUDDLE_H


#include <drx3D/Act/IGameObject.h>
#include <drx3D/Game/ActorUpr.h>



class CWaterPuddle;



class CWaterPuddleUpr
{
private:
	struct SWaterPuddle
	{
		CWaterPuddle* m_pPuddle;
		EntityId m_entityId;
	};

public:
	void AddWaterPuddle(CWaterPuddle* pPuddle);
	void RemoveWaterPuddle(CWaterPuddle* pPuddle);
	void Reset();

	CWaterPuddle* FindWaterPuddle(Vec3 point);

private:
	std::vector<SWaterPuddle> m_waterPuddles;
};



class CWaterPuddle : public CGameObjectExtensionHelper<CWaterPuddle, IGameObjectExtension>
{
public:
	CWaterPuddle();
	~CWaterPuddle();

	// IGameObjectExtension
	virtual bool Init(IGameObject * pGameObject);
	virtual void InitClient(i32 channelId);
	virtual void PostInit(IGameObject * pGameObject);
	virtual void PostInitClient(i32 channelId);
	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params);
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params);
	virtual bool GetEntityPoolSignature(TSerialize signature);
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo(TSerialize ser);
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update(SEntityUpdateContext& ctx, i32 slot);
	virtual void HandleEvent(const SGameObjectEvent& gameObjectEvent);
	virtual void ProcessEvent(SEntityEvent& entityEvent);
	virtual void SetChannelId(u16 id);
	virtual void SetAuthority(bool auth);
	virtual void PostUpdate(float frameTime);
	virtual void PostRemoteSpawn();
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	// ~IGameObjectExtension

	void ZapEnemiesOnPuddle(i32 ownTeam, EntityId shooterId, EntityId weaponId, float damage, i32 hitTypeId, IParticleEffect* hitEffect);

private:
	void ApplyHit(const SActorData& actorData, EntityId shooterId, EntityId weaponId, float damage, i32 hitTypeId, float waterLevel, IParticleEffect* hitEffect);
};



#endif
