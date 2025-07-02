// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	AI Navigation Interface.

   -------------------------------------------------------------------------
   История:
   - 3:3:2010		:	Created by Kevin Kirst

*************************************************************************/

#ifndef _INAVIGATION_H_
#define _INAVIGATION_H_

#include <drx3D/AI/IAISystem.h> // <> required for Interfuscator

struct IAIObject;

struct INavigation
{
	enum EIFMode {IF_AREASBOUNDARIES, IF_AREAS, IF_BOUNDARIES};

	// <interfuscator:shuffle>
	virtual ~INavigation() {}

	virtual float  GetNearestPointOnPath(tukk szPathName, const Vec3& vPos, Vec3& vResult, bool& bLoopPath, float& totalLength) const = 0;
	virtual void   GetPointOnPathBySegNo(tukk szPathName, Vec3& vResult, float segNo) const = 0;

	//! Returns nearest designer created path/shape.
	//! \param devalue Specifies how long the path will be unusable by others after the query.
	//! \param useStartNode If true the start point of the path is used to select nearest path instead of the nearest point on path.
	virtual tukk GetNearestPathOfTypeInRange(IAIObject* requester, const Vec3& pos, float range, i32 type, float devalue, bool useStartNode) = 0;

	// </interfuscator:shuffle>
};

#endif //_INAVIGATION_H_
