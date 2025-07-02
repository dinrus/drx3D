#ifndef RANDOMTREE_CREATOR_H_
#define RANDOMTREE_CREATOR_H_

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/IMultiBodyTreeCreator.h>

namespace drx3d_inverse
{
/// Generate a random MultiBodyTree with fixed or floating base and fixed, prismatic or revolute
/// joints
/// Uses a pseudo random number generator seeded from a random device.
class RandomTreeCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor
	/// @param max_bodies maximum number of bodies
	/// @param gravity gravitational acceleration
	/// @param use_seed if true, seed random number generator
	RandomTreeCreator(i32k max_bodies, bool use_seed = false);
	~RandomTreeCreator();
	///\copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32* num_bodies) const;
	///\copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
				vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref, vec3* body_axis_of_motion,
				idScalar* mass, vec3* body_r_body_com, mat33* body_I_body, i32* user_int,
				uk * user_ptr) const;

private:
	i32 m_num_bodies;
};
}  // namespace drx3d_inverse
#endif  // RANDOMTREE_CREATOR_H_
