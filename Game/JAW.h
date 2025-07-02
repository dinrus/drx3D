// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id:$
$DateTime$
Описание:  Class for specific JAW rocket launcher functionality
-------------------------------------------------------------------------
История:
- 22:06:2007: Created by Benito G.R.
- 30:10:2009: Ported from RocketLauncher

*************************************************************************/

#pragma once

#ifndef _JAW_H_
#define _JAW_H_

#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/Laser.h>



struct IAttachment;



class CJaw : public CWeapon, public IWeaponFiringLocator
{
private:

	typedef CWeapon BaseClass;

public:
	CJaw();

	virtual bool Init(IGameObject * pGameObject);
	virtual void Update(SEntityUpdateContext& ctx, i32 slot);
	virtual void OnReset();
	virtual bool SetAspectProfile(EEntityAspects aspect, u8 profile);
	virtual void Select(bool select);
	virtual void PickUp(EntityId pickerId, bool sound, bool select/* =true */, bool keepHistory/* =true */, tukk setup);
	virtual void Drop(float impulseScale, bool selectNext/* =true */, bool byDeath/* =false */);
	virtual bool ShouldDoPostSerializeReset() const;
	virtual void ProcessEvent(SEntityEvent& event);
	virtual void OnShoot(EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel);
	virtual void OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
	virtual bool OutOfAmmo(bool allFireModes) const;

	virtual void StartFire();
	virtual void StartFire(const SProjectileLaunchParams& launchParams);
	virtual void StopFire();

	virtual void OnEnterFirstPerson();
	virtual void OnEnterThirdPerson();

	virtual void SetDestinationEntity(EntityId targetId);

	virtual void GetAttachmentsAtHelper(tukk helper, CDrxFixedStringListT<5, 30> &attachments);

	virtual bool CanPickUp(EntityId userId) const;
	virtual bool CanDrop() const;

	virtual void FullSerialize( TSerialize ser );
	virtual void PostSerialize();

	virtual void AutoDrop();
	virtual void AddFiredRocket() { m_firedRockets++; }
	virtual bool CanFire() const;
	virtual bool CanSelect() const;
	virtual bool CanDeselect() const;


	virtual bool GetProbableHit(EntityId weaponId, const IFireMode* pFireMode, Vec3& hit);
	virtual bool GetFiringPos(EntityId weaponId, const IFireMode* pFireMode, Vec3& pos);
	virtual bool GetFiringDir(EntityId weaponId, const IFireMode* pFireMode, Vec3& dir, const Vec3& probableHit, const Vec3& firingPos);
	virtual bool GetActualWeaponDir(EntityId weaponId, const IFireMode* pFireMode, Vec3& dir, const Vec3& probableHit, const Vec3& firingPos);
	virtual bool GetFiringVelocity(EntityId weaponId, const IFireMode* pFireMode, Vec3& vel, const Vec3& firingDir);
	virtual void WeaponReleased();

	virtual float GetMovementModifier() const;
	virtual float GetRotationModifier(bool usingMouse) const;

	virtual ColorF GetSilhouetteColor() const;

	virtual void NetStartFire();
	virtual void NetShootEx(const Vec3 &pos, const Vec3 &dir, const Vec3 &vel, const Vec3 &hit, float extra, i32 predictionHandle, i32 fireModeId);
	virtual void NetSetCurrentAmmoCount(i32 count);

	virtual void AnimationEvent(ICharacterInstance *pCharacter, const AnimEventInstance &event);

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
		CWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}

	struct SRequestAmmoParams
	{
		u16 m_ammo;

		void SerializeWith(TSerialize& ser)
		{
			ser.Value("ammo", m_ammo, 'ui3');
		}
	};

	DECLARE_SERVER_RMI_NOATTACH(SvRequestAmmo, SRequestAmmoParams, eNRT_ReliableUnordered);

private:
	virtual bool CanReload() const;
	virtual void AddAmmoCapacity();
	virtual void DropAmmoCapacity();
	virtual void OnDropped(EntityId actorId, bool ownerWasAI);
	void SvActivateMissileCountermeasures(EntityId shooterId, const Vec3 &pos, const Vec3 &dir);
	bool CanAutoDrop();
	void DoAutoDrop();
	bool GiveExtraTubeToInventory(IActor* pPickerActor, IItemSystem* pItemSystem) const;

	void DropUsed();
	void DropUnused(CActor* pOwner);
	void HideRocket();
	void CreateSmokeEffect();
	void UpdatePendingShot();
	void AutoZoomOut();

	bool OnActionJawZoom(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);

	void UpdateLaser(const SEntityUpdateContext& ctx);

private:
	CLaserBeam m_laserGuider;

	EntityId m_forcedTargetId;
	i32		m_firedRockets;		//Number of rockets still flying

	u32 m_dropJAWAnimEvent;

	float	m_autoDropPendingTimer;
	float	m_dropTime;

	bool	m_auxSlotUsed;
	bool	m_auxSlotUsedBQS;
	bool	m_autoDropping;
	bool	m_controllingRocket;
	bool	m_zoomTriggerDown;
	bool	m_fireTriggerDown;
	bool	m_firePending;
	bool	m_smokeActive;
	bool	m_dropped;
	bool	m_playedDropAction;
	bool	m_fired;
	bool	m_zoomAlreadyToggled;
	bool	m_extraTubesAdded;
};


#endif // _JAW_H_
