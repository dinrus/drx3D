// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/LightningBolt.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/RecordingSystem.h>


namespace
{


	i32k maxNumForks = 4;



	class CTarget
	{
	public:
		CTarget();
		CTarget(IPhysicalEntity* pPhysicsTarget, IActor* pActorTarget, Vec3 position, float priority);

		IPhysicalEntity* GetPhysics() const {return m_physics;}
		IActor* GetActor() const {return m_actor;}
		bool IsActor() const {return m_actor != 0;}
		Vec3 GetPosition() const {return m_position;}
		float GetPriority() const {return m_priority;}
		EntityId GetEntityId() const 
		{
			IEntity* pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(m_physics);
			
			return (pEntity != NULL) ? pEntity->GetId() : 0;
		}

	private:
		IPhysicalEntity* m_physics;
		IActor* m_actor;
		Vec3 m_position;
		float m_priority;
	};



	class CTargetPriority
	{
	public:
		CTargetPriority();

		void AddTarget(const CTarget& target);
		i32 GetNumTargets() const {return m_numTargets;}
		const CTarget& GetTarget(i32 index) const {return m_targets[index];}

	private:
		void MoveUp(i32 index);
		i32 InsertIndex(const CTarget& target);
		bool HasTarget(const CTarget& target);

		CTarget m_targets[maxNumForks];
		i32 m_numTargets;
	};



	bool operator < (const CTarget& lhv, const CTarget& rhv)
	{
		if (lhv.IsActor() && !rhv.IsActor())
			return false;
		else if (!lhv.IsActor() && rhv.IsActor())
			return true;
		else
			return lhv.GetPriority() < rhv.GetPriority();
	}



	CTarget::CTarget()
		:	m_physics(0)
		,	m_actor(0)
		,	m_position(ZERO)
		,	m_priority(0.0f)
	{
	}


	CTarget::CTarget(IPhysicalEntity* pPhysicsTarget, IActor* pActorTarget, Vec3 position, float priority)
		:	m_physics(pPhysicsTarget)
		,	m_actor(pActorTarget)
		,	m_position(position)
		,	m_priority(priority)
	{
	}



	CTargetPriority::CTargetPriority()
		:	m_numTargets(0)
	{
	}



	void CTargetPriority::AddTarget(const CTarget& target)
	{
		if (!HasTarget(target))
		{
			i32 index = InsertIndex(target);
			if (index != maxNumForks)
			{
				MoveUp(index);
				assert(index>=0 && index<maxNumForks);
				m_targets[index] = target;
				m_numTargets = min(m_numTargets+1, maxNumForks);
			}
		}
	}



	void CTargetPriority::MoveUp(i32 index)
	{
		for (i32 i = maxNumForks-1; i > index; --i)
			m_targets[i] = m_targets[i-1];
	}



	i32 CTargetPriority::InsertIndex(const CTarget& target)
	{
		i32 index = 0;
		for (; index < m_numTargets; ++index)
		{
			if (m_targets[index] < target)
				break;
		}
		return index;
	}



	bool CTargetPriority::HasTarget(const CTarget& target)
	{
		for (i32 i = 0; i < maxNumForks; ++i)
		{
			if (target.GetPhysics() != 0 && m_targets[i].GetPhysics() == target.GetPhysics())
				return true;
			if (target.GetActor() != 0 && m_targets[i].GetActor() == target.GetActor())
				return true;
		}
		return false;
	}



	float CalculateDamage(const CTarget& target, const Vec3& hitPosition, const SLightningBoltParams& params, float baseDamage)
	{
		if (target.GetPosition().GetDistance(hitPosition) <= params.maxDamageDistanceThreshold)
			return baseDamage;
		return baseDamage * params.spreadDamageMult;
	}



	float CalculateImpulse(const CTarget& target, const Vec3& hitPosition, const SLightningBoltParams& params)
	{
		float distance = target.GetPosition().GetDistance(hitPosition);
		float mult = 1.0f - (distance / params.maxSpreadRadius);
		return params.impulseStrength * mult;
	}

	bool ShouldBoltArc(CWeapon* pWeapon, EntityId targetedEntityId)
	{
		bool bShouldArc = false;

		if (!pWeapon)
			return bShouldArc;

		CActor* pOwnerActor = pWeapon->GetOwnerActor();
		if (!pOwnerActor)
			return bShouldArc;

		if (!gEnv->bMultiplayer && !pOwnerActor->IsPlayer())
			return bShouldArc;

		CActor* pEntityActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(targetedEntityId));
		if (pEntityActor)
			bShouldArc = !pOwnerActor->IsFriendlyEntity(targetedEntityId);

		return bShouldArc;
	}
}


CLightningBolt::CLightningBolt()
	:	m_strikeCountDown(0.0f)
	, m_stuckProjectile(true)
	,	m_lightningId(-1)
{
}



CLightningBolt::~CLightningBolt()
{
	if(gEnv->bMultiplayer)
	{
		m_stuckProjectile.UnStick();
	}
}



void CLightningBolt::Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale)
{
	Parent::Launch(pos, dir, velocity, speedScale);

	if(CWeapon* pWeapon = GetWeapon()) //Late joiners can have the projectile created before the weapon
	{
		const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);

		i32 slot = eIGS_ThirdPerson;
		CActor* pOwnerActor = GetWeapon()->GetOwnerActor();
		if (pOwnerActor && !pOwnerActor->IsThirdPerson())
			slot = eIGS_FirstPerson;

		m_lightningId = g_pGame->GetLightningGameEffect()->TriggerSpark(
			boltParams.lightning.c_str(),
			gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial(boltParams.material.c_str()),
			 CLightningGameEffect::STarget(GetWeaponId(), slot, "muzzleflash_effect"),
			 CLightningGameEffect::STarget(GetEntityId()));
	}
}



void CLightningBolt::Update(SEntityUpdateContext &ctx, i32 updateSlot)
{
	Parent::Update(ctx, updateSlot);

	if (updateSlot != 0)
		return;

	if (m_strikeCountDown > 0.f)
	{
		m_strikeCountDown -= ctx.fFrameTime;
		if (m_strikeCountDown <= 0.0f)
		{
			OnStrike();
			Destroy();
		}
	}
	
	UpdateSparkSize();
}



void CLightningBolt::UpdateSparkSize()
{
	const float minDeviation = 0.1f;

	float flyingTime = gEnv->pTimer->GetCurrTime()-GetSpawnTime().GetSeconds();
	float deviationMult = clamp_tpl((flyingTime*2.0f-1.0f), minDeviation, 1.0f);
	g_pGame->GetLightningGameEffect()->SetSparkDeviationMult(m_lightningId, deviationMult);
}



void CLightningBolt::OnStrike()
{
	Strike();
	
	EntityEffects::SEffectAttachParams attachParams(ZERO, -m_stuckProjectile.GetNormal(), 1.0f, true, 1);
	const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);
	m_projectileEffects.AttachParticleEffect(GetCachedEffect(boltParams.strikeEffect.c_str()), attachParams);
	if(!gEnv->bMultiplayer)
	{
		m_stuckProjectile.UnStick();
	}
}

void CLightningBolt::HandleEvent(const SGameObjectEvent& event)
{
	if (event.event == eGFE_OnCollision)
	{
		EventPhysCollision* pCollision = (EventPhysCollision*)event.ptr;
		
		if(!gEnv->bMultiplayer)
		{
			//Multiplayer will carry out this code on the server when hitting a target
			CStickyProjectile::SStickParams stickParams(this, pCollision, CStickyProjectile::eO_AlignToSurface);
			m_stuckProjectile.Stick(stickParams);
		}

		i32k target = 1;
		
		IEntity* pTargetEntity = (pCollision->iForeignData[target]==PHYS_FOREIGN_ID_ENTITY) ? (IEntity*)pCollision->pForeignData[target] : NULL;

		if(pTargetEntity != NULL)
		{
			CGameRules* pGameRules = g_pGame->GetGameRules();
			ProcessHit(
				pGameRules, pTargetEntity->GetId(), float(m_damage),
				pCollision->pt, GetEntity()->GetWorldRotation().GetColumn1(), pCollision->partid[ target ] );
		}

		if(!gEnv->bMultiplayer)
		{
			//Multiplayer will carry out this code on the server when hitting a target
			const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);
			HandleArcing(m_stuckProjectile.GetStuckEntityId(), pCollision->n, boltParams);
		}		
	}

	CProjectile::HandleEvent(event);
}


void CLightningBolt::HandleArcing(EntityId target, const Vec3& collisionNormal, const SLightningBoltParams& boltParams)
{
	if (ShouldBoltArc(GetWeapon(), target))
	{
		m_strikeCountDown = boltParams.strikeTime;
		EntityEffects::SEffectAttachParams attachParams(ZERO, -collisionNormal, 1.0f, true, 1);
		m_projectileEffects.AttachParticleEffect(GetCachedEffect(boltParams.chargeEffect.c_str()), attachParams);
	}
	else
	{
		EntityEffects::SEffectAttachParams attachParams(ZERO, -collisionNormal, 1.0f, true, 1);
		m_projectileEffects.AttachParticleEffect(GetCachedEffect(boltParams.dryEffect.c_str()), attachParams);
	}
}


void CLightningBolt::OnHit(const HitInfo& rHitInfo)
{
	if(gEnv->bServer && gEnv->bMultiplayer && rHitInfo.projectileId==GetEntityId() && !m_stuckProjectile.IsStuck())
	{
		if(IEntity * pEntity = gEnv->pEntitySystem->GetEntity(rHitInfo.targetId))
		{
			CStickyProjectile::SStickParams stickParams(this, rHitInfo, CStickyProjectile::eO_AlignToSurface);
			m_stuckProjectile.Stick(stickParams);

			CHANGED_NETWORK_STATE(this, CStickyProjectile::ASPECT_STICK);

			const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);

			HandleArcing(rHitInfo.targetId, rHitInfo.normal, boltParams);
		}
	}

	Parent::OnHit(rHitInfo);
}


void CLightningBolt::ReInitFromPool()
{
	Parent::ReInitFromPool();
	m_stuckProjectile.UnStick();
	m_strikeCountDown = 0.0f;
}


void CLightningBolt::Strike()
{
	Vec3 position = GetEntity()->GetPos();

	CGameRules* pGameRules = g_pGame->GetGameRules();
	const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);

	float spreadRadius = boltParams.maxSpreadRadius;

	IPhysicalWorld* pPhysicalWorld = gEnv->pPhysicalWorld;
	IPhysicalEntity** pPhysicalEntities = 0;
	i32 objType = gEnv->bMultiplayer ? ent_living : ent_all;
	const Vec3 boxMin = position - Vec3(spreadRadius, spreadRadius, spreadRadius);
	const Vec3 boxMax = position + Vec3(spreadRadius, spreadRadius, spreadRadius);

	//TODO: multiplayer-specific change to use actor manager instead of GetEntitiesInBox for perf reasons
	i32 numEntities = pPhysicalWorld->GetEntitiesInBox(boxMin, boxMax, pPhysicalEntities, objType);

	EntityId shooterTeam = gEnv->bMultiplayer ? pGameRules->GetTeam(GetOwnerId()) : 0;

	CTargetPriority targets;

	for (i32 j = 0; j < numEntities; ++j)
	{
		IPhysicalEntity* pPhysicalEntity = pPhysicalEntities[j];
		IEntity* pTarget = gEnv->pEntitySystem->GetEntityFromPhysics(pPhysicalEntity);
		if (!pTarget || pTarget->GetId() == m_stuckProjectile.GetStuckEntityId())
			continue;
		
		IActor* pTargetActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pTarget->GetId());
		if (pTargetActor && pTargetActor->IsDead())
			continue;
		if (GetOwnerId() == pTarget->GetId())
			continue;
		if(shooterTeam && shooterTeam == pGameRules->GetTeam(pTarget->GetId()) && g_pGameCVars->g_friendlyfireratio == 0.f) //Ignore friendlies in MP
			continue;

		AABB bounds;
		pTarget->GetWorldBounds(bounds);
		Vec3 targetPos = bounds.GetCenter();
		float distanceSq = targetPos.GetSquaredDistance(position);

		if (distanceSq > sqr(boltParams.maxSpreadRadius))
			continue;

		//Avoid div 0
		float distance = max(sqrtf(distanceSq), 0.01f);
		float priority = (1.0f / distance) * bounds.GetRadius();

		targets.AddTarget(CTarget(pPhysicalEntity, pTargetActor, targetPos, priority));
	}

	for (i32 j = 0; j < targets.GetNumTargets(); ++j)
	{
		const CTarget& target = targets.GetTarget(j);

		if(gEnv->bServer)
		{
			ProcessHit(
				pGameRules, target.GetEntityId(),
				CalculateDamage(target, position, boltParams, float(m_damage)),
				target.GetPosition(), (target.GetPosition()-position).GetNormalized(), 0 );

			if (!target.IsActor())
			{
				pe_action_impulse impulse;
				float impulseStrength = CalculateImpulse(target, position, boltParams);
				impulse.impulse = (target.GetPosition() - position).GetNormalized() * impulseStrength;
				impulse.angImpulse = drx_random_componentwise(Vec3(0.5f), Vec3(1.0f)) * impulseStrength;
				target.GetPhysics()->Action(&impulse);
			}
		}

		if(!gEnv->bServer || !gEnv->IsDedicated())
			CreateSpark(position, target.GetPosition(), target.GetEntityId());
	}
}



void CLightningBolt::ProcessHit(CGameRules* gameRules, EntityId target, float damage, Vec3 position, Vec3 direction, i32 partId)
{
	if(damage > 0.f)
	{
		HitInfo hitInfo(m_ownerId, target, m_weaponId,
			damage, 0.0f, 0, partId,
			m_hitTypeId, position, direction, Vec3(ZERO));

		bool bRemote = true;

		if(!gEnv->bMultiplayer)
		{
			bRemote = false;
		}
		else if(m_stuckProjectile.IsStuck() && gEnv->bServer)
		{
			bRemote = false;
		}
		else if(!m_stuckProjectile.IsStuck() && m_ownerId == g_pGame->GetClientActorId())
		{
			bRemote = false;
		}

		hitInfo.remote = bRemote;
		hitInfo.projectileId = GetEntityId();
		hitInfo.hitViaProxy = CheckAnyProjectileFlags(ePFlag_firedViaProxy);
		hitInfo.aimed = CheckAnyProjectileFlags(ePFlag_aimedShot);

		gameRules->ClientHit(hitInfo);

		ReportHit(target);
	}
}



void CLightningBolt::CreateSpark(const Vec3& start, const Vec3& end, EntityId targetId)
{
	const SLightningBoltParams& boltParams = *(m_pAmmoParams->pLightningBoltParams);

	CItemParticleEffectCache& particleCache = g_pGame->GetGameSharedParametersStorage()->GetItemResourceCache().GetParticleEffectCache();
	IParticleEffect* pParticleEffect = particleCache.GetCachedParticle(boltParams.sparkEffect);

	if (pParticleEffect)
	{
		QuatTS location(Quat(ZERO), start);
		IParticleEmitter* pEmiter = pParticleEffect->Spawn(location);

		if (pEmiter)
		{
			ParticleTarget target;
			target.bPriority = true;
			target.bTarget = true;
			target.fRadius = 0.0f;
			target.vTarget = end;
			target.vVelocity = Vec3(ZERO);
			pEmiter->SetTarget(target);

			if(CRecordingSystem *pRecordingSystem = g_pGame->GetRecordingSystem())
			{
				pRecordingSystem->OnParticleEmitterTargetSet(pEmiter, target);
			}
		}
	}

	g_pGame->GetLightningGameEffect()->TriggerSpark(
		boltParams.lightning.c_str(),
		gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial(boltParams.material.c_str()),
		CLightningGameEffect::STarget(GetEntityId()),
		CLightningGameEffect::STarget(targetId));
}



bool CLightningBolt::NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags)
{
	bool bWasStuck = m_stuckProjectile.IsStuckToPlayer();

	m_stuckProjectile.NetSerialize(this, ser, aspect);
	
	if( !bWasStuck && m_stuckProjectile.IsStuckToPlayer() )
	{
		HandleArcing(m_stuckProjectile.GetStuckEntityId(), m_stuckProjectile.GetNormal(), (*m_pAmmoParams->pLightningBoltParams));
	}

	return Parent::NetSerialize(ser, aspect, profile, pflags);
}

NetworkAspectType CLightningBolt::GetNetSerializeAspects()
{
	return Parent::GetNetSerializeAspects() | m_stuckProjectile.GetNetSerializeAspects();
}
