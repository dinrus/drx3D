#ifndef GIMPACT_TRIANGLE_SHAPE_EX_H
#define GIMPACT_TRIANGLE_SHAPE_EX_H

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Gimpact/BoxCollision.h>
#include <drx3D/Physics/Collision/Gimpact/ClipPolygon.h>
#include <drx3D/Physics/Collision/Gimpact/GeometryOperations.h>

#define MAX_TRI_CLIPPING 16

//! Structure for collision
struct GIM_TRIANGLE_CONTACT
{
	Scalar m_penetration_depth;
	i32 m_point_count;
	Vec4 m_separating_normal;
	Vec3 m_points[MAX_TRI_CLIPPING];

	SIMD_FORCE_INLINE void copy_from(const GIM_TRIANGLE_CONTACT& other)
	{
		m_penetration_depth = other.m_penetration_depth;
		m_separating_normal = other.m_separating_normal;
		m_point_count = other.m_point_count;
		i32 i = m_point_count;
		while (i--)
		{
			m_points[i] = other.m_points[i];
		}
	}

	GIM_TRIANGLE_CONTACT()
	{
	}

	GIM_TRIANGLE_CONTACT(const GIM_TRIANGLE_CONTACT& other)
	{
		copy_from(other);
	}

	//! classify points that are closer
	void merge_points(const Vec4& plane,
					  Scalar margin, const Vec3* points, i32 point_count);
};

class PrimitiveTriangle
{
public:
	Vec3 m_vertices[3];
	Vec4 m_plane;
	Scalar m_margin;
	Scalar m_dummy;
	PrimitiveTriangle() : m_margin(0.01f)
	{
	}

	SIMD_FORCE_INLINE void buildTriPlane()
	{
		Vec3 normal = (m_vertices[1] - m_vertices[0]).cross(m_vertices[2] - m_vertices[0]);
		normal.normalize();
		m_plane.setVal(normal[0], normal[1], normal[2], m_vertices[0].dot(normal));
	}

	//! Test if triangles could collide
	bool overlap_test_conservative(const PrimitiveTriangle& other);

	//! Calcs the plane which is paralele to the edge and perpendicular to the triangle plane
	/*!
	\pre this triangle must have its plane calculated.
	*/
	SIMD_FORCE_INLINE void get_edge_plane(i32 edge_index, Vec4& plane) const
	{
		const Vec3& e0 = m_vertices[edge_index];
		const Vec3& e1 = m_vertices[(edge_index + 1) % 3];
		drx3d_edge_plane(e0, e1, m_plane, plane);
	}

	void applyTransform2(const Transform2& t)
	{
		m_vertices[0] = t(m_vertices[0]);
		m_vertices[1] = t(m_vertices[1]);
		m_vertices[2] = t(m_vertices[2]);
	}

	//! Clips the triangle against this
	/*!
	\pre clipped_points must have MAX_TRI_CLIPPING size, and this triangle must have its plane calculated.
	\return the number of clipped points
	*/
	i32 clip_triangle(PrimitiveTriangle& other, Vec3* clipped_points);

	//! Find collision using the clipping method
	/*!
	\pre this triangle and other must have their triangles calculated
	*/
	bool find_triangle_collision_clip_method(PrimitiveTriangle& other, GIM_TRIANGLE_CONTACT& contacts);
};

//! Helper class for colliding drx3D Triangle Shapes
/*!
This class implements a better getAabb method than the previous TriangleShape class
*/
class TriangleShapeEx : public TriangleShape
{
public:
	TriangleShapeEx() : TriangleShape(Vec3(0, 0, 0), Vec3(0, 0, 0), Vec3(0, 0, 0))
	{
	}

	TriangleShapeEx(const Vec3& p0, const Vec3& p1, const Vec3& p2) : TriangleShape(p0, p1, p2)
	{
	}

	TriangleShapeEx(const TriangleShapeEx& other) : TriangleShape(other.m_vertices1[0], other.m_vertices1[1], other.m_vertices1[2])
	{
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		Vec3 tv0 = t(m_vertices1[0]);
		Vec3 tv1 = t(m_vertices1[1]);
		Vec3 tv2 = t(m_vertices1[2]);

		AABB trianglebox(tv0, tv1, tv2, m_collisionMargin);
		aabbMin = trianglebox.m_min;
		aabbMax = trianglebox.m_max;
	}

	void applyTransform2(const Transform2& t)
	{
		m_vertices1[0] = t(m_vertices1[0]);
		m_vertices1[1] = t(m_vertices1[1]);
		m_vertices1[2] = t(m_vertices1[2]);
	}

	SIMD_FORCE_INLINE void buildTriPlane(Vec4& plane) const
	{
		Vec3 normal = (m_vertices1[1] - m_vertices1[0]).cross(m_vertices1[2] - m_vertices1[0]);
		normal.normalize();
		plane.setVal(normal[0], normal[1], normal[2], m_vertices1[0].dot(normal));
	}

	bool overlap_test_conservative(const TriangleShapeEx& other);
};

#endif  //GIMPACT_TRIANGLE_MESH_SHAPE_H
