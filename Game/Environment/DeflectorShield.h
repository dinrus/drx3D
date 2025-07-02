// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DEFLECTOR_SHIELD_H__
#define __DEFLECTOR_SHIELD_H__

#if _MSC_VER > 1000
# pragma once
#endif



class CProjectile;


class CDeflectorShield : public CGameObjectExtensionHelper<CDeflectorShield, IGameObjectExtension>
{
private:
	struct SDeflectedEnergy
	{
		// Note: we need to store the deflection position and direction 
		// in the local-space of the collider so that things don't get 
		// messed up when the collider has been moved during the deflection
		// delay (otherwise it might actually spawn a deflected projectile
		// inside the collider again, for example).

		// The impact position in the local-space of the shield collider.
		Vec3 m_localPosition;

		// The deflection normal direction in the local-space of the shield collider.
		Vec3 m_localDirection;

		float m_delay;
		i32 m_damage;
	};
	typedef std::deque<SDeflectedEnergy> TDeflectedEnergies;

public:
	CDeflectorShield();

	// IGameObjectExtension
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	virtual bool Init(IGameObject * pGameObject);
	virtual void PostInit(IGameObject * pGameObject);
	virtual void InitClient(i32 channelId);
	virtual void PostInitClient(i32 channelId);
	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params);
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params);
	virtual bool GetEntityPoolSignature(TSerialize signature);
	virtual void Release();
	virtual void FullSerialize(TSerialize ser);
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags);
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo(TSerialize ser);
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update(SEntityUpdateContext& ctx, i32 updateSlot);
	virtual void HandleEvent(const SGameObjectEvent& event);
	virtual void ProcessEvent(SEntityEvent& event);	
	virtual void SetChannelId(u16 id);
	virtual void SetAuthority(bool auth );
	virtual ukk  GetRMIBase() const;
	virtual void PostUpdate(float frameTime);
	virtual void PostRemoteSpawn();
	// ~IGameObjectExtension

	// Deflection control:
	void SetNonDeflectedOwnerGroup(i32 pGroupID);
	void SetPhysicsCollisionResponse(bool normalCollResonseFlag);

	// Recharge/depletion control:	
	bool IsDepleted() const;
	void Recharged();
	void Depleted();
	void SetInvulnerability(const bool invulnerableFlag);


private:
	void LoadScriptProperties();
	void PreCacheAmmoResources();

	void ProcessCollision(const EventPhysCollision& pCollision);
	void ProcessProjectile(CProjectile* pProjectile, Vec3 hitPosition, Vec3 hitNormal, Vec3 hitDirection);
	void UpdateDeflectedEnergies(float deltaTime);
	void ValidateUpdateSlot();
	void ShootDeflectedEnergy(const SDeflectedEnergy& energy);
	void PurgeDeflectedEnergiesBuffer();

	// Deflection control:
	bool CanProjectilePassThroughShield(const CProjectile* pProjectile) const;

	// Life-time:
	bool IsDead() const;

	void DebugDraw();

	TDeflectedEnergies m_deflectedEnergies;
	IEntityClass* m_pAmmoClass;
	IParticleEffect* m_pDeflectedEffect;
	i32 m_minDamage;
	i32 m_maxDamage;
	float m_dropMinDistance;
	float m_dropPerMeter;
	float m_spread;
	i32 m_hitTypeId;

	float m_energyRadius;
	float m_invEnergyDelay;

	// Any entity (or its owner) that is a member of this group will not be 
	// deflected (0 if none).
	i32 m_NonDeflectedOwnersGroupID;
};


#endif