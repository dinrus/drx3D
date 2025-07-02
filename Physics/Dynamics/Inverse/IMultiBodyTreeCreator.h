#ifndef MULTI_BODY_TREE_CREATOR_H_
#define MULTI_BODY_TREE_CREATOR_H_

#include <string>
#include <vector>
#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/IDMath.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTree.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyNameMap.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>

namespace drx3d_inverse
{
/// Interface class for initializing a MultiBodyTree instance.
/// Data to be provided is modeled on the URDF specification.
/// The user can derive from this class in order to programmatically
/// initialize a system.
class IMultiBodyTreeCreator
{
public:
	/// the dtor
	virtual ~IMultiBodyTreeCreator() {}
	/// Get the number of bodies in the system
	/// @param num_bodies write number of bodies here
	/// @return 0 on success, -1 on error
	virtual i32 getNumBodies(i32* num_bodies) const = 0;
	/// Interface for accessing link mass properties.
	/// For detailed description of data, @sa MultiBodyTree::addBody
	/// \copydoc MultiBodyTree::addBody
	virtual i32 getBody(i32k body_index, i32* parent_index, JointType* joint_type,
						vec3* parent_r_parent_body_ref, mat33* body_T_parent_ref,
						vec3* body_axis_of_motion, idScalar* mass, vec3* body_r_body_com,
						mat33* body_I_body, i32* user_int, uk * user_ptr) const = 0;
	/// @return a pointer to a name mapping utility class, or 0x0 if not available
	virtual const MultiBodyNameMap* getNameMap() const { return 0x0; }
};

/// Create a multibody object.
/// @param creator an object implementing the MultiBodyTreeCreator interface
///        that returns data defining the system
/// @return A pointer to an allocated multibodytree instance, or
///         0x0 if an error occured.
MultiBodyTree* CreateMultiBodyTree(const IMultiBodyTreeCreator& creator);
}  // namespace btInverseDynamics

// does urdf have gravity direction ??

#endif  // MULTI_BODY_TREE_CREATOR_H_
