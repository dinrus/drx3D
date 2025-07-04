
#include <drx3D/Physics/Collision/Gimpact/TriangleShapeEx.h>

void GIM_TRIANGLE_CONTACT::merge_points(const Vec4& plane,
										Scalar margin, const Vec3* points, i32 point_count)
{
	m_point_count = 0;
	m_penetration_depth = -1000.0f;

	i32 point_indices[MAX_TRI_CLIPPING];

	i32 _k;

	for (_k = 0; _k < point_count; _k++)
	{
		Scalar _dist = -drx3d_distance_point_plane(plane, points[_k]) + margin;

		if (_dist >= 0.0f)
		{
			if (_dist > m_penetration_depth)
			{
				m_penetration_depth = _dist;
				point_indices[0] = _k;
				m_point_count = 1;
			}
			else if ((_dist + SIMD_EPSILON) >= m_penetration_depth)
			{
				point_indices[m_point_count] = _k;
				m_point_count++;
			}
		}
	}

	for (_k = 0; _k < m_point_count; _k++)
	{
		m_points[_k] = points[point_indices[_k]];
	}
}

///class PrimitiveTriangle
bool PrimitiveTriangle::overlap_test_conservative(const PrimitiveTriangle& other)
{
	Scalar total_margin = m_margin + other.m_margin;
	// classify points on other triangle
	Scalar dis0 = drx3d_distance_point_plane(m_plane, other.m_vertices[0]) - total_margin;

	Scalar dis1 = drx3d_distance_point_plane(m_plane, other.m_vertices[1]) - total_margin;

	Scalar dis2 = drx3d_distance_point_plane(m_plane, other.m_vertices[2]) - total_margin;

	if (dis0 > 0.0f && dis1 > 0.0f && dis2 > 0.0f) return false;

	// classify points on this triangle
	dis0 = drx3d_distance_point_plane(other.m_plane, m_vertices[0]) - total_margin;

	dis1 = drx3d_distance_point_plane(other.m_plane, m_vertices[1]) - total_margin;

	dis2 = drx3d_distance_point_plane(other.m_plane, m_vertices[2]) - total_margin;

	if (dis0 > 0.0f && dis1 > 0.0f && dis2 > 0.0f) return false;

	return true;
}

i32 PrimitiveTriangle::clip_triangle(PrimitiveTriangle& other, Vec3* clipped_points)
{
	// edge 0

	Vec3 temp_points[MAX_TRI_CLIPPING];

	Vec4 edgeplane;

	get_edge_plane(0, edgeplane);

	i32 clipped_count = drx3d_plane_clip_triangle(
		edgeplane, other.m_vertices[0], other.m_vertices[1], other.m_vertices[2], temp_points);

	if (clipped_count == 0) return 0;

	Vec3 temp_points1[MAX_TRI_CLIPPING];

	// edge 1
	get_edge_plane(1, edgeplane);

	clipped_count = drx3d_plane_clip_polygon(edgeplane, temp_points, clipped_count, temp_points1);

	if (clipped_count == 0) return 0;

	// edge 2
	get_edge_plane(2, edgeplane);

	clipped_count = drx3d_plane_clip_polygon(
		edgeplane, temp_points1, clipped_count, clipped_points);

	return clipped_count;
}

bool PrimitiveTriangle::find_triangle_collision_clip_method(PrimitiveTriangle& other, GIM_TRIANGLE_CONTACT& contacts)
{
	Scalar margin = m_margin + other.m_margin;

	Vec3 clipped_points[MAX_TRI_CLIPPING];
	i32 clipped_count;
	//create planes
	// plane v vs U points

	GIM_TRIANGLE_CONTACT contacts1;

	contacts1.m_separating_normal = m_plane;

	clipped_count = clip_triangle(other, clipped_points);

	if (clipped_count == 0)
	{
		return false;  //Reject
	}

	//find most deep interval face1
	contacts1.merge_points(contacts1.m_separating_normal, margin, clipped_points, clipped_count);
	if (contacts1.m_point_count == 0) return false;  // too far
	//Normal pointing to this triangle
	contacts1.m_separating_normal *= -1.f;

	//Clip tri1 by tri2 edges
	GIM_TRIANGLE_CONTACT contacts2;
	contacts2.m_separating_normal = other.m_plane;

	clipped_count = other.clip_triangle(*this, clipped_points);

	if (clipped_count == 0)
	{
		return false;  //Reject
	}

	//find most deep interval face1
	contacts2.merge_points(contacts2.m_separating_normal, margin, clipped_points, clipped_count);
	if (contacts2.m_point_count == 0) return false;  // too far

	////check most dir for contacts
	if (contacts2.m_penetration_depth < contacts1.m_penetration_depth)
	{
		contacts.copy_from(contacts2);
	}
	else
	{
		contacts.copy_from(contacts1);
	}
	return true;
}

///class TriangleShapeEx: public TriangleShape

bool TriangleShapeEx::overlap_test_conservative(const TriangleShapeEx& other)
{
	Scalar total_margin = getMargin() + other.getMargin();

	Vec4 plane0;
	buildTriPlane(plane0);
	Vec4 plane1;
	other.buildTriPlane(plane1);

	// classify points on other triangle
	Scalar dis0 = drx3d_distance_point_plane(plane0, other.m_vertices1[0]) - total_margin;

	Scalar dis1 = drx3d_distance_point_plane(plane0, other.m_vertices1[1]) - total_margin;

	Scalar dis2 = drx3d_distance_point_plane(plane0, other.m_vertices1[2]) - total_margin;

	if (dis0 > 0.0f && dis1 > 0.0f && dis2 > 0.0f) return false;

	// classify points on this triangle
	dis0 = drx3d_distance_point_plane(plane1, m_vertices1[0]) - total_margin;

	dis1 = drx3d_distance_point_plane(plane1, m_vertices1[1]) - total_margin;

	dis2 = drx3d_distance_point_plane(plane1, m_vertices1[2]) - total_margin;

	if (dis0 > 0.0f && dis1 > 0.0f && dis2 > 0.0f) return false;

	return true;
}
