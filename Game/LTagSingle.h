// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: 

Ltag version of single firemode. Requires some extra functionality
for animation, projectile spawning, etc

-------------------------------------------------------------------------
История:
- 15:09:09   Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef _LTAG_SINGLE_H_
#define _LTAG_SINGLE_H_

#include <drx3D/Game/Single.h>
#include <drx3D/Game/LTAGGrenade.h>
#include <drx3D/Game/Throw.h>

class CLTagSingle : public CSingle
{
private:
	typedef CSingle Parent;
	struct EndCockingAction;
	struct ScheduleReload;

public:
	DRX_DECLARE_GTI(CLTagSingle);

	CLTagSingle();
	virtual ~CLTagSingle();

	//CSingle
	virtual void Activate(bool activate) override;
	virtual void UpdateFPView(float frameTime) override;

	virtual void OnShoot(EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel) override;

	virtual void SetProjectileLaunchParams(const SProjectileLaunchParams &launchParams) override;
	//~CSingle

	bool NextGrenadeType();
	const ELTAGGrenadeType GetCurrentGrenadeType() const;
	//bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
	//uint NetGetGrenadeType() const { return static_cast<uint>(m_grenadeType); }
	//void NetSetGrenadeType(uint type);
	void NetShootEx(const Vec3 &pos, const Vec3 &dir, const Vec3 &vel, const Vec3 &hit, float extra, i32 predictionHandle) override;

	void GetMemoryUsage(IDrxSizer * s) const override
	{
		s->AddObject(this, sizeof(*this));	
		CSingle::GetInternalMemoryUsage(s);		// collect memory of parent class
	}

protected:
	virtual void SetReloadFragmentTags(CTagState& fragTags, i32 ammoCount) override;

	void SwitchGrenades ();
	void UpdateGrenadeAttachment(ICharacterInstance* pCharacter, tukk attachmentName, tukk model);

	ELTAGGrenadeType m_grenadeType;
	bool	m_fpModelInitialised;
};



#endif
