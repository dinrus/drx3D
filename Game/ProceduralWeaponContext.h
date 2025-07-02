// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _PROCEDURAL_WEAPON_CONTEXT_H_
#define _PROCEDURAL_WEAPON_CONTEXT_H_

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Game/ProceduralWeaponAnimation.h>



class CProceduralWeaponAnimationContext : public IProceduralContext
{
private:
	struct SParams
	{
		SParams();
		SParams(IDefaultSkeleton& pSkeleton);

		i32 m_weaponTargetIdx;
		i32 m_leftHandTargetIdx;
		i32 m_rightBlendIkIdx;
	};

public:
	PROCEDURAL_CONTEXT(CProceduralWeaponAnimationContext, "ProceduralWeaponAnimationContext", 0xDF1D02E05F4048A1, 0xBC7759DCC568AA7F);

	virtual void Update(float timePassed) override;
	void SetAimDirection(Vec3 direction);
	void Initialize(IScope* pScope);
	void Finalize();
	CProceduralWeaponAnimation& GetWeaponSway() {return m_weaponSway;}

private:
	SParams m_params;
	IAnimationOperatorQueuePtr m_pPoseModifier;
	IScope* m_pScope;

	CProceduralWeaponAnimation m_weaponSway;
	Vec3 m_aimDirection;
	i32 m_instanceCount;
};



#endif
