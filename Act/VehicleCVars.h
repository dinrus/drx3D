// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	VehicleSystem CVars
   -------------------------------------------------------------------------
   История:
   - 02:01:2007  12:47 : Created by MichaelR

*************************************************************************/

#ifndef __VEHICLECVARS_H__
#define __VEHICLECVARS_H__

#pragma once

class CVehicleCVars
{
public:
#if ENABLE_VEHICLE_DEBUG
	// debug draw
	i32 v_debugdraw;
	i32   v_draw_components;
	i32   v_draw_helpers;
	i32   v_draw_seats;
	i32   v_draw_tm;
	i32   v_draw_passengers;
	i32   v_debug_mem;

	i32   v_debug_flip_over;
	i32   v_debug_reorient;

	i32   v_debugViewDetach;
	i32   v_debugViewAbove;
	float v_debugViewAboveH;
	i32   v_debugCollisionDamage;
#endif

	// dev vars
	i32 v_transitionAnimations;
	i32   v_playerTransitions;
	i32   v_autoDisable;
	i32   v_lights;
	i32   v_lights_enable_always;
	i32   v_set_passenger_tm;
	i32   v_disable_hull;
	i32   v_ragdollPassengers;
	i32   v_goliathMode;
	i32   v_show_all;
	i32   v_staticTreadDeform;
	float v_tpvDist;
	float v_tpvHeight;
	i32   v_debugSuspensionIK;
	i32   v_serverControlled;
	i32   v_clientPredict;
	i32   v_clientPredictSmoothing;
	i32   v_testClientPredict;
	float v_clientPredictSmoothingConst;
	float v_clientPredictAdditionalTime;
	float v_clientPredictMaxTime;

	i32   v_enableMannequin;

	// tweaking
	float v_slipSlopeFront;
	float v_slipSlopeRear;
	float v_slipFrictionModFront;
	float v_slipFrictionModRear;

	float v_FlippedExplosionTimeToExplode;
	float v_FlippedExplosionPlayerMinDistance;
	i32   v_FlippedExplosionRetryTimeMS;

	i32   v_vehicle_quality;
	i32   v_driverControlledMountedGuns;
	i32   v_independentMountedGuns;

	i32   v_disableEntry;

	static inline CVehicleCVars& Get()
	{
		assert(s_pThis != 0);
		return *s_pThis;
	}

private:
	friend class CVehicleSystem; // Our only creator

	CVehicleCVars(); // singleton stuff
	~CVehicleCVars();
	CVehicleCVars(const CVehicleCVars&);
	CVehicleCVars& operator=(const CVehicleCVars&);

	static CVehicleCVars* s_pThis;
};

ILINE const CVehicleCVars& VehicleCVars() { return CVehicleCVars::Get(); }

#endif // __VEHICLECVARS_H__
