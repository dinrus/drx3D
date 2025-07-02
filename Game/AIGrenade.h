// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id:$
$DateTime$
Описание:  AI grenade weapon
-------------------------------------------------------------------------
История:
- 17:01:2008: Created by Benito G.R.
							Split from OffHand to a separate, smaller and simpler class

*************************************************************************/

#ifndef __AIGRENADE_H__
#define __AIGRENADE_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Game/Weapon.h>

class CAIGrenade : public CWeapon, public IWeaponFiringLocator
{
	struct FinishGrenadeAction;
	typedef CWeapon BaseClass;

public:
	CAIGrenade();
	virtual ~CAIGrenade();

	//IWeapon
	virtual void StartFire();
	virtual void StartFire(const SProjectileLaunchParams& launchParams);
	virtual void OnAnimationEventShootGrenade(const AnimEventInstance &event);
	virtual i32  GetAmmoCount(IEntityClass* pAmmoType) const { return 1; } //Always has ammo
	virtual void OnReset();
	virtual void SetCurrentFireMode(i32 idx);

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
		CWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}

	virtual void PostSerialize();

private:
	virtual bool GetProbableHit(EntityId weaponId, const IFireMode* pFireMode, Vec3& hit);
	virtual bool GetFiringPos(EntityId weaponId, const IFireMode* pFireMode, Vec3& pos);
	virtual bool GetFiringDir(EntityId weaponId, const IFireMode* pFireMode, Vec3& dir, const Vec3& probableHit, const Vec3& firingPos);
	virtual bool GetActualWeaponDir(EntityId weaponId, const IFireMode* pFireMode, Vec3& dir, const Vec3& probableHit, const Vec3& firingPos);
	virtual bool GetFiringVelocity(EntityId weaponId, const IFireMode* pFireMode, Vec3& vel, const Vec3& firingDir);
	virtual void WeaponReleased();
	void SetFiringPos(tukk boneName);

	bool m_waitingForAnimEvent;
	Vec3 m_grenadeLaunchPosition;
	Vec3 m_grenadeLaunchVelocity;
};


#endif // __AIGRENADE_H__