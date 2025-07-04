#ifndef GIM_CONTACT_H_INCLUDED
#define GIM_CONTACT_H_INCLUDED

#include <drx3D/Physics/Collision/Gimpact/gim_geometry.h>
#include <drx3D/Physics/Collision/Gimpact/gim_radixsort.h>
#include <drx3D/Physics/Collision/Gimpact/gim_array.h>

/**
Configuration var for applying interpolation of  contact normals
*/
#ifndef NORMAL_CONTACT_AVERAGE
#define NORMAL_CONTACT_AVERAGE 1
#endif

#ifndef CONTACT_DIFF_EPSILON
#define CONTACT_DIFF_EPSILON 0.00001f
#endif

#ifndef DRX3D_CONTACT_H_STRUCTS_INCLUDED

/// Structure for collision results
///Functions for managing and sorting contacts resulting from a collision query.
///Contact lists must be create by calling \ref GIM_CREATE_CONTACT_LIST
///After querys, contact lists must be destroy by calling \ref GIM_DYNARRAY_DESTROY
///Contacts can be merge for avoid duplicate results by calling \ref gim_merge_contacts
class GIM_CONTACT
{
public:
	Vec3 m_point;
	Vec3 m_normal;
	GREAL m_depth;     //Positive value indicates interpenetration
	GREAL m_distance;  //Padding not for use
	GUINT m_feature1;  //Face number
	GUINT m_feature2;  //Face number
public:
	GIM_CONTACT()
	{
	}

	GIM_CONTACT(const GIM_CONTACT &contact) : m_point(contact.m_point),
											  m_normal(contact.m_normal),
											  m_depth(contact.m_depth),
											  m_feature1(contact.m_feature1),
											  m_feature2(contact.m_feature2)
	{
		m_point = contact.m_point;
		m_normal = contact.m_normal;
		m_depth = contact.m_depth;
		m_feature1 = contact.m_feature1;
		m_feature2 = contact.m_feature2;
	}

	GIM_CONTACT(const Vec3 &point, const Vec3 &normal,
				GREAL depth, GUINT feature1, GUINT feature2) : m_point(point),
															   m_normal(normal),
															   m_depth(depth),
															   m_feature1(feature1),
															   m_feature2(feature2)
	{
	}

	//! Calcs key for coord classification
	SIMD_FORCE_INLINE GUINT calc_key_contact() const
	{
		GINT _coords[] = {
			(GINT)(m_point[0] * 1000.0f + 1.0f),
			(GINT)(m_point[1] * 1333.0f),
			(GINT)(m_point[2] * 2133.0f + 3.0f)};
		GUINT _hash = 0;
		GUINT *_uitmp = (GUINT *)(&_coords[0]);
		_hash = *_uitmp;
		_uitmp++;
		_hash += (*_uitmp) << 4;
		_uitmp++;
		_hash += (*_uitmp) << 8;
		return _hash;
	}

	SIMD_FORCE_INLINE void interpolate_normals(Vec3 *normals, GUINT normal_count)
	{
		Vec3 vec_sum(m_normal);
		for (GUINT i = 0; i < normal_count; i++)
		{
			vec_sum += normals[i];
		}

		GREAL vec_sum_len = vec_sum.length2();
		if (vec_sum_len < CONTACT_DIFF_EPSILON) return;

		GIM_INV_SQRT(vec_sum_len, vec_sum_len);  // 1/sqrt(vec_sum_len)

		m_normal = vec_sum * vec_sum_len;
	}
};

#endif

class gim_contact_array : public gim_array<GIM_CONTACT>
{
public:
	gim_contact_array() : gim_array<GIM_CONTACT>(64)
	{
	}

	SIMD_FORCE_INLINE void push_contact(const Vec3 &point, const Vec3 &normal,
										GREAL depth, GUINT feature1, GUINT feature2)
	{
		push_back_mem();
		GIM_CONTACT &newele = back();
		newele.m_point = point;
		newele.m_normal = normal;
		newele.m_depth = depth;
		newele.m_feature1 = feature1;
		newele.m_feature2 = feature2;
	}

	SIMD_FORCE_INLINE void push_triangle_contacts(
		const GIM_TRIANGLE_CONTACT_DATA &tricontact,
		GUINT feature1, GUINT feature2)
	{
		for (GUINT i = 0; i < tricontact.m_point_count; i++)
		{
			push_back_mem();
			GIM_CONTACT &newele = back();
			newele.m_point = tricontact.m_points[i];
			newele.m_normal = tricontact.m_separating_normal;
			newele.m_depth = tricontact.m_penetration_depth;
			newele.m_feature1 = feature1;
			newele.m_feature2 = feature2;
		}
	}

	void merge_contacts(const gim_contact_array &contacts, bool normal_contact_average = true);
	void merge_contacts_unique(const gim_contact_array &contacts);
};

#endif  // GIM_CONTACT_H_INCLUDED
