// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MUZZLEEFFECT_H__
#define __MUZZLEEFFECT_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/EntityUtility/EntityEffects.h>

class CWeapon;
class CFireMode;
struct SFireModeParams;



class CMuzzleEffect
{
private:
	struct SMuzzleEffectInfo
	{
		EntityEffects::TAttachedEffectId mfId[2];

		SMuzzleEffectInfo()
		{
			mfId[0] = mfId[1] = 0;
		}

		void GetMemoryUsage(IDrxSizer *pSizer) const{}
	};

public:
	CMuzzleEffect();

	void Initialize(CFireMode* pFireMode);
	void AttachEmitters(CFireMode* pFireMode, i32 barrel);
	void DetachEmitters(CFireMode* pFireMode, i32 barrel);

	void StartFire(CFireMode* pFireMode);
	void StopFire(CFireMode* pFireMode);
	void Shoot(CFireMode* pFireMode, Vec3 potentialTarget, i32 barrel);

	void GetMemoryUsage(IDrxSizer * s) const;

private:
	void MuzzleFlashEffect(CFireMode* pFireMode, i32 barrel);
	bool GetMuzzleFireLocation(const CFireMode* pFireMode, QuatTS* location) const;
	void MuzzleBeamEffect(bool attach, CFireMode* pFireMode);
	void SetBeamTarget(CFireMode* pFireMode, Vec3 potentialTarget);

	std::vector<SMuzzleEffectInfo> m_mfIds;
	u32 m_beamFxId[2];
};


#endif
