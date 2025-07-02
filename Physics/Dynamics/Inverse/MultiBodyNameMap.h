#ifndef MULTIBODYNAMEMAP_H_
#define MULTIBODYNAMEMAP_H_

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <string>
#include <map>

namespace drx3d_inverse
{
/// \brief The MultiBodyNameMap class
/// Utility class that stores a maps from body/joint indices to/from body and joint names
class MultiBodyNameMap
{
public:
	MultiBodyNameMap();
	/// add a body to the map
	/// @param index of the body
	/// @param name name of the body
	/// @return 0 on success, -1 on failure
	i32 addBody(i32k index, const STxt& name);
	/// add a joint to the map
	/// @param index of the joint
	/// @param name name of the joint
	/// @return 0 on success, -1 on failure
	i32 addJoint(i32k index, const STxt& name);
	/// get body name from index
	/// @param index of the body
	/// @param body_name name of the body
	/// @return 0 on success, -1 on failure
	i32 getBodyName(i32k index, STxt* name) const;
	/// get joint name from index
	/// @param index of the joint
	/// @param joint_name name of the joint
	/// @return 0 on success, -1 on failure
	i32 getJointName(i32k index, STxt* name) const;
	/// get body index from name
	/// @param index of the body
	/// @param name name of the body
	/// @return 0 on success, -1 on failure
	i32 getBodyIndex(const STxt& name, i32* index) const;
	/// get joint index from name
	/// @param index of the joint
	/// @param name name of the joint
	/// @return 0 on success, -1 on failure
	i32 getJointIndex(const STxt& name, i32* index) const;

private:
	std::map<i32, STxt> m_index_to_joint_name;
	std::map<i32, STxt> m_index_to_body_name;

	std::map<STxt, i32> m_joint_name_to_index;
	std::map<STxt, i32> m_body_name_to_index;
};
}  // namespace drx3d_inverse
#endif  // MULTIBODYNAMEMAP_H_
