// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: LTAG Implementation

-------------------------------------------------------------------------
История:
- 16:09:09	: Created by Benito Gangoso Rodriguez

*************************************************************************/
#pragma once

#ifndef _LTAG_H_
#define _LTAG_H_

#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/HandGrenades.h>

class CLTag :	public CWeapon
{
public:

	CLTag();
	virtual ~CLTag();

	virtual void Reset();
	virtual void FullSerialize( TSerialize ser );
	virtual void OnSelected(bool selected);
	
	virtual void StartFire(const SProjectileLaunchParams& launchParams);

	virtual void OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
		CWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}
	virtual void ProcessEvent(SEntityEvent& event);
	//virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
	//virtual void Select(bool select);
	virtual void StartChangeFireMode();

protected:

	virtual void AnimationEvent(ICharacterInstance *pCharacter, const AnimEventInstance &event);

private:
	void UpdateGrenades();
	void HideGrenadeAttachment(ICharacterInstance* pWeaponCharacter, tukk attachmentName, bool hide);

	typedef CWeapon inherited;

	bool OnActionSwitchFireMode(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
};

#endif // _LTAG_H_