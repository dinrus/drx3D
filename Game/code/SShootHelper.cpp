// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/SShootHelper.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/GameRules.h>

#include <drx3D/Game/EntityUtility/EntityEffects.h>

void SShootHelper::DoLocalExplodeEffect(EntityId ownerID, tukk ammo, const Vec3& firePos, const Vec3& fireDir, float desiredRadius)
{
	const SAmmoParams* pAmmoParams = GetAmmoParams(ammo);
	if(!pAmmoParams)
	{
		DRX_ASSERT_MESSAGE(false, ("unable to GetAmmoParams for %s", ammo));
		return;
	}

	const SExplosionParams* pExplosionParams = pAmmoParams->pExplosion;
	if(!pExplosionParams)
	{
		DRX_ASSERT_MESSAGE(false, ("unable to SExplosionParams for %s", ammo));
		return;
	}

	
	if (!pExplosionParams->effectName.empty())
	{
		const float scaleRadius = desiredRadius > 0 ? desiredRadius/pExplosionParams->maxRadius : 1.0f;

		if (EntityEffects::SpawnParticleFX( pExplosionParams->effectName.c_str(), EntityEffects::SEffectSpawnParams( firePos, fireDir, pExplosionParams->effectScale * scaleRadius, -1.0f, false) ) == NULL)
		{
			DRX_ASSERT_MESSAGE(false, ("Unable find effect %s", pExplosionParams->effectName.c_str()));
		}
	}
}

void SShootHelper::Explode(const EntityId ownerID, const EntityId weaponID, tukk ammo, const Vec3& firePos, const Vec3& fireDir, i32 damage, float desiredRadius, bool skipExplosionEffect)
{
	if (gEnv->bServer)
	{
		const SAmmoParams* pAmmoParams = GetAmmoParams(ammo);
		if(!pAmmoParams)
		{
			DRX_ASSERT_MESSAGE(false, ("unable to GetAmmoParams for %s", ammo));
			return;
		}

		const SExplosionParams* pExplosionParams = pAmmoParams->pExplosion;
		if(!pExplosionParams)
		{
			DRX_ASSERT_MESSAGE(false, ("unable to find SExplosionParams for %s", ammo));
			return;
		}

		CGameRules *pGameRules = g_pGame->GetGameRules();
		DRX_ASSERT_MESSAGE(pGameRules, "Unable to find game rules, gonna crash");

		float scaleRadius = desiredRadius > 0 ? desiredRadius/pExplosionParams->maxRadius : 1.0f;

		DRX_ASSERT_MESSAGE(pExplosionParams->hitTypeId, string().Format("Invalid hit type '%s' in explosion params for '%s'", pExplosionParams->type.c_str(), ammo));
		DRX_ASSERT_MESSAGE(pExplosionParams->hitTypeId == pGameRules->GetHitTypeId(pExplosionParams->type.c_str()), "Sanity Check Failed: Stored explosion type id does not match the type string, possibly PreCacheLevelResources wasn't called on this ammo type");

		ExplosionInfo explosionInfo(ownerID, weaponID, 0, (float) damage, firePos, fireDir,
			pExplosionParams->minRadius * scaleRadius, pExplosionParams->maxRadius * scaleRadius,
			pExplosionParams->minPhysRadius * scaleRadius, pExplosionParams->maxPhysRadius * scaleRadius,
			0.0f, pExplosionParams->pressure, pExplosionParams->holeSize, pExplosionParams->hitTypeId);

		
		explosionInfo.SetEffect(skipExplosionEffect ? "" : pExplosionParams->effectName.c_str(), pExplosionParams->effectScale * scaleRadius, pExplosionParams->maxblurdist);
		explosionInfo.SetEffectClass(pAmmoParams->pEntityClass->GetName());
		explosionInfo.SetFriendlyFire(pExplosionParams->friendlyFire);
		explosionInfo.soundRadius = pExplosionParams->soundRadius;

		g_pGame->GetIGameFramework()->GetNetworkSafeClassId(explosionInfo.projectileClassId, ammo);

		pGameRules->QueueExplosion(explosionInfo);
	}
}

CProjectile* SShootHelper::Shoot(EntityId ownerID, const EntityId weaponID, tukk ammo, i32 hitTypeId, const Vec3& firePos, const Vec3& fireDir, i32 damage, bool isProxy)
{
	IEntityClass* pAmmoClass = gEnv->pEntitySystem ? gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammo) : NULL;
	if(!pAmmoClass)
	{
		DRX_ASSERT_MESSAGE(false, string().Format("Unable to find ammo class - %s", ammo));
		return NULL;
	}

	CProjectile* pAmmo = g_pGame->GetWeaponSystem()->SpawnAmmo(pAmmoClass, false);
	if(!pAmmo)
	{
		DRX_ASSERT_MESSAGE(false, string().Format("Unable to Spawn ammo - %s", ammo));
		return NULL;
	}

	DRX_ASSERT_MESSAGE(hitTypeId, "Invalid hit type id passed to SShootHelper::Shoot");

	pAmmo->SetParams(CProjectile::SProjectileDesc(ownerID, 0, weaponID, damage, 0.f, 0.f, 0.f, hitTypeId, 0, false));
	pAmmo->Launch(firePos, fireDir, Vec3(0.0f, 0.0f, 0.0f));
	pAmmo->SetFiredViaProxy(isProxy);
	pAmmo->SetRemote(ownerID != g_pGame->GetClientActorId());

	return pAmmo;
}

const SAmmoParams* SShootHelper::GetAmmoParams(tukk ammo)
{
	IEntityClass* pAmmoClass = gEnv->pEntitySystem ? gEnv->pEntitySystem->GetClassRegistry()->FindClass(ammo) : NULL;
	if(!pAmmoClass)
	{
		DRX_ASSERT_MESSAGE(false, string().Format("Unable to find ammo class - %s", ammo));
		return NULL;
	}

	return g_pGame->GetWeaponSystem()->GetAmmoParams(pAmmoClass);
}
