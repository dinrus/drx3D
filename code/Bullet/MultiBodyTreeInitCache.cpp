#include <drx3D/Physics/Dynamics/Inverse/details/MultiBodyTreeInitCache.h>

namespace drx3d_inverse
{
MultiBodyTree::InitCache::InitCache()
{
	m_inertias.resize(0);
	m_joints.resize(0);
	m_num_dofs = 0;
	m_root_index = -1;
}

i32 MultiBodyTree::InitCache::addBody(i32k body_index, i32k parent_index,
									  const JointType joint_type,
									  const vec3& parent_r_parent_body_ref,
									  const mat33& body_T_parent_ref,
									  const vec3& body_axis_of_motion, const idScalar mass,
									  const vec3& body_r_body_com, const mat33& body_I_body,
									  i32k user_int, uk user_ptr)
{
	switch (joint_type)
	{
		case REVOLUTE:
		case PRISMATIC:
			m_num_dofs += 1;
			break;
		case FIXED:
			// does not add a degree of freedom
			// m_num_dofs+=0;
			break;
		case SPHERICAL:
			m_num_dofs += 3;
			break;
		case FLOATING:
			m_num_dofs += 6;
			break;
		default:
			drx3d_id_error_message("unknown joint type %d\n", joint_type);
			return -1;
	}

	if (-1 == parent_index)
	{
		if (m_root_index >= 0)
		{
			drx3d_id_error_message("trying to add body %d as root, but already added %d as root body\n",
								body_index, m_root_index);
			return -1;
		}
		m_root_index = body_index;
	}

	JointData joint;
	joint.m_child = body_index;
	joint.m_parent = parent_index;
	joint.m_type = joint_type;
	joint.m_parent_pos_parent_child_ref = parent_r_parent_body_ref;
	joint.m_child_T_parent_ref = body_T_parent_ref;
	joint.m_child_axis_of_motion = body_axis_of_motion;

	InertiaData body;
	body.m_mass = mass;
	body.m_body_pos_body_com = body_r_body_com;
	body.m_body_I_body = body_I_body;

	m_inertias.push_back(body);
	m_joints.push_back(joint);
	m_user_int.push_back(user_int);
	m_user_ptr.push_back(user_ptr);
	return 0;
}
i32 MultiBodyTree::InitCache::getInertiaData(i32k index, InertiaData* inertia) const
{
	if (index < 0 || index > static_cast<i32>(m_inertias.size()))
	{
		drx3d_id_error_message("index out of range\n");
		return -1;
	}

	*inertia = m_inertias[index];
	return 0;
}

i32 MultiBodyTree::InitCache::getUserInt(i32k index, i32* user_int) const
{
	if (index < 0 || index > static_cast<i32>(m_user_int.size()))
	{
		drx3d_id_error_message("index out of range\n");
		return -1;
	}
	*user_int = m_user_int[index];
	return 0;
}

i32 MultiBodyTree::InitCache::getUserPtr(i32k index, uk * user_ptr) const
{
	if (index < 0 || index > static_cast<i32>(m_user_ptr.size()))
	{
		drx3d_id_error_message("index out of range\n");
		return -1;
	}
	*user_ptr = m_user_ptr[index];
	return 0;
}

i32 MultiBodyTree::InitCache::getJointData(i32k index, JointData* joint) const
{
	if (index < 0 || index > static_cast<i32>(m_joints.size()))
	{
		drx3d_id_error_message("index out of range\n");
		return -1;
	}
	*joint = m_joints[index];
	return 0;
}

i32 MultiBodyTree::InitCache::buildIndexSets()
{
	// NOTE: This function assumes that proper indices were provided
	//	   User2InternalIndex from utils can be used to facilitate this.

	m_parent_index.resize(numBodies());
	for (idArrayIdx j = 0; j < m_joints.size(); j++)
	{
		const JointData& joint = m_joints[j];
		m_parent_index[joint.m_child] = joint.m_parent;
	}

	return 0;
}
}  // namespace drx3d_inverse
