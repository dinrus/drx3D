#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTree.h>

#include <cmath>
#include <limits>
#include <vector>

#include <drx3D/Physics/Dynamics/Inverse/IDMath.h>
#include <drx3D/Physics/Dynamics/Inverse/details/MultiBodyTreeImpl.h>
#include <drx3D/Physics/Dynamics/Inverse/details/MultiBodyTreeInitCache.h>

namespace drx3d_inverse
{
MultiBodyTree::MultiBodyTree()
	: m_is_finalized(false),
	  m_mass_parameters_are_valid(true),
	  m_accept_invalid_mass_parameters(false),
	  m_impl(0x0),
	  m_init_cache(0x0)
{
	m_init_cache = new InitCache();
}

MultiBodyTree::~MultiBodyTree()
{
	delete m_impl;
	delete m_init_cache;
}

void MultiBodyTree::setAcceptInvalidMassParameters(bool flag)
{
	m_accept_invalid_mass_parameters = flag;
}

bool MultiBodyTree::getAcceptInvalidMassProperties() const
{
	return m_accept_invalid_mass_parameters;
}

i32 MultiBodyTree::getBodyOrigin(i32k body_index, vec3 *world_origin) const
{
	return m_impl->getBodyOrigin(body_index, world_origin);
}

i32 MultiBodyTree::getBodyCoM(i32k body_index, vec3 *world_com) const
{
	return m_impl->getBodyCoM(body_index, world_com);
}

i32 MultiBodyTree::getBodyTransform(i32k body_index, mat33 *world_T_body) const
{
	return m_impl->getBodyTransform(body_index, world_T_body);
}
i32 MultiBodyTree::getBodyAngularVelocity(i32k body_index, vec3 *world_omega) const
{
	return m_impl->getBodyAngularVelocity(body_index, world_omega);
}
i32 MultiBodyTree::getBodyLinearVelocity(i32k body_index, vec3 *world_velocity) const
{
	return m_impl->getBodyLinearVelocity(body_index, world_velocity);
}

i32 MultiBodyTree::getBodyLinearVelocityCoM(i32k body_index, vec3 *world_velocity) const
{
	return m_impl->getBodyLinearVelocityCoM(body_index, world_velocity);
}

i32 MultiBodyTree::getBodyAngularAcceleration(i32k body_index, vec3 *world_dot_omega) const
{
	return m_impl->getBodyAngularAcceleration(body_index, world_dot_omega);
}
i32 MultiBodyTree::getBodyLinearAcceleration(i32k body_index, vec3 *world_acceleration) const
{
	return m_impl->getBodyLinearAcceleration(body_index, world_acceleration);
}

i32 MultiBodyTree::getParentRParentBodyRef(i32k body_index, vec3 *r) const
{
	return m_impl->getParentRParentBodyRef(body_index, r);
}

i32 MultiBodyTree::getBodyTParentRef(i32k body_index, mat33 *T) const
{
	return m_impl->getBodyTParentRef(body_index, T);
}

i32 MultiBodyTree::getBodyAxisOfMotion(i32k body_index, vec3 *axis) const
{
	return m_impl->getBodyAxisOfMotion(body_index, axis);
}

void MultiBodyTree::printTree() { m_impl->printTree(); }
void MultiBodyTree::printTreeData() { m_impl->printTreeData(); }

i32 MultiBodyTree::numBodies() const { return m_impl->m_num_bodies; }

i32 MultiBodyTree::numDoFs() const { return m_impl->m_num_dofs; }

i32 MultiBodyTree::calculatedrx3d_inverse(const vecx &q, const vecx &u, const vecx &dot_u,
											vecx *joint_forces)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculatedrx3d_inverse(q, u, dot_u, joint_forces))
	{
		drx3d_id_error_message("error in inverse dynamics calculation\n");
		return -1;
	}
	return 0;
}

i32 MultiBodyTree::calculateMassMatrix(const vecx &q, const bool update_kinematics,
									   const bool initialize_matrix,
									   const bool set_lower_triangular_matrix, matxx *mass_matrix)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 ==
		m_impl->calculateMassMatrix(q, update_kinematics, initialize_matrix,
									set_lower_triangular_matrix, mass_matrix))
	{
		drx3d_id_error_message("error in mass matrix calculation\n");
		return -1;
	}
	return 0;
}

i32 MultiBodyTree::calculateMassMatrix(const vecx &q, matxx *mass_matrix)
{
	return calculateMassMatrix(q, true, true, true, mass_matrix);
}

i32 MultiBodyTree::calculateKinematics(const vecx &q, const vecx &u, const vecx &dot_u)
{
	vec3 world_gravity(m_impl->m_world_gravity);
	// temporarily set gravity to zero, to ensure we get the actual accelerations
	setZero(m_impl->m_world_gravity);

	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculateKinematics(q, u, dot_u,
										  MultiBodyTree::MultiBodyImpl::POSITION_VELOCITY_ACCELERATION))
	{
		drx3d_id_error_message("error in kinematics calculation\n");
		return -1;
	}

	m_impl->m_world_gravity = world_gravity;
	return 0;
}

i32 MultiBodyTree::calculatePositionKinematics(const vecx &q)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculateKinematics(q, q, q,
										  MultiBodyTree::MultiBodyImpl::POSITION_VELOCITY))
	{
		drx3d_id_error_message("error in kinematics calculation\n");
		return -1;
	}
	return 0;
}

i32 MultiBodyTree::calculatePositionAndVelocityKinematics(const vecx &q, const vecx &u)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculateKinematics(q, u, u,
										  MultiBodyTree::MultiBodyImpl::POSITION_VELOCITY))
	{
		drx3d_id_error_message("error in kinematics calculation\n");
		return -1;
	}
	return 0;
}

#if (defined DRX3D_ID_HAVE_MAT3X) && (defined DRX3D_ID_WITH_JACOBIANS)
i32 MultiBodyTree::calculateJacobians(const vecx &q, const vecx &u)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculateJacobians(q, u,
										 MultiBodyTree::MultiBodyImpl::POSITION_VELOCITY))
	{
		drx3d_id_error_message("error in jacobian calculation\n");
		return -1;
	}
	return 0;
}

i32 MultiBodyTree::calculateJacobians(const vecx &q)
{
	if (false == m_is_finalized)
	{
		drx3d_id_error_message("system has not been initialized\n");
		return -1;
	}
	if (-1 == m_impl->calculateJacobians(q, q,
										 MultiBodyTree::MultiBodyImpl::POSITION_ONLY))
	{
		drx3d_id_error_message("error in jacobian calculation\n");
		return -1;
	}
	return 0;
}

i32 MultiBodyTree::getBodyDotJacobianTransU(i32k body_index, vec3 *world_dot_jac_trans_u) const
{
	return m_impl->getBodyDotJacobianTransU(body_index, world_dot_jac_trans_u);
}

i32 MultiBodyTree::getBodyDotJacobianRotU(i32k body_index, vec3 *world_dot_jac_rot_u) const
{
	return m_impl->getBodyDotJacobianRotU(body_index, world_dot_jac_rot_u);
}

i32 MultiBodyTree::getBodyJacobianTrans(i32k body_index, mat3x *world_jac_trans) const
{
	return m_impl->getBodyJacobianTrans(body_index, world_jac_trans);
}

i32 MultiBodyTree::getBodyJacobianRot(i32k body_index, mat3x *world_jac_rot) const
{
	return m_impl->getBodyJacobianRot(body_index, world_jac_rot);
}

#endif

i32 MultiBodyTree::addBody(i32 body_index, i32 parent_index, JointType joint_type,
						   const vec3 &parent_r_parent_body_ref, const mat33 &body_T_parent_ref,
						   const vec3 &body_axis_of_motion_, idScalar mass,
						   const vec3 &body_r_body_com, const mat33 &body_I_body,
						   i32k user_int, uk user_ptr)
{
	if (body_index < 0)
	{
		drx3d_id_error_message("body index must be positive (got %d)\n", body_index);
		return -1;
	}
	vec3 body_axis_of_motion(body_axis_of_motion_);
	switch (joint_type)
	{
		case REVOLUTE:
		case PRISMATIC:
			// check if axis is unit vector
			if (!isUnitVector(body_axis_of_motion))
			{
				drx3d_id_warning_message(
					"axis of motion not a unit axis ([%f %f %f]), will use normalized vector\n",
					body_axis_of_motion(0), body_axis_of_motion(1), body_axis_of_motion(2));
				idScalar length = DRX3D_ID_SQRT(DRX3D_ID_POW(body_axis_of_motion(0), 2) +
											 DRX3D_ID_POW(body_axis_of_motion(1), 2) +
											 DRX3D_ID_POW(body_axis_of_motion(2), 2));
				if (length < DRX3D_ID_SQRT(std::numeric_limits<idScalar>::min()))
				{
					drx3d_id_error_message("axis of motion vector too short (%e)\n", length);
					return -1;
				}
				body_axis_of_motion = (1.0 / length) * body_axis_of_motion;
			}
			break;
		case FIXED:
			break;
		case FLOATING:
			break;
		case SPHERICAL:
			break;
		default:
			drx3d_id_error_message("unknown joint type %d\n", joint_type);
			return -1;
	}

	// sanity check for mass properties. Zero mass is OK.
	if (mass < 0)
	{
		m_mass_parameters_are_valid = false;
		drx3d_id_error_message("Body %d has invalid mass %e\n", body_index, mass);
		if (!m_accept_invalid_mass_parameters)
		{
			return -1;
		}
	}

	if (!isValidInertiaMatrix(body_I_body, body_index, FIXED == joint_type))
	{
		m_mass_parameters_are_valid = false;
		// error message printed in function call
		if (!m_accept_invalid_mass_parameters)
		{
			return -1;
		}
	}

	if (!isValidTransform2Matrix(body_T_parent_ref))
	{
		return -1;
	}

	return m_init_cache->addBody(body_index, parent_index, joint_type, parent_r_parent_body_ref,
								 body_T_parent_ref, body_axis_of_motion, mass, body_r_body_com,
								 body_I_body, user_int, user_ptr);
}

i32 MultiBodyTree::getParentIndex(i32k body_index, i32 *parent_index) const
{
	return m_impl->getParentIndex(body_index, parent_index);
}

i32 MultiBodyTree::getUserInt(i32k body_index, i32 *user_int) const
{
	return m_impl->getUserInt(body_index, user_int);
}

i32 MultiBodyTree::getUserPtr(i32k body_index, uk *user_ptr) const
{
	return m_impl->getUserPtr(body_index, user_ptr);
}

i32 MultiBodyTree::setUserInt(i32k body_index, i32k user_int)
{
	return m_impl->setUserInt(body_index, user_int);
}

i32 MultiBodyTree::setUserPtr(i32k body_index, uk const user_ptr)
{
	return m_impl->setUserPtr(body_index, user_ptr);
}

i32 MultiBodyTree::finalize()
{
	i32k &num_bodies = m_init_cache->numBodies();
	i32k &num_dofs = m_init_cache->numDoFs();

	if (num_dofs < 0)
	{
		drx3d_id_error_message("Need num_dofs>=1, but num_dofs= %d\n", num_dofs);
		//return -1;
	}

	// 1 allocate internal MultiBody structure
	m_impl = new MultiBodyImpl(num_bodies, num_dofs);

	// 2 build new index set assuring index(parent) < index(child)
	if (-1 == m_init_cache->buildIndexSets())
	{
		return -1;
	}
	m_init_cache->getParentIndexArray(&m_impl->m_parent_index);

	// 3 setup internal kinematic and dynamic data
	for (i32 index = 0; index < num_bodies; index++)
	{
		InertiaData inertia;
		JointData joint;
		if (-1 == m_init_cache->getInertiaData(index, &inertia))
		{
			return -1;
		}
		if (-1 == m_init_cache->getJointData(index, &joint))
		{
			return -1;
		}

		RigidBody &rigid_body = m_impl->m_body_list[index];

		rigid_body.m_mass = inertia.m_mass;
		rigid_body.m_body_mass_com = inertia.m_mass * inertia.m_body_pos_body_com;
		rigid_body.m_body_I_body = inertia.m_body_I_body;
		rigid_body.m_joint_type = joint.m_type;
		rigid_body.m_parent_pos_parent_body_ref = joint.m_parent_pos_parent_child_ref;
		rigid_body.m_body_T_parent_ref = joint.m_child_T_parent_ref;
		rigid_body.m_parent_pos_parent_body_ref = joint.m_parent_pos_parent_child_ref;
		rigid_body.m_joint_type = joint.m_type;

		i32 user_int;
		if (-1 == m_init_cache->getUserInt(index, &user_int))
		{
			return -1;
		}
		if (-1 == m_impl->setUserInt(index, user_int))
		{
			return -1;
		}

		uk user_ptr;
		if (-1 == m_init_cache->getUserPtr(index, &user_ptr))
		{
			return -1;
		}
		if (-1 == m_impl->setUserPtr(index, user_ptr))
		{
			return -1;
		}

		// Set joint Jacobians. Note that the dimension is always 3x1 here to avoid variable sized
		// matrices.
		switch (rigid_body.m_joint_type)
		{
			case REVOLUTE:
				rigid_body.m_Jac_JR(0) = joint.m_child_axis_of_motion(0);
				rigid_body.m_Jac_JR(1) = joint.m_child_axis_of_motion(1);
				rigid_body.m_Jac_JR(2) = joint.m_child_axis_of_motion(2);
				rigid_body.m_Jac_JT(0) = 0.0;
				rigid_body.m_Jac_JT(1) = 0.0;
				rigid_body.m_Jac_JT(2) = 0.0;
				break;
			case PRISMATIC:
				rigid_body.m_Jac_JR(0) = 0.0;
				rigid_body.m_Jac_JR(1) = 0.0;
				rigid_body.m_Jac_JR(2) = 0.0;
				rigid_body.m_Jac_JT(0) = joint.m_child_axis_of_motion(0);
				rigid_body.m_Jac_JT(1) = joint.m_child_axis_of_motion(1);
				rigid_body.m_Jac_JT(2) = joint.m_child_axis_of_motion(2);
				break;
			case FIXED:
				// NOTE/TODO: dimension really should be zero ..
				rigid_body.m_Jac_JR(0) = 0.0;
				rigid_body.m_Jac_JR(1) = 0.0;
				rigid_body.m_Jac_JR(2) = 0.0;
				rigid_body.m_Jac_JT(0) = 0.0;
				rigid_body.m_Jac_JT(1) = 0.0;
				rigid_body.m_Jac_JT(2) = 0.0;
				break;
			case SPHERICAL:
				// NOTE/TODO: this is not really correct.
				// the Jacobians should be 3x3 matrices here !
				rigid_body.m_Jac_JR(0) = 0.0;
				rigid_body.m_Jac_JR(1) = 0.0;
				rigid_body.m_Jac_JR(2) = 0.0;
				rigid_body.m_Jac_JT(0) = 0.0;
				rigid_body.m_Jac_JT(1) = 0.0;
				rigid_body.m_Jac_JT(2) = 0.0;
				break;
			case FLOATING:
				// NOTE/TODO: this is not really correct.
				// the Jacobians should be 3x3 matrices here !
				rigid_body.m_Jac_JR(0) = 0.0;
				rigid_body.m_Jac_JR(1) = 0.0;
				rigid_body.m_Jac_JR(2) = 0.0;
				rigid_body.m_Jac_JT(0) = 0.0;
				rigid_body.m_Jac_JT(1) = 0.0;
				rigid_body.m_Jac_JT(2) = 0.0;
				break;
			default:
				drx3d_id_error_message("unsupported joint type %d\n", rigid_body.m_joint_type);
				return -1;
		}
	}

	// 4 assign degree of freedom indices & build per-joint-type index arrays
	if (-1 == m_impl->generateIndexSets())
	{
		drx3d_id_error_message("generating index sets\n");
		return -1;
	}

	// 5 do some pre-computations ..
	m_impl->calculateStaticData();

	// 6. make sure all user forces are set to zero, as this might not happen
	//	in the vector ctors.
	m_impl->clearAllUserForcesAndMoments();

	m_is_finalized = true;
	return 0;
}

i32 MultiBodyTree::setGravityInWorldFrame(const vec3 &gravity)
{
	return m_impl->setGravityInWorldFrame(gravity);
}

i32 MultiBodyTree::getJointType(i32k body_index, JointType *joint_type) const
{
	return m_impl->getJointType(body_index, joint_type);
}

i32 MultiBodyTree::getJointTypeStr(i32k body_index, tukk* joint_type) const
{
	return m_impl->getJointTypeStr(body_index, joint_type);
}

i32 MultiBodyTree::getDoFOffset(i32k body_index, i32 *q_offset) const
{
	return m_impl->getDoFOffset(body_index, q_offset);
}

i32 MultiBodyTree::setBodyMass(i32k body_index, idScalar mass)
{
	return m_impl->setBodyMass(body_index, mass);
}

i32 MultiBodyTree::setBodyFirstMassMoment(i32k body_index, const vec3 &first_mass_moment)
{
	return m_impl->setBodyFirstMassMoment(body_index, first_mass_moment);
}

i32 MultiBodyTree::setBodySecondMassMoment(i32k body_index, const mat33 &second_mass_moment)
{
	return m_impl->setBodySecondMassMoment(body_index, second_mass_moment);
}

i32 MultiBodyTree::getBodyMass(i32k body_index, idScalar *mass) const
{
	return m_impl->getBodyMass(body_index, mass);
}

i32 MultiBodyTree::getBodyFirstMassMoment(i32k body_index, vec3 *first_mass_moment) const
{
	return m_impl->getBodyFirstMassMoment(body_index, first_mass_moment);
}

i32 MultiBodyTree::getBodySecondMassMoment(i32k body_index, mat33 *second_mass_moment) const
{
	return m_impl->getBodySecondMassMoment(body_index, second_mass_moment);
}

void MultiBodyTree::clearAllUserForcesAndMoments() { m_impl->clearAllUserForcesAndMoments(); }

i32 MultiBodyTree::addUserForce(i32k body_index, const vec3 &body_force)
{
	return m_impl->addUserForce(body_index, body_force);
}

i32 MultiBodyTree::addUserMoment(i32k body_index, const vec3 &body_moment)
{
	return m_impl->addUserMoment(body_index, body_moment);
}

}  // namespace drx3d_inverse
