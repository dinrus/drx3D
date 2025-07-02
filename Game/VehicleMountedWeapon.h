// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id:$
$DateTime$
Описание:  Mounted machine gun that can be ripped off by the player
and move around with it - vehicle mounted version
-------------------------------------------------------------------------
История:
- 16:02:2010: Created by SNH

*************************************************************************/

#pragma once

#ifndef _VEHICLE_MOUNTED_WEAPON_H_
#define _VEHICLE_MOUNTED_WEAPON_H_

#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Animation/DrxCharAnimationParams.h>
#include <drx3D/Game/HeavyMountedWeapon.h>

struct IVehicle;
struct IVehiclePart;
struct IVehicleSeat;

class CVehicleMountedWeapon : public CHeavyMountedWeapon
{
public:

	CVehicleMountedWeapon();

	// CWeapon
	virtual void StartUse(EntityId userId);
	virtual void ApplyViewLimit(EntityId userId, bool apply);

	virtual void StartFire();

	virtual void Update(SEntityUpdateContext& ctx, i32 update);

	virtual void SetAmmoCount(IEntityClass* pAmmoType, i32 count);
	virtual void SetInventoryAmmoCount(IEntityClass* pAmmoType, i32 count);

	virtual bool CanZoom() const;

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));
		CHeavyMountedWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}	

	virtual bool ShouldBindOnInit() const { return false; }

	virtual bool ApplyActorRecoil() const { return (m_pOwnerSeat == m_pSeatUser); }  

	virtual void FullSerialize(TSerialize ser);
	virtual void PostSerialize();
	// ~CWeapon

	virtual void Use(EntityId userId);
	virtual void StopUse(EntityId userId);
	bool CanRipOff() const;

	bool CanUse(EntityId userId) const;

	virtual void MountAtEntity(EntityId entityId, const Vec3 &pos, const Ang3 &angles);

protected:

	bool CheckWaterLevel() const;
	virtual void PerformRipOff(CActor* pOwner);
	virtual void FinishRipOff();
	void ResetState();

	EntityId m_vehicleId;
	IVehicleSeat* m_pOwnerSeat; // owner seat of the weapon
	IVehicleSeat* m_pSeatUser; // seat of the weapons user

private:

	void CorrectRipperEntityPosition(float timeStep);

	Quat	m_previousVehicleRotation;
	Vec3    m_previousWSpaceOffsetPosition; 
	Vec3	m_localRipUserOffset; 
	float	m_dtWaterLevelCheck;
	bool	m_usedThisFrame; //Stop this item being used multiple times in a single frame (As you end up exiting then re-entering)
};

#endif // _VEHICLEHMG_H_
