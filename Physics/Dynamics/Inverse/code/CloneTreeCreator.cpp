#include "../CloneTreeCreator.h"

#include <cstdio>
#include <drx/Core/Core.h>

namespace drx3d_inverse
{
#define CHECK_NULLPTR()                                  \
	do                                                   \
	{                                                    \
		if (m_reference == 0x0)                          \
		{                                                \
			throw drx::Exc("m_reference == 0x0\n"); \
		}                                                \
	} while (0)

#define TRY(x)                                             \
	do                                                     \
	{                                                      \
		if (x == -1)                                       \
		{                                                  \
			throw drx::Exc("ошибка при вызове " #x "\n"); \
		}                                                  \
	} while (0)
CloneTreeCreator::CloneTreeCreator(const MultiBodyTree* reference)
{
	m_reference = reference;
}

CloneTreeCreator::~CloneTreeCreator() {}

i32 CloneTreeCreator::getNumBodies(i32* num_bodies) const
{
	CHECK_NULLPTR();
	*num_bodies = m_reference->numBodies();
	return 0;
}

i32 CloneTreeCreator::getBody(i32k body_index, i32* parent_index, JointType* joint_type,
							  vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref,
							  vec3* body_axis_of_motion, idScalar* mass, vec3* body_r_body_com,
							  mat33* body_I_body, i32* user_int, uk * user_ptr) const
{
	CHECK_NULLPTR();
	TRY(m_reference->getParentIndex(body_index, parent_index));
	TRY(m_reference->getJointType(body_index, joint_type));
	TRY(m_reference->getParentRParentBodyRef(body_index, parent_r_parent_body_ref));
	TRY(m_reference->getBodyTParentRef(body_index, body_T_parent_ref));
	TRY(m_reference->getBodyAxisOfMotion(body_index, body_axis_of_motion));
	TRY(m_reference->getBodyMass(body_index, mass));
	TRY(m_reference->getBodyFirstMassMoment(body_index, body_r_body_com));
	TRY(m_reference->getBodySecondMassMoment(body_index, body_I_body));
	TRY(m_reference->getUserInt(body_index, user_int));
	TRY(m_reference->getUserPtr(body_index, user_ptr));

	return 0;
}
}  // namespace drx3d_inverse
