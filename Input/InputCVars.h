// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INPUTCVARS_H__
#define __INPUTCVARS_H__

struct ICVar;

class CInputCVars
{
public:
	i32   i_debug;
	i32   i_forcefeedback;

	i32   i_mouse_buffered;
	float i_mouse_sensitivity;
	float i_mouse_accel;
	float i_mouse_accel_max;
	float i_mouse_smooth;
	float i_mouse_inertia;

#if DRX_PLATFORM_WINDOWS
	i32 i_mouse_scroll_coordinate_origin;
#endif

	i32   i_bufferedkeys;

	i32   i_xinput;
	i32   i_xinput_poll_time;

	i32   i_xinput_deadzone_handling;

	i32   i_debugDigitalButtons;

	i32   i_kinSkeletonSmoothType;
	i32   i_kinectDebug;
	i32   i_useKinect;
	i32   i_seatedTracking;

#if DRX_PLATFORM_DURANGO
	i32   i_useDurangoKinectSpeech;
	i32   i_useDurangoKinectSpeechMode;
	i32   i_showDurangoKinectAudioStatus;
	float i_durangoKinectSpeechConfidenceThreshold;
#endif

	float i_kinSkeletonMovedDistance;

	//Double exponential smoothing parameters
	float i_kinGlobalExpSmoothFactor;
	float i_kinGlobalExpCorrectionFactor;
	float i_kinGlobalExpPredictionFactor;
	float i_kinGlobalExpJitterRadius;
	float i_kinGlobalExpDeviationRadius;

#if DRX_PLATFORM_WINDOWS
	i32    i_kinectXboxConnect;
	i32    i_kinectXboxConnectPort;
	ICVar* i_kinectXboxConnectIP;
#endif

#ifdef USE_SYNERGY_INPUT
	ICVar* i_synergyServer;
	ICVar* i_synergyScreenName;
#endif

	CInputCVars();
	~CInputCVars();
};

extern CInputCVars* g_pInputCVars;
#endif //__INPUTCVARS_H__
