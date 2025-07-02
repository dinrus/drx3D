// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/SkeletonAnim.h>

#include <drx3D/Animation/CharacterUpr.h>
#include <float.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/SkeletonAnim.h>
#include <drx3D/Animation/Command_Buffer.h>
#include <drx3D/Animation/Command_Commands.h>
#include <drx3D/Animation/ParametricSampler.h>
#include <drx3D/Sys/DrxUnitTest.h>
#include <numeric>

namespace
{

ILINE bool CreateCommands(const CPoseModifierQueue& queue, Command::CBuffer& buffer)
{
	const CPoseModifierQueue::SEntry* pEntries = queue.GetEntries();
	const uint entryCount = queue.GetEntryCount();

	for (uint i = 0; i < entryCount; ++i)
	{
		Command::PoseModifier* pCommand = buffer.CreateCommand<Command::PoseModifier>();
		if (!pCommand)
			return false;

		pCommand->m_TargetBuffer = Command::TargetBuffer;
		pCommand->m_pPoseModifier = pEntries[i].poseModifier.get();
	}

	return true;
}

u32 MergeParametricExamples(u32k numExamples, const f32* const exampleBlendWeights, i16k* const exampleLocalAnimationIds, f32* mergedExampleWeightsOut, i32* mergedExampleIndicesOut)
{
	u32 mergedExamplesCount = 0;
	for (u32 exampleIndex = 0; exampleIndex < numExamples; exampleIndex++)
	{
		const f32 weight = exampleBlendWeights[exampleIndex];
		i16k localAnimationId = exampleLocalAnimationIds[exampleIndex];

		bool found = false;
		for (u32 i = 0; i < mergedExamplesCount; ++i)
		{
			i32k mergedExampleIndex = mergedExampleIndicesOut[i];
			i16k mergedAnimationLocalAnimationId = exampleLocalAnimationIds[mergedExampleIndex];
			if (localAnimationId == mergedAnimationLocalAnimationId)
			{
				mergedExampleWeightsOut[i] += weight;
				found = true;
				break;
			}
		}

		if (!found)
		{
			mergedExampleWeightsOut[mergedExamplesCount] = weight;
			mergedExampleIndicesOut[mergedExamplesCount] = exampleIndex;
			mergedExamplesCount++;
		}
	}

	return mergedExamplesCount;
}

u32 MergeParametricExamples(const SParametricSamplerInternal& parametricSampler, f32 mergedExampleWeightsOut[MAX_LMG_ANIMS], i32 mergedExampleIndicesOut[MAX_LMG_ANIMS])
{
	u32k numExamples = parametricSampler.m_numExamples;
	const f32* const exampleBlendWeights = parametricSampler.m_fBlendWeight;
	i16k* const exampleLocalAnimationIds = parametricSampler.m_nAnimID;

	return MergeParametricExamples(numExamples, exampleBlendWeights, exampleLocalAnimationIds, mergedExampleWeightsOut, mergedExampleIndicesOut);
}
} //endns

DRX_UNIT_TEST(Test_MergeParametricExamples_ContainsZeroWeights)
{
	const f32 exampleBlendWeights[] = { 0.f, 1.f, -1.f, 0.f, 1.f };
	i16k exampleLocalAnimationIds[] = { 22, 44, 44, 22, 44 };
	u32k examplesCount = 5;

	i32 mergedExampleIndices[5];
	f32 mergedExampleWeights[5];
	u32k mergedExampleCount = MergeParametricExamples(examplesCount, exampleBlendWeights, exampleLocalAnimationIds, mergedExampleWeights, mergedExampleIndices);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleCount, 2);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[0], 0);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[1], 1);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[0], 0.f, 0.01f);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[1], 1.f, 0.01f);
}

DRX_UNIT_TEST(Test_MergeParametricExamples_NoZeroWeights)
{
	const f32 exampleBlendWeights[] = { -0.1f, 0.9f, -0.1f, 0.f, 0.3f };
	i16k exampleLocalAnimationIds[] = { 22, 44, 66, 22, 44 };
	u32k examplesCount = 5;

	i32 mergedExampleIndices[5];
	f32 mergedExampleWeights[5];
	u32k mergedExampleCount = MergeParametricExamples(examplesCount, exampleBlendWeights, exampleLocalAnimationIds, mergedExampleWeights, mergedExampleIndices);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleCount, 3);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[0], 0);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[1], 1);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[2], 2);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[0], -0.1f, 0.01f);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[1], 1.2f, 0.01f);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[2], -0.1f, 0.01f);
}

DRX_UNIT_TEST(Test_MergeParametricExamples_WeightsAddToZero)
{
	const f32 exampleBlendWeights[] = { -1.f, 1.f, -1.f, 1.f, 1.f };
	i16k exampleLocalAnimationIds[] = { 22, 44, 44, 22, 44 };
	u32k examplesCount = 5;

	i32 mergedExampleIndices[5];
	f32 mergedExampleWeights[5];
	u32k mergedExampleCount = MergeParametricExamples(examplesCount, exampleBlendWeights, exampleLocalAnimationIds, mergedExampleWeights, mergedExampleIndices);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleCount, 2);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[0], 0);
	DRX_UNIT_TEST_CHECK_EQUAL(mergedExampleIndices[1], 1);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[0], 0.f, 0.01f);
	DRX_UNIT_TEST_CHECK_CLOSE(mergedExampleWeights[1], 1.f, 0.01f);
}

namespace
{
void BaseEvaluationLMG_CheckWeight(const IDefaultSkeleton& defaultSkeleton, const CAnimationSet& animationSet, const CAnimation& animation)
{
	u32k localAnimationId = animation.GetAnimationId();

	const ModelAnimationHeader* const pModelAnimationHeader = animationSet.GetModelAnimationHeader(localAnimationId);
	assert(pModelAnimationHeader);
	assert(pModelAnimationHeader->m_nAssetType == LMG_File);

	const SParametricSamplerInternal* const pParametric = static_cast<const SParametricSamplerInternal*>(animation.GetParametricSampler());
	assert(pParametric);

	const f32 blendWeightSum = std::accumulate(pParametric->m_fBlendWeight, pParametric->m_fBlendWeight + pParametric->m_numExamples, 0.f);
	if (0.09f < fabsf(blendWeightSum - 1.0f))
	{
		tukk const animationFilename = pModelAnimationHeader->GetFilePath();
		tukk const modelFilePath = defaultSkeleton.GetModelFilePath();
		gEnv->pLog->LogError("DinrusXAnimation: Blendspace weights for animation '%s' don't sum up to 1. blendWeightSum: %f. Model: '%s'", animationFilename, blendWeightSum, modelFilePath);
	}

}

void BaseEvaluationLMG_DebugDrawSegmentation(const CCharInstance& characterInstance, const CSkeletonAnim& skeletonAnim, const CAnimationSet& animationSet, const CAnimation& animation)
{
#ifdef BLENDSPACE_VISUALIZATION
	i32k debugSegmentation = Console::GetInst().ca_DebugSegmentation;
	if (debugSegmentation <= 0)
	{
		return;
	}

	u32k localAnimationId = animation.GetAnimationId();

	const ModelAnimationHeader* const pModelAnimationHeader = animationSet.GetModelAnimationHeader(localAnimationId);
	assert(pModelAnimationHeader);
	assert(pModelAnimationHeader->m_nAssetType == LMG_File);

	const SParametricSamplerInternal* const pParametric = static_cast<const SParametricSamplerInternal*>(animation.GetParametricSampler());
	assert(pParametric);

	const bool displayParameterInfo = (2 < debugSegmentation);
	const bool displayDetailedSegmentationInfo = (1 < debugSegmentation || displayParameterInfo);
	const bool displayGenericSegmentationInfo = (characterInstance.GetCharEditMode() || displayDetailedSegmentationInfo);

	if (displayGenericSegmentationInfo)
	{
		const float fColor0[4] = { 1, 0.5f, 0, 1 };
		tukk const animationName = pModelAnimationHeader->GetAnimName();
		const f32 normalizedTime = skeletonAnim.GetAnimationNormalizedTime(&animation);
		const int8 currentSegmentIndex = animation.GetCurrentSegmentIndex();
		const f32 segmentNormalizedTime = animation.GetCurrentSegmentNormalizedTime();
		g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.5f, fColor0, false, "[BSpaceTime: %f SegCount: %d] rAnimTime: %f BSpaceID: %d AName: %s", segmentNormalizedTime, currentSegmentIndex, normalizedTime, localAnimationId, animationName);
		g_YLine += 13;
	}

	if (displayDetailedSegmentationInfo)
	{
		for (u32 i = 0; i < pParametric->m_numExamples; i++)
		{
			const f32 weight = pParametric->m_fBlendWeight[i];
			u32k nID = pParametric->m_nAnimID[i];
			i32k nGlobalID = animationSet.GetGlobalIDByAnimID_Fast(pParametric->m_nAnimID[i]);
			i32k segcount = pParametric->m_nSegmentCounter[0][i];
			const f32 segmentNormalizedTime = animation.GetCurrentSegmentNormalizedTime();
			const ModelAnimationHeader* const pAnimExample = animationSet.GetModelAnimationHeader(nID);
			if (pAnimExample)
			{
				tukk const animationName = pAnimExample->GetAnimName();
				float fColor1[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

				if (pParametric->m_fPlaybackScale[i] == 1.0f)
				{
					fColor1[0] = 1;
					fColor1[1] = 1;
					fColor1[2] = 1;
				}
				else
				{
					fColor1[0] = 0;
					fColor1[1] = 1;
					fColor1[2] = 0;
				}

				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor1, false, "%02d - [rAnimTime: %f SegCount: %d] Weight:%f nAnimID: %d AName: %s", i, segmentNormalizedTime, segcount, weight, nID, animationName);
				g_YLine += 12;
			}
		}
	}

	if (displayParameterInfo)
	{
		u32k globalAnimationId = pModelAnimationHeader->m_nGlobalAnimId;
		const GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[globalAnimationId];
		u32k numParameter = rLMG.m_arrParameter.size();
		for (u32 i = pParametric->m_numExamples; i < numParameter; i++)
		{
			i32k i0 = rLMG.m_arrParameter[i].i0;
			i32k i1 = rLMG.m_arrParameter[i].i1;
			tukk const pAnimName0 = rLMG.m_arrParameter[i0].m_animName.GetName_DEBUG();
			tukk const pAnimName1 = rLMG.m_arrParameter[i1].m_animName.GetName_DEBUG();

			const ModelAnimationHeader* const pAnimExample0 = animationSet.GetModelAnimationHeader(i0);
			const ModelAnimationHeader* const pAnimExample1 = animationSet.GetModelAnimationHeader(i1);
			if (pAnimExample0 && pAnimExample1)
			{
				const float fColor1[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor1, false, "%02d - AName0: %s AName1: %s", i, pAnimName0, pAnimName1);
				g_YLine += 12;
			}
		}
	}
#endif
}
}

CSkeletonAnim::EFillCommandBufferResult CSkeletonAnim::FillCommandBuffer(const QuatTS& location, Command::CBuffer& buffer)
{
	DEFINE_PROFILER_FUNCTION();

	CCharInstance* __restrict pInstance = m_pInstance;
	CSkeletonPose* __restrict pSkeletonPose = m_pSkeletonPose;

	CAnimationSet* pAnimationSet = pInstance->m_pDefaultSkeleton->m_pAnimationSet;

	pSkeletonPose->m_feetLock.Reset();

	buffer.Initialize(pInstance, location);

	g_pCharacterUpr->g_AnimationUpdates++;

	//compute number of animation in this layer
	const DynArray<CAnimation>& rCurLayer = m_layers[0].m_transitionQueue.m_animations;
	u32 numAnimsInLayer = rCurLayer.size();
	u32 numActiveAnims = 0;
	for (u32 a = 0; a < numAnimsInLayer; a++)
	{
		if (!rCurLayer[a].IsActivated())
			break;
		numActiveAnims++;
	}

	SAnimationPoseModifierParams poseModifierParams;
	poseModifierParams.pCharacterInstance = pInstance;
	Skeleton::CPoseData* pPoseData = pSkeletonPose->GetPoseDataWriteable();
	poseModifierParams.pPoseData = pPoseData;
	float fInstanceDeltaTime = pInstance->m_fDeltaTime;
	if (fInstanceDeltaTime != 0.0f)
		poseModifierParams.timeDelta = fInstanceDeltaTime;
	else
		poseModifierParams.timeDelta = pInstance->m_fOriginalDeltaTime;
	poseModifierParams.location = location;

	f32 allowMultiLayerAnimationWeight = numActiveAnims ? 0.0f : 1.0f;

	bool useFeetLocking = false;
	if (numActiveAnims)
	{
		f32 pfUserData[NUM_ANIMATION_USER_DATA_SLOTS] = { 0.0f };
		for (u32 a = 0; a < numActiveAnims; a++)
		{
			const CAnimation& rCurAnimation = rCurLayer[a];

			const f32 fTransitionWeight = rCurAnimation.GetTransitionWeight();

			const bool currentAnimationAllowMultiLayer = ((rCurAnimation.m_nStaticFlags & CA_DISABLE_MULTILAYER) == 0);
			const f32 currentAnimationAllowMultilayerWeight = f32(currentAnimationAllowMultiLayer);
			allowMultiLayerAnimationWeight += currentAnimationAllowMultilayerWeight * fTransitionWeight;

			const f32* __restrict pSrc = &rCurAnimation.m_fUserData[0];

			for (i32 i = 0; i < NUM_ANIMATION_USER_DATA_SLOTS; i++)
				pfUserData[i] += pSrc[i] * fTransitionWeight;
		}

		memcpy(&m_fUserData[0], pfUserData, NUM_ANIMATION_USER_DATA_SLOTS * sizeof(f32));

		if (pSkeletonPose->m_bFullSkeletonUpdate == 0)
			return EFillCommandBufferResult::NoAnimationPlaying;

		Command::ClearPoseBuffer* clearbuffer = buffer.CreateCommand<Command::ClearPoseBuffer>();
		clearbuffer->m_TargetBuffer = Command::TargetBuffer;
		clearbuffer->m_nJointStatus = 0;
		clearbuffer->m_nPoseInit = 0;

		for (u32 i = 0; i < numActiveAnims; i++)
		{
			if (rCurLayer[i].GetParametricSampler() == NULL)
				Commands_BasePlayback(rCurLayer[i], buffer);
			else
				Commands_BaseEvaluationLMG(rCurLayer[i], Command::TargetBuffer, buffer);
		}

		Command::NormalizeFull* norm = buffer.CreateCommand<Command::NormalizeFull>();
		norm->m_TargetBuffer = Command::TargetBuffer;

		CreateCommands(m_layers[0].m_poseModifierQueue, buffer);

		// Store feet position
		u32 upperLayersUsageCounter = 0;
		for (u32 i = 1; i < numVIRTUALLAYERS; i++)
		{
			upperLayersUsageCounter += m_layers[i].m_transitionQueue.m_animations.size();
			upperLayersUsageCounter += m_layers[i].m_poseModifierQueue.GetEntryCount();
		}
		const bool feetLockingEnabled = (Console::GetInst().ca_LockFeetWithIK != 0);
		const bool playingAnimationsInLayer0 = (0 < numActiveAnims);
		const bool usingUpperLayers = (0 < upperLayersUsageCounter);
		useFeetLocking = (feetLockingEnabled && playingAnimationsInLayer0 && usingUpperLayers);
		if (useFeetLocking)
		{
			Command::PoseModifier* ac = buffer.CreateCommand<Command::PoseModifier>();
			ac->m_TargetBuffer = Command::TargetBuffer;
			ac->m_pPoseModifier = pSkeletonPose->m_feetLock.Store();
		}
	}

	const f32 upperLayersWeightFactor = allowMultiLayerAnimationWeight;
	const bool needToCalculateUpperLayerAnimations = (0.f < upperLayersWeightFactor);
	if (needToCalculateUpperLayerAnimations)
	{
		for (u32 i = 1; i < numVIRTUALLAYERS; i++)
		{
			CreateCommands_AnimationsInUpperLayer(i, pAnimationSet, upperLayersWeightFactor, buffer);
			CreateCommands(m_layers[i].m_poseModifierQueue, buffer);
		}
	}
	else
	{
		for (u32 i = 1; i < numVIRTUALLAYERS; i++)
		{
			CreateCommands(m_layers[i].m_poseModifierQueue, buffer);
		}
	}

	if (useFeetLocking)
	{
		Command::PoseModifier* ac = buffer.CreateCommand<Command::PoseModifier>();
		ac->m_TargetBuffer = Command::TargetBuffer;
		ac->m_pPoseModifier = pSkeletonPose->m_feetLock.Restore();
	}

	EFillCommandBufferResult result = EFillCommandBufferResult::NoAnimationPlaying;

	// we check the command count here to fully comply to the old logic.
	// this should be simplified as soon as the full implications are considered
	if (buffer.GetCommandCount() > 0)
		result = EFillCommandBufferResult::AnimationPlaying;

	buffer.CreateCommand<Command::ComputeAbsolutePose>();
	buffer.CreateCommand<Command::ProcessAnimationDrivenIk>();
	buffer.CreateCommand<Command::PhysicsSync>();

	// NOTE: Temporary layer -1 PoseModifier queue to allow post-physics sync
	// PoseModifiers to be executed. This should not be needed once the
	// physics sync itself will become a PoseModifier.
	const CPoseModifierQueue& poseModifierQueue = m_pInstance->m_SkeletonAnim.m_poseModifierQueue;
	const CPoseModifierQueue::SEntry* pEntries = poseModifierQueue.GetEntries();
	uint entryCount = poseModifierQueue.GetEntryCount();

	for (uint i = 0; i < entryCount; ++i, ++pEntries)
	{
		Command::PoseModifier* cmd = buffer.CreateCommand<Command::PoseModifier>();
		cmd->m_pPoseModifier = pEntries->poseModifier.get();
	}

	if (m_pSkeletonPose->m_bInstanceVisible)
		m_pInstance->m_AttachmentUpr.CreateCommands(buffer);

	return result;
}

////////////////////////////////////////////////////////////////////////////
//                    playback of one single animation                    //
////////////////////////////////////////////////////////////////////////////
void CSkeletonAnim::Commands_BasePlayback(const CAnimation& rAnim, Command::CBuffer& buffer)
{
	CAnimationSet* pAnimationSet = m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
	u32 nAnimID = rAnim.GetAnimationId();
	u32 nSampleRateHZ30 = rAnim.m_nStaticFlags & CA_KEYFRAME_SAMPLE_30Hz;

	const ModelAnimationHeader* pMAG = pAnimationSet->GetModelAnimationHeader(nAnimID);
	assert(pMAG);
	i32 nEGlobalID = pMAG->m_nGlobalAnimId;
	if (pMAG->m_nAssetType == CAF_File)
	{
		const GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[nEGlobalID];

		Command::SampleAddAnimFull* ac = buffer.CreateCommand<Command::SampleAddAnimFull>();
		ac->m_nEAnimID = nAnimID;
		ac->m_flags = 0;
		ac->m_flags |= m_AnimationDrivenMotion ? Command::SampleAddAnimFull::Flag_ADMotion : 0;
		ac->m_fWeight = rAnim.GetTransitionWeight();

		f32 time_new0 = rAnim.GetCurrentSegmentNormalizedTime();
		i32 segtotal = rCAF.m_Segments - 1;
		i32 segcount = rAnim.m_currentSegmentIndex[0];
		f32 segdur = rCAF.GetSegmentDuration(segcount);
		f32 totdur = rCAF.GetTotalDuration();
		f32 segbase = rCAF.m_SegmentsTime[segcount];
		f32 percent = segdur / totdur;
		f32 time_new = time_new0 * percent + segbase;
		ac->m_fETimeNew = time_new; // this is a percentage value between 0-1
		if (nSampleRateHZ30)
		{
			f32 fKeys = totdur * ANIMATION_30Hz;
			f32 fKeyTime = time_new * fKeys;
			ac->m_fETimeNew = f32(u32(fKeyTime + 0.45f)) / fKeys;
		}

#ifdef BLENDSPACE_VISUALIZATION
		if (Console::GetInst().ca_DebugSegmentation && m_pInstance->GetCharEditMode())
		{
			u8 s2 = rAnim.GetCurrentSegmentIndex();
			tukk aname = pMAG->GetAnimName();
			float fColor1[4] = { 1, 0, 0, 1 };
			g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.3f, fColor1, false, "[rAnimTime: %f segcount: %d] example %d  aname: %s  %d", rAnim.m_fAnimTime[0], segcount, nAnimID, aname, s2);
			g_YLine += 10;
		}
#endif

	}
	if (pMAG->m_nAssetType == AIM_File)
	{
		const GlobalAnimationHeaderAIM& rAIM = g_AnimationUpr.m_arrGlobalAIM[nEGlobalID];
		Command::SampleAddPoseFull* ac = buffer.CreateCommand<Command::SampleAddPoseFull>();
		ac->m_nEAnimID = nAnimID;
		ac->m_flags = 0;
		ac->m_fWeight = rAnim.GetTransitionWeight();
		ac->m_fETimeNew = rAnim.GetCurrentSegmentNormalizedTime(); //this is a percentage value between 0-1
		f32 fDuration = max(1.0f / ANIMATION_30Hz, rAIM.m_fTotalDuration);
		f32 fKeys = fDuration * ANIMATION_30Hz;
		f32 fKeyTime = ac->m_fETimeNew * fKeys;
		ac->m_fETimeNew = f32(u32(fKeyTime + 0.45f)) / fKeys;
		assert(ac->m_fETimeNew >= 0.0f && ac->m_fETimeNew <= 1.0f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//                       evaluation of a locomotion group                                //
///////////////////////////////////////////////////////////////////////////////////////////
void CreateCommandsForLmgAnimation(const bool animationDrivenMotion, const CAnimationSet& animationSet, u32k nTargetBuffer, const CAnimation& animation, Command::CBuffer& buffer)
{
	const f32 animationTransitionAndPlaybackWeight = animation.GetTransitionWeight() * animation.GetPlaybackWeight();
	if (animationTransitionAndPlaybackWeight <= 0.f)
	{
		return;
	}

	u32k localAnimationId = animation.GetAnimationId();

	const ModelAnimationHeader* const pModelAnimationHeader = animationSet.GetModelAnimationHeader(localAnimationId);
	assert(pModelAnimationHeader);
	assert(pModelAnimationHeader->m_nAssetType == LMG_File);

	u32k globalAnimationId = pModelAnimationHeader->m_nGlobalAnimId;
	const GlobalAnimationHeaderLMG& animationGlobalAnimationHeaderLMG = g_AnimationUpr.m_arrGlobalLMG[globalAnimationId];

	const DynArray<u32>& jointList = animationGlobalAnimationHeaderLMG.m_jointList;
	const bool hasJointList = !jointList.empty();
	if (hasJointList)
	{
		Command::JointMask* pCommand = buffer.CreateCommand<Command::JointMask>();
		pCommand->m_pMask = &jointList[0];
		pCommand->m_count = jointList.size();
	}

	{
		const SParametricSamplerInternal* const pParametricSampler = static_cast<const SParametricSamplerInternal*>(animation.GetParametricSampler());
		assert(pParametricSampler);

		f32 mergedExampleWeights[MAX_LMG_ANIMS];
		i32 mergedExampleIndices[MAX_LMG_ANIMS];
		u32k mergedExamplesCount = MergeParametricExamples(*pParametricSampler, mergedExampleWeights, mergedExampleIndices);

		for (u32 i = 0; i < mergedExamplesCount; i++)
		{
			const f32 exampleWeight = mergedExampleWeights[i];
			const float finalExampleWeight = exampleWeight * animationTransitionAndPlaybackWeight;
			if (finalExampleWeight == 0.f)
			{
				continue;
			}

			i32k exampleIndex = mergedExampleIndices[i];
			i16k exampleLocalAnimationId = pParametricSampler->m_nAnimID[exampleIndex];

			const ModelAnimationHeader* const pExampleModelAnimationHeader = animationSet.GetModelAnimationHeader(exampleLocalAnimationId);
			assert(pExampleModelAnimationHeader);

			i32k exampleGlobalAnimationId = pExampleModelAnimationHeader->m_nGlobalAnimId;

			assert(pExampleModelAnimationHeader->m_nAssetType == CAF_File);
			const GlobalAnimationHeaderCAF& exampleGlobalAnimationHeaderCAF = g_AnimationUpr.m_arrGlobalCAF[exampleGlobalAnimationId];

			const f32 animationNormalizedTime = animation.GetCurrentSegmentNormalizedTime();
			i32k exampleAnimationCurrentSegmentIndex = pParametricSampler->m_nSegmentCounter[0][exampleIndex];

			const bool isFullBody = nTargetBuffer == Command::TargetBuffer;
			if (isFullBody)
			{
				Command::SampleAddAnimFull* fetch = buffer.CreateCommand<Command::SampleAddAnimFull>();
				fetch->m_nEAnimID = exampleLocalAnimationId;
				fetch->m_fWeight = finalExampleWeight;
				fetch->m_flags = animationDrivenMotion ? Command::SampleAddAnimFull::Flag_ADMotion : 0;
				fetch->m_fETimeNew = exampleGlobalAnimationHeaderCAF.GetNTimeforEntireClip(exampleAnimationCurrentSegmentIndex, animationNormalizedTime);
			}
			else
			{
				Command::SampleAddAnimPart* fetch = buffer.CreateCommand<Command::SampleAddAnimPart>();
				fetch->m_nEAnimID = exampleLocalAnimationId;
				fetch->m_fWeight = finalExampleWeight;
				fetch->m_fAnimTime = exampleGlobalAnimationHeaderCAF.GetNTimeforEntireClip(exampleAnimationCurrentSegmentIndex, animationNormalizedTime);
				fetch->m_TargetBuffer = nTargetBuffer;
			}
		}
	}

	if (hasJointList)
	{
		Command::JointMask* pCommand = buffer.CreateCommand<Command::JointMask>();
		pCommand->m_pMask = NULL;
		pCommand->m_count = 0;
	}
}

void CSkeletonAnim::Commands_BaseEvaluationLMG(const CAnimation& rAnim, u32k nTargetBuffer, Command::CBuffer& buffer)
{
	const bool animationDrivenMotion = (m_AnimationDrivenMotion != 0);
	assert(m_pInstance->m_pDefaultSkeleton->m_pAnimationSet);
	const CAnimationSet& animationSet = *m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
	CreateCommandsForLmgAnimation(animationDrivenMotion, animationSet, nTargetBuffer, rAnim, buffer);

#ifndef _RELEASE
	BaseEvaluationLMG_CheckWeight(*m_pInstance->m_pDefaultSkeleton, animationSet, rAnim);
	BaseEvaluationLMG_DebugDrawSegmentation(*m_pInstance, *this, animationSet, rAnim);
#endif
}

void CSkeletonAnim::CreateCommands_AnimationsInUpperLayer(u32 layerIndex, CAnimationSet* pAnimationSet, const f32 upperLayersWeightFactor, Command::CBuffer& buffer)
{
	u32 nLayerAnims = 0;
	u8 nIsAdditiveAnimation = 0;
	u8 nIsOverrideAnimation = 0;

	//If an animations is active (i.e. reload) it will overwrite the pose-modifier in the same layer
	DynArray<CAnimation>& layer = m_layers[layerIndex].m_transitionQueue.m_animations;
	u32 animCount = layer.size();
	for (u32 j = 0; j < animCount; ++j)
	{
		if (!layer[j].IsActivated())
			break;

		const CAnimation& anim = layer[j];
		const ModelAnimationHeader* pAnim = pAnimationSet->GetModelAnimationHeader(anim.GetAnimationId());
		if (pAnim->m_nAssetType == AIM_File)
			continue;

		if (pAnim->m_nAssetType == CAF_File)
		{
			GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pAnim->m_nGlobalAnimId];
			nIsOverrideAnimation += rCAF.IsAssetAdditive() == 0;
			nIsAdditiveAnimation += rCAF.IsAssetAdditive() != 0;
		}
		else if (pAnim->m_nAssetType == LMG_File)
		{
			if (const SParametricSamplerInternal* const pLmg = (SParametricSamplerInternal*)anim.GetParametricSampler())
			{
				if (const ModelAnimationHeader* const pFirstExample = pAnimationSet->GetModelAnimationHeader(pLmg->m_nAnimID[0]))
				{
					GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pFirstExample->m_nGlobalAnimId];
					nIsOverrideAnimation += rCAF.IsAssetAdditive() == 0;
					nIsAdditiveAnimation += rCAF.IsAssetAdditive() != 0;
				}
			}
		}

		if (nIsOverrideAnimation && nIsAdditiveAnimation)
		{
			//end of chain -> create command to blend a Temp-Buffer with the Base-Layer
			u8 wasAdditiveChain = nIsAdditiveAnimation > nIsOverrideAnimation;
			Command::PerJointBlending* pBlendNode = buffer.CreateCommand<Command::PerJointBlending>();
			pBlendNode->m_SourceBuffer = Command::TmpBuffer;     //accumulated pose (either additive or override)
			pBlendNode->m_TargetBuffer = Command::TargetBuffer;
			pBlendNode->m_BlendMode = wasAdditiveChain;
			nIsAdditiveAnimation = wasAdditiveChain ? 0 : nIsAdditiveAnimation;
			nIsOverrideAnimation = wasAdditiveChain ? nIsOverrideAnimation : 0;
		}

		if (nIsOverrideAnimation == 1 || nIsAdditiveAnimation == 1)
		{
			Command::ClearPoseBuffer* clearbuffer = buffer.CreateCommand<Command::ClearPoseBuffer>();
			clearbuffer->m_TargetBuffer = Command::TmpBuffer;
			clearbuffer->m_nJointStatus = 0;
			clearbuffer->m_nPoseInit = nIsAdditiveAnimation;
		}

		if (anim.GetParametricSampler() != NULL)
		{
			Commands_BaseEvaluationLMG(anim, Command::TmpBuffer, buffer);
		}
		else
		{
			Commands_LPlayback(anim, Command::TmpBuffer, Command::TargetBuffer, layerIndex, upperLayersWeightFactor, buffer);
		}

		if (nIsOverrideAnimation)
			nIsOverrideAnimation = 0x7f;
		if (nIsAdditiveAnimation)
			nIsAdditiveAnimation = 0x7f;

		nLayerAnims++;
	}

	if (nLayerAnims)
	{
		//create command to blend a Temp-Buffer with the Base-Layer
		Command::PerJointBlending* pBlendNode = buffer.CreateCommand<Command::PerJointBlending>();
		pBlendNode->m_SourceBuffer = Command::TmpBuffer;     //accumulated pose (either additive or override)
		pBlendNode->m_TargetBuffer = Command::TargetBuffer;
		pBlendNode->m_BlendMode = nIsAdditiveAnimation;
	}
}

////////////////////////////////////////////////////////////////////////////
//                    playback of one single animation                    //
////////////////////////////////////////////////////////////////////////////
void CSkeletonAnim::Commands_LPlayback(const CAnimation& rAnim, u32 nTargetBuffer, u32 nSourceBuffer, u32 nVLayer, f32 weightFactor, Command::CBuffer& buffer)
{
	i32k nAnimID = rAnim.GetAnimationId();

	assert(m_pInstance->m_pDefaultSkeleton->m_pAnimationSet);
	const CAnimationSet& animationSet = *m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
	const ModelAnimationHeader* const pAnim = animationSet.GetModelAnimationHeader(nAnimID);
	assert(pAnim->m_nAssetType == CAF_File);

	const f32 w0 = rAnim.GetTransitionWeight(); //this is a percentage value between 0-1
	const f32 w1 = rAnim.GetPlaybackWeight();   //this is a percentage value between 0-1
	const f32 w2 = weightFactor;
	const f32 w3 = m_layers[nVLayer].m_transitionQueue.m_fLayerBlendWeight;
	const f32 combinedWeight = w0*w1*w2*w3; //this is a percentage value between 0-1
	
	Command::SampleAddAnimPart* ac = buffer.CreateCommand<Command::SampleAddAnimPart>();
	ac->m_TargetBuffer = nTargetBuffer;
	ac->m_SourceBuffer = nSourceBuffer;
	ac->m_nEAnimID = rAnim.GetAnimationId();
	ac->m_fAnimTime = rAnim.GetCurrentSegmentNormalizedTime();
	ac->m_fWeight = combinedWeight;

#if defined(USE_PROTOTYPE_ABS_BLENDING)
	if (rAnim.m_pJointMask && (rAnim.m_pJointMask->weightList.size() > 0))
	{
		ac->m_maskJointIDs = strided_pointer<i32k>(&rAnim.m_pJointMask->weightList[0].jointID, sizeof(SJointMask::SJointWeight));
		ac->m_maskJointWeights = strided_pointer<const float>(&rAnim.m_pJointMask->weightList[0].weight, sizeof(SJointMask::SJointWeight));
		ac->m_maskNumJoints = rAnim.m_pJointMask->weightList.size();
	}
	else
	{
		ac->m_maskNumJoints = 0;
	}
#endif //!defined(USE_PROTOTYPE_ABS_BLENDING)

	const bool nSampleRateHZ30 = rAnim.HasStaticFlag(CA_KEYFRAME_SAMPLE_30Hz);
	if (nSampleRateHZ30)
	{
		const GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pAnim->m_nGlobalAnimId];
		f32 fDuration = std::max(rCAF.GetTotalDuration(), (1.0f / ANIMATION_30Hz));
		f32 fKeys = fDuration * ANIMATION_30Hz;
		f32 fKeyTime = ac->m_fAnimTime * fKeys;
		ac->m_fAnimTime = f32(u32(fKeyTime + 0.45f)) / fKeys;
		//	float fColor2[4] = {1,0,0,1};
		//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 2.3f, fColor2, false,"fKeys: %f  fKeyTime: %f   m_fETimeNew: %f",fKeys,fKeyTime,ac->m_fAnimTime);
		//	g_YLine+=23;
	}

	m_IsAnimPlaying |= 0xfffe;
}
