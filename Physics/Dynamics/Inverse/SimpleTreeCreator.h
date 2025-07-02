#ifndef SIMPLETREECREATOR_H_
#define SIMPLETREECREATOR_H_

#include <drx3D/Physics/Dynamics/Inverse/IMultiBodyTreeCreator.h>

namespace drx3d_inverse
{
/// minimal "tree" (chain)
class SimpleTreeCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor
	/// @param dim number of bodies
	SimpleTreeCreator(i32 dim);
	// dtor
	~SimpleTreeCreator() {}
	///\copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32* num_bodies) const;
	///\copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
				vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref, vec3* body_axis_of_motion,
				idScalar* mass, vec3* body_r_body_com, mat33* body_I_body, i32* user_int,
				uk * user_ptr) const;

private:
	i32 m_num_bodies;
	idScalar m_mass;
	mat33 m_body_T_parent_ref;
	vec3 m_parent_r_parent_body_ref;
	vec3 m_body_r_body_com;
	mat33 m_body_I_body;
	vec3 m_axis;
};
}  // namespace drx3d_inverse
#endif  // SIMPLETREECREATOR_H_
