// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 14:05:2010		Created by Steve Humphreys
*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/GameAchievements.h>

#include <drx3D/CoreX/Lobby/IDrxReward.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/WeaponSystem.h>

const float THROW_TIME_THRESHOLD = 5.0f;

CGameAchievements::CGameAchievements()
: m_lastPlayerThrownObject(0)
, m_lastPlayerKillBulletId(0)
, m_lastPlayerKillGrenadeId(0)
, m_killsWithOneGrenade(0)
, m_allowAchievements(true)
{
	m_lastPlayerKillBulletId = 0;
	g_pGame->GetIGameFramework()->RegisterListener(this, "CGameAchievements", FRAMEWORKLISTENERPRIORITY_GAME);

	CGameLobbyUpr *pGameLobbyUpr = g_pGame->GetGameLobbyUpr();
	DRX_ASSERT(pGameLobbyUpr || gEnv->IsEditor());
	if(pGameLobbyUpr)
	{
		pGameLobbyUpr->AddPrivateGameListener(this);
	}
}

CGameAchievements::~CGameAchievements()
{
	CGameLobbyUpr *pGameLobbyUpr = g_pGame->GetGameLobbyUpr();
	if(pGameLobbyUpr)
	{
		pGameLobbyUpr->RemovePrivateGameListener(this);
	}

	g_pGame->GetIGameFramework()->UnregisterListener(this);
	RemoveHUDEventListeners();
}

void CGameAchievements::GiveAchievement(i32 achievement)
{
	if(AllowAchievement())
	{
		CPersistantStats::GetInstance()->OnGiveAchievement(achievement);

		IDrxLobby* pLobby = gEnv->pNetwork->GetLobby();
		IPlatformOS* pOS = gEnv->pSystem->GetPlatformOS();
		if(pLobby != NULL && pOS != NULL)
		{
			IDrxReward* pReward = pLobby->GetReward();
			if(pReward)
			{
				u32 user = g_pGame->GetExclusiveControllerDeviceIndex();
				if(user != IPlatformOS::Unknown_User)
				{
					u32 achievementId = achievement + ACHIEVEMENT_STARTINDEX;

					//-- Award() only puts awards into a queue to be awarded.
					//-- This code here only asserts that the award was added to the queue successfully, not if the award was successfully unlocked.
					//-- Function has been modified for a DrxLobbyTaskId, callback and callback args parameters, similar to most other lobby functions,
					//-- to allow for callback to trigger when the award is successfully unlocked (eCLE_Success) or failed to unlock (eCLE_InternalError).
					//-- In the case of trying to unlock an award that has already been unlocked, the callback will return eCLE_Success.
					//-- Successful return value of the Award function has been changed from incorrect eCRE_Awarded to more sensible eCRE_Queued.
					DRX_TODO(3,9,2010, "Register a callback to inform game when queued award is processed successfully or failed.");

					EDrxRewardError error = pReward->Award(user, achievementId, NULL, NULL, NULL);
					DrxLog("Award error %d", error);
					DRX_ASSERT(error == eCRE_Queued);
				}
			}
		}
	}
	else
	{
		DrxLog("Not Awarding achievement - have been disabled");
	}
}

bool CGameAchievements::AllowAchievement()
{
	return m_allowAchievements;
}

void CGameAchievements::OnActionEvent(const SActionEvent& event)
{
	// assuming that we don't want to detect anything in MP.
	if(event.m_event == eAE_inGame && !gEnv->bMultiplayer)
	{
		CGameRules* pGR = g_pGame->GetGameRules();
		if(pGR)
		{
			pGR->RegisterKillListener(this);
		}

		m_lastPlayerThrownObject = 0;
		m_lastPlayerKillBulletId = 0;
		m_lastPlayerKillGrenadeId = 0;
		m_killsWithOneGrenade = 0;
	}

	// NB: by the time the eAE_unloadlevel event is sent the game
	//	rules is already null: can't unregister.
}

void CGameAchievements::OnHUDEvent(const SHUDEvent& event)
{
	// assuming that we don't want to detect anything in MP.
	if(event.eventType == eHUDEvent_OnEntityScanned && !gEnv->bMultiplayer)
	{
		g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_ScanObject, 1);
	}
}

void CGameAchievements::OnEntityKilled(const HitInfo &hitInfo)
{
	// target must be an AI
	IActor* pTarget = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.targetId);
	if(!pTarget || pTarget->IsPlayer())
		return;

	// shooter might be null, if this is a collision, but will be checked otherwise
	IActor* pShooter = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(hitInfo.shooterId);

	switch(CategoriseHit(hitInfo.type))
	{	
		case eCHT_Bullet:
		{
			// ignore AI shots
			if(!pShooter || !pShooter->IsPlayer())
				break;

			CProjectile* pProjectile = g_pGame->GetWeaponSystem()->GetProjectile(hitInfo.projectileId);
			assert(pProjectile);
			const CTimeValue& spawnTime = pProjectile ? pProjectile->GetSpawnTime() : 0.0f;
			if(spawnTime == 0.0f)
				break;

			if(hitInfo.projectileId == m_lastPlayerKillBulletId
				&& spawnTime == m_lastPlayerKillBulletSpawnTime)
			{
				// same projectile as previously, trigger the 'two kills one bullet' objective
				
			}
			else
			{
				// save for later
				m_lastPlayerKillBulletId = hitInfo.projectileId;
				m_lastPlayerKillBulletSpawnTime = spawnTime;
			}
		}
		break;

		case eCHT_Grenade:
		{
			if(!pShooter || !pShooter->IsPlayer())
				break;

			CProjectile* pGrenade = g_pGame->GetWeaponSystem()->GetProjectile(hitInfo.projectileId);
			const CTimeValue& spawnTime = pGrenade ? pGrenade->GetSpawnTime() : 0.0f;
			if(spawnTime == 0.0f)
				break;

			if(hitInfo.projectileId == m_lastPlayerKillGrenadeId
				&& spawnTime == m_lastPlayerKillGrenadeSpawnTime)
			{
				if(++m_killsWithOneGrenade == 3)
				{
					
				}
			}
			else
			{
				// save for later
				m_lastPlayerKillGrenadeId = hitInfo.projectileId;
				m_lastPlayerKillGrenadeSpawnTime = spawnTime;
				m_killsWithOneGrenade = 1;
			}
			
		}
		break;

		case eCHT_Collision:
		{
			CTimeValue now = gEnv->pTimer->GetFrameStartTime();
			if(hitInfo.weaponId != 0 
				&& hitInfo.weaponId == m_lastPlayerThrownObject
				&& (now-m_lastPlayerThrownTime) < THROW_TIME_THRESHOLD)
			{
				// AI was killed by an object the player threw in the last x seconds...
				g_pGame->GetPersistantStats()->IncrementClientStats(EIPS_ThrownObjectKill, 1);
			}
		}
		break;

		default:
		break;
	}
}

void CGameAchievements::PlayerThrewObject(EntityId object)
{
	if(gEnv->bMultiplayer)
		return;

	m_lastPlayerThrownObject = object;
	m_lastPlayerThrownTime = gEnv->pTimer->GetFrameStartTime();
}

void CGameAchievements::AddHUDEventListeners()
{
	CHUDEventDispatcher::AddHUDEventListener(this, "OnEntityScanned");
}

void CGameAchievements::RemoveHUDEventListeners()
{
	CHUDEventDispatcher::RemoveHUDEventListener(this);
}

CGameAchievements::ECategorisedHitType CGameAchievements::CategoriseHit(i32 hitType)
{
	ECategorisedHitType outType;

	if(hitType == CGameRules::EHitType::Collision)
	{
		outType = eCHT_Collision;
	}
	else if(hitType == CGameRules::EHitType::Frag)
	{
		outType = eCHT_Grenade;
	}
	else if(hitType == CGameRules::EHitType::Bullet)
	{
		outType = eCHT_Bullet;
	}
	else
	{
		outType = eCHT_Other;
	}

	return outType;
}

void CGameAchievements::SetPrivateGame(const bool privateGame)
{
	m_allowAchievements = !privateGame;
}
