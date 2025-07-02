// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SSHOOT_HELPER_H__
#define __SSHOOT_HELPER_H__

class CProjectile;
struct SAmmoParams;

class SShootHelper
{
public:
	static CProjectile* Shoot(EntityId ownerID, const EntityId weaponID, tukk ammo, i32 hitTypeId, const Vec3& firePos, const Vec3& fireDir, i32 damage, bool isProxy = false);
	static void Explode(const EntityId ownerID, const EntityId weaponID, tukk ammo, const Vec3& firePos, const Vec3& fireDir, i32 damage, float desiredRadius = -1.0f, bool skipExplosionEffect = false);
	static void DoLocalExplodeEffect(EntityId ownerID, tukk ammo, const Vec3& firePos, const Vec3& fireDir, float desiredRadius = -1.0f);
private:
	static const SAmmoParams* GetAmmoParams(tukk ammo);
};

#endif