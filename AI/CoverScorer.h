// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CoverScorer_h__
#define __CoverScorer_h__
#pragma once

#include <drx3D/AI/Cover.h>

struct ICoverLocationScorer
{
	virtual ~ICoverLocationScorer(){}
	struct Params
	{
		Vec3            location;
		Vec3            direction;
		float           height;

		Vec3            userLocation;
		float           totalLength;

		ECoverUsageType usage;

		Vec3            target;
	};

	virtual float Score(const Params& params) const = 0;
};

struct DefaultCoverScorer : public ICoverLocationScorer
{
public:
	float         ScoreByDistance(const ICoverLocationScorer::Params& params) const;
	float         ScoreByDistanceToTarget(const ICoverLocationScorer::Params& params) const;
	float         ScoreByAngle(const ICoverLocationScorer::Params& params) const;
	float         ScoreByCoverage(const ICoverLocationScorer::Params& params) const;

	virtual float Score(const ICoverLocationScorer::Params& params) const;
};

#endif //__CoverScorer_h__
