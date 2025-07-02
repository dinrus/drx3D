// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CoverSurface_h__
#define __CoverSurface_h__
#pragma once

#include <drx3D/AI/Cover.h>
#include <drx3D/AI/CoverPath.h>

class CoverSurface
{
	typedef ICoverSampler::Sample Sample;
public:
	CoverSurface();
	CoverSurface(const ICoverSystem::SurfaceInfo& surfaceInfo);

	static void FreeStaticData()
	{
		stl::free_container(s_simplifiedPoints);
	}

	struct Segment
	{
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wconstant-conversion"
#endif

#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic ignored "-Woverflow"
	#else
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Woverflow"
	#endif
#endif
		Segment()
		{
			normal = ZERO;
			leftIdx = std::numeric_limits<u16>::max();
			rightIdx = std::numeric_limits<u16>::max();
			flags = 0;
			length = 0;
		}
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic error "-Woverflow"
	#else
		#pragma GCC diagnostic pop
	#endif
#endif

		Segment(const Vec3& n, float len, u16 left, u16 right, u16 _flags)
			: normal(n)
			, length(len)
			, leftIdx(left)
			, rightIdx(right)
			, flags(_flags)
		{
		}

		enum Flags
		{
			Dynamic  = 1 << 0,
			Disabled = 1 << 1,
		};

		Vec3   normal;
		float  length;

		u16 leftIdx  : 14;
		u16 rightIdx : 14;
		u16 flags    : 4;
	};

	bool           IsValid() const;
	void           Clear();
	void           Swap(CoverSurface& other);

	u32         GetSampleCount() const;
	bool           GetSurfaceInfo(ICoverSystem::SurfaceInfo* surfaceInfo) const;

	u32         GetSegmentCount() const;

	Segment&       GetSegment(u16 segmentIdx);
	const Segment& GetSegment(u16 segmentIdx) const;

	const AABB&    GetAABB() const;
	u32k   GetFlags() const;

	void           ReserveSamples(u32 sampleCount);
	void           AddSample(const Sample& sample);
	void           Generate();
	void           GenerateLocations();

	u32         GetLocationCount() const;
	Vec3           GetLocation(u16 location, float offset = 0.0f, float* height = 0, Vec3* normal = 0) const;

	bool           IsPointInCover(const Vec3& eye, const Vec3& point) const;
	bool           IsCircleInCover(const Vec3& eye, const Vec3& center, float radius) const;
	bool           CalculatePathCoverage(const Vec3& eye, const CoverPath& coverPath, CoverInterval* interval) const;
	bool           GetCoverOcclusionAt(const Vec3& eye, const Vec3& point, float* heightSq) const;
	bool           GetCoverOcclusionAt(const Vec3& eye, const Vec3& center, float radius, float* heightSq) const;

	bool           GenerateCoverPath(float distanceToCover, CoverPath* path, bool skipSimplify = false) const;

	void           DebugDraw() const;

private:
	typedef std::vector<Vec3> Points;
	void SimplifyCoverPath(Points& points) const;

	static Points s_simplifiedPoints;

	bool ILINE IsPointBehindSegment(const Vec3& eye, const Vec3& point, const Segment& segment) const
	{
		const Sample& left = m_samples[segment.leftIdx];
		const Sample& right = m_samples[segment.rightIdx];

		Lineseg ray(eye, point);
		Lineseg seg(left.position, right.position);

		if (!Overlap::Lineseg_Lineseg2D(ray, seg))
			return false;

		const float BottomGuard = 2.0f;

		Vec3 leftBottom = left.position;
		leftBottom.z -= BottomGuard;
		Vec3 rightBottom = right.position;
		rightBottom.z -= BottomGuard;

		Vec3 leftTop = left.position;
		leftTop.z += static_cast<float>(left.GetHeightInteger()) * Sample::GetHeightToFloatConverter();

		Vec3 hit;

		if (!Intersect::Lineseg_Triangle(ray, rightBottom, leftTop, leftBottom, hit))
		{
			Vec3 rightTop = right.position;
			rightTop.z += static_cast<float>(right.GetHeightInteger()) * Sample::GetHeightToFloatConverter();

			if (!Intersect::Lineseg_Triangle(ray, rightBottom, rightTop, leftTop, hit))
				return false;
		}

		return true;
	}

	bool ILINE GetOcclusionBehindSegment(const Vec3& eye, const Vec3& point, const Segment& segment, float* heightSq) const
	{
		DRX_PROFILE_FUNCTION(PROFILE_AI);

		const Sample& left = m_samples[segment.leftIdx];
		const Sample& right = m_samples[segment.rightIdx];

		Lineseg ray(eye, point);
		Lineseg seg(left.position, right.position);

		float a;
		float b;

		if (!Intersect::Lineseg_Lineseg2D(ray, seg, a, b))
			return false;

		const float BottomGuard = 2.0f;

		Vec3 leftBottom = left.position;
		leftBottom.z -= BottomGuard;
		Vec3 rightBottom = right.position;
		rightBottom.z -= BottomGuard;

		Vec3 leftTop = left.position;

		float leftHeight = static_cast<float>(left.GetHeightInteger()) * Sample::GetHeightToFloatConverter();
		float rightHeight = static_cast<float>(right.GetHeightInteger()) * Sample::GetHeightToFloatConverter();
		leftTop.z += leftHeight;

		Vec3 hit;

		if (!Intersect::Lineseg_Triangle(ray, rightBottom, leftTop, leftBottom, hit))
		{
			Vec3 rightTop = right.position;
			rightTop.z += rightHeight;

			if (!Intersect::Lineseg_Triangle(ray, rightBottom, rightTop, leftTop, hit))
				return false;
		}

		float h = leftHeight + b * (rightHeight - leftHeight);

		Vec3 top = left.position + b * (right.position - left.position);
		top.z += h;

		Vec3 dir = (top - eye) * 9999.0f;

		float t;
		*heightSq = Distance::Point_LinesegSq(point, Lineseg(eye, eye + dir), t);

		return true;
	}

	bool IntersectCoverPlane(const Vec3& origin, const Vec3& length, const Plane& plane, Vec3* point, float* t) const;
	void FindCoverPlanes(const Vec3& eye, Plane& left, Plane& right) const;

	typedef std::vector<Sample> Samples;
	Samples m_samples;

	typedef std::vector<Segment> Segments;
	Segments m_segments;

	struct Location
	{
		enum
		{
			IntegerPartBitCount = 12,
		};

		enum Flags
		{
			LeftEdge  = 1 << 0,
			RightEdge = 1 << 1,
		};

		u16 segmentIdx;
		u16 offset;
		u16 height;
		u16 flags;

		Location()
		{
		}

		Location(u16k _segmentIdx, float _offset, float _height)
			: segmentIdx(_segmentIdx)
		{
			SetOffset(_offset);
			SetHeight(_height);
		}

		ILINE void SetOffset(float _offset)
		{
			assert((0.0f <= _offset) && (_offset <= 1.0f));
			offset = (u16)(_offset * (float)0xffff);
		}

		ILINE float GetOffset() const
		{
			return offset * (1.0f / (float)0xffff);
		}

		ILINE void SetHeight(float _height)
		{
			height = (u16)(_height * ((1 << IntegerPartBitCount) - 1));
		}

		ILINE float GetHeight() const
		{
			return height * (1.0f / (float)((1 << IntegerPartBitCount) - 1));
		}
	};

	typedef std::vector<Location> Locations;
	Locations m_locations;

	AABB      m_aabb;
	u32    m_flags;
};

#endif //__CoverSurface_h__
