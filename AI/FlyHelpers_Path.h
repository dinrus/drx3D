// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLY_HELPERS__PATH__H__
#define __FLY_HELPERS__PATH__H__

#include <drx3D/CoreX/Math/Drx_Geo.h>

namespace FlyHelpers
{

class Path
{
public:
	Path();
	~Path();

	void        Clear();

	void        AddPoint(const Vec3& point);

	const Vec3& GetPoint(const size_t pointIndex) const;

	Lineseg     GetSegment(const size_t segmentIndex) const;
	float       GetSegmentLength(const size_t segmentIndex) const;

	size_t      GetPointCount() const;
	size_t      GetSegmentCount() const;

	void        MakeLooping();

	float       GetPathDistanceToPoint(const size_t pointIndex) const;
	float       GetTotalPathDistance() const;

private:
	std::vector<Vec3>  m_points;
	std::vector<float> m_segmentDistances;
	std::vector<float> m_pathLengthsToPoint;
};

}

#endif
