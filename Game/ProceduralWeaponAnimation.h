// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _PROCEDURAL_WEAPON_ANIMATION_H_
#define _PROCEDURAL_WEAPON_ANIMATION_H_

#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/Game/WeaponOffset.h>
#include <drx3D/Game/WeaponLookOffset.h>
#include <drx3D/Game/WeaponRecoilOffset.h>
#include <drx3D/Game/WeaponStrafeOffset.h>
#include <drx3D/Game/WeaponZoomOffset.h>
#include <drx3D/Game/WeaponBumpOffset.h>
#include <drx3D/Game/WeaponOffsetInput.h>

#include <drx3D/Animation/IDrxAnimation.h>

class CPlayer;
class CWeaponOffsetInput;


class CProceduralWeaponAnimation
{
public:
	CProceduralWeaponAnimation();

	void Update(float deltaTime);

	CWeaponZoomOffset& GetZoomOffset() {return m_weaponZoomOffset;}
	CLookOffset& GetLookOffset() {return m_lookOffset;}
	CStrafeOffset& GetStrafeOffset() {return m_strafeOffset;}
	CRecoilOffset& GetRecoilOffset() {return m_recoilOffset;}
	CBumpOffset& GetBumpOffset() {return m_bumpOffset;}
	void AddCustomOffset(const QuatT& offset);

	QuatT GetRightOffset() const {return m_rightOffset;}
	QuatT GetLeftOffset() const {return m_leftOffset;}

private:
	void UpdateDebugState();
	void ComputeOffsets(float deltaTime);
	void ResetCustomOffset();

	CWeaponZoomOffset m_weaponZoomOffset;
	CLookOffset m_lookOffset;
	CStrafeOffset m_strafeOffset;
	CRecoilOffset m_recoilOffset;
	CBumpOffset m_bumpOffset;
	QuatT m_customOffset;

	QuatT m_rightOffset;
	QuatT m_leftOffset;

	CWeaponOffsetInput::TWeaponOffsetInput m_weaponOffsetInput;
	bool m_debugInput;
};



#endif
