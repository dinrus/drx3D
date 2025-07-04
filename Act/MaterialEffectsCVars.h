// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	CVars for the MaterialEffects subsystem
   -------------------------------------------------------------------------
   История:
   - 02:03:2006  12:00 : Created by AlexL

*************************************************************************/

#ifndef __MATERIALEFFECTS_CVARS_H__
#define __MATERIALEFFECTS_CVARS_H__

#pragma once

struct ICVar;

class CMaterialEffectsCVars
{
public:

	float  mfx_ParticleImpactThresh;    // "Impact threshold for particle effects. Default: 1.38" );
	float  mfx_SoundImpactThresh;       //"Impact threshold for sound effects. Default: 1.5" );
	float  mfx_RaisedSoundImpactThresh; // "Impact threshold for sound effects if we're rolling. Default: 3.5" );
	i32    mfx_Debug;                   // "Turns on MaterialEffects debug messages." );
	i32    mfx_DebugFlowGraphFX;        // "Turns on MaterialEffects FlowGraph FX debbug messages." );
	i32    mfx_DebugVisual;
	ICVar* mfx_DebugVisualFilter;
	i32    mfx_Enable;
	i32    mfx_EnableFGEffects;
	i32    mfx_EnableAttachedEffects;
	i32    mfx_SerializeFGEffects;     // "Serialize Material Effect based effects. Default: On"
	float  mfx_pfx_maxDist;
	float  mfx_pfx_maxScale;
	float  mfx_pfx_minScale;

	// Timeout avoids playing effects too often.
	// Effects should still be played at lower velocities, but not too often
	// as they cannot be distinguished when played too close together and are slowing down the system.
	// Therefore a tweakeable timeout
	float mfx_Timeout;

	static inline CMaterialEffectsCVars& Get()
	{
		assert(s_pThis != 0);
		return *s_pThis;
	}

private:
	friend class CDrxAction; // Our only creator

	CMaterialEffectsCVars(); // singleton stuff
	~CMaterialEffectsCVars();
	CMaterialEffectsCVars(const CMaterialEffectsCVars&);
	CMaterialEffectsCVars& operator=(const CMaterialEffectsCVars&);

	static CMaterialEffectsCVars* s_pThis;
};

#endif // __MATERIALEFFECTS_CVARS_H__
