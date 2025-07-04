// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements several utility functions for vehicles

   -------------------------------------------------------------------------
   История:

*************************************************************************/
#ifndef __VEHICLEUTILS_H__
#define __VEHICLEUTILS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IVehicleSystem.h"
#include "VehicleCVars.h"

class CVehiclePartAnimated;

// this enables (soft) NAN-checking
#ifdef _DEBUG
	#define VEHICLE_VALIDATE_MATH
#endif

// validation functions
#ifndef VEHICLE_VALIDATE_MATH
	#define VALIDATE_VEC(a)  a
	#define VALIDATE_MAT(a)  a
	#define VALIDATE_AABB(a) a
#else
	#define VALIDATE_VEC(a)  VehicleUtils::ValidateVec(a)
	#define VALIDATE_MAT(a)  VehicleUtils::ValidateMat(a)
	#define VALIDATE_AABB(a) VehicleUtils::ValidateAABB(a)
#endif

namespace VehicleUtils
{
inline const Vec3& ValidateVec(const Vec3& vec)
{
	DRX_ASSERT(!_isnan(vec.x + vec.y + vec.z));
	return vec;
}

inline const AABB& ValidateAABB(const AABB& aabb)
{
	ValidateVec(aabb.min);
	ValidateVec(aabb.max);

	return aabb;
}

inline const Matrix34& ValidateMat(const Matrix34& mat)
{
	for (i32 i = 0; i < 4; ++i)
		ValidateVec(mat.GetColumn(i));

	if (!mat.IsOrthonormal())
	{
		DrxLog("[VehicleUtils]: !IsOrthonormal");
	}
	return mat;
}

inline void LogVector(const Vec3& v)
{
	DrxLog("%.2f %.2f %.2f", v.x, v.y, v.z);
}

inline void LogMatrix(tukk label, const Matrix34& tm)
{
#if ENABLE_VEHICLE_DEBUG
	if (VehicleCVars().v_debugdraw < eVDB_Parts)
		return;

	DrxLog("<%s>", label);
	LogVector(tm.GetColumn(0));
	LogVector(tm.GetColumn(1));
	LogVector(tm.GetColumn(2));
	LogVector(tm.GetColumn(3));
	DrxLog("----------------");
#endif
}

void DumpDamageBehaviorEvent(const SVehicleDamageBehaviorEventParams& params);
void DrawTM(const Matrix34& tm, tukk name = "VehicleUtils", bool clear = false);

void DumpSlots(IVehicle* pVehicle);

}

#endif //__VEHICLEUTILS_H__
