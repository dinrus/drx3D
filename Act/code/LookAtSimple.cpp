// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/LookAtSimple.h>

using namespace AnimPoseModifier;

/*
   CLookAtSimple
 */

DRXREGISTER_CLASS(CLookAtSimple)

//

CLookAtSimple::CLookAtSimple()
{
	m_state.jointId = -1;
	m_state.jointOffsetRelative.zero();
	m_state.targetGlobal.zero();
	m_state.weight = 1.0f;
};

//

bool CLookAtSimple::ValidateJointId(IDefaultSkeleton& rIDefaultSkeleton)
{
	if (m_stateExecute.jointId < 0)
		return false;
	return m_stateExecute.jointId < rIDefaultSkeleton.GetJointCount();
}

// IAnimationPoseModifier

bool CLookAtSimple::Prepare(const SAnimationPoseModifierParams& params)
{
	m_stateExecute = m_state;
	return true;
}

bool CLookAtSimple::Execute(const SAnimationPoseModifierParams& params)
{
	DRX_PROFILE_FUNCTION(PROFILE_ANIMATION);

	const IDefaultSkeleton& rIDefaultSkeleton = params.GetIDefaultSkeleton();
	const QuatT& transformation = params.pPoseData->GetJointAbsolute(m_stateExecute.jointId);

	Vec3 offsetAbsolute = m_stateExecute.jointOffsetRelative * transformation.q;
	Vec3 targetAbsolute = m_stateExecute.targetGlobal - params.location.t;
	Vec3 directionAbsolute = (targetAbsolute - (transformation.t + offsetAbsolute)) * params.location.q;
	directionAbsolute.Normalize();

	params.pPoseData->SetJointAbsoluteO(m_stateExecute.jointId, Quat::CreateNlerp(
	                                      transformation.q, Quat::CreateRotationVDir(directionAbsolute, -gf_PI * 0.5f), m_stateExecute.weight));

	i32 parentJointId = params.GetIDefaultSkeleton().GetJointParentIDByID(m_stateExecute.jointId);
	if (parentJointId < 0)
	{
		params.pPoseData->SetJointRelative(m_stateExecute.jointId, transformation);
		return true;
	}

	params.pPoseData->SetJointRelative(m_stateExecute.jointId,
	                                   params.pPoseData->GetJointAbsolute(parentJointId).GetInverted() * transformation);
	return true;
}

void CLookAtSimple::Synchronize()
{
}
