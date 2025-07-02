// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Helper class to setup / update ik torso aim for first person 

-------------------------------------------------------------------------
История:
- 20-8-2009		Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Game/IKTorsoAim_Helper.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>

#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/WeaponFPAiming.h>

CIKTorsoAim_Helper::CIKTorsoAim_Helper()
: m_initialized(false)
, m_enabled(false)
, m_blendFactor(0.0f)
, m_blendFactorPosition(0.0f)
{
	DrxCreateClassInstance<CIKTorsoAim>("AnimationPoseModifier_IKTorsoAim", m_ikTorsoAim); 
	DrxCreateClassInstance<ITransformationPinning>("AnimationPoseModifier_TransformationPin", m_transformationPin); 
}

void CIKTorsoAim_Helper::Update( CIKTorsoAim_Helper::SIKTorsoParams& ikTorsoParams )
{
	if (!m_initialized)
	{
		Init(ikTorsoParams);
	}

	i32k  STAPLayer	= GetGameConstCVar(g_stapLayer);

	const bool justTurnedOn = (m_blendFactor == 0.0f) && m_enabled;
	const float frameTime = gEnv->pTimer->GetFrameTime();
	const float delta = (frameTime * ikTorsoParams.blendRate);
	const float newBlendFactor = m_enabled ? m_blendFactor + delta : m_blendFactor - delta;
	m_blendFactor = clamp_tpl(newBlendFactor, 0.0f, 1.0f);
	const bool blendPosition = ikTorsoParams.needsSTAPPosition;
	if (justTurnedOn)
	{
		m_blendFactorPosition = blendPosition ? 1.0f : 0.0f;
	}
	else
	{
		const float newBlendFactorPos = blendPosition ? m_blendFactorPosition + delta : m_blendFactorPosition - delta;
		m_blendFactorPosition = clamp_tpl(newBlendFactorPos, 0.0f, 1.0f);
	}

	//const float XPOS = 200.0f;
	//const float YPOS = 110.0f;
	//const float FONT_SIZE = 4.0f;
	//const float FONT_COLOUR[4] = {1,1,1,1};
	//gEnv->pRenderer->Draw2dLabel(XPOS, YPOS, FONT_SIZE, FONT_COLOUR, false, "CIKTorsoAim_Helper::Update: %s", m_blendTime > 0.0f ? "update" : "dont update");

	if (m_blendFactor <= 0.0f)
		return;

	CIKTorsoAim *torsoAim = m_ikTorsoAim.get();

	torsoAim->SetBlendWeight(m_blendFactor);
	torsoAim->SetBlendWeightPosition(m_blendFactorPosition);
	torsoAim->SetTargetDirection(ikTorsoParams.aimDirection);
	torsoAim->SetViewOffset(ikTorsoParams.viewOffset);
	torsoAim->SetAbsoluteTargetPosition(ikTorsoParams.targetPosition);

	ikTorsoParams.character->GetISkeletonAnim()->PushPoseModifier(STAPLayer, cryinterface_cast<IAnimationPoseModifier>(m_ikTorsoAim), "IKTorsoAimHelper");

	if (ikTorsoParams.shadowCharacter)
	{
		m_transformationPin->SetBlendWeight(ikTorsoParams.updateTranslationPinning ? m_blendFactor : 0.0f);
		ikTorsoParams.character->GetISkeletonAnim()->PushPoseModifier(15, cryinterface_cast<IAnimationPoseModifier>(m_transformationPin), "IKTorsoAimHelper");
	}
}


void CIKTorsoAim_Helper::Init( CIKTorsoAim_Helper::SIKTorsoParams& ikTorsoParams )
{
	m_initialized = true;

	i32 effectorJoint = ikTorsoParams.effectorJoint;
	if (effectorJoint == -1)
		effectorJoint = ikTorsoParams.character->GetIDefaultSkeleton().GetJointIDByName("Bip01 Camera");

	i32 aimJoint = ikTorsoParams.aimJoint;
	if (aimJoint == -1)
		aimJoint = ikTorsoParams.character->GetIDefaultSkeleton().GetJointIDByName("Bip01 Spine2");

	i32 pinJoint = ikTorsoParams.pinJoint;
	if (pinJoint == -1)
		pinJoint = ikTorsoParams.character->GetIDefaultSkeleton().GetJointIDByName("Bip01 Spine1");

	assert(effectorJoint != -1);
	assert(aimJoint != -1);
	assert(pinJoint != -1);

	m_ikTorsoAim->SetJoints(effectorJoint, aimJoint);

	u32k numWeights = 3;
	f32 weights[numWeights] = {0.4f, 0.75f, 1.0f};
	
	m_ikTorsoAim->SetFeatherWeights(numWeights, weights);

	if (ikTorsoParams.shadowCharacter)
	{
		m_transformationPin->SetSource(ikTorsoParams.shadowCharacter);
		m_transformationPin->SetJoint(pinJoint);
	}
}


void CIKTorsoAim_Helper::Enable(bool snap)
{
	m_enabled = true;
	if (snap)
	{
		m_blendFactor = 1.0f;
	}
}

void CIKTorsoAim_Helper::Disable(bool snap)
{
	m_enabled = false;
	if (snap)
	{
		m_blendFactor = 0.0f;
	}
}

const QuatT &CIKTorsoAim_Helper::GetLastEffectorTransform() const
{
	return m_ikTorsoAim->GetLastProcessedEffector();
}
