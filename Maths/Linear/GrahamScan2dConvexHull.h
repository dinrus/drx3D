#ifndef GRAHAM_SCAN_2D_CONVEX_HULL_H
#define GRAHAM_SCAN_2D_CONVEX_HULL_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct GrahamVector3 : public Vec3
{
	GrahamVector3(const Vec3& org, i32 orgIndex)
		: Vec3(org),
		  m_orgIndex(orgIndex)
	{
	}
	Scalar m_angle;
	i32 m_orgIndex;
};

struct AngleCompareFunc
{
	Vec3 m_anchor;
	AngleCompareFunc(const Vec3& anchor)
		: m_anchor(anchor)
	{
	}
	bool operator()(const GrahamVector3& a, const GrahamVector3& b) const
	{
		if (a.m_angle != b.m_angle)
			return a.m_angle < b.m_angle;
		else
		{
			Scalar al = (a - m_anchor).length2();
			Scalar bl = (b - m_anchor).length2();
			if (al != bl)
				return al < bl;
			else
			{
				return a.m_orgIndex < b.m_orgIndex;
			}
		}
	}
};

inline void GrahamScanConvexHull2D(AlignedObjectArray<GrahamVector3>& originalPoints, AlignedObjectArray<GrahamVector3>& hull, const Vec3& normalAxis)
{
	Vec3 axis0, axis1;
	PlaneSpace1(normalAxis, axis0, axis1);

	if (originalPoints.size() <= 1)
	{
		for (i32 i = 0; i < originalPoints.size(); i++)
			hull.push_back(originalPoints[0]);
		return;
	}
	//step1 : find anchor point with smallest projection on axis0 and move it to first location
	for (i32 i = 0; i < originalPoints.size(); i++)
	{
		//		const Vec3& left = originalPoints[i];
		//		const Vec3& right = originalPoints[0];
		Scalar projL = originalPoints[i].dot(axis0);
		Scalar projR = originalPoints[0].dot(axis0);
		if (projL < projR)
		{
			originalPoints.swap(0, i);
		}
	}

	//also precompute angles
	originalPoints[0].m_angle = -1e30f;
	for (i32 i = 1; i < originalPoints.size(); i++)
	{
		Vec3 ar = originalPoints[i] - originalPoints[0];
		Scalar ar1 = axis1.dot(ar);
		Scalar ar0 = axis0.dot(ar);
		if (ar1 * ar1 + ar0 * ar0 < FLT_EPSILON)
		{
			originalPoints[i].m_angle = 0.0f;
		}
		else
		{
			originalPoints[i].m_angle = Atan2Fast(ar1, ar0);
		}
	}

	//step 2: sort all points, based on 'angle' with this anchor
	AngleCompareFunc comp(originalPoints[0]);
	originalPoints.quickSortInternal(comp, 1, originalPoints.size() - 1);

	i32 i;
	for (i = 0; i < 2; i++)
		hull.push_back(originalPoints[i]);

	//step 3: keep all 'convex' points and discard concave points (using back tracking)
	for (; i != originalPoints.size(); i++)
	{
		bool isConvex = false;
		while (!isConvex && hull.size() > 1)
		{
			Vec3& a = hull[hull.size() - 2];
			Vec3& b = hull[hull.size() - 1];
			isConvex = Cross(a - b, a - originalPoints[i]).dot(normalAxis) > 0;
			if (!isConvex)
				hull.pop_back();
			else
				hull.push_back(originalPoints[i]);
		}

		if (hull.size() == 1)
		{
			hull.push_back(originalPoints[i]);
		}
	}
}

#endif  //GRAHAM_SCAN_2D_CONVEX_HULL_H
