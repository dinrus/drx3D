// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Grenades

-------------------------------------------------------------------------
История:
- 11:12:2009  10:30 : Created by Claire Allan

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Grenade.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/AI/IAIObject.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/SmokeUpr.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/GameRules.h>

//------------------------------------------------------------------------
CGrenade::CGrenade() : m_detonationFailed(false)
{
}

//------------------------------------------------------------------------
CGrenade::~CGrenade()
{
}

//------------------------------------------------------------------------
void CGrenade::HandleEvent(const SGameObjectEvent &event)
{
	if (CheckAnyProjectileFlags(ePFlag_destroying | ePFlag_needDestruction) || m_detonationFailed)
		return;

	if (event.event == eGFE_OnCollision)
	{
		EventPhysCollision *pCollision = (EventPhysCollision *)event.ptr;

		float bouncy, friction;
		u32 pierceabilityMat;
		gEnv->pPhysicalWorld->GetSurfaceParameters(pCollision->idmat[1], bouncy, friction, pierceabilityMat);
		pierceabilityMat &= sf_pierceable_mask;

		u32k pierceabilityProj = GetAmmoParams().pParticleParams ? GetAmmoParams().pParticleParams->iPierceability : 13;
		if ((pierceabilityMat > pierceabilityProj))
			return;

		i32 srcId = 0;
		i32 trgId = 1;
		IEntity* pTargetEntity = NULL;

		// Notify AI system about grenades.
		if (gEnv->pAISystem)
		{
			IAIObject* pAI = 0;
			if ((pAI = GetEntity()->GetAI()) != NULL && pAI->GetAIType() == AIOBJECT_GRENADE)
			{
				// Associate event with vehicle if the shooter is in a vehicle (tank cannon shot, etc)
				EntityId ownerId = m_ownerId;
				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId);
				if (pActor && pActor->GetLinkedVehicle() && pActor->GetLinkedVehicle()->GetEntityId())
					ownerId = pActor->GetLinkedVehicle()->GetEntityId();

				SAIStimulus stim(AISTIM_GRENADE, AIGRENADE_COLLISION, ownerId, GetEntityId(),
					GetEntity()->GetWorldPos(), ZERO, 12.0f);
				gEnv->pAISystem->RegisterStimulus(stim);
			}
		}

		if (m_pAmmoParams && m_pAmmoParams->pGrenadeParams)
		{
			if(!CheckAnyProjectileFlags(ePFlag_collided))
			{
				if(m_pAmmoParams->pGrenadeParams->onImpactLifetime > 0.f)
				{
					SetLifeTime(m_pAmmoParams->pGrenadeParams->onImpactLifetime);
				}
			}

			if(m_pAmmoParams->pGrenadeParams->detonateOnImpact || m_pAmmoParams->pGrenadeParams->detonateOnActorImpact)
			{
				if (m_pAmmoParams->safeExplosion > 0.0f && (m_initial_pos - GetEntity()->GetWorldPos()).len2() < sqr(m_pAmmoParams->safeExplosion))
				{
					m_detonationFailed = true;
					TrailEffect(false);
					TrailSound(false);
				}
				else
				{
					ResolveTarget(pCollision, trgId, srcId, pTargetEntity);

					ISurfaceType* pSurfaceType = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceType(pCollision->idmat[trgId]);
					IPhysicalEntity* pTarget = pCollision->pEntity[trgId];
					bool isParticle = (pTarget->GetType() == PE_PARTICLE);

					if (pSurfaceType && !isParticle)
					{
						if (pSurfaceType->GetBreakability()<=0)
						{
							if (m_pAmmoParams->pGrenadeParams->detonateOnImpact || (pTargetEntity && g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pTargetEntity->GetId())))
							{
								CProjectile::SExplodeDesc explodeDesc(true);
								explodeDesc.impact = true;
								explodeDesc.pos = pCollision->pt;
								explodeDesc.normal = pCollision->n;
								explodeDesc.vel = pCollision->vloc[srcId];
								explodeDesc.targetId = pTargetEntity ? pTargetEntity->GetId() : 0;
								Explode(explodeDesc);
							}
						}
					}
				}				
			}
		}

		//nearby players
		if(m_pAmmoParams && m_pAmmoParams->pExplosion && m_hitTypeId == CGameRules::EHitType::Frag)
		{
			if(m_ownerId == g_pGame->GetIGameFramework()->GetClientActorId())
			{
				g_pGame->GetPersistantStats()->UpdateClientGrenadeBounce(pCollision->pt, m_pAmmoParams->pExplosion->maxRadius);
			}
		}
	}

	BaseClass::HandleEvent(event);
}

//------------------------------------------------------------------------
void CGrenade::Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale /*=1.0f*/)
{
	BaseClass::Launch(pos, dir, velocity, speedScale);

	OnLaunch();

	m_detonationFailed = false;

	if(!gEnv->bMultiplayer)
	{
		IAIObject* pAI = 0;
		if ((pAI = GetEntity()->GetAI()) != NULL && pAI->GetAIType() == AIOBJECT_GRENADE)
		{
			IEntity *pOwnerEntity = gEnv->pEntitySystem->GetEntity(m_ownerId);
			pe_status_dynamics dyn;
			pe_status_dynamics dynProj;
			if (pOwnerEntity->GetPhysics() 
				&& pOwnerEntity->GetPhysics()->GetStatus(&dyn) 
				&& GetEntity()->GetPhysics()->GetStatus(&dynProj) && gEnv->pAISystem)
			{

				Vec3 ownerVel( dyn.v);
				Vec3 grenadeDir(dynProj.v.GetNormalizedSafe());

				// Trigger the signal at the predicted landing position.
				Vec3 predictedPos = pos;
				float dummySpeed;
				if (GetWeapon())
				GetWeapon()->PredictProjectileHit(pOwnerEntity->GetPhysics(), pos, dir, velocity, speedScale * m_pAmmoParams->speed, predictedPos, dummySpeed);
				
				// Associate event with vehicle if the shooter is in a vehicle (tank cannon shot, etc)
				EntityId ownerId = pOwnerEntity->GetId();
				IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId);
				if (pActor && pActor->GetLinkedVehicle() && pActor->GetLinkedVehicle()->GetEntityId())
					ownerId = pActor->GetLinkedVehicle()->GetEntityId();

				SAIStimulus stim(AISTIM_GRENADE, AIGRENADE_THROWN, ownerId, GetEntityId(),
														predictedPos, ZERO, 20.0f);
				gEnv->pAISystem->RegisterStimulus(stim);
			}
		}
	}
}

//------------------------------------------------------------------------
void CGrenade::Explode(const CProjectile::SExplodeDesc& explodeDesc)
{
	if(gEnv->bServer)
	{
		if((m_pAmmoParams->pGrenadeParams == NULL) || !m_pAmmoParams->pGrenadeParams->allowDetonationDelay || !CheckForDelayedDetonation(explodeDesc.pos))
		{
#if ENABLE_HUD_EXTRA_DEBUG
			if(g_pGame->GetUI()->GetCVars()->hud_threat_stopGrenadesExploding)
			{ 
				return; 
			}
#endif // ENABLE_HUD_EXTRA_DEBUG

			if(m_pAmmoParams->armTime < m_totalLifetime)
			{
				BaseClass::Explode(explodeDesc);
			}
			else
			{
				m_detonationFailed = true;
				ProcessFailedDetonation();
			}
		}
	}
	else
	{
		m_bShouldHaveExploded = true;
	}
}

//------------------------------------------------------------------------
bool CGrenade::ShouldCollisionsDamageTarget() const
{
	return true;
}

//------------------------------------------------------------------------
void CGrenade::ProcessEvent(SEntityEvent &event)
{
	if (event.event == ENTITY_EVENT_TIMER && event.nParam[0] == ePTIMER_LIFETIME)
	{
		if (m_detonationFailed)
		{
			Destroy();
			return;
		}
	}

	CProjectile::ProcessEvent(event);
}

//------------------------------------------------------------------------
void CSmokeGrenade::Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale /*=1.0f*/)
{
	inherited::Launch(pos, dir, velocity, speedScale);

	CSmokeUpr::GetSmokeUpr()->CreateNewSmokeInstance(GetEntityId(), m_ownerId, g_pGameCVars->g_smokeGrenadeRadius);
}
