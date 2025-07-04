// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

//! Contains information about what hide spots you want the AI system to return.
struct HidespotQueryContext
{
	HidespotQueryContext()
		: minRange(0.0f)
		, maxRange(0.0f)
		, hideFromPos(ZERO)
		, centerOfQuery(ZERO)
		, onlyThoseThatGiveCover(true)
		, onlyCollidable(false)
		, maxPoints(0)
		, pCoverPos(NULL)
		, pCoverObjPos(NULL)
		, pCoverObjDir(NULL)
	{

	}

	float        minRange;
	float        maxRange;
	Vec3         hideFromPos;
	Vec3         centerOfQuery;
	bool         onlyThoseThatGiveCover;
	bool         onlyCollidable;

	u32 maxPoints;
	Vec3*        pCoverPos;
	Vec3*        pCoverObjPos;
	Vec3*        pCoverObjDir;
};

//! \endcond