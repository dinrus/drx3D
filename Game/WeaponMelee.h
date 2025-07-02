// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: C++ Weapon Melee Implementation

This is a dedicated weapon for the melee intended to replace
the current 'firemode' implementation.

Allows us to implement the new C3 melee system

-------------------------------------------------------------------------
История:
- Created 21/9/11 by Stephen M. North

*************************************************************************/
#pragma once

#ifndef __WeaponMelee_h__
#define __WeaponMelee_h__

#include <drx3D/Game/Weapon.h>

struct SMeleeActions;
class CWeaponMelee : public CWeapon
{
	typedef CWeapon BaseClass;
public:

	enum EMeleeStatus
	{
		EMeleeStatus_Default = 0,
		EMeleeStatus_Left,
		EMeleeStatus_Right,
		EMeleeStatus_KillingBlow,
		EMeleeStatus_FinishingMove,
		EMeleeStatus_NUM
	};

	EMeleeStatus GetMeleeAttackAction();
	EMeleeStatus GetMeleeStatusCurrent() const { return m_meleeStatusCurrent; }

protected:

	virtual void OnSelected( bool selected );
	virtual void StartFire();
	virtual void MeleeAttack( bool bShort );
	virtual void Select(bool select);
	virtual void Update(SEntityUpdateContext& ctx, i32);
	virtual void SetOwnerId(EntityId ownerId);
	virtual bool CanModify() const;
	virtual bool CanMeleeAttack() const;

public:

	CWeaponMelee();
	virtual ~CWeaponMelee();

private:

	EMeleeStatus m_meleeStatusNext;
	EMeleeStatus m_meleeStatusCurrent;

	float m_meleeAnimationTime;
	float m_timeSinceAction;
	i32 m_numberInCombo;

	EMeleeStatus GetMeleeAttackWeapon();
	void RestoreWeapon( const bool bLazyRestore );
	float GetAnimTime() const;
};

#endif // __WeaponMelee_h__
