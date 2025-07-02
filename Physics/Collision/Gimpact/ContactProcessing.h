#ifndef DRX3D_CONTACT_H_INCLUDED
#define DRX3D_CONTACT_H_INCLUDED

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Gimpact/TriangleShapeEx.h>
#include <drx3D/Physics/Collision/Gimpact/ContactProcessingStructs.h>

class ContactArray : public AlignedObjectArray<GIM_CONTACT>
{
public:
	ContactArray()
	{
		reserve(64);
	}

	SIMD_FORCE_INLINE void push_contact(
		const Vec3 &point, const Vec3 &normal,
		Scalar depth, i32 feature1, i32 feature2)
	{
		push_back(GIM_CONTACT(point, normal, depth, feature1, feature2));
	}

	SIMD_FORCE_INLINE void push_triangle_contacts(
		const GIM_TRIANGLE_CONTACT &tricontact,
		i32 feature1, i32 feature2)
	{
		for (i32 i = 0; i < tricontact.m_point_count; i++)
		{
			push_contact(
				tricontact.m_points[i],
				tricontact.m_separating_normal,
				tricontact.m_penetration_depth, feature1, feature2);
		}
	}

	void merge_contacts(const ContactArray &contacts, bool normal_contact_average = true);

	void merge_contacts_unique(const ContactArray &contacts);
};

#endif  // GIM_CONTACT_H_INCLUDED
