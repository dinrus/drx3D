#ifndef BTMULTIBODYTREECREATOR_H_
#define BTMULTIBODYTREECREATOR_H_

#include <vector>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTreeCreator.h>
#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/IMultiBodyTreeCreator.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>

namespace drx3d_inverse
{
/// MultiBodyTreeCreator implementation for converting
/// a btMultiBody forward dynamics model into a MultiBodyTree inverse dynamics model
class MultiBodyTreeCreator : public IMultiBodyTreeCreator
{
public:
	/// ctor
	MultiBodyTreeCreator() noexcept;
	/// dtor
	~MultiBodyTreeCreator() {}
	/// extract model data from a btMultiBody
	/// @param mb pointer to btMultiBody to convert
	/// @param verbose if true, some information is printed
	/// @return -1 on error, 0 on success
	i32 createFromBtMultiBody(const MultiBody *mb, const bool verbose = false);
	/// \copydoc MultiBodyTreeCreator::getNumBodies
	i32 getNumBodies(i32 *num_bodies) const;
	///\copydoc MultiBodyTreeCreator::getBody
	i32 getBody(i32k body_index, i32 *parent_index, JointType *joint_type,
				vec3 *parent_r_parent_body_ref, mat33 *body_T_parent_ref,
				vec3 *body_axis_of_motion, idScalar *mass, vec3 *body_r_body_com,
				mat33 *body_I_body, i32 *user_int, uk *user_ptr) const;

private:
	// internal struct holding data extracted from btMultiBody
	struct LinkData
	{
		i32 parent_index;
		JointType joint_type;
		vec3 parent_r_parent_body_ref;
		mat33 body_T_parent_ref;
		vec3 body_axis_of_motion;
		idScalar mass;
		vec3 body_r_body_com;
		mat33 body_I_body;
	};
	idArray<LinkData>::type m_data;
	bool m_initialized;
};
}  // namespace drx3d_inverse

#endif  // BTMULTIBODYTREECREATOR_H_
