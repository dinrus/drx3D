// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NO_WEAPON__
#define __NO_WEAPON__

#pragma once

#include <drx3D/Game/Weapon.h>

class CNoWeapon: public CWeapon
{
	typedef CWeapon BaseClass;

public:

	CNoWeapon();
	virtual ~CNoWeapon();

	virtual void OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	virtual void Select(bool select);

	virtual bool UpdateAimAnims(SParams_WeaponFPAiming &aimAnimParams);

protected:
	virtual bool ShouldDoPostSerializeReset() const;

private:

	void RegisterActions();
	bool OnActionMelee(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
};

#endif