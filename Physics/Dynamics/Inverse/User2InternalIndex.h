#ifndef USER2INTERNALINDEX_HPP
#define USER2INTERNALINDEX_HPP
#include <map>
#include <vector>

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>

namespace drx3d_inverse
{
/// Convert arbitrary indexing scheme to internal indexing
/// used for MultiBodyTree
class User2InternalIndex
{
public:
	/// Ctor
	User2InternalIndex();
	/// add body to index maps
	/// @param body index of body to add (external)
	/// @param parent index of parent body (external)
	void addBody(i32k body, i32k parent);
	/// build mapping from external to internal indexing
	/// @return 0 on success, -1 on failure
	i32 buildMapping();
	/// get internal index from external index
	/// @param user external (user) index
	/// @param internal pointer for storage of corresponding internal index
	/// @return 0 on success, -1 on failure
	i32 user2internal(i32k user, i32 *internal) const;
	/// get internal index from external index
	/// @param user external (user) index
	/// @param internal pointer for storage of corresponding internal index
	/// @return 0 on success, -1 on failure
	i32 internal2user(i32k internal, i32 *user) const;

private:
	i32 findRoot(i32 index);
	void recurseIndexSets(i32k user_body_index);
	bool m_map_built;
	std::map<i32, i32> m_user_parent_index_map;
	std::map<i32, i32> m_user_to_internal;
	std::map<i32, i32> m_internal_to_user;
	std::map<i32, std::vector<i32> > m_user_child_indices;
	i32 m_current_index;
};
}  // namespace drx3d_inverse

#endif  // USER2INTERNALINDEX_HPP
