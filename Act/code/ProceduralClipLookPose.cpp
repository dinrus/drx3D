// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/Act/Serialization.h>

struct SLookPoseParams : public IProceduralParams
{
	SLookPoseParams()
		: blendTime(1.f)
		, scopeLayer(0)
	{
	}

	virtual void Serialize(Serialization::IArchive& ar) override
	{
		ar(Serialization::Decorators::AnimationName<SAnimRef>(animRef), "Animation", "Animation");
		ar(blendTime, "BlendTime", "Blend Time");
		ar(scopeLayer, "ScopeLayer", "Scope Layer");
	}

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = animRef.c_str();
	}

	SAnimRef animRef;
	float    blendTime;
	u32   scopeLayer;
};

class CProceduralClipLookPose : public TProceduralClip<SLookPoseParams>
{
public:
	CProceduralClipLookPose()
		: m_IKLayer(0)
		, m_token(kInvalidToken)
		, m_paramTargetCrc(0)
	{
	}

	virtual void OnEnter(float blendTime, float duration, const SLookPoseParams& params)
	{
		IF (m_charInstance == NULL, false)
			return;

		m_paramTargetCrc = CCrc32::ComputeLowercase("LookTarget");
		m_token = kInvalidToken;

		Vec3 lookAtTarget;
		if (!GetParam(m_paramTargetCrc, lookAtTarget))
		{
			lookAtTarget = m_entity->GetWorldPos();
			lookAtTarget += m_entity->GetForwardDir() * 10.0f;
		}

		const float smoothTime = params.blendTime;
		u32k ikLayer = m_scope->GetBaseLayer() + params.scopeLayer;
		IAnimationPoseBlenderDir* poseBlenderLook = m_charInstance->GetISkeletonPose()->GetIPoseBlenderLook();
		if (poseBlenderLook)
		{
			poseBlenderLook->SetState(true);
			const bool wasPoseBlenderActive = (0.01f < poseBlenderLook->GetBlend());
			if (!wasPoseBlenderActive)
			{
				poseBlenderLook->SetTarget(lookAtTarget);
			}
			poseBlenderLook->SetPolarCoordinatesSmoothTimeSeconds(smoothTime);
			poseBlenderLook->SetLayer(ikLayer);
			poseBlenderLook->SetFadeInSpeed(blendTime);
		}

		m_IKLayer = ikLayer;

		StartLookAnimation(blendTime);
	}

	virtual void OnExit(float blendTime)
	{
		IF (m_charInstance == NULL, false)
			return;

		IAnimationPoseBlenderDir* poseBlenderLook = m_charInstance->GetISkeletonPose()->GetIPoseBlenderLook();

		m_charInstance->GetISkeletonAnim()->StopAnimationInLayer(m_IKLayer, blendTime);

		StopLookAnimation(blendTime);

		if (poseBlenderLook)
		{
			poseBlenderLook->SetState(false);
			poseBlenderLook->SetFadeOutSpeed(blendTime);
		}

	}

	virtual void Update(float timePassed)
	{
		IF (m_charInstance == NULL, false)
			return;

		Vec3 lookAtTarget;
		if (GetParam(m_paramTargetCrc, lookAtTarget))
		{
			IAnimationPoseBlenderDir* poseBlenderLook = m_charInstance->GetISkeletonPose()->GetIPoseBlenderLook();
			if (poseBlenderLook)
			{
				poseBlenderLook->SetTarget(lookAtTarget);
			}
		}
	}

private:

	void StartLookAnimation(const float blendTime)
	{
		const SLookPoseParams& params = GetParams();
		if (params.animRef.IsEmpty())
		{
			return;
		}

		i32k animID = m_charInstance->GetIAnimationSet()->GetAnimIDByCRC(params.animRef.crc);
		if (animID >= 0)
		{
			assert(animID <= 65535);

			m_token = GetNextToken((u16)animID);

			DrxCharAnimationParams animParams;
			animParams.m_fTransTime = blendTime;
			animParams.m_nLayerID = m_IKLayer;
			animParams.m_nUserToken = m_token;
			animParams.m_nFlags = CA_LOOP_ANIMATION | CA_ALLOW_ANIM_RESTART;
			m_charInstance->GetISkeletonAnim()->StartAnimationById(animID, animParams);
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Couldn't find look animation '%s' for character '%s'", params.animRef.c_str(), m_charInstance->GetFilePath());
		}
	}

	void StopLookAnimation(const float blendTime)
	{
		const SLookPoseParams& params = GetParams();
		if (params.animRef.IsEmpty())
		{
			return;
		}

		DRX_ASSERT(m_token != kInvalidToken);

		ISkeletonAnim& skeletonAnimation = *m_charInstance->GetISkeletonAnim();

		i32k animationCount = skeletonAnimation.GetNumAnimsInFIFO(m_IKLayer);
		for (i32 i = 0; i < animationCount; ++i)
		{
			const CAnimation& animation = skeletonAnimation.GetAnimFromFIFO(m_IKLayer, i);
			u32k animationToken = animation.GetUserToken();
			if (animationToken == m_token)
			{
				const bool isTopAnimation = (i == (animationCount - 1));
				if (isTopAnimation)
				{
					skeletonAnimation.StopAnimationInLayer(m_IKLayer, blendTime);
				}
				// If we found an proc clip look animation not in the top it might be indicative that we're reusing this layer for non look animations,
				// but we can assume that in 99% of cases we're already blending out, since there's a top animation that should be blending in.
				return;
			}
		}
	}

	static u32 GetNextToken(u16k animId)
	{
		static u16 s_currentToken = 0;
		s_currentToken++;

		return ((animId << 16) | s_currentToken);
	}

	const static u32 kInvalidToken = 0xFFFFFFFF;

	u32              m_IKLayer;
	u32              m_token;

	u32              m_paramTargetCrc;
};

REGISTER_PROCEDURAL_CLIP(CProceduralClipLookPose, "LookPose");
