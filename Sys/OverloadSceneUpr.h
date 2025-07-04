// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _OVERLOAD_SCENE_MANAGER_
#define _OVERLOAD_SCENE_MANAGER_

#pragma once

// Includes
#include <drx3D/Sys/IOverloadSceneUpr.h>

// Defines
#ifndef _RELEASE
	#define DEBUG_OVERLOAD_SCENE_MANAGER 1
#else
	#define DEBUG_OVERLOAD_SCENE_MANAGER 0
#endif

#define SCENE_PERFORMANCE_FRAME_HISTORY 64

// Forward declares
struct IDebugHistoryUpr;
struct IDebugHistory;
struct IStatoscopeDataGroup;

//==================================================================================================
// Name: SOverloadSceneStats
// Desc: Overload scene stats
// Author: James Chilvers
//==================================================================================================
struct SScenePerformanceStats
{
	SScenePerformanceStats()
	{
		Reset();
	}

	void Reset()
	{
		frameRate = 0.0f;
		gpuFrameRate = 0.0f;
	}

	float frameRate;
	float gpuFrameRate;
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: COverloadSceneUpr
// Desc: Manages overload values (eg CPU,GPU etc)
//			 1.0="everything is ok"  0.0="very bad frame rate"
//			 various systems can use this information and control what is currently in the scene
// Author: James Chilvers
//==================================================================================================
class COverloadSceneUpr : public IOverloadSceneUpr
{
	friend class COverloadDG;

public:

	COverloadSceneUpr();
	virtual ~COverloadSceneUpr();

	virtual void Reset();

	virtual void Update();

	virtual void OverrideScale(float frameScale, float dt);
	virtual void ResetScale(float dt);

private:

	void  ResetDefaultValues();
	void  InitialiseCVars();
	void  UpdateStats();
	void  CalculateSmoothedStats();
	void  ResizeFB();

	float CalcFBScale();       // performs all lerping and returns final framebuffer scale

#if DEBUG_OVERLOAD_SCENE_MANAGER
	void DebugDrawDisplay();
	void DebugDrawGraphs();

	i32                   osm_debug, osm_stress;
	IDebugHistoryUpr* m_pDebugHistoryUpr;
	IDebugHistory*        m_pDebugHistory;
#endif

	// cvars
	i32                    osm_enabled;
	i32                    osm_historyLength;
	float                  osm_targetFPS;
	float                  osm_targetFPSTolerance;
	float                  osm_fbScaleDeltaUp, osm_fbScaleDeltaDown;
	float                  osm_fbMinScale;

	SScenePerformanceStats m_smoothedSceneStats;
	SScenePerformanceStats m_sceneStats[SCENE_PERFORMANCE_FRAME_HISTORY];
	i32                    m_currentFrameStat;

	// current output scale, set to the renderer
	float m_fbScale;

	// Lerping behaviour is to lerp from autoscale to (lerp between cur/dest override)
	//
	//                                        m_fbOverrideCurScale
	//                                                |
	//                                                |
	//    m_fbAutoScale  ---- m_lerpAuto --->  m_lerpOverride
	//                                                |
	//                                                v
	//                                        m_fbOverrideDestScale

	// framebuffer scales to support lerping & overriding of calculated scale
	float m_fbAutoScale, m_fbOverrideCurScale, m_fbOverrideDestScale;

	struct ScaleLerp
	{
		bool  m_reversed;     // Normally lerp is 0 -> 1. If reversed it's 1 -> 0
		float m_start;        // time in seconds
		float m_length;       // time in seconds
	};

	// lerpAuto is the lerp between auto scale and whatever override is.
	// lerpOverride is the lerp between m_fbOverrideCurScale and m_fbOverrideDestScale
	ScaleLerp m_lerpAuto, m_lerpOverride;

	// State is the current destination of any lerps.
	enum ScaleState
	{
		FBSCALE_AUTO,
		FBSCALE_OVERRIDE,
	} m_scaleState;

#if ENABLE_STATOSCOPE
	IStatoscopeDataGroup* m_pOverloadDG;
#endif
};//------------------------------------------------------------------------------------------------

#endif // _OVERLOAD_SCENE_MANAGER_
