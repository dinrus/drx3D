// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Binocular Implementation

-------------------------------------------------------------------------
История:
- 18:12:2005   14:00 : Created by M�rcio Martins

*************************************************************************/
#ifndef __BINOCULAR_H__
#define __BINOCULAR_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Game/Weapon.h>


class CBinocular :	public CWeapon
{
	typedef CWeapon BaseClass;

public:

	CBinocular();

	virtual void OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	
	virtual bool OnActionSpecial(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	virtual void Select(bool select);
	virtual void UpdateFPView(float frameTime);
	virtual bool AllowZoomToggle() { return false; }

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
		CWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}
	virtual bool CanModify() const;
	virtual bool CanFire() const;
	virtual void StartFire();
	virtual void OnZoomIn();
	virtual void OnZoomOut();

	virtual bool AllowInteraction(EntityId interactionEntity, EInteractionType interactionType);

protected:
	
	virtual bool ShouldDoPostSerializeReset() const;

private:

	virtual ~CBinocular();

	bool ShouldUseSoundAttenuation(const CActor& ownerActor) const;
	void SwitchSoundAttenuation(const CActor& ownerActor, const float coneInRadians) const;
	void UpdateSoundAttenuation(const CActor& ownerActor) const;

	bool OnActionChangeZoom(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	bool OnActionSprint(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	bool TrumpAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
};

#endif // __BINOCULAR_H__