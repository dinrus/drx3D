// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SmokeUpr.cpp
//  Version:     v1.00
//  Created:     26/05/2010 by Richard Semmens
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/SmokeUpr.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Math/Drx_GeoDistance.h>
#include <drx3D/Game/Audio/AudioSignalPlayer.h>

#include <drx3D/Act/IVehicleSystem.h>

#include <drx3D/Game/Effects/SceneBlurGameEffect.h>

#ifndef _RELEASE
#include <drx3D/Game/GameCVars.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#endif

const float CSmokeUpr::kInitialDelay				= 4.0f;
const float CSmokeUpr::kMaxPhysicsSleepTime = 0.5f;
const float CSmokeUpr::kMaxSmokeRadius			= 4.0f;
const float CSmokeUpr::kSmokeLingerTime			= 1.0f;
const float CSmokeUpr::kSmokeEmitEndTime		= 16.0f;
const float CSmokeUpr::kSmokeRadiusIncrease	= 3.0f;
u32k CSmokeUpr::kMaxSmokeEffectsInSameArea = 1;
const float CSmokeUpr::kBlurStrength = 2.0f;
const float CSmokeUpr::kBlurBrightness = 0.6f;
const float CSmokeUpr::kBlurContrast = 0.0f;
const float CSmokeUpr::kClientReduceBlurDelta = 1.0f;

CSmokeUpr * CSmokeUpr::GetSmokeUpr()
{
	static CSmokeUpr s_smokeUpr;
	return &s_smokeUpr;
}

CSmokeUpr::CSmokeUpr()
	: m_pExplosionParticleEffect(nullptr)
	, m_pInsideSmokeParticleEffect(nullptr)
	, m_pOutsideSmokeParticleEffect(nullptr)
	, m_pInsideSmokeEmitter(nullptr)
	, m_numActiveSmokeInstances(0)
	, m_clientBlurAmount(0.0f)
	, m_clientInSmoke(false)
	, m_loadedParticleEffects(false)
{
	std::memset(PRFETCH_PADDING, 0, sizeof(PRFETCH_PADDING));
	Init();
}

CSmokeUpr::~CSmokeUpr()
{
	ReleaseObstructionObjects();
}

void CSmokeUpr::Reset()
{
	ReleaseObstructionObjects();
	Init();
}

void CSmokeUpr::ReleaseObstructionObjects()
{
	i32k kInitialNumActiveSmokeInstances = m_numActiveSmokeInstances;
	for(i32 i = 0; i < kInitialNumActiveSmokeInstances; i++)
	{
		SSmokeInstance& smokeInstance = m_smokeInstances[i];

		smokeInstance.RemoveObstructionObject();
	}

	ReleaseParticleEffects();
}

void CSmokeUpr::LoadParticleEffects()
{
	if(!m_loadedParticleEffects)
	{
		m_pExplosionParticleEffect = gEnv->pParticleUpr->FindEffect("smoke_and_fire.occlusion_smoke.explosion");
		if(m_pExplosionParticleEffect)
		{
			m_pExplosionParticleEffect->AddRef();
		}
		else
		{
			DrxLog("###### SmokeUpr ExplosionParticleEffect not found");
		}

		m_pInsideSmokeParticleEffect = gEnv->pParticleUpr->FindEffect("smoke_and_fire.occlusion_smoke.within");
		if(m_pInsideSmokeParticleEffect)
		{
			m_pInsideSmokeParticleEffect->AddRef();
		}
		else
		{
			DrxLog("###### SmokeUpr InsideSmokeParticleEffect not found");
		}

		m_pOutsideSmokeParticleEffect = gEnv->pParticleUpr->FindEffect("smoke_and_fire.occlusion_smoke.distant");
		if(m_pOutsideSmokeParticleEffect)
		{
			m_pOutsideSmokeParticleEffect->AddRef();
		}
		else
		{
			DrxLog("###### SmokeUpr OutsideSmokeParticleEffect not found");
		}

		m_loadedParticleEffects = true;
	}
}

void CSmokeUpr::ReleaseParticleEffects()
{
	if(m_pInsideSmokeEmitter)
	{
		gEnv->pParticleUpr->DeleteEmitter(m_pInsideSmokeEmitter);
		SAFE_RELEASE(m_pInsideSmokeEmitter);
	}

	SAFE_RELEASE(m_pExplosionParticleEffect);
	SAFE_RELEASE(m_pInsideSmokeParticleEffect);
	SAFE_RELEASE(m_pOutsideSmokeParticleEffect);

	m_loadedParticleEffects = false;
}

void CSmokeUpr::Init()
{
	memset(m_smokeInstances, 0, sizeof(SSmokeInstance) * MAX_SMOKE_INSTANCES);
	
	m_numActiveSmokeInstances = 0;

	SetSmokeSoundmood(false);

	LoadParticleEffects();
}

void CSmokeUpr::Update(float dt)
{
	PrefetchLine(m_smokeInstances, 0);
	PrefetchLine(m_smokeInstances, 128);

	i32k kInitialNumActiveSmokeInstances = m_numActiveSmokeInstances;

	//Update all smoke instances
	
	for(i32 i = 0; i < kInitialNumActiveSmokeInstances; i++)
	{
		SSmokeInstance& smokeInstance = m_smokeInstances[i];

		PrefetchLine(&smokeInstance, 128);

		UpdateSmokeInstance(smokeInstance, dt);
	}

#ifndef _RELEASE
	DrawSmokeDebugSpheres();
#endif

	//Remove any smoke instances marked for deletion

	i32 iNumActiveSmokeInstances = kInitialNumActiveSmokeInstances;
	for(i32 i = 0; i < iNumActiveSmokeInstances; i++)
	{
		SSmokeInstance& smokeInstance = m_smokeInstances[i];

		//If this item is marked for deletion, copy the last element of the array
		//	over it
		if(smokeInstance.state == eSIS_ForDeletion)
		{
			smokeInstance.RemoveObstructionObject();

			i32k iLastIndex = iNumActiveSmokeInstances - 1;
			m_smokeInstances[i] = m_smokeInstances[iLastIndex];
			m_smokeInstances[iLastIndex].state = eSIS_Unassigned;
			m_smokeInstances[iLastIndex].pObstructObject = NULL;
			
			iNumActiveSmokeInstances--;
			i--;
		}
	}

	m_numActiveSmokeInstances = iNumActiveSmokeInstances;

	//Update smoke soundmood and screen effects for client
	float insideSmokeFactor = 0.0f;
	const bool clientInsideSmoke = ClientInsideSmoke(insideSmokeFactor);

	SetSmokeSoundmood(clientInsideSmoke);
	SetBlurredVision(insideSmokeFactor, dt);

	if(clientInsideSmoke)
	{
		if(m_pInsideSmokeEmitter == NULL)
		{
			// Start smoke
			Matrix34 spawnLocation;
			spawnLocation.SetIdentity();
			if (m_pInsideSmokeParticleEffect)
			{
				m_pInsideSmokeEmitter = m_pInsideSmokeParticleEffect->Spawn(false,spawnLocation);

				if(m_pInsideSmokeEmitter)
				{
					m_pInsideSmokeEmitter->AddRef();

					EntityId localActorId = g_pGame->GetIGameFramework()->GetClientActorId();
					IEntity* pLocalActorEntity = gEnv->pEntitySystem->GetEntity(localActorId);
					if(pLocalActorEntity)
					{
						m_pInsideSmokeEmitter->SetEntity(pLocalActorEntity,0);
					}
				}
			}
		}
		if(m_pInsideSmokeEmitter && !m_pInsideSmokeEmitter->IsAlive())
		{			
			m_pInsideSmokeEmitter->Activate(true);
		}
	}
	else
	{
		if(m_pInsideSmokeEmitter && m_pInsideSmokeEmitter->IsAlive())
		{
			m_pInsideSmokeEmitter->SetEntity(NULL,0);
			m_pInsideSmokeEmitter->Activate(false);
		}
	}
}

void CSmokeUpr::CullOtherSmokeEffectsInProximityWhenGrenadeHasLanded(SSmokeInstance& smokeInstance, IEntity* pGrenade)
{
	if(pGrenade && (smokeInstance.state == eSIS_Active_PhysicsAwake))
	{
		IPhysicalEntity* pPhysEnt = pGrenade->GetPhysics();
		if(pPhysEnt)
		{
			pe_status_awake psa;
			if(pPhysEnt->GetStatus(&psa) == false)
			{
				// Was awake, now asleep...
				SSmokeInstance* pOldestSmokeInstanceToDelete = NULL;
				i32k kNumActiveSmokeInstances = m_numActiveSmokeInstances;
				u32 otherSmokeInstanceProximityCount = 0;
				SSmokeInstance* pOtherSmokeInstance = NULL;

				PrefetchLine(m_smokeInstances, 0);
				PrefetchLine(m_smokeInstances, 128);

				// Find oldest smoke grenade effect in proximity
				for(i32 i=0; i<kNumActiveSmokeInstances; i++)
				{
					pOtherSmokeInstance = &m_smokeInstances[i];

					if(pOtherSmokeInstance != &smokeInstance)
					{
						PrefetchLine(pOtherSmokeInstance, 128);

						const float fDistanceSq = Distance::Point_PointSq(smokeInstance.vPositon, pOtherSmokeInstance->vPositon);

						const float fCombinedRadiusSq = sqr(smokeInstance.fMaxRadius) + sqr(pOtherSmokeInstance->fMaxRadius);
						if(fDistanceSq < fCombinedRadiusSq)
						{
							otherSmokeInstanceProximityCount++;
							if(pOldestSmokeInstanceToDelete == NULL || (pOldestSmokeInstanceToDelete->fTimer < pOtherSmokeInstance->fTimer))
							{
								pOldestSmokeInstanceToDelete = pOtherSmokeInstance;
							}
						}
					}
				}

				// Delete old smoke effect in proximity if there are too many
				if(pOldestSmokeInstanceToDelete && (otherSmokeInstanceProximityCount >= kMaxSmokeEffectsInSameArea))
				{
					IEntity * pGrenadeToDeleteParticleEmitterFor = gEnv->pEntitySystem->GetEntity(pOldestSmokeInstanceToDelete->grenadeId);
					if(pGrenadeToDeleteParticleEmitterFor)
					{
						i32 slotCount = pGrenadeToDeleteParticleEmitterFor->GetSlotCount();
						for(i32 i=0; i<slotCount; i++)
						{
							IParticleEmitter* pParticleEmitter = pGrenadeToDeleteParticleEmitterFor->GetParticleEmitter(i);
							if(pParticleEmitter)
							{
								pParticleEmitter->Activate(false);
							}
						}
					}
					pOldestSmokeInstanceToDelete->state = eSIS_ForDeletion;
				}

				smokeInstance.state = eSIS_Active_PhysicsAsleep;
			}
		}
	}
}

void CSmokeUpr::UpdateSmokeInstance(SSmokeInstance& smokeInstance, float dt)
{
	IEntity * pGrenade = gEnv->pEntitySystem->GetEntity(smokeInstance.grenadeId);

	CullOtherSmokeEffectsInProximityWhenGrenadeHasLanded(smokeInstance,pGrenade);

	if(pGrenade)
	{
		//Update the radius of the smoke according to the speed of movement,
		//	it should reduce to nothing when moving fast enough, and should increase
		//	when the grenade is stationary
		Vec3 vNewPosition		= pGrenade->GetPos();

		Vec3 vPositionDiff	= vNewPosition - smokeInstance.vPositon;

		const float fDistanceTravelled = vPositionDiff.len();

		const float fSpeed = fDistanceTravelled * __fres(dt);

		const float fNewRadius = smokeInstance.fCurrentRadius + (float)__fsel( smokeInstance.fTimer-kInitialDelay, ((kSmokeRadiusIncrease - fSpeed) * dt), 0.0f);
		
		smokeInstance.fCurrentRadius = clamp_tpl(fNewRadius, 0.0f, kMaxSmokeRadius);

		// If grenade on ground, then override its timer to be maximum of kMaxPhysicsSleepTime
		if((smokeInstance.state == eSIS_Active_PhysicsAsleep) && (smokeInstance.fTimer < kInitialDelay-kMaxPhysicsSleepTime))
		{
			smokeInstance.fTimer = kInitialDelay-kMaxPhysicsSleepTime;
		}

		const float previousTimer = smokeInstance.fTimer;
		smokeInstance.fTimer = smokeInstance.fTimer + dt;

		if(previousTimer < kInitialDelay && smokeInstance.fTimer >= kInitialDelay)
		{
			// Spawn explosion particle effect
			if (m_pExplosionParticleEffect)
			{
				m_pExplosionParticleEffect->Spawn(false,IParticleEffect::ParticleLoc(vNewPosition));
			}

			// Spawn smoke particle effect
			// Find a free slot on the grenade
			SEntitySlotInfo dummySlotInfo;
			i32 i=0;
			while (pGrenade->GetSlotInfo(i, dummySlotInfo))
			{
				i++;
			}
			if (m_pOutsideSmokeParticleEffect)
			{
				pGrenade->LoadParticleEmitter(i, m_pOutsideSmokeParticleEffect);
			}
		}

		//Update the smoke's position
		smokeInstance.vPositon = vNewPosition;

		if (smokeInstance.pObstructObject)
		{
			pe_params_pos pos;
			pos.scale = clamp_tpl(smokeInstance.fCurrentRadius * __fres(kMaxSmokeRadius), 0.1f, 1.0f);
			pos.pos = vNewPosition;
			smokeInstance.pObstructObject->SetParams(&pos);
		}
	}
	else
	{
		//The grenade entity has been deleted, so smoke has stopped being produced
		if(smokeInstance.fTimer > kSmokeEmitEndTime)
		{
			//The smoke has cleared, delete the SmokeInstance
			smokeInstance.state = eSIS_ForDeletion;
		}
		else
		{
			//Count down the remaining life, and reduce the radius
			smokeInstance.fTimer					+= dt;
			smokeInstance.fCurrentRadius  = max(smokeInstance.fCurrentRadius - (dt * kMaxSmokeRadius / kSmokeLingerTime), 0.0f);

			if (smokeInstance.pObstructObject)
			{
				pe_params_pos pos;
				pos.scale = clamp_tpl(smokeInstance.fCurrentRadius * __fres(kMaxSmokeRadius), 0.1f, 1.0f);

				smokeInstance.pObstructObject->SetParams(&pos);
			}
		}
	}
}

void CSmokeUpr::CreateNewSmokeInstance(EntityId grenadeId, EntityId grenadeOwnerId, float fMaxRadius)
{
	IEntity * pGrenade = gEnv->pEntitySystem->GetEntity(grenadeId);

	i32 iNewSmokeInstanceIndex = m_numActiveSmokeInstances;

	//If all of the smoke instances are used up, get the index of the next one to be deleted
	//	and re-use that instance
	if(iNewSmokeInstanceIndex >= MAX_SMOKE_INSTANCES)
	{
		float fLeastLifeLeft = kSmokeLingerTime;

		iNewSmokeInstanceIndex = 0;

		for(i32 i = 0; i < iNewSmokeInstanceIndex; i++)
		{
			SSmokeInstance& smokeInstance = m_smokeInstances[i];
			if(smokeInstance.fTimer < fLeastLifeLeft)
			{
				iNewSmokeInstanceIndex = i;
				fLeastLifeLeft=smokeInstance.fTimer;
			}
		}
	}
	else
	{
		m_numActiveSmokeInstances++;
	}

	//Fill out the smoke instance with the new data
	SSmokeInstance& newSmokeInstance = m_smokeInstances[iNewSmokeInstanceIndex];
 	newSmokeInstance.state					= eSIS_Active_PhysicsAwake;
 	newSmokeInstance.vPositon				= pGrenade->GetPos();
	newSmokeInstance.grenadeId			= grenadeId;
 	newSmokeInstance.fMaxRadius			= fMaxRadius;
 	newSmokeInstance.fCurrentRadius = 0.1f;
 	newSmokeInstance.fTimer					= 0.0f;

	if (!gEnv->bMultiplayer)
	{
		CreateSmokeObstructionObject(newSmokeInstance);

		if (gEnv->pAISystem)
		{
			// Associate event with vehicle if the shooter is in a vehicle (tank cannon shot, etc)
			EntityId ownerId = grenadeId;
			IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId);
			IVehicle* pVehicle = pActor ? pActor->GetLinkedVehicle() : NULL;
			if (pVehicle)
			{
				ownerId = pVehicle->GetEntityId();
			}

			SAIStimulus stim(AISTIM_GRENADE, AIGRENADE_SMOKE, ownerId, grenadeId, newSmokeInstance.vPositon, ZERO, kMaxSmokeRadius*1.5f);
			gEnv->pAISystem->RegisterStimulus(stim);
		}
	}
}

bool CSmokeUpr::CanSeePointThroughSmoke(const Vec3& vTarget, const Vec3& vSource) const
{
	i32k kNumActiveSmokeInstances = m_numActiveSmokeInstances;

	PrefetchLine(m_smokeInstances, 0);
	PrefetchLine(m_smokeInstances, 128);

	Lineseg lineseg(vSource, vTarget);
	
	for(i32 i = 0; i < kNumActiveSmokeInstances; i++)
	{
		const SSmokeInstance& smokeInstance = m_smokeInstances[i];
		
		PrefetchLine(&smokeInstance, 128);
		
		float fTValue;

		float fDistanceSq = Distance::Point_LinesegSq(smokeInstance.vPositon, lineseg, fTValue);

		if(fDistanceSq < sqr(smokeInstance.fCurrentRadius))
		{
			return false;
		}
	}

	return true;
}

bool CSmokeUpr::CanSeeEntityThroughSmoke(const EntityId entityId) const
{
	Vec3 clientPos;
	GetClientPos(clientPos);
	return CanSeeEntityThroughSmoke(entityId, clientPos);
}

bool CSmokeUpr::CanSeeEntityThroughSmoke(const EntityId entityId, const Vec3& vSource) const
{
	IEntity * pEntity = gEnv->pEntitySystem->GetEntity(entityId);

	if(pEntity)
	{

		Vec3 vTargetPosn = pEntity->GetPos();
		vTargetPosn.z += 1.5f;	//Approximate head height

		return CanSeePointThroughSmoke(vTargetPosn, vSource);

	}

	return false;
}

void CSmokeUpr::GetClientPos(Vec3& pos) const
{
	EntityId localActorId = g_pGame->GetIGameFramework()->GetClientActorId();

	IEntity * pEntity = gEnv->pEntitySystem->GetEntity(localActorId);

	pos.Set(0.f, 0.f, 0.f);

	if(pEntity)
	{
		pos = pEntity->GetPos();
		pos.z += 1.5f;	//Approximate head height
	}
}

bool CSmokeUpr::ClientInsideSmoke(float& outInsideFactor) const
{
	Vec3 clientPos;
	GetClientPos(clientPos);
	return IsPointInSmoke(clientPos, outInsideFactor);
}

bool CSmokeUpr::IsPointInSmoke(const Vec3& vPos, float& outInsideFactor) const
{
	i32k kNumActiveSmokeInstances = m_numActiveSmokeInstances;

	PrefetchLine(m_smokeInstances, 0);
	PrefetchLine(m_smokeInstances, 128);

	outInsideFactor = 0.0f;
	
	for(i32 i = 0; i < kNumActiveSmokeInstances; i++)
	{
		const SSmokeInstance& smokeInstance = m_smokeInstances[i];

		PrefetchLine(&smokeInstance, 128);

		const float fDistanceSq = Distance::Point_PointSq(vPos, smokeInstance.vPositon);
		const float fCurrentRadiusSq = sqr(smokeInstance.fCurrentRadius);
		if(fDistanceSq < fCurrentRadiusSq)
		{
			outInsideFactor = 1.0f - ((float)sqrt_fast_tpl(fDistanceSq) * (float)__fres(sqrt_fast_tpl(fCurrentRadiusSq)));
			return true;
		}
	}

	return false;
}

void CSmokeUpr::SetSmokeSoundmood(const bool enable)
{
	const bool clientInSmoke = enable;
	if(clientInSmoke != m_clientInSmoke)
	{
		CAudioSignalPlayer::JustPlay(clientInSmoke ? "SmokeEnter" : "SmokeLeave");
		m_clientInSmoke = enable;
	}
}

void CSmokeUpr::SetBlurredVision( const float blurAmmount, const float frameTime )
{
	float newBlurAmount = clamp_tpl(blurAmmount * kBlurStrength, 0.0f, 1.0f);

	if (m_clientBlurAmount == newBlurAmount)
		return;

	if(newBlurAmount > m_clientBlurAmount)
	{
		m_clientBlurAmount = newBlurAmount;
	}
	else
	{
		m_clientBlurAmount = LERP(m_clientBlurAmount, newBlurAmount, frameTime * kClientReduceBlurDelta);
	}

	CSceneBlurGameEffect* pSceneBlurGameEffect = g_pGame->GetSceneBlurGameEffect();
	DRX_ASSERT(pSceneBlurGameEffect != NULL);
	pSceneBlurGameEffect->SetBlurAmount(m_clientBlurAmount, CSceneBlurGameEffect::eGameEffectUsage_SmokeUpr);

	gEnv->p3DEngine->SetPostEffectParam("Global_User_Brightness", LERP(1.0f,kBlurBrightness,m_clientBlurAmount));
	gEnv->p3DEngine->SetPostEffectParam("Global_User_Contrast", LERP(1.0f,kBlurContrast,m_clientBlurAmount));
}

void CSmokeUpr::CreateSmokeObstructionObject( SSmokeInstance& smokeInstance )
{
	smokeInstance.RemoveObstructionObject();

	pe_params_pos pos;
	pos.scale = 0.1f;
	pos.pos = smokeInstance.vPositon;

	smokeInstance.pObstructObject = gEnv->pPhysicalWorld->CreatePhysicalEntity(PE_STATIC, &pos);

	if (smokeInstance.pObstructObject)
	{
		primitives::sphere sphere;
		sphere.center = Vec3(0,0,0);
		sphere.r = kMaxSmokeRadius;
		i32 obstructID = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceTypeIdByName("mat_obstruct");
		IGeometry *pGeom = gEnv->pPhysicalWorld->GetGeomUpr()->CreatePrimitive(primitives::sphere::type, &sphere);
		phys_geometry *geometry = gEnv->pPhysicalWorld->GetGeomUpr()->RegisterGeometry(pGeom, obstructID);
		pGeom->Release();
		pe_geomparams params;
		params.flags = geom_colltype14;
		geometry->nRefCount = 0; // automatically delete geometry
		smokeInstance.pObstructObject->AddGeometry(geometry, &params);
	}
}

#ifndef _RELEASE
void CSmokeUpr::DrawSmokeDebugSpheres()
{
	if(g_pGameCVars->g_debugSmokeGrenades)
	{
		SAuxGeomRenderFlags oldFlags = gEnv->pRenderer->GetIRenderAuxGeom()->GetRenderFlags();
		SAuxGeomRenderFlags newFlags = oldFlags;
		newFlags.SetAlphaBlendMode(e_AlphaBlended);

		gEnv->pRenderer->GetIRenderAuxGeom()->SetRenderFlags(newFlags);

		i32k kNumActiveSmokeInstances = m_numActiveSmokeInstances;

		ColorB smokeDebugColor(255, 0, 0, 70);

		for(i32 i = 0; i < kNumActiveSmokeInstances; i++)
		{
			//DRAW SPHERE
			SSmokeInstance& smokeInstance = m_smokeInstances[i];
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(smokeInstance.vPositon, smokeInstance.fCurrentRadius, smokeDebugColor);
		}

		gEnv->pRenderer->GetIRenderAuxGeom()->SetRenderFlags(oldFlags);

		// Draw blur amount to screen
		ColorF textCol(0.0f,1.0f,0.0f,1.0f);
		gEnv->pRenderer->Draw2dLabel(50.0f,20.0f,1.4f,&textCol.r,false,"Client Blur Amount: %f",m_clientBlurAmount);
	}
}
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SSmokeInstance::RemoveObstructionObject()
{
	if (pObstructObject)
	{
		gEnv->pPhysicalWorld->DestroyPhysicalEntity(pObstructObject);
		pObstructObject = NULL;
	}
}