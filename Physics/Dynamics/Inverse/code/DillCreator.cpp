#include "../DillCreator.h"
#include <cmath>
#include <drx/Core/Core.h>

namespace drx3d_inverse
{
DillCreator::DillCreator(i32 level)
	: m_level(level),
	  m_num_bodies(DRX3D_ID_POW(2, level))
{
	m_parent.resize(m_num_bodies);
	m_parent_r_parent_body_ref.resize(m_num_bodies);
	m_body_T_parent_ref.resize(m_num_bodies);
	m_body_axis_of_motion.resize(m_num_bodies);
	m_mass.resize(m_num_bodies);
	m_body_r_body_com.resize(m_num_bodies);
	m_body_I_body.resize(m_num_bodies);

	// generate names (for debugging)
	for (i32 i = 0; i < m_num_bodies; i++)
	{
		m_parent[i] = i - 1;

		// all z-axis (DH convention)
		m_body_axis_of_motion[i](0) = 0.0;
		m_body_axis_of_motion[i](1) = 0.0;
		m_body_axis_of_motion[i](2) = 1.0;
	}

	// recursively build data structures
	m_current_body = 0;
	i32k parent = -1;
	const idScalar d_DH = 0.0;
	const idScalar a_DH = 0.0;
	const idScalar alpha_DH = 0.0;

	if (-1 == recurseDill(m_level, parent, d_DH, a_DH, alpha_DH))
	{
		throw drx::Exc("recurseDill failed\n");
		abort();
	}
}

DillCreator::~DillCreator() {}

i32 DillCreator::getNumBodies(i32* num_bodies) const
{
	*num_bodies = m_num_bodies;
	return 0;
}

i32 DillCreator::getBody(i32k body_index, i32* parent_index, JointType* joint_type,
						 vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref,
						 vec3* body_axis_of_motion, idScalar* mass, vec3* body_r_body_com,
						 mat33* body_I_body, i32* user_int, uk * user_ptr) const
{
	if (body_index < 0 || body_index >= m_num_bodies)
	{
		throw drx::Exc("invalid body index\n");// %d\n", body_index);
		return -1;
	}
	*parent_index = m_parent[body_index];
	*joint_type = REVOLUTE;
	*parent_r_parent_body_ref = m_parent_r_parent_body_ref[body_index];
	*body_T_parent_ref = m_body_T_parent_ref[body_index];
	*body_axis_of_motion = m_body_axis_of_motion[body_index];
	*mass = m_mass[body_index];
	*body_r_body_com = m_body_r_body_com[body_index];
	*body_I_body = m_body_I_body[body_index];

	*user_int = 0;
	*user_ptr = 0;
	return 0;
}

i32 DillCreator::recurseDill(i32k level, i32k parent, const idScalar d_DH_in,
							 const idScalar a_DH_in, const idScalar alpha_DH_in)
{
	if (level < 0)
	{
		throw drx::Exc("invalid level parameter\n");// (%d)\n", level);
		return -1;
	}

	if (m_current_body >= m_num_bodies || m_current_body < 0)
	{
		throw drx::Exc("invalid body parameter\n");// (%d, num_bodies: %d)\n", m_current_body,
						//	m_num_bodies);
		return -1;
	}

	idScalar size = DRX3D_ID_MAX(level, 1);
	i32k body = m_current_body;
	//  length = 0.1 * size;
	//  with = 2 * 0.01 * size;

	/// these parameters are from the paper ...
	/// TODO: add proper citation
	m_parent[body] = parent;
	m_mass[body] = 0.1 * DRX3D_ID_POW(size, 3);
	m_body_r_body_com[body](0) = 0.05 * size;
	m_body_r_body_com[body](1) = 0;
	m_body_r_body_com[body](2) = 0;
	// initialization
	for (i32 i = 0; i < 3; i++)
	{
		m_parent_r_parent_body_ref[body](i) = 0;
		for (i32 j = 0; j < 3; j++)
		{
			m_body_I_body[body](i, j) = 0.0;
			m_body_T_parent_ref[body](i, j) = 0.0;
		}
	}
        const idScalar size_5 = std::pow(size, 5);
        m_body_I_body[body](0, 0) = size_5 / 0.2e6;
        m_body_I_body[body](1, 1) = size_5 * 403 / 1.2e6;
	m_body_I_body[body](2, 2) = m_body_I_body[body](1, 1);

	getVecMatFromDH(0, 0, a_DH_in, alpha_DH_in, &m_parent_r_parent_body_ref[body],
					&m_body_T_parent_ref[body]);

	// attach "level" Dill systems of levels 1...level
	for (i32 i = 1; i <= level; i++)
	{
		idScalar d_DH = 0.01 * size;
		if (i == level)
		{
			d_DH = 0.0;
		}
		const idScalar a_DH = i * 0.1;
		const idScalar alpha_DH = i * DRX3D_ID_PI / 3.0;
		m_current_body++;
		recurseDill(i - 1, body, d_DH, a_DH, alpha_DH);
	}

	return 0;  // ok!
}
}  // namespace drx3d_inverse
