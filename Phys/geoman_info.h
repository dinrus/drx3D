// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "geoman.h"

STRUCT_INFO_BEGIN(phys_geometry_serialize)
	STRUCT_VAR_INFO(dummy0, TYPE_INFO(i32))
	STRUCT_VAR_INFO(Ibody, TYPE_INFO(Vec3))
	STRUCT_VAR_INFO(q, TYPE_INFO(quaternionf))
	STRUCT_VAR_INFO(origin, TYPE_INFO(Vec3))
	STRUCT_VAR_INFO(V, TYPE_INFO(float))
	STRUCT_VAR_INFO(nRefCount, TYPE_INFO(i32))
	STRUCT_VAR_INFO(surface_idx, TYPE_INFO(i32))
	STRUCT_VAR_INFO(dummy1, TYPE_INFO(i32))
	STRUCT_VAR_INFO(nMats, TYPE_INFO(i32))
STRUCT_INFO_END(phys_geometry_serialize)

