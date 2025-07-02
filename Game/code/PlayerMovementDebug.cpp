// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Player movement debug for non-release builds
-------------------------------------------------------------------------
История:
- 16:07:2010   Extracted from CPlayer code by Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PlayerMovementDebug.h>

#ifdef PLAYER_MOVEMENT_DEBUG_ENABLED
#include <drx3D/Game/GameCVars.h>

#include <drx3D/Act/IDebugHistory.h>

CPlayerDebugMovement::CPlayerDebugMovement()
: m_pDebugHistoryUpr(NULL)
{

}

CPlayerDebugMovement::~CPlayerDebugMovement()
{
	SAFE_RELEASE(m_pDebugHistoryUpr);
}


void CPlayerDebugMovement::DebugGraph_AddValue( tukk id, float value ) const
{

	if ((m_pDebugHistoryUpr == NULL) || (id == NULL))
		return;

	// NOTE: It's alright to violate the const here. The player is a good common owner for debug graphs, 
	// but it's also not non-const in all places, even though graphs might want to be added from those places.
	IDebugHistory* pDebugHistory = const_cast<IDebugHistoryUpr*>(m_pDebugHistoryUpr)->GetHistory(id);	
	if (pDebugHistory)
	{
		pDebugHistory->AddValue(value);
	}

}

void CPlayerDebugMovement::Debug( const IEntity* pPlayerEntity )
{
	DRX_ASSERT(pPlayerEntity);

	bool debug = true;

	tukk filter = g_pGameCVars->pl_debug_filter->GetString();
	tukk name = pPlayerEntity->GetName();
	if ((strcmp(filter, "0") != 0) && (strcmp(filter, name) != 0))
	{
		debug = false;
	}

	if (!debug)
	{
		if (m_pDebugHistoryUpr)
		{
			m_pDebugHistoryUpr->Clear();
		}
		return;
	}

	if (m_pDebugHistoryUpr == NULL)
	{
		m_pDebugHistoryUpr = g_pGame->GetIGameFramework()->CreateDebugHistoryUpr();
	}
	DRX_ASSERT(m_pDebugHistoryUpr);

	bool showReqVelo = (g_pGameCVars->pl_debug_movement != 0);
	m_pDebugHistoryUpr->LayoutHelper("ReqVelo", NULL, showReqVelo, -20, 20, 0, 5, 0.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("ReqVeloX", NULL, showReqVelo, -20, 20, -5, 5, 1.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("ReqVeloY", NULL, showReqVelo, -20, 20, -5, 5, 2.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("ReqVeloZ", NULL, showReqVelo, -20, 20, -5, 5, 3.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("ReqRotZ", NULL, showReqVelo, -360, 360, -5, 5, 4.0f, 0.0f);

	m_pDebugHistoryUpr->LayoutHelper("PhysVelReq", NULL, showReqVelo, -20, 20, 0, 5, 0.0f, 1.0f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVelReqX", NULL, showReqVelo, -20, 20, -5, 5, 1.0f, 1.0f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVelReqY", NULL, showReqVelo, -20, 20, -5, 5, 2.0f, 1.0f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVelReqZ", NULL, showReqVelo, -20, 20, -5, 5, 3.0f, 1.0f, 1,0.9f);

	m_pDebugHistoryUpr->LayoutHelper("PhysVelo", NULL, showReqVelo, -20, 20, 0, 5, 0.0f, 1.9f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloX", NULL, showReqVelo, -20, 20, -5, 5, 1.0f, 1.9f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloY", NULL, showReqVelo, -20, 20, -5, 5, 2.0f, 1.9f, 1,0.9f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloZ", NULL, showReqVelo, -20, 20, -5, 5, 3.0f, 1.9f, 1,0.9f);

	m_pDebugHistoryUpr->LayoutHelper("PhysVeloUn", NULL, showReqVelo, -20, 20, 0, 5, 0.0f, 2.8f, 1,0.2f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloUnX", NULL, showReqVelo, -20, 20, -5, 5, 1.0f, 2.8f, 1,0.2f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloUnY", NULL, showReqVelo, -20, 20, -5, 5, 2.0f, 2.8f, 1,0.2f);
	m_pDebugHistoryUpr->LayoutHelper("PhysVeloUnZ", NULL, showReqVelo, -20, 20, -5, 5, 3.0f, 2.8f, 1,0.2f);

	bool showReqAim = (g_pGameCVars->pl_debug_aiming != 0);
	m_pDebugHistoryUpr->LayoutHelper("AimH", NULL, showReqAim, -0.2f, 0.2f, -0.1f, 0.1f, 0.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AxxAimH", NULL, showReqAim, -1.0f, 1.0f, -0.1f, 0.1f, 1.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimHAxxMul", NULL, showReqAim, 0, 10, 0, 1, 2.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimHAxxFrac", NULL, showReqAim, 0, 1, 0, 1, 3.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimHAxxTime", NULL, showReqAim, 0, 2, 0, 1, 4.0f, 0.0f);

	m_pDebugHistoryUpr->LayoutHelper("AimV", NULL, showReqAim, -0.2f, 0.2f, -0.1f, 0.1f, 0.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AxxAimV", NULL, showReqAim, -1.0f, 1.0f, -0.1f, 0.1f, 1.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimVAxxMul", NULL, showReqAim, 0, 10, 0, 1, 2.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimVAxxFrac", NULL, showReqAim, 0, 1, 0, 1, 3.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimVAxxTime", NULL, showReqAim, 0, 2, 0, 1, 4.0f, 1.0f);

	bool showAimAssist = false;
	m_pDebugHistoryUpr->LayoutHelper("AimFollowH", NULL, showAimAssist, -0.05f, 0.05f, -0.05f, 0.05f, 0.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimFollowV", NULL, showAimAssist, -0.05f, 0.05f, -0.05f, 0.05f, 1.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimScale", NULL, showAimAssist, 0, 1, 0, 1, 2.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimDeltaH", NULL, showAimAssist, -0.03f, +0.03f, -0.03f, +0.03f, 0.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AimDeltaV", NULL, showAimAssist, -0.03f, +0.03f, -0.03f, +0.03f, 1.0f, 1.0f);

	m_pDebugHistoryUpr->LayoutHelper("FollowCoolDownH", NULL, showAimAssist, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 2.0f);
	m_pDebugHistoryUpr->LayoutHelper("FollowCoolDownV", NULL, showAimAssist, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 2.0f);

	/*
	m_pDebugHistoryUpr->LayoutHelper("InputMoveX", NULL, showReqVelo, -1, 1, -1, 1, 0.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("InputMoveY", NULL, showReqVelo, -1, 1, -1, 1, 1.0f, 1.0f);
	/**/

	/*
	bool showVelo = true;
	m_pDebugHistoryUpr->LayoutHelper("Velo", NULL, showVelo, -20, 20, 0, 8, 0.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("VeloX", NULL, showVelo, -20, 20, -5, 5, 1.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("VeloY", NULL, showVelo, -20, 20, -5, 5, 2.0f, 0.0f);
	m_pDebugHistoryUpr->LayoutHelper("VeloZ", NULL, showVelo, -20, 20, -5, 5, 3.0f, 0.0f);
	/**/

	/*
	m_pDebugHistoryUpr->LayoutHelper("Axx", NULL, showVelo, -20, 20, -1, 1, 0.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AxxX", NULL, showVelo, -20, 20, -1, 1, 1.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AxxY", NULL, showVelo, -20, 20, -1, 1, 2.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("AxxZ", NULL, showVelo, -20, 20, -1, 1, 3.0f, 1.0f);
	/**/

	//m_pDebugHistoryUpr->LayoutHelper("ModelOffsetX", NULL, true, 0, 1, -0.5, 0.5, 5.0f, 0.5f);
	//m_pDebugHistoryUpr->LayoutHelper("ModelOffsetY", NULL, true, 0, 1, 0, 1, 5.0f, 1.5f, 1.0f, 0.5f);

	//*
	bool showJump = (g_pGameCVars->pl_debug_jumping != 0);
	m_pDebugHistoryUpr->LayoutHelper("OnGround", NULL, showJump, 0, 1, 0, 1, 5.0f, 0.5f, 1.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("Jumping", NULL, showJump, 0, 1, 0, 1, 5.0f, 1.0f, 1.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("Flying", NULL, showJump, 0, 1, 0, 1, 5.0f, 1.5f, 1.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("StuckTimer", NULL, showJump, 0, 0.5, 0, 0.5, 5.0f, 2.0f, 1.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("InAirTimer", NULL, showJump, 0, 5, 0, 5, 4.0f, 2.0f, 1.0f, 1.0f);
	m_pDebugHistoryUpr->LayoutHelper("InWaterTimer", NULL, showJump, -5, 5, -0.5, 0.5, 4, 3);
	m_pDebugHistoryUpr->LayoutHelper("OnGroundTimer", NULL, showJump, 0, 5, 0, 5, 4.0f, 1.0f, 1.0f, 1.0f);
	/**/

	//*
	m_pDebugHistoryUpr->LayoutHelper("GroundSlope", NULL, showJump, 0, 90, 0, 90, 0, 3);
	m_pDebugHistoryUpr->LayoutHelper("GroundSlopeMod", NULL, showJump, 0, 90, 0, 90, 1, 3);
	/**/

	//m_pDebugHistoryUpr->LayoutHelper("ZGDashTimer", NULL, showVelo, -20, 20, -0.5, 0.5, 5.0f, 0.5f);
	/*
	m_pDebugHistoryUpr->LayoutHelper("StartTimer", NULL, showVelo, -20, 20, -0.5, 0.5, 5.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("DodgeFraction", NULL, showVelo, 0, 1, 0, 1, 5.0f, 1.5f, 1.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("RampFraction", NULL, showVelo, 0, 1, 0, 1, 5.0f, 2.0f, 1.0f, 0.5f);
	m_pDebugHistoryUpr->LayoutHelper("ThrustAmp", NULL, showVelo, 0, 5, 0, 5, 5.0f, 2.5f, 1.0f, 0.5f);
	*/

}

void CPlayerDebugMovement::LogFallDamage( const IEntity* pPlayerEntity, const float velocityFraction, const float impactVelocity, const float damage )
{
	DRX_ASSERT(pPlayerEntity);

	if (g_pGameCVars->pl_health.debug_FallDamage != 0)
	{
		tukk side = gEnv->bServer ? "Server" : "Client";

		tukk color = "";
		if (velocityFraction < 0.33f)
			color = "$6"; // Yellow
		else if (velocityFraction < 0.66f)
			color = "$8"; // Orange
		else
			color = "$4"; // Red

		DrxLog("%s[%s][%s] ImpactVelo=%3.2f, FallDamage=%3.1f", color, side, pPlayerEntity->GetName(), impactVelocity, damage);
	}
}

void CPlayerDebugMovement::LogFallDamageNone( const IEntity* pPlayerEntity, const float impactVelocity )
{
	DRX_ASSERT(pPlayerEntity);

	if (g_pGameCVars->pl_health.debug_FallDamage != 0)
	{
		if (impactVelocity > 0.5f)
		{
			tukk side = gEnv->bServer ? "Server" : "Client";
			tukk color = "$3"; // Green
			DrxLog("%s[%s][%s] ImpactVelo=%3.2f, FallDamage: NONE", 
				color, side, pPlayerEntity->GetName(), impactVelocity);
		}
	}
}

void CPlayerDebugMovement::LogVelocityStats( const IEntity* pPlayerEntity, const pe_status_living& livStat, const float fallSpeed, const float impactVelocity )
{
	DRX_ASSERT(pPlayerEntity);

	if (g_pGameCVars->pl_health.debug_FallDamage == 2)
	{
		const Vec3 pos = pPlayerEntity->GetWorldPos();
		tukk side = gEnv->bServer ? "Server" : "Client";
		DrxLog("[%s] liv.vel=%0.1f,%0.1f,%3.2f liv.velU=%0.1f,%0.1f,%3.2f impactVel=%3.2f posZ=%3.2f (liv.velReq=%0.1f,%0.1f,%3.2f) (fallspeed=%3.2f) gt=%3.3f, pt=%3.3f", 
			side, 
			livStat.vel.x, livStat.vel.y, livStat.vel.z, 
			livStat.velUnconstrained.x, livStat.velUnconstrained.y, livStat.velUnconstrained.z, 
			impactVelocity, 
			/*pos.x, pos.y,*/ pos.z, 
			livStat.velRequested.x, livStat.velRequested.y, livStat.velRequested.z, 
			fallSpeed, 
			gEnv->pTimer->GetCurrTime(), gEnv->pPhysicalWorld->GetPhysicsTime());
	}
}

void CPlayerDebugMovement::GetInternalMemoryUsage( IDrxSizer * pSizer ) const
{
	pSizer->AddObject(m_pDebugHistoryUpr);
}

#endif //PLAYER_MOVEMENT_DEBUG_ENABLED