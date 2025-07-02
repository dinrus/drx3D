// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/SkeletonPose.h>
#include<drx3D/Animation/FacialModel.h>
#include <drx3D/Animation/TransitionQueue.h>
#include <drx3D/Animation/PoseModifier.h>

class CSkeletonAnim;
class CSkeletonPose;
class CCharInstance;
struct ModelAnimationHeader;

namespace CharacterInstanceProcessing
{
struct SContext;
}

namespace Command
{
class CBuffer;
}

class CLayer
{
public:
	CTransitionQueue   m_transitionQueue;
	CPoseModifierQueue m_poseModifierQueue;
};

#define numVIRTUALLAYERS (ISkeletonAnim::LayerCount)

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
class DRX_ALIGN(128) CSkeletonAnim: public ISkeletonAnim
{
private:
	friend class CSkeletonAnimTask;

public:
	CSkeletonAnim();
	~CSkeletonAnim();

	CFacialDisplaceInfo m_facialDisplaceInfo;

	size_t SizeOfThis();
	void GetMemoryUsage(IDrxSizer * pSizer) const;
	void Serialize(TSerialize ser);

	void SetAnimationDrivenMotion(u32 ts);
	void SetMirrorAnimation(u32 ts);
	u32 GetAnimationDrivenMotion() const { return m_AnimationDrivenMotion; };

	void SetTrackViewExclusive(u32 i);
	void SetTrackViewMixingWeight(u32 layer, f32 weight);

	u32 GetTrackViewStatus() const { return m_TrackViewExclusive; };

	virtual float CalculateCompleteBlendSpaceDuration(const CAnimation &rAnimation) const;

	virtual void SetAnimationNormalizedTime(CAnimation * pAnimation, f32 normalizedTime, bool entireClip = true);
	virtual f32 GetAnimationNormalizedTime(u32 nLayer, u32 num) const;
	virtual f32 GetAnimationNormalizedTime(const CAnimation * pAnimation) const;

	void SetLayerNormalizedTimeAndSegment(f32 normalizedTime, i32 nEOC, i32 nAnimID0, i32 nAnimID1, u8 nSegment0, u8 nSegment1);
	virtual void SetLayerNormalizedTime(u32 layer, f32 normalizedTime);
	virtual f32 GetLayerNormalizedTime(u32 layer) const;

	// sets the animation speed scale for layers
	void SetLayerPlaybackScale(i32 nLayer, f32 fSpeed);
	f32 GetLayerPlaybackScale(u32 nLayer) const
	{
		if (nLayer < numVIRTUALLAYERS)
			return m_layers[nLayer].m_transitionQueue.m_fLayerPlaybackScale;
		return 1.0f;
	}

	f32 GetLayerBlendWeight(i32 nLayer);
	void SetLayerBlendWeight(i32 nLayer, f32 fMult);

	ILINE u8 GetActiveLayer(u8 layer)            { return m_layers[layer].m_transitionQueue.m_bActive; }
	ILINE void  SetActiveLayer(u8 layer, u8 val) { m_layers[layer].m_transitionQueue.m_bActive = val ? true : false; }

	virtual void SetDesiredMotionParam(EMotionParamID id, float value, float deltaTime);
	virtual bool GetDesiredMotionParam(EMotionParamID id, float& value) const;

	void ProcessAnimations(const QuatTS &rAnimCharLocationCurr);
	u32 BlendUpr(f32 deltatime, DynArray<CAnimation> &arrLayer, u32 nLayer);
	void BlendUprDebug();
	u32 BlendUprDebug(DynArray<CAnimation> &arrLayer, u32 nLayer);
	void LayerBlendUpr(f32 fDeltaTime, u32 nLayer);
	void DebugLogTransitionQueueState();

	CLayer m_layers[numVIRTUALLAYERS];
	CPoseModifierQueue m_poseModifierQueue;
	CPoseModifierSetupPtr m_pPoseModifierSetup;

	i32 (* m_pEventCallback)(ICharacterInstance*, uk );
	uk m_pEventCallbackData;
	void SetEventCallback(CallBackFuncType func, uk pdata)
	{
		m_pEventCallback = func;
		m_pEventCallbackData = pdata;
	}
	AnimEventInstance GetLastAnimEvent() { return m_LastAnimEvent; };

	void AnimCallback(CAnimation & arrAFIFO);
	void AnimCallbackInternal(bool sendAnimEventsForTimeOld, f32 normalizedTimeOld, f32 normalizedTimeNew, struct SAnimCallbackParams& params);

	AnimEventInstance m_LastAnimEvent;

	u32 m_IsAnimPlaying;

	u8 m_AnimationDrivenMotion;
	u8 m_ShowDebugText;
	u16 m_MirrorAnimation;

	void SetDebugging(u32 debugFlags);

	bool m_TrackViewExclusive : 1;

	Vec3 GetCurrentVelocity() const;
	const QuatT& GetRelMovement() const;
	f32          GetUserData(i32 i) const { return m_fUserData[i]; }
	void InitSkeletonAnim(CCharInstance * pInstance, CSkeletonPose * pSkeletonPose);
	void ProcessAnimationUpdate(const QuatTS rAnimLocationCurr);
	void FinishAnimationComputations();

	f32 m_fUserData[NUM_ANIMATION_USER_DATA_SLOTS];

	CSkeletonPose* m_pSkeletonPose;
	CCharInstance* m_pInstance;

	bool m_bTimeUpdated;

	mutable bool m_bCachedRelativeMovementValid; // whether or not the m_cachedRelativeMovement is valid
	mutable QuatT m_cachedRelativeMovement;

	IAnimationPoseModifierPtr m_transformPinningPoseModifier;

	IAnimationPoseModifierSetupPtr      GetPoseModifierSetup()       { return m_pPoseModifierSetup; }
	IAnimationPoseModifierSetupConstPtr GetPoseModifierSetup() const { return m_pPoseModifierSetup; }

	bool PushPoseModifier(u32 layer, IAnimationPoseModifierPtr poseModifier, tukk name);

	void PoseModifiersPrepare(const QuatTS &location);
	void PoseModifiersExecutePost(Skeleton::CPoseData & poseData, const QuatTS &location);
	void PoseModifiersSynchronize();
	void PoseModifiersSwapBuffersAndClearActive();

	// Interface
public:
	bool StartAnimation(tukk szAnimName0, const DrxCharAnimationParams &Params);
	bool StartAnimationById(i32 id, const DrxCharAnimationParams &Params);
	bool StopAnimationInLayer(i32 nLayer, f32 BlendOutTime);
	bool StopAnimationsAllLayers();

	CAnimation*       FindAnimInFIFO(u32 nUserToken, i32 nLayer = 1);
	const CAnimation* FindAnimInFIFO(u32 nUserToken, i32 nLayer = 1) const;

	// Ported to CTransitionQueue
	i32               GetNumAnimsInFIFO(u32 nLayer) const                                  { return m_layers[nLayer].m_transitionQueue.GetAnimationCount(); }
	CAnimation&       GetAnimFromFIFO(u32 nLayer, u32 num)                              { return m_layers[nLayer].m_transitionQueue.GetAnimation(num); }
	const CAnimation& GetAnimFromFIFO(u32 nLayer, u32 num) const                        { return m_layers[nLayer].m_transitionQueue.GetAnimation(num); }
	bool              RemoveAnimFromFIFO(u32 nLayer, u32 num, bool forceRemove = false) { return m_layers[nLayer].m_transitionQueue.RemoveAnimation(num, forceRemove); }
	void              ClearFIFOLayer(u32 nLayer)                                           { m_layers[nLayer].m_transitionQueue.Clear(); }
	// makes sure there's no anim in this layer's queue that could cause a delay (useful when you want to play an
	// animation that you want to be 100% sure is going to be transitioned to immediately)
	virtual void RemoveTransitionDelayConditions(u32 nLayer)                                           { m_layers[nLayer].m_transitionQueue.RemoveDelayConditions(); }
	void         ManualSeekAnimationInFIFO(u32 nLayer, u32 num, float time, bool triggerAnimEvents) { m_layers[nLayer].m_transitionQueue.ManualSeekAnimation(num, time, triggerAnimEvents, *m_pInstance); }

	virtual QuatT CalculateRelativeMovement(const float deltaTime, const bool CurrNext = 0) const;

	enum class EFillCommandBufferResult
	{
		AnimationPlaying, NoAnimationPlaying
	};

	EFillCommandBufferResult FillCommandBuffer(const QuatTS &location, Command::CBuffer & buffer);
private:
	u32 AnimationToQueue(const ModelAnimationHeader * pAnim0, i32 a0, f32 btime, const DrxCharAnimationParams &AnimParams);
	u32 EvaluateTransitionFlags(CAnimation * arrAnimFiFo, u32 numAnims);

	u32 IsAnimationInMemory(CAnimationSet * pAnimationSet, CAnimation * pAnimation);
	void UpdateParameters(CAnimation * arrAnimFiFo, u32 nMaxActiveInQueue, u32 nLayer, f32 fDeltaTime);

	// Ported to CTransitionQueue
	void AppendAnimationToQueue(i32 nLayer, const CAnimation& rAnim) { m_layers[nLayer].m_transitionQueue.PushAnimation(rAnim); }
	void UpdateAnimationTime(CAnimation & rAnimation, u32 nLayer, u32 NumAnimsInQueue, u32 AnimNo, u32 idx);
	u32 CheckIsCAFLoaded(CAnimationSet * pAnimationSet, i32 nAnimID);
	u32 GetMaxSegments(const CAnimation &rAnimation) const;

public:
	SParametricSamplerInternal* AllocateRuntimeParametric();

#ifdef EDITOR_PCDEBUGCODE
	bool ExportHTRAndICAF(tukk szAnimationName, tukk savePath) const;
	bool ExportVGrid(tukk szAnimationName) const;
#endif
private:
	void CreateCommands_AnimationsInUpperLayer(u32 layerIndex, CAnimationSet * pAnimationSet, const f32 upperLayersWeightFactor, Command::CBuffer & buffer);

	void Commands_BasePlayback(const CAnimation &rAnim, Command::CBuffer & buffer);
	void Commands_BaseEvaluationLMG(const CAnimation &rAnim, u32 nTargetBuffer, Command::CBuffer & buffer);
	void Commands_LPlayback(const CAnimation &rAnim, u32 nTargetBuffer, u32 nSourceBuffer, u32 nVLayer, f32 weightFactor, Command::CBuffer & buffer);

	void ParseLayer0(const CAnimation &rAnim, struct AnimInfo* ainfo, u32 & acounter, f32 fDeltaTime, u32k idx) const;
	void Extract_DeltaMovement(struct AnimInfo* pAInfo, u32& acounter, struct SDeltaMotion* pDeltaMotion) const;
	void GetOP_CubicInterpolation(IController * pRootController, u32 IsCycle, f32 fStartKey, f32 fNumKeys, f32 fKeyTime, Quat & rot, Vec3 & pos) const;
	IController* GetRootController(GlobalAnimationHeaderCAF& rCAF) const;
};
