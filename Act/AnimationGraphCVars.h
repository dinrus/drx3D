// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	CVars for the AnimationGraph Subsystem

   -------------------------------------------------------------------------
   История:
   - 02:03:2006  12:00 : Created by AlexL

*************************************************************************/

#ifndef __ANIMATIONGRAPH_CVARS_H__
#define __ANIMATIONGRAPH_CVARS_H__

#pragma once

struct ICVar;

struct CAnimationGraphCVars
{
	i32                                 m_debugExactPos; // Enable exact positioning debug

	float                               m_landingBobTimeFactor;
	float                               m_landingBobLandTimeFactor;

	float                               m_distanceForceNoIk;
	float                               m_distanceForceNoLegRaycasts;
	i32                                 m_spectatorCollisions;
	i32                                 m_groundAlignAll;

	i32                                 m_entityAnimClamp;

	float                               m_clampDurationEntity;
	float                               m_clampDurationAnimation;

	i32                                 m_MCMH;
	i32                                 m_MCMV;
	i32                                 m_MCMFilter;
	i32                                 m_TemplateMCMs;
	i32                                 m_forceColliderModePlayer;
	i32                                 m_forceColliderModeAI;
	i32                                 m_enableExtraSolidCollider;
	i32                                 m_debugLocations;
	ICVar*                              m_pDebugFilter;
	i32                                 m_debugText;
	i32                                 m_debugMotionParams;
	i32                                 m_debugLocationsGraphs;
	i32                                 m_debugAnimError;
	i32                                 m_debugAnimTarget;
	i32                                 m_debugColliderMode;
	ICVar*                              m_debugColliderModeName;
	i32                                 m_debugMovementControlMethods;
	i32                                 m_debugTempValues;
	i32                                 m_debugFrameTime;
	i32                                 m_debugEntityParams;
	i32                                 m_forceSimpleMovement;
	i32                                 m_disableSlidingContactEvents;
	i32                                 m_debugAnimEffects;
	i32                                 m_useMovementPrediction;
	i32                                 m_useQueuedRotation;

	float                               m_turnSpeedParamScale;
	i32                                 m_enableTurnSpeedSmoothing;
	i32                                 m_enableTurnAngleSmoothing;
	i32                                 m_enableTravelSpeedSmoothing;

	static inline CAnimationGraphCVars& Get()
	{
		assert(s_pThis != 0);
		return *s_pThis;
	}

private:
	friend class CDrxAction; // Our only creator

	CAnimationGraphCVars(); // singleton stuff
	~CAnimationGraphCVars();
	CAnimationGraphCVars(const CAnimationGraphCVars&);
	CAnimationGraphCVars& operator=(const CAnimationGraphCVars&);

	static CAnimationGraphCVars* s_pThis;
};

#endif // __ANIMATIONGRAPH_CVARS_H__
