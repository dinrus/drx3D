#include "../User2InternalIndex.h"
#include <drx/Core/Core.h>

namespace drx3d_inverse
{
User2InternalIndex::User2InternalIndex() : m_map_built(false) {}

void User2InternalIndex::addBody(i32k body, i32k parent)
{
	m_user_parent_index_map[body] = parent;
}

i32 User2InternalIndex::findRoot(i32 index)
{
	if (0 == m_user_parent_index_map.count(index))
	{
		return index;
	}
	return findRoot(m_user_parent_index_map[index]);
}

// modelled after URDF2Bullet.cpp:void ComputeParentIndices(const
// URDFImporterInterface& u2b, URDF2BulletCachedData& cache, i32 urdfLinkIndex,
// i32 urdfParentIndex)
void User2InternalIndex::recurseIndexSets(i32k user_body_index)
{
	m_user_to_internal[user_body_index] = m_current_index;
	m_current_index++;
	for (size_t i = 0; i < m_user_child_indices[user_body_index].size(); i++)
	{
		recurseIndexSets(m_user_child_indices[user_body_index][i]);
	}
}

i32 User2InternalIndex::buildMapping()
{
	// find root index
	i32 user_root_index = -1;
	for (std::map<i32, i32>::iterator it = m_user_parent_index_map.begin();
		 it != m_user_parent_index_map.end(); it++)
	{
		i32 current_root_index = findRoot(it->second);
		if (it == m_user_parent_index_map.begin())
		{
			user_root_index = current_root_index;
		}
		else
		{
			if (user_root_index != current_root_index)
			{
				throw drx::Exc(drx::Format("multiple roots (at least) %d and %d\n", user_root_index,
									current_root_index));
				return -1;
			}
		}
	}

	// build child index map
	for (std::map<i32, i32>::iterator it = m_user_parent_index_map.begin();
		 it != m_user_parent_index_map.end(); it++)
	{
		m_user_child_indices[it->second].push_back(it->first);
	}

	m_current_index = -1;
	// build internal index set
	m_user_to_internal[user_root_index] = -1;  // add map for root link
	recurseIndexSets(user_root_index);

	// reverse mapping
	for (std::map<i32, i32>::iterator it = m_user_to_internal.begin();
		 it != m_user_to_internal.end(); it++)
	{
		m_internal_to_user[it->second] = it->first;
	}

	m_map_built = true;
	return 0;
}

i32 User2InternalIndex::user2internal(i32k user, i32 *internal) const
{
	if (!m_map_built)
	{
		return -1;
	}

	std::map<i32, i32>::const_iterator it;
	it = m_user_to_internal.find(user);
	if (it != m_user_to_internal.end())
	{
		*internal = it->second;
		return 0;
	}
	else
	{
		throw drx::Exc(drx::Format("no user index %d\n", user));
		return -1;
	}
}

i32 User2InternalIndex::internal2user(i32k internal, i32 *user) const
{
	if (!m_map_built)
	{
		return -1;
	}

	std::map<i32, i32>::const_iterator it;
	it = m_internal_to_user.find(internal);
	if (it != m_internal_to_user.end())
	{
		*user = it->second;
		return 0;
	}
	else
	{
		throw drx::Exc(drx::Format("no internal index %d\n", internal));
		return -1;
	}
}
}  // namespace drx3d_inverse
