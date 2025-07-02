#ifndef CLONETREE_CREATOR_H_
#define CLONETREE_CREATOR_H_

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include "IMultiBodyTreeCreator.h"

namespace drx3d_inverse
{
/// Generate an identical multibody tree from a reference system.
class CloneTreeCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor
	/// @param reference the MultiBodyTree to clone
	CloneTreeCreator(const MultiBodyTree* reference);
	~CloneTreeCreator();
	///\copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32* num_bodies) const;
	///\copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
				vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref, vec3* body_axis_of_motion,
				idScalar* mass, vec3* body_r_body_com, mat33* body_I_body, i32* user_int,
				uk * user_ptr) const;

private:
	const MultiBodyTree* m_reference;
};
}  // namespace drx3d_inverse
#endif  // CLONETREE_CREATOR_H_
