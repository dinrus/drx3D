#ifndef DRX3D_CLIP_POLYGON_H_INCLUDED
#define DRX3D_CLIP_POLYGON_H_INCLUDED

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/GeometryUtil.h>

SIMD_FORCE_INLINE Scalar drx3d_distance_point_plane(const Vec4 &plane, const Vec3 &point)
{
	return point.dot(plane) - plane[3];
}

/*! Vector blending
Takes two vectors a, b, blends them together*/
SIMD_FORCE_INLINE void drx3d_vec_blend(Vec3 &vr, const Vec3 &va, const Vec3 &vb, Scalar blend_factor)
{
	vr = (1 - blend_factor) * va + blend_factor * vb;
}

//! This function calcs the distance from a 3D plane
SIMD_FORCE_INLINE void drx3d_plane_clip_polygon_collect(
	const Vec3 &point0,
	const Vec3 &point1,
	Scalar dist0,
	Scalar dist1,
	Vec3 *clipped,
	i32 &clipped_count)
{
	bool _prevclassif = (dist0 > SIMD_EPSILON);
	bool _classif = (dist1 > SIMD_EPSILON);
	if (_classif != _prevclassif)
	{
		Scalar blendfactor = -dist0 / (dist1 - dist0);
		drx3d_vec_blend(clipped[clipped_count], point0, point1, blendfactor);
		clipped_count++;
	}
	if (!_classif)
	{
		clipped[clipped_count] = point1;
		clipped_count++;
	}
}

//! Clips a polygon by a plane
/*!
*\return The count of the clipped counts
*/
SIMD_FORCE_INLINE i32 drx3d_plane_clip_polygon(
	const Vec4 &plane,
	const Vec3 *polygon_points,
	i32 polygon_point_count,
	Vec3 *clipped)
{
	i32 clipped_count = 0;

	//clip first point
	Scalar firstdist = drx3d_distance_point_plane(plane, polygon_points[0]);
	;
	if (!(firstdist > SIMD_EPSILON))
	{
		clipped[clipped_count] = polygon_points[0];
		clipped_count++;
	}

	Scalar olddist = firstdist;
	for (i32 i = 1; i < polygon_point_count; i++)
	{
		Scalar dist = drx3d_distance_point_plane(plane, polygon_points[i]);

		drx3d_plane_clip_polygon_collect(
			polygon_points[i - 1], polygon_points[i],
			olddist,
			dist,
			clipped,
			clipped_count);

		olddist = dist;
	}

	//RETURN TO FIRST  point

	drx3d_plane_clip_polygon_collect(
		polygon_points[polygon_point_count - 1], polygon_points[0],
		olddist,
		firstdist,
		clipped,
		clipped_count);

	return clipped_count;
}

//! Clips a polygon by a plane
/*!
*\param clipped must be an array of 16 points.
*\return The count of the clipped counts
*/
SIMD_FORCE_INLINE i32 drx3d_plane_clip_triangle(
	const Vec4 &plane,
	const Vec3 &point0,
	const Vec3 &point1,
	const Vec3 &point2,
	Vec3 *clipped  // an allocated array of 16 points at least
)
{
	i32 clipped_count = 0;

	//clip first point0
	Scalar firstdist = drx3d_distance_point_plane(plane, point0);
	;
	if (!(firstdist > SIMD_EPSILON))
	{
		clipped[clipped_count] = point0;
		clipped_count++;
	}

	// point 1
	Scalar olddist = firstdist;
	Scalar dist = drx3d_distance_point_plane(plane, point1);

	drx3d_plane_clip_polygon_collect(
		point0, point1,
		olddist,
		dist,
		clipped,
		clipped_count);

	olddist = dist;

	// point 2
	dist = drx3d_distance_point_plane(plane, point2);

	drx3d_plane_clip_polygon_collect(
		point1, point2,
		olddist,
		dist,
		clipped,
		clipped_count);
	olddist = dist;

	//RETURN TO FIRST  point0
	drx3d_plane_clip_polygon_collect(
		point2, point0,
		olddist,
		firstdist,
		clipped,
		clipped_count);

	return clipped_count;
}

#endif  // GIM_TRI_COLLISION_H_INCLUDED
