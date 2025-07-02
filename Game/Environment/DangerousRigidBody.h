// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DANGEROUS_RIGID_BODY_H__
#define __DANGEROUS_RIGID_BODY_H__

#include <drx3D/Act/IGameObject.h>

class CDangerousRigidBody : public CGameObjectExtensionHelper<CDangerousRigidBody, IGameObjectExtension, 1>
{
public:
	static const NetworkAspectType ASPECT_DAMAGE_STATUS	= eEA_GameServerC;
	
	static i32 sDangerousRigidBodyHitTypeId;

	CDangerousRigidBody();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId);
	virtual void PostInit(IGameObject *pGameObject) {}
	virtual void PostInitClient(i32 channelId) {}
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext &ctx, i32 updateSlot) {};
	virtual void PostUpdate(float frameTime) {}
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent(const SGameObjectEvent& event) {};
	virtual void ProcessEvent(SEntityEvent& event);
	virtual void SetChannelId(u16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {};
	virtual bool GetEntityPoolSignature( TSerialize signature );
	// IGameObjectExtension

	void SetIsDangerous(bool isDangerous, EntityId triggerPlayerId);

private:
	void Reset();

	float	m_damageDealt;
	float m_lastHitTime;
	float m_timeBetweenHits;
	bool m_dangerous;
	bool m_friendlyFireEnabled;
	u8 m_activatorTeam;
};

#endif //__DANGEROUS_RIGID_BODY_H__
