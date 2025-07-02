// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "obbtree.h"

STRUCT_INFO_BEGIN(OBBnode)
	STRUCT_VAR_INFO(axes, TYPE_ARRAY(3, TYPE_INFO(Vec3)))
	STRUCT_VAR_INFO(center, TYPE_INFO(Vec3))
	STRUCT_VAR_INFO(size, TYPE_INFO(Vec3))
	STRUCT_VAR_INFO(iparent, TYPE_INFO(i32))
	STRUCT_VAR_INFO(ichild, TYPE_INFO(i32))
	STRUCT_VAR_INFO(ntris, TYPE_INFO(i32))
STRUCT_INFO_END(OBBnode)

