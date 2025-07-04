// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   scriptobjectboids.cpp
//  Version:     v1.00
//  Created:     17/5/2002 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Game/ScriptBind_Boids.h>
#include <drx3D/Game/BoidsProxy.h>
#include <drx3D/Game/Flock.h>
#include <drx3D/Game/BugsFlock.h>
#include <drx3D/Game/ChickenBoids.h>
#include <drx3D/Game/FrogBoids.h>
#include <drx3D/Game/BirdsFlock.h>
#include <drx3D/Game/FishFlock.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/Player.h>

//////////////////////////////////////////////////////////////////////////
CScriptBind_Boids::CScriptBind_Boids(ISystem *pSystem) : m_pScriptSystem(nullptr)
{
	m_pSystem = pSystem;
	CScriptableBase::Init(pSystem->GetIScriptSystem(), pSystem);
	SetGlobalName("Boids");

	// For Xennon.
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Boids::

	SCRIPT_REG_TEMPLFUNC(CreateFlock,"entity,nType,tParamsTable");
	SCRIPT_REG_TEMPLFUNC(SetFlockParams,"entity,tParamsTable");
	SCRIPT_REG_TEMPLFUNC(EnableFlock,"entity,bEnable" );
	SCRIPT_REG_TEMPLFUNC(SetFlockPercentEnabled,"entity,fPercentage" );
	SCRIPT_REG_TEMPLFUNC(OnBoidHit,"boidEntity,hit" );
	SCRIPT_REG_TEMPLFUNC(SetAttractionPoint, "entity,point");
	SCRIPT_REG_TEMPLFUNC(CanPickup,"flockEntity,boidEntity");
	SCRIPT_REG_TEMPLFUNC(GetUsableMessage,"flockEntity");
	SCRIPT_REG_TEMPLFUNC(OnPickup,"flockEntity,boidEntity,isPickup,throwSpeed");

	//SCRIPT_REG_TEMPLFUNC(Land, "flockEntity");
	//SCRIPT_REG_TEMPLFUNC(TakeOff, "flockEntity");

	m_pMethodsTable->SetValue( "FLOCK_BIRDS",EFLOCK_BIRDS );
	m_pMethodsTable->SetValue( "FLOCK_FISH",EFLOCK_FISH );
	m_pMethodsTable->SetValue( "FLOCK_BUGS",EFLOCK_BUGS );
	m_pMethodsTable->SetValue( "FLOCK_CHICKENS",EFLOCK_CHICKENS );
	m_pMethodsTable->SetValue( "FLOCK_FROGS",EFLOCK_FROGS );
	m_pMethodsTable->SetValue( "FLOCK_TURTLES",EFLOCK_TURTLES );

	DefineConstIntCVarName("boids_enable", boids_enabled, 1, VF_CHEAT|VF_CHEAT_NOCHECK, 
		"Enable disables the boids. If set to 0 before the starting of the game will prevent"
		" the creation of the flocks.");
}

//////////////////////////////////////////////////////////////////////////
CScriptBind_Boids::~CScriptBind_Boids(void)
{
}

//////////////////////////////////////////////////////////////////////////
IEntity* CScriptBind_Boids::GetEntity( IScriptTable *pEntityTable )
{
	if (!pEntityTable)
		return 0;
	
	ScriptHandle handle;
	if (pEntityTable->GetValue( "id",handle ))
	{
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity((EntityId)handle.n);
		return pEntity;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CFlock* CScriptBind_Boids::GetFlock( IScriptTable *pEntityTable )
{
	if(boids_enabled == 0)
		return 0;

	IEntity *pEntity = GetEntity(pEntityTable);
	if (!pEntity)
	{
		GameWarning( "Boids: called with an invalid entity" );
		return 0;
	}
	CBoidsProxy *pProxy = (CBoidsProxy*)pEntity->GetProxy(ENTITY_PROXY_BOIDS);
	if (!pProxy)
	{
		GameWarning( "Boids: specified entity is not a flock" );
		return 0;
	}
	return pProxy->GetFlock();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::CreateFlock( IFunctionHandler *pH, SmartScriptTable entity,i32 nType,SmartScriptTable paramTable )
{
	if(boids_enabled == 0)
		return 0;

	IEntity *pEntity = GetEntity(entity);
	if (!pEntity)
	{
		GameWarning( "Boids.CreateFlock called with an invalid entity" );
		return pH->EndFunction();
	}

	pEntity->SetFlags( pEntity->GetFlags() | ENTITY_FLAG_CLIENT_ONLY );

	CFlock *pFlock = 0;
	switch (nType) {
	case EFLOCK_BIRDS:
		pFlock = new CBirdsFlock(pEntity);
		break;
	case EFLOCK_CHICKENS:
		pFlock = new CChickenFlock(pEntity);
		break;
	case EFLOCK_FISH:
		pFlock = new CFishFlock(pEntity);
		break;
	case EFLOCK_FROGS:
		pFlock = new CFrogFlock(pEntity);
		break;
	case EFLOCK_TURTLES:
		pFlock = new CTurtleFlock(pEntity);
		break;
	case EFLOCK_BUGS:
		pFlock = new CBugsFlock(pEntity);
		break;
	}
	if (pFlock==NULL)
	{
		GameWarning( "Boids.CreateFlock wrong flock type %d specified",nType );
		return pH->EndFunction();
	}

	//////////////////////////////////////////////////////////////////////////
	// Creates a boids proxy for this entity, and attach flock to it.
	CBoidsProxyPtr pBoidsProxy = ComponentCreateAndRegister_DeleteWithRelease<CBoidsProxy>( IComponent::SComponentInitializer(pEntity), true );
	pEntity->SetProxy( ENTITY_PROXY_BOIDS,pBoidsProxy );

	SBoidContext bc;
	pFlock->GetBoidSettings( bc );

	SBoidsCreateContext ctx;
	if (ReadParamsTable( paramTable,bc,ctx ))
	{
		bc.entity = pEntity;
		pFlock->SetBoidSettings( bc );
		pFlock->CreateBoids( ctx );
	}

	// Assign flock to boids proxy and update some settings.
	pBoidsProxy->SetFlock(pFlock);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::SetFlockParams(IFunctionHandler *pH,SmartScriptTable entity,SmartScriptTable paramTable)
{
	CFlock *flock = GetFlock(entity);
	if (!flock)
		return pH->EndFunction();

	string currModel = flock->GetModelName();
	i32 currCount = flock->GetBoidsCount();
	SBoidContext bc;
	flock->GetBoidSettings(bc);

	SBoidsCreateContext ctx;
	if (ReadParamsTable( paramTable,bc,ctx ))
	{
		flock->SetBoidSettings( bc );
		string model = "";
		if (!ctx.models.empty())
			model  = ctx.models[0];
		if ((!model.empty()  && stricmp(model.c_str(),currModel.c_str()) == 0) ||
				(ctx.boidsCount > 0 && ctx.boidsCount != currCount))
		{
			flock->CreateBoids( ctx );
		}
	}

	IEntity *pEntity = GetEntity(entity);
	if (pEntity)
	{
		CBoidsProxy *pBoidsProxy = (CBoidsProxy*)pEntity->GetProxy(ENTITY_PROXY_BOIDS);
		if (pBoidsProxy)
		{
			// Update 
			pBoidsProxy->SetFlock(flock);
		}
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::EnableFlock(IFunctionHandler *pH,SmartScriptTable entity,bool bEnable)
{
	CFlock *flock = GetFlock(entity);
	if (flock)
		flock->SetEnabled( bEnable );

	return pH->EndFunction();
}

i32 CScriptBind_Boids::SetFlockPercentEnabled(IFunctionHandler *pH,SmartScriptTable entity,i32 percent )
{
	CFlock *flock = GetFlock(entity);

	if (flock)
		flock->SetPercentEnabled( percent );

	return pH->EndFunction();
}

i32 CScriptBind_Boids::SetAttractionPoint(IFunctionHandler *pH,SmartScriptTable entity, Vec3 point )
{
	CBirdsFlock *pFlock = (CBirdsFlock *)GetFlock(entity);
	if (pFlock)
		pFlock->SetAttractionPoint(point);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
bool CScriptBind_Boids::ReadParamsTable(IScriptTable *pTable, struct SBoidContext &bc,SBoidsCreateContext &ctx )
{
	pTable->BeginSetGetChain();
	float fval;
	tukk str;

	ctx.models.clear();
	ctx.boidsCount = 0;
	pTable->GetValueChain( "count",ctx.boidsCount );
	if (pTable->GetValueChain( "model",str ))
	{
		ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model1",str ))
	{
		if (str[0])
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model2",str ))
	{
		if (str[0])
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model3",str ))
	{
		if (str[0])
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model4",str ))
	{
		if (str[0])
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "character",str ))
	{
		ctx.characterModel = str;
	}
	if (pTable->GetValueChain( "animation",str ))
	{
		ctx.animation = str;
	}

	pTable->GetValueChain( "behavior",bc.behavior );

	pTable->GetValueChain( "boid_mass",bc.fBoidMass);

	pTable->GetValueChain( "boid_size",bc.boidScale );
	pTable->GetValueChain( "boid_size_random",bc.boidRandomScale );
	pTable->GetValueChain( "min_height",bc.MinHeight );
	pTable->GetValueChain( "max_height",bc.MaxHeight );
	pTable->GetValueChain( "min_attract_distance",bc.MinAttractDistance );
	pTable->GetValueChain( "max_attract_distance",bc.MaxAttractDistance );
	
	if(bc.MinAttractDistance <=0.05f)
		bc.MinAttractDistance = 0.05f;
	if(bc.MaxAttractDistance <=bc.MinAttractDistance)
		bc.MaxAttractDistance =bc.MinAttractDistance +0.05f;

	pTable->GetValueChain( "min_speed",bc.MinSpeed );
	pTable->GetValueChain( "max_speed",bc.MaxSpeed );
	
	pTable->GetValueChain( "factor_align",bc.factorAlignment );
	pTable->GetValueChain( "factor_cohesion",bc.factorCohesion );
	pTable->GetValueChain( "factor_separation",bc.factorSeparation );
	pTable->GetValueChain( "factor_origin",bc.factorAttractToOrigin );
	pTable->GetValueChain( "factor_keep_height",bc.factorKeepHeight );
	pTable->GetValueChain( "factor_avoid_land",bc.factorAvoidLand );
	pTable->GetValueChain( "factor_random_accel",bc.factorRandomAccel );
	pTable->GetValueChain( "flight_time",bc.flightTime );
	pTable->GetValueChain( "factor_take_off",bc.factorTakeOff );
	pTable->GetValueChain( "land_deceleration_height",bc.landDecelerationHeight);

	pTable->GetValueChain( "max_anim_speed",bc.MaxAnimationSpeed );
	pTable->GetValueChain( "follow_player",bc.followPlayer );
	pTable->GetValueChain( "no_landing",bc.noLanding );
	pTable->GetValueChain( "start_on_ground",bc.bStartOnGround );
	pTable->GetValueChain( "avoid_water",bc.bAvoidWater );
	pTable->GetValueChain( "avoid_obstacles",bc.avoidObstacles );
	pTable->GetValueChain( "max_view_distance",bc.maxVisibleDistance );
	pTable->GetValueChain( "max_animation_distance",bc.animationMaxDistanceSq);
	bc.animationMaxDistanceSq *= bc.animationMaxDistanceSq;
	pTable->GetValueChain( "spawn_from_point", bc.bSpawnFromPoint );
	pTable->GetValueChain( "pickable_alive", bc.bPickableWhenAlive );
	pTable->GetValueChain( "pickable_dead", bc.bPickableWhenDead );
	pTable->GetValueChain( "pickable_message", bc.pickableMessage );
	pTable->GetValueChain( "spawn_radius",bc.fSpawnRadius);
	//pTable->GetValueChain( "boid_radius",bc.fBoidRadius);
	pTable->GetValueChain( "gravity_at_death",bc.fGravity);
	pTable->GetValueChain( "boid_mass",bc.fBoidMass);
	if (pTable->GetValueChain( "fov_angle",fval ))
	{
		fval = fval/2.0f; // Half angle used for cos of fov.
		bc.cosFovAngle = cos_tpl(fval*gf_PI/180.0f);
	}

	pTable->GetValueChain("invulnerable", bc.bInvulnerable);

	SmartScriptTable groundTable;
	if (pTable->GetValueChain("ground",groundTable))
	{
		groundTable->BeginSetGetChain();
		groundTable->GetValueChain( "factor_align",bc.factorAlignmentGround );
		groundTable->GetValueChain( "factor_cohesion",bc.factorCohesionGround );
		groundTable->GetValueChain( "factor_separation",bc.factorSeparationGround );
		groundTable->GetValueChain( "factor_origin",bc.factorAttractToOriginGround );
		groundTable->GetValueChain( "walk_speed",bc.walkSpeed);
		groundTable->GetValueChain( "offset",bc.groundOffset);
		groundTable->GetValueChain( "walk_to_idle_duration",bc.fWalkToIdleDuration);
		groundTable->GetValueChain( "on_ground_idle_duration_min",bc.fOnGroundIdleDurationMin);
		groundTable->GetValueChain( "on_ground_idle_duration_max",bc.fOnGroundIdleDurationMax);
		groundTable->GetValueChain( "on_ground_walk_duration_min",bc.fOnGroundWalkDurationMin);
		groundTable->GetValueChain( "on_ground_walk_duration_max",bc.fOnGroundWalkDurationMax);

		groundTable->EndSetGetChain();
	}

	SmartScriptTable audio;
	if (pTable->GetValueChain("Audio", audio))
	{
		bc.audio.clear();
		for (i32 i = 1; i < audio->Count(); ++i)
		{
			str = "";
			if (audio->GetAt(i, str))
			{
				AudioControlId audioControlID = INVALID_AUDIO_CONTROL_ID;
				gEnv->pAudioSystem->GetAudioTriggerId(str, audioControlID);
				bc.audio.push_back(audioControlID);
			}
		}
	}

	SmartScriptTable animations;
	if (pTable->GetValueChain("Animations",animations))
	{
		bc.animations.clear();	// related to CE-1736: clear contents from a previous call
		for (i32 i = 1; i < 100; i++)
		{
			str = "";
			if (animations->GetAt(i,str))
			{
				bc.animations.push_back(str);
			}
			else
				break;
		}
	}

	pTable->EndSetGetChain();

	return true;
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::OnBoidHit( IFunctionHandler *pH,SmartScriptTable flockEntity,SmartScriptTable boidEntity,SmartScriptTable hit )
{
	CFlock *flock = GetFlock(flockEntity);
	IEntity *pBoidEntity = GetEntity(boidEntity);
	if (flock != NULL && pBoidEntity != NULL)
	{
		flock->OnBoidHit( pBoidEntity->GetId(),hit );
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::CanPickup(IFunctionHandler *pH, SmartScriptTable flockEntity, SmartScriptTable boidEntity)
{
	CFlock* flock = GetFlock(flockEntity);
	IEntity* pBoidEntity = GetEntity(boidEntity);

	if (flock != NULL && pBoidEntity != NULL)
	{
		CBoidObject* pBoidObject = NULL;

		for (i32 i = 0; i < flock->GetBoidsCount(); ++i)
		{
			if (flock->GetBoid(i)->GetId() == pBoidEntity->GetId())
			{
				pBoidObject = flock->GetBoid(i);
				break;
			}
		}

		if (pBoidObject != NULL)
		{
			SBoidContext bc;
			flock->GetBoidSettings(bc);

			return pH->EndFunction((pBoidObject->IsDead() && bc.bPickableWhenDead) || (!pBoidObject->IsDead() && bc.bPickableWhenAlive));
		}
	}
	return pH->EndFunction(0);
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::GetUsableMessage(IFunctionHandler *pH, SmartScriptTable flockEntity)
{
	CFlock* flock = GetFlock(flockEntity);
	IActor* pActor = g_pGame->GetIGameFramework()->GetClientActor();

	if (pActor)
	{
		CPlayer* pPlayer = static_cast<CPlayer*>(pActor);

		DrxFixedStringT<64> finalString;
		DrxFixedStringT<64> tempString;

		if (pPlayer && !pPlayer->IsInPickAndThrowMode() && flock != NULL)
		{
			SBoidContext bc;
			flock->GetBoidSettings(bc);

			tempString.Format("@ui_boid_pickup %s", bc.pickableMessage);
			finalString = CHUDUtils::LocalizeString(tempString.c_str());
		}
		return pH->EndFunction(finalString.c_str());
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Boids::OnPickup(IFunctionHandler *pH, SmartScriptTable flockEntity, SmartScriptTable boidEntity, bool bPickup, float fThrowSpeed)
{
	CFlock* flock = GetFlock(flockEntity);
	IEntity* pBoidEntity = GetEntity(boidEntity);

	if (flock != NULL && pBoidEntity != NULL)
	{
		CBoidObject* pBoidObject = NULL;

		for (i32 i = 0; i < flock->GetBoidsCount(); ++i)
		{
			if (flock->GetBoid(i)->GetId() == pBoidEntity->GetId())
			{
				pBoidObject = flock->GetBoid(i);
				break;
			}
		}

		if (pBoidObject != NULL)
		{
			pBoidObject->OnPickup(bPickup, fThrowSpeed);
		}
	}

	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////

/*
i32 CScriptBind_Boids::Land(IFunctionHandler *pH, SmartScriptTable flockEntity)
{
	CFlock *pFlock = GetFlock(flockEntity);
	if(pFlock)
	{
		if(pFlock->GetType() == EFLOCK_BIRDS)
		{
			CBirdsFlock* pBirdsFlock = static_cast<CBirdsFlock*>(pFlock);
			pBirdsFlock->Land();
		}
		else
		{
			GameWarning( "Boids.Land(): wrong flock type %d specified",pFlock->GetType() );
		}
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////

i32 CScriptBind_Boids::TakeOff(IFunctionHandler *pH, SmartScriptTable flockEntity)
{
	CFlock *pFlock = GetFlock(flockEntity);
	if(pFlock)
	{
		if(pFlock->GetType() == EFLOCK_BIRDS)
		{
			CBirdsFlock* pBirdsFlock = static_cast<CBirdsFlock*>(pFlock);
			pBirdsFlock->TakeOff();
		}
		else
		{
			GameWarning( "Boids.Land(): wrong flock type %d specified",pFlock->GetType() );
		}
	}

	return pH->EndFunction();
}

*/