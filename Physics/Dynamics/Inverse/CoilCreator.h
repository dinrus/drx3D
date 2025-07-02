#ifndef COILCREATOR_H_
#define COILCREATOR_H_

#include <drx3D/Physics/Dynamics/Inverse/IMultiBodyTreeCreator.h>

namespace drx3d_inverse
{
/// Creator class for building a "coil" system as intruduced as benchmark example in
/// Featherstone (1999), "A Divide-and-Conquer Articulated-Body Algorithm for Parallel O(log(n))
/// Calculation of Rigid-Body Dynamics. Part 2: Trees, Loops, and Accuracy.",  The International
/// Journal of Robotics Research 18 (9): 876–892. doi : 10.1177 / 02783649922066628.
///
/// This is a serial chain, with an initial configuration resembling a coil.
class CoilCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor.
	/// @param n the number of bodies in the system
	CoilCreator(i32 n);
	/// dtor
	~CoilCreator();
	// \copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32* num_bodies) const;
	// \copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
				vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref, vec3* body_axis_of_motion,
				idScalar* mass, vec3* body_r_body_com, mat33* body_I_body, i32* user_int,
				uk * user_ptr) const;

private:
	i32 m_num_bodies;
	std::vector<i32> m_parent;
	vec3 m_parent_r_parent_body_ref;
	mat33 m_body_T_parent_ref;
	vec3 m_body_axis_of_motion;
	idScalar m_mass;
	vec3 m_body_r_body_com;
	mat33 m_body_I_body;
};
}  // namespace drx3d_inverse
#endif
