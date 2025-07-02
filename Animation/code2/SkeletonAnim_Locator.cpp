// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/SkeletonAnim.h>

#include <float.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/CharacterInstance.h>
#include<drx3D/Animation/FacialInstance.h>
#include <drx3D/Animation/Command_Buffer.h>
#include <drx3D/Animation/ControllerOpt.h>
#include <drx3D/Animation/ParametricSampler.h>
#include <drx3D/Animation/DrawHelper.h>

struct SDeltaMotion
{
	f32  m_moveSpeed;
	f32  m_moveDistance;
	f32  m_turnSpeed;
	f32  m_turnDistance;
	f32  m_travelDir;
	f32  m_slope;

	Vec3 m_translation; //just for debugging

	SDeltaMotion()
	{
		m_moveSpeed = 0.0f;
		m_moveDistance = 0.0f;
		m_turnSpeed = 0.0f;
		m_turnDistance = 0.0f;
		m_travelDir = 0.0f;
		m_slope = 0.0f;
		m_translation = Vec3(ZERO);
	}
	~SDeltaMotion() {}
};

namespace LocatorHelper
{
ILINE f32 ExtractMoveSpeed(const QuatT& rel)
{
	return rel.t.GetLength();
}

ILINE f32 ExtractTurnSpeed(const QuatT& rel)
{
	Vec3 v = rel.q.GetColumn1();
	return atan2_tpl(-v.x, v.y);
}

ILINE f32 ExtractTravelDir(const QuatT& rel)
{
	f32 tdir = atan2_tpl(-rel.t.x, rel.t.y);
	if (rel.t.y < 0)
	{
		if (rel.t.x < 0)
			tdir = +gf_PI - tdir;
		else
			tdir = -gf_PI - tdir;
	}
	return tdir;
}

ILINE f32 ExtractSlope(const QuatT& rel)
{
	f32 tdir = atan2_tpl(-rel.t.x, rel.t.y);
	Vec3 v = rel.t * Matrix33::CreateRotationZ(tdir);
	return atan2_tpl(v.z, v.y);
}
}

struct AnimInfo
{
	i16 m_nAnimID;
	i16 m_nEOC;
	f32   m_fAnimTime;
	f32   m_fAnimTimePrev;
	f32   m_fWeight;
	f32   m_fAnimDelta;
	f32   m_fPlaybackScale;

	i32   m_Dimensions;
	int8  m_Parameter[8];         //the parameters that are stored in a ParaGroup
	int8  m_IsPreInitialized[8];  //0=time / 1=distance / -1 pre-initilized (don't extract at all)
	f32   m_PreInitializedVal[8]; //this is coming from te XML-file (no need to evaluete the Locator)
};

//-----------------------------------------------------------------
//----   Evaluate Locator                                      ----
//-----------------------------------------------------------------
QuatT CSkeletonAnim::CalculateRelativeMovement(const f32 fDeltaTimeOrig, const bool CurrNext) const PREFAST_SUPPRESS_WARNING(6262)
{
	if (m_AnimationDrivenMotion == 0)
		return QuatT(IDENTITY);

	const DynArray<CAnimation>& rCurLayer = m_layers[0].m_transitionQueue.m_animations;

	u32k numAnimsInLayer = rCurLayer.size();
	if (numAnimsInLayer == 0)
		return QuatT(IDENTITY);

	u32 numActiveAnims = 0;
	for (u32 a = 0; a < numAnimsInLayer; a++)
	{
		const CAnimation& rAnimation = rCurLayer[a];
		if (!rAnimation.IsActivated())
			break;

		numActiveAnims++;
	}

	if (numActiveAnims == 0)
		return QuatT(IDENTITY);

	const f32 fDeltaTime = fDeltaTimeOrig * m_layers[0].m_transitionQueue.m_fLayerPlaybackScale;

	u32 acounter = 0;
	AnimInfo arrAnimInfo[512];
	for (u32 i = 0; i < numActiveAnims; i++)
	{
		const CAnimation& rAnimation = rCurLayer[i];
		ParseLayer0(rAnimation, arrAnimInfo, acounter, fDeltaTime, CurrNext);
	}

	f32 weightSum = 0.0f;
	for (u32 a = 0; a < acounter; a++)
	{
		AnimInfo& rAnimInfo = arrAnimInfo[a];
		const float weight = rAnimInfo.m_fWeight;
		if (fabsf(weight) < 0.001f)
		{
			rAnimInfo.m_fWeight = 0.0f;
		}
		else
		{
			weightSum += weight;
		}
	}

	if (weightSum < 0.01f)
		return QuatT(IDENTITY);

	for (u32 a = 0; a < acounter; a++)
	{
		AnimInfo& rAnimInfo = arrAnimInfo[a];
		rAnimInfo.m_fWeight /= weightSum;
	}

	SDeltaMotion DeltaMotion;
	Extract_DeltaMovement(&arrAnimInfo[0], acounter, &DeltaMotion);

	f32 fRelMoveSpeed = DeltaMotion.m_moveSpeed + DeltaMotion.m_moveDistance;
	f32 fRelTurnSpeed = DeltaMotion.m_turnSpeed + DeltaMotion.m_turnDistance;
	f32 fRelTravelDir = DeltaMotion.m_travelDir;
	f32 fRelSlope = DeltaMotion.m_slope;
	Vec3 VecTranslation = DeltaMotion.m_translation;
	if (VecTranslation.y < 0)
	{
		if (VecTranslation.x < 0)
			fRelTravelDir = +gf_PI * 0.5f - (fRelTravelDir - gf_PI * 0.5f);
		else
			fRelTravelDir = -gf_PI * 0.5f - (fRelTravelDir + gf_PI * 0.5f);
	}
	Quat q = Quat::CreateRotationZ(fRelTurnSpeed);
	Vec3 t = Quat::CreateRotationZ(fRelTravelDir) * (Quat::CreateRotationX(fRelSlope) * Vec3(0, fRelMoveSpeed, 0));
	//Vec3 t = VecTranslation;

	const QuatT relativeMovement(q, t);
	//	float fColor[4] = {0,1,0,1};
	//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 3.3f, fColor, false,"m_RelativeMovement: %f %f %f",relativeMovement.t.x,relativeMovement.t.y,relativeMovement.t.z );
	//	g_YLine+=36.0f;

	if (Console::GetInst().ca_DrawLocator && CurrNext == 0)
	{
		QuatT location = QuatT(m_pInstance->m_location);
		QuatT offset = relativeMovement.GetInverted();
		if (m_AnimationDrivenMotion)
			offset.t = Vec3(ZERO);

		QuatT BodyLocation = location * offset;
		BodyLocation.t += Vec3(0, 0, 0.0000001f);
		const QuatT* parrDefJoints = m_pInstance->m_pDefaultSkeleton->m_poseDefaultData.GetJointsAbsolute();
		Vec3 vBodyDirection = parrDefJoints[0].q.GetColumn1();
		DrawHelper::Arrow(BodyLocation, vBodyDirection, 1.0f, RGBA8(0x00, 0xff, 0x00, 0x00));

		//In character tool/editor we use the per-instance playback scale to mimic global time changes.
		//In the game this would change the speed of the locator. However, in these tools we want the apparent locator speed to stay the same,
		//so we divide out the scale by using the modified deltatime, not the original."
		f32 fDT = m_pInstance->m_CharEditMode ? m_pInstance->m_fDeltaTime : m_pInstance->m_fOriginalDeltaTime;
		if (fDT)
		{
			f32 fMoveSpeed = DeltaMotion.m_moveSpeed / fDT;
			f32 fTurnSpeed = DeltaMotion.m_turnSpeed / fDT;
			f32 fTravelDir = fRelTravelDir;
			f32 fSlope = DeltaMotion.m_slope;
			DrawHelper::CurvedArrow(BodyLocation, fMoveSpeed, fTravelDir, fTurnSpeed, fSlope, RGBA8(0xff, 0xff, 0x00, 0x00));
			if (Console::GetInst().ca_DrawLocator == 2 || m_pInstance->m_CharEditMode)
			{
				float fColor1[4] = { 1, 1, 1, 1 };
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 2.3f, fColor1, false, "fMoveSpeed: %f  fTurnSpeed: %f  fTravelDir: %f  fSlope: %f (%f)", fMoveSpeed, fTurnSpeed, fTravelDir, fSlope, RAD2DEG(fSlope));
				g_YLine += 26.0f;
			}
		}
	}

	return relativeMovement;
}

void CSkeletonAnim::ParseLayer0(const CAnimation& rAnim, AnimInfo* pAInfo, u32& acounter, f32 fDeltaTime, u32k idx) const
{
	//-----------------------------------------------------------------------------
	//----  implementation of root-priority                             -----------
	//-----------------------------------------------------------------------------
	if (rAnim.HasStaticFlag(CA_FULL_ROOT_PRIORITY))
	{
		if (rAnim.GetTransitionWeight())
		{
			for (u32 a = 0; a < acounter; a++)
				pAInfo[a].m_fWeight = 0;
		}
	}

	const SParametricSamplerInternal* pParametric = (SParametricSamplerInternal*)rAnim.GetParametricSampler();
	if (pParametric == NULL)
	{
		if (fabsf(rAnim.GetTransitionWeight()) < 0.001f)
			return;

		//regular asset
		CAnimationSet* pAnimationSet = m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
		const ModelAnimationHeader* pMAG = pAnimationSet->GetModelAnimationHeader(rAnim.GetAnimationId());
		assert(pMAG);
		if (pMAG->m_nAssetType != CAF_File)
			return;
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pMAG->m_nGlobalAnimId];
		pAInfo[acounter].m_nEOC = 0;
		pAInfo[acounter].m_nAnimID = rAnim.GetAnimationId();
		pAInfo[acounter].m_fWeight = rAnim.GetTransitionWeight();         //this is a percentage value between 0-1
		pAInfo[acounter].m_fPlaybackScale = rAnim.m_fPlaybackScale;
		pAInfo[acounter].m_Dimensions = 0;     //this is a usual CAF

		i32 segcountOld = rAnim.m_currentSegmentIndexPrev[idx];
		pAInfo[acounter].m_fAnimTimePrev = rCAF.GetNTimeforEntireClip(segcountOld, rAnim.m_fAnimTimePrev[idx]); //this is a percentage value between 0-1 for the ENTIRE animation
		i32 segcountNew = rAnim.m_currentSegmentIndex[idx];
		pAInfo[acounter].m_fAnimTime = rCAF.GetNTimeforEntireClip(segcountNew, rAnim.m_fAnimTime[idx]); //this is a percentage value between 0-1 for the ENTIRE animation
		if (rAnim.m_DynFlags[idx] & CA_EOC)
			pAInfo[acounter].m_nEOC = segcountNew == 0;

		const f32 totdur = rCAF.GetTotalDuration();
		const f32 fRealTimeNew = pAInfo[acounter].m_fAnimTime * totdur;
		const f32 fRealTimeOld = pAInfo[acounter].m_fAnimTimePrev * totdur;
		f32 fAnimDelta = fRealTimeNew - fRealTimeOld;
		if (pAInfo[acounter].m_nEOC)
			fAnimDelta = totdur - fRealTimeOld + fRealTimeNew;
		pAInfo[acounter].m_fAnimDelta = fAnimDelta;

		//		float fColor2[4] = {1,1,0,1};
		//		g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.9f, fColor2, false,"fRealTimeNew: %f fDeltaTime: %f fAssetDelta: %f",fRealTimeNew,fDeltaTime,pAInfo[acounter].m_fAnimDelta);
		//		g_YLine+=20;
		acounter++;
	}
	else
	{

		CAnimationSet* pInstModelAnimationSet = m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
		const ModelAnimationHeader* pAnim = pInstModelAnimationSet->GetModelAnimationHeader(rAnim.GetAnimationId());
		if (pAnim->m_nAssetType != LMG_File)
			DrxFatalError("DinrusXAnimation: no VEG"); //can happen only in weird cases (mem-corruption, etc)

		u32 numLMGs = g_AnimationUpr.m_arrGlobalLMG.size();
		if (u32(pAnim->m_nGlobalAnimId) >= numLMGs)
			DrxFatalError("DinrusXAnimation: VEG-ID out of range"); //can happen only in weird cases (mem-corruption, etc)

		GlobalAnimationHeaderLMG& rLMG = g_AnimationUpr.m_arrGlobalLMG[pAnim->m_nGlobalAnimId];
		f32 fBlendWeights = 0;
		u32 numLMGParams = rLMG.m_arrParameter.size();
		for (u32 s = 0; s < pParametric->m_numExamples; s++)
		{
			f32 fWeight = pParametric->m_fBlendWeight[s] * rAnim.GetTransitionWeight();
			fBlendWeights += pParametric->m_fBlendWeight[s]; // only used in the assert below
			if (fabsf(fWeight) < 0.001f)
				continue;

			i32 nAnimID = pParametric->m_nAnimID[s];
			const ModelAnimationHeader* pMAH = pInstModelAnimationSet->GetModelAnimationHeader(nAnimID);
			assert(pMAH);
			assert(pMAH->m_nAssetType == CAF_File);
			GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pMAH->m_nGlobalAnimId];

			pAInfo[acounter].m_nEOC = 0;
			pAInfo[acounter].m_nAnimID = nAnimID;
			pAInfo[acounter].m_fWeight = fWeight;
			pAInfo[acounter].m_fPlaybackScale = rAnim.m_fPlaybackScale * pParametric->m_fPlaybackScale[s];

			pAInfo[acounter].m_Dimensions = rLMG.m_Dimensions;
			for (i32 d = 0; d < rLMG.m_Dimensions; d++)
			{
				pAInfo[acounter].m_Parameter[d] = rLMG.m_DimPara[d].m_ParaID;
				pAInfo[acounter].m_IsPreInitialized[d] = 0; //use delta extraction
				if (numLMGParams >= pParametric->m_numExamples && rLMG.m_arrParameter[s].m_UseDirectlyForDeltaMotion[d])
				{
					//this skips real-time extractions. Use it only for parameters which are constant over the entire motion
					pAInfo[acounter].m_IsPreInitialized[d] = 1;                                 //no delta extraction
					pAInfo[acounter].m_PreInitializedVal[d] = rLMG.m_arrParameter[s].m_Para[d]; //take this value directly
				}
			}

			pAInfo[acounter].m_Dimensions += rLMG.m_ExtractionParams;
			for (i32 d = 0; d < rLMG.m_ExtractionParams; d++)
			{
				pAInfo[acounter].m_Parameter[d + rLMG.m_Dimensions] = rLMG.m_ExtPara[d].m_ParaID;
				pAInfo[acounter].m_IsPreInitialized[d + rLMG.m_Dimensions] = 0; //use delta extraction
			}

			i32 nMaxSegments = rCAF.m_Segments - 1;
			i32 segcountOld = pParametric->m_nSegmentCounterPrev[idx][s];
			i32 segcountNew = pParametric->m_nSegmentCounter[idx][s];
			pAInfo[acounter].m_fAnimTimePrev = rCAF.GetNTimeforEntireClip(segcountOld, rAnim.m_fAnimTimePrev[idx]); //this is a percentage value between 0-1 for the ENTIRE animation
			pAInfo[acounter].m_fAnimTime = rCAF.GetNTimeforEntireClip(segcountNew, rAnim.m_fAnimTime[idx]);         //this is a percentage value between 0-1 for the ENTIRE animation
			if (rAnim.m_DynFlags[idx] & CA_EOC)
			{
				u32 neoc = (rAnim.m_DynFlags[idx] & CA_NEGATIVE_EOC);
				if (neoc != CA_NEGATIVE_EOC && segcountOld == nMaxSegments && segcountNew == 0)
					pAInfo[acounter].m_nEOC = +1; //time forward: loop from last to first segment
				if (neoc == CA_NEGATIVE_EOC && segcountNew == nMaxSegments && segcountOld == 0)
					pAInfo[acounter].m_nEOC = -1; //time backward: loop from first to last segment
			}

			const f32 totdur = rCAF.GetTotalDuration();
			const f32 fRealTimeNew = pAInfo[acounter].m_fAnimTime * totdur;
			const f32 fRealTimeOld = pAInfo[acounter].m_fAnimTimePrev * totdur;
			f32 fAnimDelta = fRealTimeNew - fRealTimeOld;
			if (pAInfo[acounter].m_nEOC)
				fAnimDelta = totdur - fRealTimeOld + fRealTimeNew;
			pAInfo[acounter].m_fAnimDelta = fAnimDelta;

			//	float fColor2[4] = {1,1,0,1};
			//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.9f, fColor2, false,"%f - %f = %f  fDeltaTime: %f fAssetDelta: %f segcount: %d",pAInfo[acounter].m_fAnimTime,pAInfo[acounter].m_fAnimTimePrev, pAInfo[acounter].m_fAnimTime-pAInfo[acounter].m_fAnimTimePrev,    fDeltaTime,pAInfo[acounter].m_fAnimDelta,segcount);
			//	g_YLine+=20;

			acounter++;
		}
		assert((fBlendWeights == 0) || fabsf(fBlendWeights - 1.0f) < 0.05f);
	}

}

void CSkeletonAnim::Extract_DeltaMovement(AnimInfo* pAInfo, u32& acounter2, SDeltaMotion* pDeltaMotion) const
{
	const f32 fDeltaTime = m_pInstance->m_fDeltaTime;
	if (fDeltaTime < 0)
		DrxFatalError("DinrusXAnimation: reverse playback of animations is not supported any more");

	u32 nTimebasedExtraction = true;
	if (fDeltaTime == 0.0f) //probably paused
		nTimebasedExtraction = false;

	const f32 fPlaybackScale = fDeltaTime * m_layers[0].m_transitionQueue.m_fLayerPlaybackScale;

	for (u32 a = 0; a < acounter2; a++)
	{
		if (pAInfo[a].m_fWeight == 0.0f)
			continue;

		assert(pAInfo[a].m_nAnimID >= 0);
		CAnimationSet* pAnimationSet = m_pInstance->m_pDefaultSkeleton->m_pAnimationSet;
		const ModelAnimationHeader* pMAG = pAnimationSet->GetModelAnimationHeader(pAInfo[a].m_nAnimID);
		assert(pMAG);
		i32 nEGlobalID = pMAG->m_nGlobalAnimId;
		assert(pMAG->m_nAssetType == CAF_File);
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[nEGlobalID];
		IController* pRootController = GetRootController(rCAF);
		if (pRootController == 0)
			continue;

		f32 fStartKey = rCAF.m_fStartSec * ANIMATION_30Hz;
		f32 fTotalKeys = rCAF.m_fTotalDuration * ANIMATION_30Hz;
		i32 isCycle = rCAF.IsAssetCycle();

		f32 fKeyTimeNew = rCAF.NTime2KTime(pAInfo[a].m_fAnimTime);
		QuatT _new;
		_new.SetIdentity();
		GetOP_CubicInterpolation(pRootController, isCycle, fStartKey, fTotalKeys, fKeyTimeNew, _new.q, _new.t);
		_new.q.v.x = 0;
		_new.q.v.y = 0;
		_new.q.Normalize();                              //we want only z-rotations

		f32 fKeyTimeOld = rCAF.NTime2KTime(pAInfo[a].m_fAnimTimePrev);
		QuatT _old;
		_old.SetIdentity();
		GetOP_CubicInterpolation(pRootController, isCycle, fStartKey, fTotalKeys, fKeyTimeOld, _old.q, _old.t);
		if (pAInfo[a].m_nEOC > 0)
		{
			f32 fKeyTime1 = rCAF.NTime2KTime(1);
			QuatT EndKey;
			GetOP_CubicInterpolation(pRootController, isCycle, fStartKey, fTotalKeys, fKeyTime1, EndKey.q, EndKey.t);
			_old = EndKey.GetInverted() * _old; //EOC forwards
		}
		if (pAInfo[a].m_nEOC < 0)
		{
			f32 fKeyTime1 = rCAF.NTime2KTime(1);
			QuatT EndKey;
			GetOP_CubicInterpolation(pRootController, isCycle, fStartKey, fTotalKeys, fKeyTime1, EndKey.q, EndKey.t);
			_old = EndKey * _old; //EOC backwards
		}
		_old.q.v.x = 0;
		_old.q.v.y = 0;
		_old.q.Normalize();                              //we want only z-rotations
		QuatT delta1 = _old.GetInverted() * _new;

		u32 numDimensions = pAInfo[a].m_Dimensions;
		if (numDimensions && nTimebasedExtraction) //if >0 then this CAF is part of a ParaGroup
		{
			for (u32 d = 0; d < numDimensions; d++)
			{
				switch (pAInfo[a].m_Parameter[d])
				{
				case eMotionParamID_TravelSpeed:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_moveSpeed += pAInfo[a].m_fWeight * pAInfo[a].m_PreInitializedVal[d] * fPlaybackScale; //move-speed for this frame
					else
						pDeltaMotion->m_moveSpeed += (pAInfo[a].m_fAnimDelta) ? pAInfo[a].m_fWeight * LocatorHelper::ExtractMoveSpeed(delta1) / pAInfo[a].m_fAnimDelta * fPlaybackScale * pAInfo[a].m_fPlaybackScale : 0.0f; //movespeed extraction
					break;

				case eMotionParamID_TravelDist:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_moveDistance += pAInfo[a].m_fWeight * pAInfo[a].m_PreInitializedVal[d] * fPlaybackScale; //move-distance for this frame
					else
						pDeltaMotion->m_moveDistance += pAInfo[a].m_fWeight * LocatorHelper::ExtractMoveSpeed(delta1); //move-distance extraction
					break;

				case eMotionParamID_TurnSpeed:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_turnSpeed += pAInfo[a].m_fWeight * pAInfo[a].m_PreInitializedVal[d] * fPlaybackScale; //turn-speed for this frame
					else
						pDeltaMotion->m_turnSpeed += (pAInfo[a].m_fAnimDelta) ? pAInfo[a].m_fWeight * LocatorHelper::ExtractTurnSpeed(delta1) / pAInfo[a].m_fAnimDelta * fPlaybackScale * pAInfo[a].m_fPlaybackScale : 0.0f; //turn-speed extraction
					break;

				case eMotionParamID_TurnAngle:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_turnDistance += pAInfo[a].m_fWeight * pAInfo[a].m_PreInitializedVal[d] * fPlaybackScale; //turn-angle for this frame
					else
						pDeltaMotion->m_turnDistance += pAInfo[a].m_fWeight * LocatorHelper::ExtractTurnSpeed(delta1); //turn-angle extraction
					break;

				case eMotionParamID_TravelAngle:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_travelDir += pAInfo[a].m_fWeight * LocatorHelper::ExtractTravelDir(QuatT(IDENTITY, Quat::CreateRotationZ(pAInfo[a].m_PreInitializedVal[d]).GetColumn1()));
					else
						pDeltaMotion->m_travelDir += pAInfo[a].m_fWeight * LocatorHelper::ExtractTravelDir(delta1);
					break;

				case eMotionParamID_TravelSlope:
					if (pAInfo[a].m_IsPreInitialized[d])
						pDeltaMotion->m_slope += pAInfo[a].m_fWeight * pAInfo[a].m_PreInitializedVal[d];
					else
						pDeltaMotion->m_slope += pAInfo[a].m_fWeight * LocatorHelper::ExtractSlope(delta1);
					break;

				}
			}

			pDeltaMotion->m_translation += pAInfo[a].m_fWeight * delta1.t; //just for debugging
		}
		else
		{
			//this is a normal un-parameterized asset
			pDeltaMotion->m_moveSpeed += pAInfo[a].m_fWeight * LocatorHelper::ExtractMoveSpeed(delta1);
			pDeltaMotion->m_turnSpeed += pAInfo[a].m_fWeight * LocatorHelper::ExtractTurnSpeed(delta1);
			pDeltaMotion->m_travelDir += pAInfo[a].m_fWeight * LocatorHelper::ExtractTravelDir(delta1);
			pDeltaMotion->m_slope += pAInfo[a].m_fWeight * LocatorHelper::ExtractSlope(delta1);
			pDeltaMotion->m_translation += pAInfo[a].m_fWeight * delta1.t; //just for debugging
		}

		//float fColor2[4] = {1,1,0,1};
		//g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColor2, false,"m_turnSpeed: %f   w: %f  name %s",pDeltaMotion->m_turnSpeed, pAInfo[a].m_fWeight, rCAF.GetFilePath() );
		//g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.3f, fColor2, false,"m_travelDir: %f   w: %f  name %s",pDeltaMotion->m_travelDir, pAInfo[a].m_fWeight, rCAF.GetFilePath() );
		//g_YLine+=13;
	}

}

//-----------------------------------------------------------------------

ILINE void CSkeletonAnim::GetOP_CubicInterpolation(IController* pRootController, u32 IsCycle, f32 fStartKey, f32 fTotalKeys, f32 fKeyTime, Quat& rot, Vec3& pos) const
{
	pRootController->GetOP(fKeyTime, rot, pos);   //linearly interpolated rotation and position

	if (Console::GetInst().ca_DrawLocator == 0)
		return;
	//only for the locator we do cubic spline evaluation. This is just for debugging.
	//The visual difference between linear- and cubic-interpolation is neglectable.
	f32 fKeysNo = floor(fKeyTime);
	i32 numKeys = i32(fTotalKeys);
	if (numKeys < 4)
		return; //cubic interpolation needs at least 4 keys

	//do a cubic spline interpolation between 4 points
	i32 numLastKey = numKeys - 1 + i32(fStartKey);
	f32 t = fKeyTime - fKeysNo;
	if (fKeysNo == fStartKey && IsCycle)
	{
		Vec3 v0, v1, v2, v3;
		QuatT q0;
		pRootController->GetOP(f32(numLastKey) + 0, q0.q, q0.t);
		QuatT q1;
		pRootController->GetOP(f32(numLastKey) + 1, q1.q, q1.t);
		v0 = (q0 * q1.GetInverted()).t;
		pRootController->GetP(fKeysNo + 0, v1);
		pRootController->GetP(fKeysNo + 1, v2);
		pRootController->GetP(fKeysNo + 2, v3);
		Vec3 ip0;
		ip0.SetQuadraticSpline(v0, v1, v2, t * 0.5f + 0.5f);      //quadratic spline interpolation between v1 and v2 using v0,v1,v2
		Vec3 ip1;
		ip1.SetQuadraticSpline(v1, v2, v3, t * 0.5f);             //quadratic spline interpolation between v1 and v2 using v1,v2,v3
		pos.SetLerp(ip0, ip1, t);
	}

	if (fKeysNo > fStartKey && fKeysNo < numLastKey)
	{
		Vec3 v0, v1, v2, v3;
		pRootController->GetP(fKeysNo - 1, v0);
		pRootController->GetP(fKeysNo + 0, v1);
		pRootController->GetP(fKeysNo + 1, v2);
		pRootController->GetP(fKeysNo + 2, v3);
		Vec3 ip0;
		ip0.SetQuadraticSpline(v0, v1, v2, t * 0.5f + 0.5f);      //quadratic spline interpolation between v1 and v2 using v0,v1,v2
		Vec3 ip1;
		ip1.SetQuadraticSpline(v1, v2, v3, t * 0.5f);             //quadratic spline interpolation between v1 and v2 using v1,v2,v3
		pos.SetLerp(ip0, ip1, t);
	}

	if (fKeysNo == numLastKey && IsCycle)
	{
		Vec3 v0, v1, v2, v3;
		QuatT qLastKey;
		pRootController->GetP(fKeysNo - 1, v0);  //15
		pRootController->GetP(fKeysNo + 0, v1);  //16
		pRootController->GetOP(fKeysNo + 1, qLastKey.q, qLastKey.t);
		v2 = qLastKey.t;
		pRootController->GetP(1, v3);
		v3 = qLastKey * v3;                                        //fKeysNo+2
		Vec3 ip0;
		ip0.SetQuadraticSpline(v0, v1, v2, t * 0.5f + 0.5f);      //quadratic spline interpolation between v1 and v2 using v0,v1,v2
		Vec3 ip1;
		ip1.SetQuadraticSpline(v1, v2, v3, t * 0.5f);             //quadratic spline interpolation between v1 and v2 using v1,v2,v3
		pos.SetLerp(ip0, ip1, t);
	}

}

//-----------------------------------------------------------------------

IController* CSkeletonAnim::GetRootController(GlobalAnimationHeaderCAF& rGAH) const
{

	if (rGAH.IsAssetOnDemand())
	{
		assert(rGAH.IsAssetLoaded());
		if (rGAH.IsAssetLoaded() == 0)
			return 0;
	}

	if (rGAH.m_nControllers2)
	{
		if (rGAH.m_nControllers == 0)
		{
			u32 dba_exists = 0;
			if (rGAH.m_FilePathDBACRC32)
			{
				size_t numDBA_Files = g_AnimationUpr.m_arrGlobalHeaderDBA.size();
				for (u32 d = 0; d < numDBA_Files; d++)
				{
					CGlobalHeaderDBA& pGlobalHeaderDBA = g_AnimationUpr.m_arrGlobalHeaderDBA[d];
					if (rGAH.m_FilePathDBACRC32 != pGlobalHeaderDBA.m_FilePathDBACRC32)
						continue;
					dba_exists++;
					break;
				}

			}

			if (dba_exists)
			{
				if (Console::GetInst().ca_DebugCriticalErrors)
				{
					//this case is virtually impossible, unless something went wrong with a DBA or maybe a CAF in a DBA was compressed to death and all controllers removed
					//	tukk mname = state.m_pInstance->GetFilePath();
					//	f32 fColor[4] = {1,1,0,1};
					//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 1.2f, fColor, false,"model: %s",mname);
					//	g_YLine+=0x10;
					//	g_pAuxGeom->Draw2dLabel( 1,g_YLine, 2.3f, fColor, false,"No Controllers found in Asset: %02x %08x %s",rGAH.m_nControllers2,rGAH.m_FilePathDBACRC32,rGAH.m_FilePath.c_str() );
					//	g_YLine+=23.0f;
					DrxFatalError("DinrusXAnimation: No Controllers found in Asset: %s", rGAH.GetFilePath());
				}
				g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, 0, "No Controllers found in Asset: %s", rGAH.GetFilePath());
			}

			return 0;  //return and don't play animation, because we don't have any controllers
		}
	}

	const CDefaultSkeleton::SJoint* pModelJoint = &m_pInstance->m_pDefaultSkeleton->m_arrModelJoints[0];
	return rGAH.GetControllerByJointCRC32(pModelJoint[0].m_nJointCRC32);
}
