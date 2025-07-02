#include <drx3D/Physics/Dynamics/Inverse/RandomTreeCreator.h>
#include <drx/Core/Core.h>
#include <cstdio>

#include <drx3D/Physics/Dynamics/Inverse/IDRandomUtil.h>

namespace drx3d_inverse
{
RandomTreeCreator::RandomTreeCreator(i32k max_bodies, bool random_seed)
{
	// seed generator
	if (random_seed)
	{
		randomInit();  // seeds with time()
	}
	else
	{
		randomInit(1);  // seeds with 1
	}
	m_num_bodies = randomInt(1, max_bodies);
}

RandomTreeCreator::~RandomTreeCreator() {}

i32 RandomTreeCreator::getNumBodies(i32* num_bodies) const
{
	*num_bodies = m_num_bodies;
	return 0;
}

i32 RandomTreeCreator::getBody(i32k body_index, i32* parent_index, JointType* joint_type,
							   vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref,
							   vec3* body_axis_of_motion, idScalar* mass, vec3* body_r_body_com,
							   mat33* body_I_body, i32* user_int, uk * user_ptr) const
{
	if (0 == body_index)
	{  //root body
		*parent_index = -1;
	}
	else
	{
		*parent_index = randomInt(0, body_index - 1);
	}

	switch (randomInt(0, 3))
	{
		case 0:
			*joint_type = FIXED;
			break;
		case 1:
			*joint_type = REVOLUTE;
			break;
		case 2:
			*joint_type = PRISMATIC;
			break;
		case 3:
			*joint_type = FLOATING;
			break;
		default:
			throw drx::Exc("randomInt() result out of range\n");
			return -1;
	}

	(*parent_r_parent_body_ref)(0) = randomFloat(-1.0, 1.0);
	(*parent_r_parent_body_ref)(1) = randomFloat(-1.0, 1.0);
	(*parent_r_parent_body_ref)(2) = randomFloat(-1.0, 1.0);

	bodyTParentFromAxisAngle(randomAxis(), randomFloat(-DRX3D_ID_PI, DRX3D_ID_PI), body_T_parent_ref);

	*body_axis_of_motion = randomAxis();
	*mass = randomMass();
	(*body_r_body_com)(0) = randomFloat(-1.0, 1.0);
	(*body_r_body_com)(1) = randomFloat(-1.0, 1.0);
	(*body_r_body_com)(2) = randomFloat(-1.0, 1.0);
	const double a = randomFloat(-DRX3D_ID_PI, DRX3D_ID_PI);
	const double b = randomFloat(-DRX3D_ID_PI, DRX3D_ID_PI);
	const double c = randomFloat(-DRX3D_ID_PI, DRX3D_ID_PI);
	vec3 ii = randomInertiaPrincipal();
	mat33 ii_diag;
	setZero(ii_diag);
	ii_diag(0, 0) = ii(0);
	ii_diag(1, 1) = ii(1);
	ii_diag(2, 2) = ii(2);
	*body_I_body = transformX(a) * transformY(b) * transformZ(c) * ii_diag *
				   transformZ(-c) * transformY(-b) * transformX(-a);
	*user_int = 0;
	*user_ptr = 0;

	return 0;
}
}  // namespace drx3d_inverse
