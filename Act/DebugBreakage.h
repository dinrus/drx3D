// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DEBUG_NET_BREAKAGE__H__
#define __DEBUG_NET_BREAKAGE__H__

#pragma once

// Enable logging of breakage events and breakage serialisation
#define DEBUG_NET_BREAKAGE 0

#ifndef XYZ
	#define XYZ(v) (v).x, (v).y, (v).z
#endif

#if !defined(_RELEASE) && DEBUG_NET_BREAKAGE

	#include <drx3D/CoreX/String/StringUtils.h>

	#define LOGBREAK(...) DrxLogAlways("brk: " __VA_ARGS__)

static void LOGBREAK_STATOBJ(IStatObj* pObj)
{
	if (pObj)
	{
		u32 breakable = pObj->GetBreakableByGame();
		i32 idBreakable = pObj->GetIDMatBreakable();
		Vec3 bbmin = pObj->GetBoxMin();
		Vec3 bbmax = pObj->GetBoxMax();
		Vec3 vegCentre = pObj->GetVegCenter();
		tukk filePath = pObj->GetFilePath();
		tukk geoName = pObj->GetGeoName();
		i32 subObjCount = pObj->GetSubObjectCount();
		LOGBREAK("StatObj: bbmin = (%f, %f, %f) bbmax = (%f, %f, %f)", XYZ(bbmin), XYZ(bbmax));
		LOGBREAK("StatObj: vegCentre = (%f, %f, %f)", XYZ(vegCentre));
		LOGBREAK("StatObj: breakable = %d, idBreakable = %d, subObjCount = %d", breakable, idBreakable, subObjCount);
		LOGBREAK("StatObj: filePath = '%s', geoName = '%s'", filePath, geoName);
		LOGBREAK("StatObj: filePathHash = %d", DrxStringUtils::CalculateHash(filePath));
	}
}

#else

	#define LOGBREAK(...)
	#define LOGBREAK_STATOBJ(x)

#endif

#endif // __DEBUG_NET_BREAKAGE__H__
