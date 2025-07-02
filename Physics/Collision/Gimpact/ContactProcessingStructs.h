#ifndef DRX3D_CONTACT_H_STRUCTS_INCLUDED
#define DRX3D_CONTACT_H_STRUCTS_INCLUDED

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Gimpact/TriangleShapeEx.h>

/**
Configuration var for applying interpolation of  contact normals
*/
#define NORMAL_CONTACT_AVERAGE 1

#define CONTACT_DIFF_EPSILON 0.00001f

///The GIM_CONTACT is an internal GIMPACT structure, similar to ManifoldPoint.
///@todo: remove and replace GIM_CONTACT by ManifoldPoint.
class GIM_CONTACT
{
public:
	Vec3 m_point;
	Vec3 m_normal;
	Scalar m_depth;     //Positive value indicates interpenetration
	Scalar m_distance;  //Padding not for use
	i32 m_feature1;       //Face number
	i32 m_feature2;       //Face number
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
	}

	GIM_CONTACT(const Vec3 &point, const Vec3 &normal,
				Scalar depth, i32 feature1, i32 feature2) : m_point(point),
															  m_normal(normal),
															  m_depth(depth),
															  m_feature1(feature1),
															  m_feature2(feature2)
	{
	}

	//! Calcs key for coord classification
	SIMD_FORCE_INLINE u32 calc_key_contact() const
	{
		i32 _coords[] = {
			(i32)(m_point[0] * 1000.0f + 1.0f),
			(i32)(m_point[1] * 1333.0f),
			(i32)(m_point[2] * 2133.0f + 3.0f)};
		u32 _hash = 0;
		u32 *_uitmp = (u32 *)(&_coords[0]);
		_hash = *_uitmp;
		_uitmp++;
		_hash += (*_uitmp) << 4;
		_uitmp++;
		_hash += (*_uitmp) << 8;
		return _hash;
	}

	SIMD_FORCE_INLINE void interpolate_normals(Vec3 *normals, i32 normal_count)
	{
		Vec3 vec_sum(m_normal);
		for (i32 i = 0; i < normal_count; i++)
		{
			vec_sum += normals[i];
		}

		Scalar vec_sum_len = vec_sum.length2();
		if (vec_sum_len < CONTACT_DIFF_EPSILON) return;

		//GIM_INV_SQRT(vec_sum_len,vec_sum_len); // 1/sqrt(vec_sum_len)

		m_normal = vec_sum / Sqrt(vec_sum_len);
	}
};

#endif  // DRX3D_CONTACT_H_STRUCTS_INCLUDED
