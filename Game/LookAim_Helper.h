// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   LookAim_Helper.h
//  Version:     v1.00
//  Created:     25/01/2010 by Sven Van Soom
//  Описание: Helper class for setting up and updating Looking and Aiming
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _LOOKAIM_HELPER_H_
#define _LOOKAIM_HELPER_H_

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/LookAtSimple.h>

class CPlayer;


class CLookAim_Helper
{
public:
	CLookAim_Helper();

	void UpdateLook(CPlayer* pPlayer, ICharacterInstance* pCharacter, bool bEnabled, f32 FOV, const Vec3& LookAtTarget, u32k lookIKLayer);
	void UpdateDynamicAimPoses(CPlayer* pPlayer, ICharacterInstance* pCharacter, const struct SActorParams& params, const bool aimEnabled, i32k aimIKLayer, const Vec3& vAimTarget, const struct SMovementState& curMovementState);

	void Reset()
	{
		m_initialized = false;
	}

	bool CanHandFire(i32 hand) const { return 0 != (m_availableHandsForFiring & (1 << hand)); }

private:

	void Init(CPlayer* pPlayer, ICharacterInstance* pCharacter);

	bool m_initialized;

	// Looking
	bool m_canUseLookAtComplex;
	bool m_canUseLookAtSimple;
	std::shared_ptr<AnimPoseModifier::CLookAtSimple> m_lookAtSimple;
	float m_lookAtWeight; // only used for LookAtSimple now as old LookIK also does this
	float m_lookAtFadeInSpeed; // only used for LookAtSimple now as old LookIK also does this
	float m_lookAtFadeOutSpeed; // only used for LookAtSimple now as old LookIK also does this
	Vec3 m_lookAtInterpolatedTargetGlobal; // only used for LookAtSimple now as old LookIK also does this
	Vec3 m_lookAtTargetRate; // for generating smooth motion
	Vec3 m_lookAtTargetGlobal; // only used for LookAtSimple now as old LookIK also does this
	float m_lookAtTargetSmoothTime; // only used for LookAtSimple now as old LookIK also does this

	// Aiming
	i32 m_availableHandsForFiring; // flags where bit i represents hand IItem::eItemHand
	i32 m_lastAimPoseAnimID;
	float m_lastAimYaw;
	bool m_aimIsSwitchingArms;
	Vec3 m_vLastAimTarget;
};

#endif