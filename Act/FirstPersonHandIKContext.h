// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __FIRST_PERSON_HAND_IK_CONTEXT_H__
#define __FIRST_PERSON_HAND_IK_CONTEXT_H__

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Act/IDrxMannequin.h>

class CFirstPersonHandIKContext : public IProceduralContext
{
private:
	struct SParams
	{
		SParams();
		SParams(IDefaultSkeleton* pIDefaultSkeleton);

		i32 m_weaponTargetIdx;
		i32 m_leftHandTargetIdx;
		i32 m_rightBlendIkIdx;
	};

	CFirstPersonHandIKContext();
	virtual ~CFirstPersonHandIKContext() {}

public:
	PROCEDURAL_CONTEXT(CFirstPersonHandIKContext, "FirstPersonHandIK", "d8a55b34-9caa-4b53-89bc-f1708d565bc3"_drx_guid);

	virtual void Initialize(ICharacterInstance* pCharacterInstance);
	virtual void Finalize();
	virtual void Update(float timePassed) override;

	virtual void SetAimDirection(Vec3 aimDirection);
	virtual void AddRightOffset(QuatT offset);
	virtual void AddLeftOffset(QuatT offset);

private:
	SParams                    m_params;
	IAnimationOperatorQueuePtr m_pPoseModifier;
	ICharacterInstance*        m_pCharacterInstance;

	QuatT                      m_rightOffset;
	QuatT                      m_leftOffset;
	Vec3                       m_aimDirection;
	i32                        m_instanceCount;
};

#endif
