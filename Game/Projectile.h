// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Act/IGameObject.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Game/TracerUpr.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/AmmoParams.h>
#include <drx3D/Game/PlayerPlugin.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/GameTypeInfo.h>

#include <drx3D/Game/EntityUtility/EntityEffects.h>

#include <drx3D/Game/AI/HazardModule/HazardShared.h>

class CPlayer;
struct SEntityProximityQuery;

namespace Projectile
{
	float GetPointBlankMultiplierAtRange(float fRange, float fPointBlankDistance, float fPointBlankFalloffDistance, float fPointBlankMultiplier);
};

typedef std::vector<EntityId> TActorIds;

class CProjectile :
	public CGameObjectExtensionHelper<CProjectile, IGameObjectExtension>,
	public IHitListener,	public IGameObjectProfileUpr
{
public:
	DRX_DECLARE_GTI_BASE(CProjectile);

	enum ProjectileTimer
	{
		ePTIMER_LIFETIME		= 0x110,
		ePTIMER_SHOWTIME		= 0x111,
		ePTIMER_STICKY			= 0x112,
		ePTIMER_ACTIVATION	= 0x113,
		ePTIMER_BEACONDELAY = 0x114,
	};

	enum ProjectileFlags
	{
		ePFlag_none								= 0,
		ePFlag_collided						= BIT(0),
		ePFlag_hitListener				= BIT(1),
		ePFlag_remote							= BIT(2),
		ePFlag_destroying					= BIT(3),
		ePFlag_noBulletHits				= BIT(4),
		ePFlag_knocksTarget				= BIT(5),
		ePFlag_launched						= BIT(6),
		ePFlag_threatTrailEmitted	= BIT(7),
		ePFlag_delayedDetonation	= BIT(8),
		ePFlag_firedViaProxy			= BIT(9),
		ePFlag_needDestruction		= BIT(10),
		ePFlag_ownerIsPlayer			= BIT(11),
		ePFlag_linked							= BIT(12),
		ePFlag_hitRecorded				= BIT(13),
		ePFlag_failedDetonation		= BIT(14),
		ePFlag_aimedShot					= BIT(15),
		ePFlag_dontNetSerialisePhysics = BIT(16),
		ePFlag_hitListener_mp_OnExplosion_only = BIT(17),
		ePFlag_electricHit				= BIT(18),
	};

	struct SProjectileDesc
	{
		SProjectileDesc(EntityId _ownerId, EntityId _hostId, EntityId _weaponId, i32 _damage, float _damageFallOffStart, float _damageFallOffAmount, float _damageFallOffMin, i32 _hitTypeId, int8 _bulletPierceabilityModifier, bool _aimedShot);

		EntityId ownerId;
		EntityId hostId;
		EntityId weaponId;
		i32 damage;
		float damageFallOffStart;
		float damageFallOffAmount;
		float damageFallOffMin;
		float pointBlankAmount;
		float pointBlankDistance;
		float pointBlankFalloffDistance;
		i32 hitTypeId;
		int8 bulletPierceabilityModifier;
		bool aimedShot;
	};

	struct SExplodeDesc
	{
		SExplodeDesc(bool _destroy);

		Vec3 pos;
		Vec3 normal;
		Vec3 vel;
		EntityId targetId;
		tukk overrideEffectClassName;
		bool destroy;
		bool impact;
	};

	struct SElectricHitTarget
	{
		SElectricHitTarget(IPhysicalEntity* pProjectilePhysics, EventPhysCollision* pCollision);

		EntityId entity;
		Vec3 hitPosition;
		Vec3 hitDirection;
		Vec3 hitNormal;
		i32 partId;
		i32 matId;
	};

	struct SMaterialLookUp
	{
		enum EType
		{
			eType_Water = 0,
			eType_Count
		};

		SMaterialLookUp()
			: m_initialized(false)
		{
			memset(&m_lookUp[0], -1, sizeof(i32) * eType_Count);
		}

		void Init();

		ILINE bool IsMaterial(i32 materialId, EType type)
		{
			DRX_ASSERT(type >= 0 && type < eType_Count);
			return (m_lookUp[type] == materialId);
		}

	private:
		i32  m_lookUp[eType_Count];
		bool m_initialized;
	};

	static const NetworkAspectType ASPECT_DETONATION	= eEA_GameServerDynamic;

	CProjectile();
	virtual ~CProjectile();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId) {};
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(i32 channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
	virtual NetworkAspectType GetNetSerializeAspects();
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize ser );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext &ctx, i32 updateSlot);
	virtual void PostUpdate(float frameTime ) {};
	virtual void PostRemoteSpawn();
	virtual void HandleEvent( const SGameObjectEvent &);
	virtual void ProcessEvent(SEntityEvent &);
	virtual void SetChannelId(u16 id) {};
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	virtual i32  GetMemorySize() { return sizeof(*this); };
	//~IGameObjectExtension

	// IGameObjectProfileUpr
	virtual bool SetAspectProfile( EEntityAspects aspect, u8 profile );
	virtual u8 GetDefaultProfile( EEntityAspects aspect );
	// ~IGameObjectProfileUpr

	//IHitListener
	virtual void OnHit(const HitInfo&);
	virtual void OnExplosion(const ExplosionInfo&);
	virtual void OnServerExplosion(const ExplosionInfo&);
	//~IHitListener

	virtual void ReInitFromPool();
	virtual void SetParams(const SProjectileDesc& projectileDesc);
  virtual void SetDestination(const Vec3& pos){}
  virtual void SetDestination(EntityId targetId){}
	virtual void Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale=1.0f);
	virtual bool IsAlive() const;
	virtual void Deflected(const Vec3& dir) {};
	virtual bool Detonate();
	virtual bool CanDetonate() { return true; }
	virtual void Explode(const SExplodeDesc& explodeDesc);

	virtual void SetDamageCap(float cap) {};
	virtual void UpdateLinkedDamage(EntityId hitActorId, float totalAccumDamage) {};

	void Physicalize();
	void SetVelocity(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale=1.0f, Vec3* appliedVelocityOut=NULL, i32 bThreadSafe=0);
	void Destroy();
	bool IsRemote() const;
	void SetLifeTime(float lifeTime);
	void SetRemote(bool remote);
	void LoadGeometry();
	void SetFiredViaProxy(bool proxy);
	void SetKnocksTargetInfo(const SFireModeParams* pParams);
	virtual void CreateBulletTrail(const Vec3& hitPos);

	ILINE void BindToTracer(int8 tracerIdx) { m_boundToTracerIdx = tracerIdx; }
	ILINE i32  GetTracerIdx() const { return m_boundToTracerIdx; }
	ILINE void BindToThreatTrailTracer(int8 tracerIdx) { m_threatTrailTracerIdx = tracerIdx; }
	ILINE i32  GetThreatTrailTracerIdx() const { return m_threatTrailTracerIdx; }

	CWeapon *GetWeapon() const;
	ILINE EntityId	GetWeaponId() const { return m_weaponId; }
	ILINE EntityId	GetOwnerId()const { return m_ownerId; }
	ILINE float			GetSpeed() const { return m_pAmmoParams->speed; }
	ILINE bool			IsLinked() const { return CheckAllProjectileFlags(ePFlag_linked); }
	ILINE void			SetHitReported() { SetProjectileFlags(ePFlag_hitRecorded); }

	void TrailSound(bool enable, const Vec3 &dir=Vec3(0.0f,1.0f,0.0f));
	void UpdateWhiz(const Vec3& projectilePos, bool destroy);
	void WhizSound(const Vec3 &pos);
	void TrailEffect(bool enable);
	void FlashbangEffect(const SFlashbangParams* flashbang);
	void Ricochet(EventPhysCollision *pCollision);
	void RicochetSound(const Vec3 &pos);
	void InitWithAI();
	void ResolveTarget(EventPhysCollision* pCollision, i32& targetId, i32& sourceId, IEntity*& pTargetEntity) const;

	IParticleEffect* GetCachedEffect(tukk effectName) const;

	inline float GetLifeTime() const { return m_pAmmoParams? m_pAmmoParams->lifetime : 0.0f; }
	bool IsPredicted() const { return m_pAmmoParams? m_pAmmoParams->predictSpawn != 0 : false; }

	//Helper function to initialize particle params in exceptional cases
	void SetDefaultParticleParams(pe_params_particle *pParams);

	const SAmmoParams& GetAmmoParams() const {return *m_pAmmoParams;}

	inline void SetProjectileFlags(u32 flags, bool on)		{ m_projectileFlags = on ? (m_projectileFlags | flags) : (m_projectileFlags & ~flags); }
	inline void SetProjectileFlags(u32 flags)						{ m_projectileFlags |= flags; }
	inline void ClearProjectileFlags(u32 flags)					{ m_projectileFlags &= ~flags; }
	inline bool CheckAnyProjectileFlags(u32 flags) const { return (m_projectileFlags & flags) ? true : false; }
	inline bool CheckAllProjectileFlags(u32 flags) const	{ return (m_projectileFlags & flags) == flags; }

	bool HasDetonationBeenDelayed() const;
	void SetDetonationHasBeenDelayed(bool delayed);
	bool CheckForDelayedDetonation(Vec3 pos);

	bool HasFailedDetonation() const;
	void SetFailedDetonation(bool failed);

	const Vec3& GetInitialPos() const { return m_initial_pos; }
	const Vec3& GetLastPos()		const { return m_last; }

	// Determine if logic connected to collision event should be executed
	bool ProcessCollisionEvent(IEntity *pTarget) const;

	const CTimeValue& GetSpawnTime() const { return m_spawnTime; }

	bool ShouldHaveExploded() const { return m_bShouldHaveExploded; }

	void RegisterLinkedProjectile(u8 shotIndex);

	bool IsGrenade() const;

	i32 GetHitType() const {return m_hitTypeId;}
	i32 GetDamage() const {return m_damage;}
	void MultiplyDamage(float multipier) { m_damage = (i32)((float)m_damage * multipier); }
	virtual EntityId GetStuckToEntityId() const { return 0; }

	static SMaterialLookUp&  GetMaterialLookUp() { return s_materialLookup; }

	void SetAmmoCost(i32 cost) { m_ammoCost = cost; }
	i32 GetAmmoCost() const { return m_ammoCost; }

protected:
	struct SInfo : public ISerializableInfo
	{
		EntityId hostId;
		EntityId ownerId;
		EntityId weaponId;
		Vec3 pos;
		Vec3 dir;
		Vec3 vel;
		i32 bulletPierceMod;
		void SerializeWith( TSerialize ser );
	};

	void DestroyObstructObject();

	void OnLaunch() const;
	void GetActorsInArea(TActorIds& actorIds, float range, const Vec3& center);

	virtual bool ShouldCollisionsDamageTarget() const;
	bool CanCollisionsDamageTarget(IEntity *pTarget) const;
	void DoCollisionDamage(EventPhysCollision* pCollision, IEntity *pTarget);

	void ProcessFailedDetonation();

	virtual void SetUpParticleParams(IEntity* pOwnerEntity, u8 pierceabilityModifier);

	//IEntityAudioProxy *GetAudioProxy();

	void DestroyImmediate();

	void ReportHit(EntityId targetId);

	ILINE bool RequiresDelayedDestruct() const  { return (m_pAmmoParams && (m_pAmmoParams->mpProjectileDestructDelay > 0.f)); }

	bool ProximityDetector(float proxyRadius);

	bool ProximityDetector_MP(float proxyRadius);
	bool ProximityDetector_SP(float proxyRadius);

	void ProcessElectricHit(const SElectricHitTarget& target);


	void FillOutProjectileSpawnInfo(SInfo* pSpawnInfo) const;

	CTimeValue m_spawnTime;
	const SAmmoParams			*m_pAmmoParams;
	IPhysicalEntity *m_pPhysicalEntity;

	EntityEffects::CEffectsController	m_projectileEffects;

	u32		m_projectileFlags;

	Vec3			m_last;
	Vec3			m_initial_pos;
	Vec3			m_initial_dir;
	Vec3			m_initial_vel;

	float			m_minDamageForKnockDown;
	float			m_minDamageForKnockDownLeg;
	float			m_totalLifetime;
	float			m_scaledEffectval;
	float			m_mpDestructionDelay;

	EntityEffects::TAttachedEffectId	m_trailEffectId;
	AudioControlId m_whizTriggerID;
	AudioControlId m_ricochetTriggerID;
	//i32				m_trailSoundId;
	i32				m_damage;
	i32				m_hitTypeId;
	i32				m_hitPoints;
	i32				m_chanceToKnockDownLeg; // 0..100
	i32				m_ammoCost;

	EntityId	m_ownerId;
	EntityId	m_hostId;
	EntityId	m_weaponId;

	IPhysicalEntity *m_obstructObject;
	int8			m_bullet_pierceability_modifier;
	u8			m_currentPhysProfile;
	int8			m_boundToTracerIdx;  //Tracer manager supports only 96 traces at the moment, int8 is enough
	int8			m_threatTrailTracerIdx;

	bool			m_trailSoundEnable;
	bool			m_bShouldHaveExploded;
	bool			m_exploded;

	static SMaterialLookUp s_materialLookup;

private:
	// Hazard notification:
	const SHazardAmmoParams* GetHazardParams() const;
	bool            ShouldGenerateHazardArea() const;
	void            RetrieveHazardAreaPoseInFrontOfProjectile(const Vec3& estimatedForwardNormal, Vec3* hazardStartPos, Vec3* hazardForwardNormal) const;
	void            RegisterHazardArea();
	void            SyncHazardArea(const Vec3& estimatedForwardNormal);
	void            UnregisterHazardArea();

	// The hazard ID (undefined if none has been registered).
	HazardSystem::HazardProjectileID m_HazardID;
};
