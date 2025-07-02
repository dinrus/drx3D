#ifndef DRX3D_BASIC_GEOMETRY_OPERATIONS_H_INCLUDED
#define DRX3D_BASIC_GEOMETRY_OPERATIONS_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/BoxCollision.h>

#define PLANEDIREPSILON 0.0000001f
#define PARALELENORMALS 0.000001f

#define DRX3D_CLAMP(number, minval, maxval) (number < minval ? minval : (number > maxval ? maxval : number))

/// Calc a plane from a triangle edge an a normal. plane is a vec4f
SIMD_FORCE_INLINE void drx3d_edge_plane(const Vec3 &e1, const Vec3 &e2, const Vec3 &normal, Vec4 &plane)
{
	Vec3 planenormal = (e2 - e1).cross(normal);
	planenormal.normalize();
	plane.setVal(planenormal[0], planenormal[1], planenormal[2], e2.dot(planenormal));
}

//***************** SEGMENT and LINE FUNCTIONS **********************************///

/*! Finds the closest point(cp) to (v) on a segment (e1,e2)
 */
SIMD_FORCE_INLINE void drx3d_closest_point_on_segment(
	Vec3 &cp, const Vec3 &v,
	const Vec3 &e1, const Vec3 &e2)
{
	Vec3 n = e2 - e1;
	cp = v - e1;
	Scalar _scalar = cp.dot(n) / n.dot(n);
	if (_scalar < 0.0f)
	{
		cp = e1;
	}
	else if (_scalar > 1.0f)
	{
		cp = e2;
	}
	else
	{
		cp = _scalar * n + e1;
	}
}

//! line plane collision
/*!
*\return
	-0  if the ray never intersects
	-1 if the ray collides in front
	-2 if the ray collides in back
*/

SIMD_FORCE_INLINE i32 drx3d_line_plane_collision(
	const Vec4 &plane,
	const Vec3 &vDir,
	const Vec3 &vPoint,
	Vec3 &pout,
	Scalar &tparam,
	Scalar tmin, Scalar tmax)
{
	Scalar _dotdir = vDir.dot(plane);

	if (Fabs(_dotdir) < PLANEDIREPSILON)
	{
		tparam = tmax;
		return 0;
	}

	Scalar _dis = drx3d_distance_point_plane(plane, vPoint);
	char returnvalue = _dis < 0.0f ? 2 : 1;
	tparam = -_dis / _dotdir;

	if (tparam < tmin)
	{
		returnvalue = 0;
		tparam = tmin;
	}
	else if (tparam > tmax)
	{
		returnvalue = 0;
		tparam = tmax;
	}
	pout = tparam * vDir + vPoint;
	return returnvalue;
}

//! Find closest points on segments
SIMD_FORCE_INLINE void drx3d_segment_collision(
	const Vec3 &vA1,
	const Vec3 &vA2,
	const Vec3 &vB1,
	const Vec3 &vB2,
	Vec3 &vPointA,
	Vec3 &vPointB)
{
	Vec3 AD = vA2 - vA1;
	Vec3 BD = vB2 - vB1;
	Vec3 N = AD.cross(BD);
	Scalar tp = N.length2();

	Vec4 _M;  //plane

	if (tp < SIMD_EPSILON)  //ARE PARALELE
	{
		//project B over A
		bool invert_b_order = false;
		_M[0] = vB1.dot(AD);
		_M[1] = vB2.dot(AD);

		if (_M[0] > _M[1])
		{
			invert_b_order = true;
			DRX3D_SWAP_NUMBERS(_M[0], _M[1]);
		}
		_M[2] = vA1.dot(AD);
		_M[3] = vA2.dot(AD);
		//mid points
		N[0] = (_M[0] + _M[1]) * 0.5f;
		N[1] = (_M[2] + _M[3]) * 0.5f;

		if (N[0] < N[1])
		{
			if (_M[1] < _M[2])
			{
				vPointB = invert_b_order ? vB1 : vB2;
				vPointA = vA1;
			}
			else if (_M[1] < _M[3])
			{
				vPointB = invert_b_order ? vB1 : vB2;
				drx3d_closest_point_on_segment(vPointA, vPointB, vA1, vA2);
			}
			else
			{
				vPointA = vA2;
				drx3d_closest_point_on_segment(vPointB, vPointA, vB1, vB2);
			}
		}
		else
		{
			if (_M[3] < _M[0])
			{
				vPointB = invert_b_order ? vB2 : vB1;
				vPointA = vA2;
			}
			else if (_M[3] < _M[1])
			{
				vPointA = vA2;
				drx3d_closest_point_on_segment(vPointB, vPointA, vB1, vB2);
			}
			else
			{
				vPointB = invert_b_order ? vB1 : vB2;
				drx3d_closest_point_on_segment(vPointA, vPointB, vA1, vA2);
			}
		}
		return;
	}

	N = N.cross(BD);
	_M.setVal(N[0], N[1], N[2], vB1.dot(N));

	// get point A as the plane collision point
	drx3d_line_plane_collision(_M, AD, vA1, vPointA, tp, Scalar(0), Scalar(1));

	/*Closest point on segment*/
	vPointB = vPointA - vB1;
	tp = vPointB.dot(BD);
	tp /= BD.dot(BD);
	tp = DRX3D_CLAMP(tp, 0.0f, 1.0f);

	vPointB = tp * BD + vB1;
}

#endif  // GIM_VECTOR_H_INCLUDED
