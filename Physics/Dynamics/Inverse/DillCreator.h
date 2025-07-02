#ifndef DILLCREATOR_H_
#define DILLCREATOR_H_

#include "IMultiBodyTreeCreator.h"

namespace drx3d_inverse
{
/// Creator class for building a "Dill" system as intruduced as benchmark example in
/// Featherstone (1999), "A Divide-and-Conquer Articulated-Body Algorithm for Parallel O(log(n))
/// Calculation of Rigid-Body Dynamics. Part 2: Trees, Loops, and Accuracy.",  The International
/// Journal of Robotics Research 18 (9): 876–892. doi : 10.1177 / 02783649922066628.
///
/// This is a self-similar branched tree, somewhat resembling a dill plant
class DillCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor
	/// @param levels the number of dill levels
	DillCreator(i32 levels);
	/// dtor
	~DillCreator();
	///\copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32* num_bodies) const;
	///\copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
				vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref, vec3* body_axis_of_motion,
				idScalar* mass, vec3* body_r_body_com, mat33* body_I_body, i32* user_int,
				uk * user_ptr) const;

private:
	/// recursively generate dill bodies.
	/// TODO better documentation
	i32 recurseDill(i32k levels, i32k parent, const idScalar d_DH_in,
					const idScalar a_DH_in, const idScalar alpha_DH_in);
	i32 m_level;
	i32 m_num_bodies;
	idArray<i32>::type m_parent;
	idArray<vec3>::type m_parent_r_parent_body_ref;
	idArray<mat33>::type m_body_T_parent_ref;
	idArray<vec3>::type m_body_axis_of_motion;
	idArray<idScalar>::type m_mass;
	idArray<vec3>::type m_body_r_body_com;
	idArray<mat33>::type m_body_I_body;
	i32 m_current_body;
};
}  // namespace drx3d_inverse
#endif
