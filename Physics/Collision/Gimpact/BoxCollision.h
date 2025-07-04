#ifndef DRX3D_BOX_COLLISION_H_INCLUDED
#define DRX3D_BOX_COLLISION_H_INCLUDED

#include <drx3D/Maths/Linear/Transform2.h>

///Swap numbers
#define DRX3D_SWAP_NUMBERS(a, b) \
	{                         \
		a = a + b;            \
		b = a - b;            \
		a = a - b;            \
	}

#define DRX3D_MAX(a, b) (a < b ? b : a)
#define DRX3D_MIN(a, b) (a > b ? b : a)

#define DRX3D_GREATER(x, y) Fabs(x) > (y)

#define DRX3D_MAX3(a, b, c) DRX3D_MAX(a, DRX3D_MAX(b, c))
#define DRX3D_MIN3(a, b, c) DRX3D_MIN(a, DRX3D_MIN(b, c))

enum eDRX3D_PLANE_INTERSECTION_TYPE
{
	DRX3D_CONST_BACK_PLANE = 0,
	DRX3D_CONST_COLLIDE_PLANE,
	DRX3D_CONST_FRONT_PLANE
};

//SIMD_FORCE_INLINE bool test_cross_edge_box(
//	const Vec3 & edge,
//	const Vec3 & absolute_edge,
//	const Vec3 & pointa,
//	const Vec3 & pointb, const Vec3 & extend,
//	i32 dir_index0,
//	i32 dir_index1
//	i32 component_index0,
//	i32 component_index1)
//{
//	// dir coords are -z and y
//
//	const Scalar dir0 = -edge[dir_index0];
//	const Scalar dir1 = edge[dir_index1];
//	Scalar pmin = pointa[component_index0]*dir0 + pointa[component_index1]*dir1;
//	Scalar pmax = pointb[component_index0]*dir0 + pointb[component_index1]*dir1;
//	//find minmax
//	if(pmin>pmax)
//	{
//		DRX3D_SWAP_NUMBERS(pmin,pmax);
//	}
//	//find extends
//	const Scalar rad = extend[component_index0] * absolute_edge[dir_index0] +
//					extend[component_index1] * absolute_edge[dir_index1];
//
//	if(pmin>rad || -rad>pmax) return false;
//	return true;
//}
//
//SIMD_FORCE_INLINE bool test_cross_edge_box_X_axis(
//	const Vec3 & edge,
//	const Vec3 & absolute_edge,
//	const Vec3 & pointa,
//	const Vec3 & pointb, Vec3 & extend)
//{
//
//	return test_cross_edge_box(edge,absolute_edge,pointa,pointb,extend,2,1,1,2);
//}
//
//
//SIMD_FORCE_INLINE bool test_cross_edge_box_Y_axis(
//	const Vec3 & edge,
//	const Vec3 & absolute_edge,
//	const Vec3 & pointa,
//	const Vec3 & pointb, Vec3 & extend)
//{
//
//	return test_cross_edge_box(edge,absolute_edge,pointa,pointb,extend,0,2,2,0);
//}
//
//SIMD_FORCE_INLINE bool test_cross_edge_box_Z_axis(
//	const Vec3 & edge,
//	const Vec3 & absolute_edge,
//	const Vec3 & pointa,
//	const Vec3 & pointb, Vec3 & extend)
//{
//
//	return test_cross_edge_box(edge,absolute_edge,pointa,pointb,extend,1,0,0,1);
//}

#define TEST_CROSS_EDGE_BOX_MCR(edge, absolute_edge, pointa, pointb, _extend, i_dir_0, i_dir_1, i_comp_0, i_comp_1) \
	{                                                                                                               \
		const Scalar dir0 = -edge[i_dir_0];                                                                       \
		const Scalar dir1 = edge[i_dir_1];                                                                        \
		Scalar pmin = pointa[i_comp_0] * dir0 + pointa[i_comp_1] * dir1;                                          \
		Scalar pmax = pointb[i_comp_0] * dir0 + pointb[i_comp_1] * dir1;                                          \
		if (pmin > pmax)                                                                                            \
		{                                                                                                           \
			DRX3D_SWAP_NUMBERS(pmin, pmax);                                                                            \
		}                                                                                                           \
		const Scalar abs_dir0 = absolute_edge[i_dir_0];                                                           \
		const Scalar abs_dir1 = absolute_edge[i_dir_1];                                                           \
		const Scalar rad = _extend[i_comp_0] * abs_dir0 + _extend[i_comp_1] * abs_dir1;                           \
		if (pmin > rad || -rad > pmax) return false;                                                                \
	}

#define TEST_CROSS_EDGE_BOX_X_AXIS_MCR(edge, absolute_edge, pointa, pointb, _extend)       \
	{                                                                                      \
		TEST_CROSS_EDGE_BOX_MCR(edge, absolute_edge, pointa, pointb, _extend, 2, 1, 1, 2); \
	}

#define TEST_CROSS_EDGE_BOX_Y_AXIS_MCR(edge, absolute_edge, pointa, pointb, _extend)       \
	{                                                                                      \
		TEST_CROSS_EDGE_BOX_MCR(edge, absolute_edge, pointa, pointb, _extend, 0, 2, 2, 0); \
	}

#define TEST_CROSS_EDGE_BOX_Z_AXIS_MCR(edge, absolute_edge, pointa, pointb, _extend)       \
	{                                                                                      \
		TEST_CROSS_EDGE_BOX_MCR(edge, absolute_edge, pointa, pointb, _extend, 1, 0, 0, 1); \
	}

//! Returns the dot product between a vec3f and the col of a matrix
SIMD_FORCE_INLINE Scalar drx3d_mat3_dot_col(
	const Matrix3x3 &mat, const Vec3 &vec3, i32 colindex)
{
	return vec3[0] * mat[0][colindex] + vec3[1] * mat[1][colindex] + vec3[2] * mat[2][colindex];
}

//!  Class for transforming a model1 to the space of model0
ATTRIBUTE_ALIGNED16(class)
DRX3D_BOX_BOX_TRANSFORM_CACHE
{
public:
	Vec3 m_T1to0;    //!< Transforms translation of model1 to model 0
	Matrix3x3 m_R1to0;  //!< Transforms Rotation of model1 to model 0, equal  to R0' * R1
	Matrix3x3 m_AR;     //!< Absolute value of m_R1to0

	SIMD_FORCE_INLINE void calc_absolute_matrix()
	{
		//		static const Vec3 vepsi(1e-6f,1e-6f,1e-6f);
		//		m_AR[0] = vepsi + m_R1to0[0].absolute();
		//		m_AR[1] = vepsi + m_R1to0[1].absolute();
		//		m_AR[2] = vepsi + m_R1to0[2].absolute();

		i32 i, j;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				m_AR[i][j] = 1e-6f + Fabs(m_R1to0[i][j]);
			}
		}
	}

	DRX3D_BOX_BOX_TRANSFORM_CACHE()
	{
	}

	//! Calc the transformation relative  1 to 0. Inverts matrics by transposing
	SIMD_FORCE_INLINE void calc_from_homogenic(const Transform2 &trans0, const Transform2 &trans1)
	{
		Transform2 temp_trans = trans0.inverse();
		temp_trans = temp_trans * trans1;

		m_T1to0 = temp_trans.getOrigin();
		m_R1to0 = temp_trans.getBasis();

		calc_absolute_matrix();
	}

	//! Calcs the full invertion of the matrices. Useful for scaling matrices
	SIMD_FORCE_INLINE void calc_from_full_invert(const Transform2 &trans0, const Transform2 &trans1)
	{
		m_R1to0 = trans0.getBasis().inverse();
		m_T1to0 = m_R1to0 * (-trans0.getOrigin());

		m_T1to0 += m_R1to0 * trans1.getOrigin();
		m_R1to0 *= trans1.getBasis();

		calc_absolute_matrix();
	}

	SIMD_FORCE_INLINE Vec3 transform(const Vec3 &point) const
	{
		return point.dot3(m_R1to0[0], m_R1to0[1], m_R1to0[2]) + m_T1to0;
	}
};

#define BOX_PLANE_EPSILON 0.000001f

//! Axis aligned box
ATTRIBUTE_ALIGNED16(class)
AABB
{
public:
	Vec3 m_min;
	Vec3 m_max;

	AABB()
	{
	}

	AABB(const Vec3 &V1,
		   const Vec3 &V2,
		   const Vec3 &V3)
	{
		m_min[0] = DRX3D_MIN3(V1[0], V2[0], V3[0]);
		m_min[1] = DRX3D_MIN3(V1[1], V2[1], V3[1]);
		m_min[2] = DRX3D_MIN3(V1[2], V2[2], V3[2]);
		m_min[3] = 0.f;

		m_max[0] = DRX3D_MAX3(V1[0], V2[0], V3[0]);
		m_max[1] = DRX3D_MAX3(V1[1], V2[1], V3[1]);
		m_max[2] = DRX3D_MAX3(V1[2], V2[2], V3[2]);
		m_max[3] = 0.f;
	}

	AABB(const Vec3 &V1,
		   const Vec3 &V2,
		   const Vec3 &V3,
		   Scalar margin)
	{
		m_min[0] = DRX3D_MIN3(V1[0], V2[0], V3[0]);
		m_min[1] = DRX3D_MIN3(V1[1], V2[1], V3[1]);
		m_min[2] = DRX3D_MIN3(V1[2], V2[2], V3[2]);
		m_min[3] = 0.f;

		m_max[0] = DRX3D_MAX3(V1[0], V2[0], V3[0]);
		m_max[1] = DRX3D_MAX3(V1[1], V2[1], V3[1]);
		m_max[2] = DRX3D_MAX3(V1[2], V2[2], V3[2]);
		m_max[3] = 0.f;

		m_min[0] -= margin;
		m_min[1] -= margin;
		m_min[2] -= margin;
		m_max[0] += margin;
		m_max[1] += margin;
		m_max[2] += margin;
	}

	AABB(const AABB &other) : m_min(other.m_min), m_max(other.m_max)
	{
	}

	AABB(const AABB &other, Scalar margin) : m_min(other.m_min), m_max(other.m_max)
	{
		m_min[0] -= margin;
		m_min[1] -= margin;
		m_min[2] -= margin;
		m_max[0] += margin;
		m_max[1] += margin;
		m_max[2] += margin;
	}

	SIMD_FORCE_INLINE void invalidate()
	{
		m_min[0] = SIMD_INFINITY;
		m_min[1] = SIMD_INFINITY;
		m_min[2] = SIMD_INFINITY;
		m_min[3] = 0.f;
		m_max[0] = -SIMD_INFINITY;
		m_max[1] = -SIMD_INFINITY;
		m_max[2] = -SIMD_INFINITY;
		m_max[3] = 0.f;
	}

	SIMD_FORCE_INLINE void increment_margin(Scalar margin)
	{
		m_min[0] -= margin;
		m_min[1] -= margin;
		m_min[2] -= margin;
		m_max[0] += margin;
		m_max[1] += margin;
		m_max[2] += margin;
	}

	SIMD_FORCE_INLINE void copy_with_margin(const AABB &other, Scalar margin)
	{
		m_min[0] = other.m_min[0] - margin;
		m_min[1] = other.m_min[1] - margin;
		m_min[2] = other.m_min[2] - margin;
		m_min[3] = 0.f;

		m_max[0] = other.m_max[0] + margin;
		m_max[1] = other.m_max[1] + margin;
		m_max[2] = other.m_max[2] + margin;
		m_max[3] = 0.f;
	}

	template <typename CLASS_POINT>
	SIMD_FORCE_INLINE void calc_from_triangle(
		const CLASS_POINT &V1,
		const CLASS_POINT &V2,
		const CLASS_POINT &V3)
	{
		m_min[0] = DRX3D_MIN3(V1[0], V2[0], V3[0]);
		m_min[1] = DRX3D_MIN3(V1[1], V2[1], V3[1]);
		m_min[2] = DRX3D_MIN3(V1[2], V2[2], V3[2]);
		m_min[3] = 0.f;

		m_max[0] = DRX3D_MAX3(V1[0], V2[0], V3[0]);
		m_max[1] = DRX3D_MAX3(V1[1], V2[1], V3[1]);
		m_max[2] = DRX3D_MAX3(V1[2], V2[2], V3[2]);
		m_max[3] = 0.f;
	}

	template <typename CLASS_POINT>
	SIMD_FORCE_INLINE void calc_from_triangle_margin(
		const CLASS_POINT &V1,
		const CLASS_POINT &V2,
		const CLASS_POINT &V3, Scalar margin)
	{
		m_min[0] = DRX3D_MIN3(V1[0], V2[0], V3[0]);
		m_min[1] = DRX3D_MIN3(V1[1], V2[1], V3[1]);
		m_min[2] = DRX3D_MIN3(V1[2], V2[2], V3[2]);
		m_min[3] = 0.f;

		m_max[0] = DRX3D_MAX3(V1[0], V2[0], V3[0]);
		m_max[1] = DRX3D_MAX3(V1[1], V2[1], V3[1]);
		m_max[2] = DRX3D_MAX3(V1[2], V2[2], V3[2]);
		m_max[3] = 0.f;

		m_min[0] -= margin;
		m_min[1] -= margin;
		m_min[2] -= margin;
		m_max[0] += margin;
		m_max[1] += margin;
		m_max[2] += margin;
	}

	//! Apply a transform to an AABB
	SIMD_FORCE_INLINE void appy_transform(const Transform2 &trans)
	{
		Vec3 center = (m_max + m_min) * 0.5f;
		Vec3 extends = m_max - center;
		// Compute new center
		center = trans(center);

		Vec3 textends = extends.dot3(trans.getBasis().getRow(0).absolute(),
										  trans.getBasis().getRow(1).absolute(),
										  trans.getBasis().getRow(2).absolute());

		m_min = center - textends;
		m_max = center + textends;
	}

	//! Apply a transform to an AABB
	SIMD_FORCE_INLINE void appy_transform_trans_cache(const DRX3D_BOX_BOX_TRANSFORM_CACHE &trans)
	{
		Vec3 center = (m_max + m_min) * 0.5f;
		Vec3 extends = m_max - center;
		// Compute new center
		center = trans.transform(center);

		Vec3 textends = extends.dot3(trans.m_R1to0.getRow(0).absolute(),
										  trans.m_R1to0.getRow(1).absolute(),
										  trans.m_R1to0.getRow(2).absolute());

		m_min = center - textends;
		m_max = center + textends;
	}

	//! Merges a Box
	SIMD_FORCE_INLINE void merge(const AABB &box)
	{
		m_min[0] = DRX3D_MIN(m_min[0], box.m_min[0]);
		m_min[1] = DRX3D_MIN(m_min[1], box.m_min[1]);
		m_min[2] = DRX3D_MIN(m_min[2], box.m_min[2]);

		m_max[0] = DRX3D_MAX(m_max[0], box.m_max[0]);
		m_max[1] = DRX3D_MAX(m_max[1], box.m_max[1]);
		m_max[2] = DRX3D_MAX(m_max[2], box.m_max[2]);
	}

	//! Merges a point
	template <typename CLASS_POINT>
	SIMD_FORCE_INLINE void merge_point(const CLASS_POINT &point)
	{
		m_min[0] = DRX3D_MIN(m_min[0], point[0]);
		m_min[1] = DRX3D_MIN(m_min[1], point[1]);
		m_min[2] = DRX3D_MIN(m_min[2], point[2]);

		m_max[0] = DRX3D_MAX(m_max[0], point[0]);
		m_max[1] = DRX3D_MAX(m_max[1], point[1]);
		m_max[2] = DRX3D_MAX(m_max[2], point[2]);
	}

	//! Gets the extend and center
	SIMD_FORCE_INLINE void get_center_extend(Vec3 & center, Vec3 & extend) const
	{
		center = (m_max + m_min) * 0.5f;
		extend = m_max - center;
	}

	//! Finds the intersecting box between this box and the other.
	SIMD_FORCE_INLINE void find_intersection(const AABB &other, AABB &intersection) const
	{
		intersection.m_min[0] = DRX3D_MAX(other.m_min[0], m_min[0]);
		intersection.m_min[1] = DRX3D_MAX(other.m_min[1], m_min[1]);
		intersection.m_min[2] = DRX3D_MAX(other.m_min[2], m_min[2]);

		intersection.m_max[0] = DRX3D_MIN(other.m_max[0], m_max[0]);
		intersection.m_max[1] = DRX3D_MIN(other.m_max[1], m_max[1]);
		intersection.m_max[2] = DRX3D_MIN(other.m_max[2], m_max[2]);
	}

	SIMD_FORCE_INLINE bool has_collision(const AABB &other) const
	{
		if (m_min[0] > other.m_max[0] ||
			m_max[0] < other.m_min[0] ||
			m_min[1] > other.m_max[1] ||
			m_max[1] < other.m_min[1] ||
			m_min[2] > other.m_max[2] ||
			m_max[2] < other.m_min[2])
		{
			return false;
		}
		return true;
	}

	/*! \brief Finds the Ray intersection parameter.
	\param aabb Aligned box
	\param vorigin A vec3f with the origin of the ray
	\param vdir A vec3f with the direction of the ray
	*/
	SIMD_FORCE_INLINE bool collide_ray(const Vec3 &vorigin, const Vec3 &vdir) const
	{
		Vec3 extents, center;
		this->get_center_extend(center, extents);
		;

		Scalar Dx = vorigin[0] - center[0];
		if (DRX3D_GREATER(Dx, extents[0]) && Dx * vdir[0] >= 0.0f) return false;
		Scalar Dy = vorigin[1] - center[1];
		if (DRX3D_GREATER(Dy, extents[1]) && Dy * vdir[1] >= 0.0f) return false;
		Scalar Dz = vorigin[2] - center[2];
		if (DRX3D_GREATER(Dz, extents[2]) && Dz * vdir[2] >= 0.0f) return false;

		Scalar f = vdir[1] * Dz - vdir[2] * Dy;
		if (Fabs(f) > extents[1] * Fabs(vdir[2]) + extents[2] * Fabs(vdir[1])) return false;
		f = vdir[2] * Dx - vdir[0] * Dz;
		if (Fabs(f) > extents[0] * Fabs(vdir[2]) + extents[2] * Fabs(vdir[0])) return false;
		f = vdir[0] * Dy - vdir[1] * Dx;
		if (Fabs(f) > extents[0] * Fabs(vdir[1]) + extents[1] * Fabs(vdir[0])) return false;
		return true;
	}

	SIMD_FORCE_INLINE void projection_interval(const Vec3 &direction, Scalar &vmin, Scalar &vmax) const
	{
		Vec3 center = (m_max + m_min) * 0.5f;
		Vec3 extend = m_max - center;

		Scalar _fOrigin = direction.dot(center);
		Scalar _fMaximumExtent = extend.dot(direction.absolute());
		vmin = _fOrigin - _fMaximumExtent;
		vmax = _fOrigin + _fMaximumExtent;
	}

	SIMD_FORCE_INLINE eDRX3D_PLANE_INTERSECTION_TYPE plane_classify(const Vec4 &plane) const
	{
		Scalar _fmin, _fmax;
		this->projection_interval(plane, _fmin, _fmax);

		if (plane[3] > _fmax + BOX_PLANE_EPSILON)
		{
			return DRX3D_CONST_BACK_PLANE;  // 0
		}

		if (plane[3] + BOX_PLANE_EPSILON >= _fmin)
		{
			return DRX3D_CONST_COLLIDE_PLANE;  //1
		}
		return DRX3D_CONST_FRONT_PLANE;  //2
	}

	SIMD_FORCE_INLINE bool overlapping_trans_conservative(const AABB &box, Transform2 &trans1_to_0) const
	{
		AABB tbox = box;
		tbox.appy_transform(trans1_to_0);
		return has_collision(tbox);
	}

	SIMD_FORCE_INLINE bool overlapping_trans_conservative2(const AABB &box,
														   const DRX3D_BOX_BOX_TRANSFORM_CACHE &trans1_to_0) const
	{
		AABB tbox = box;
		tbox.appy_transform_trans_cache(trans1_to_0);
		return has_collision(tbox);
	}

	//! transcache is the transformation cache from box to this AABB
	SIMD_FORCE_INLINE bool overlapping_trans_cache(
		const AABB &box, const DRX3D_BOX_BOX_TRANSFORM_CACHE &transcache, bool fulltest) const
	{
		//Taken from OPCODE
		Vec3 ea, eb;  //extends
		Vec3 ca, cb;  //extends
		get_center_extend(ca, ea);
		box.get_center_extend(cb, eb);

		Vec3 T;
		Scalar t, t2;
		i32 i;

		// Class I : A's basis vectors
		for (i = 0; i < 3; i++)
		{
			T[i] = transcache.m_R1to0[i].dot(cb) + transcache.m_T1to0[i] - ca[i];
			t = transcache.m_AR[i].dot(eb) + ea[i];
			if (DRX3D_GREATER(T[i], t)) return false;
		}
		// Class II : B's basis vectors
		for (i = 0; i < 3; i++)
		{
			t = drx3d_mat3_dot_col(transcache.m_R1to0, T, i);
			t2 = drx3d_mat3_dot_col(transcache.m_AR, ea, i) + eb[i];
			if (DRX3D_GREATER(t, t2)) return false;
		}
		// Class III : 9 cross products
		if (fulltest)
		{
			i32 j, m, n, o, p, q, r;
			for (i = 0; i < 3; i++)
			{
				m = (i + 1) % 3;
				n = (i + 2) % 3;
				o = i == 0 ? 1 : 0;
				p = i == 2 ? 1 : 2;
				for (j = 0; j < 3; j++)
				{
					q = j == 2 ? 1 : 2;
					r = j == 0 ? 1 : 0;
					t = T[n] * transcache.m_R1to0[m][j] - T[m] * transcache.m_R1to0[n][j];
					t2 = ea[o] * transcache.m_AR[p][j] + ea[p] * transcache.m_AR[o][j] +
						 eb[r] * transcache.m_AR[i][q] + eb[q] * transcache.m_AR[i][r];
					if (DRX3D_GREATER(t, t2)) return false;
				}
			}
		}
		return true;
	}

	//! Simple test for planes.
	SIMD_FORCE_INLINE bool collide_plane(
		const Vec4 &plane) const
	{
		eDRX3D_PLANE_INTERSECTION_TYPE classify = plane_classify(plane);
		return (classify == DRX3D_CONST_COLLIDE_PLANE);
	}

	//! test for a triangle, with edges
	SIMD_FORCE_INLINE bool collide_triangle_exact(
		const Vec3 &p1,
		const Vec3 &p2,
		const Vec3 &p3,
		const Vec4 &triangle_plane) const
	{
		if (!collide_plane(triangle_plane)) return false;

		Vec3 center, extends;
		this->get_center_extend(center, extends);

		const Vec3 v1(p1 - center);
		const Vec3 v2(p2 - center);
		const Vec3 v3(p3 - center);

		//First axis
		Vec3 diff(v2 - v1);
		Vec3 abs_diff = diff.absolute();
		//Test With X axis
		TEST_CROSS_EDGE_BOX_X_AXIS_MCR(diff, abs_diff, v1, v3, extends);
		//Test With Y axis
		TEST_CROSS_EDGE_BOX_Y_AXIS_MCR(diff, abs_diff, v1, v3, extends);
		//Test With Z axis
		TEST_CROSS_EDGE_BOX_Z_AXIS_MCR(diff, abs_diff, v1, v3, extends);

		diff = v3 - v2;
		abs_diff = diff.absolute();
		//Test With X axis
		TEST_CROSS_EDGE_BOX_X_AXIS_MCR(diff, abs_diff, v2, v1, extends);
		//Test With Y axis
		TEST_CROSS_EDGE_BOX_Y_AXIS_MCR(diff, abs_diff, v2, v1, extends);
		//Test With Z axis
		TEST_CROSS_EDGE_BOX_Z_AXIS_MCR(diff, abs_diff, v2, v1, extends);

		diff = v1 - v3;
		abs_diff = diff.absolute();
		//Test With X axis
		TEST_CROSS_EDGE_BOX_X_AXIS_MCR(diff, abs_diff, v3, v2, extends);
		//Test With Y axis
		TEST_CROSS_EDGE_BOX_Y_AXIS_MCR(diff, abs_diff, v3, v2, extends);
		//Test With Z axis
		TEST_CROSS_EDGE_BOX_Z_AXIS_MCR(diff, abs_diff, v3, v2, extends);

		return true;
	}
};

//! Compairison of transformation objects
SIMD_FORCE_INLINE bool CompareTransformsEqual(const Transform2 &t1, const Transform2 &t2)
{
	if (!(t1.getOrigin() == t2.getOrigin())) return false;

	if (!(t1.getBasis().getRow(0) == t2.getBasis().getRow(0))) return false;
	if (!(t1.getBasis().getRow(1) == t2.getBasis().getRow(1))) return false;
	if (!(t1.getBasis().getRow(2) == t2.getBasis().getRow(2))) return false;
	return true;
}

#endif  // GIM_BOX_COLLISION_H_INCLUDED
