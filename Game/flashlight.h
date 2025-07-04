// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef FLASH_LIGHT_H
#define FLASH_LIGHT_H

#pragma once


#include <drx3D/Game/Accessory.h>


class CWeapon;


class CFlashLight : public CAccessory
{
public:
	CFlashLight();
	~CFlashLight();

	virtual void OnAttach(bool attach);
	virtual void OnParentSelect(bool select);
	virtual void OnEnterFirstPerson();
	virtual void OnEnterThirdPerson();

	void ToggleFlashLight();
	static void EnableFlashLight(bool enable);
	static bool IsFlashLightEnabled();

private:
	void EnableLight(bool enable);
	void EnableFogVolume(CWeapon* pWeapon, i32 slot, bool enable);

	CWeapon* GetWeapon() const;

	EntityEffects::TAttachedEffectId m_lightId;
	EntityId m_fogVolume;

	static bool m_lightEnabled;
	static bool m_flashLightEnabled;
};


#endif
