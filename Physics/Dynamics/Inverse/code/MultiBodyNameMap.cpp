#include "../MultiBodyNameMap.h"
#include <drx/Core/Core.h>

namespace drx3d_inverse
{
MultiBodyNameMap::MultiBodyNameMap() {}

i32 MultiBodyNameMap::addBody(i32k index, const STxt& name)
{
	if (m_index_to_body_name.count(index) > 0)
	{
		throw drx::Exc(drx::Format("trying to add index %d again\n", index));
		return -1;
	}
	if (m_body_name_to_index.count(name) > 0)
	{
		throw drx::Exc(drx::Format("trying to add name %s again\n", name.c_str()));
		return -1;
	}

	m_index_to_body_name[index] = name;
	m_body_name_to_index[name] = index;

	return 0;
}

i32 MultiBodyNameMap::addJoint(i32k index, const STxt& name)
{
	if (m_index_to_joint_name.count(index) > 0)
	{
		throw drx::Exc(drx::Format("попытка ещё раз добавить индекс %d\n", index));
		return -1;
	}
	if (m_joint_name_to_index.count(name) > 0)
	{
		throw drx::Exc(drx::Format("попытка ещё раз добавить имя %s\n", name.c_str()));
		return -1;
	}

	m_index_to_joint_name[index] = name;
	m_joint_name_to_index[name] = index;

	return 0;
}

i32 MultiBodyNameMap::getBodyName(i32k index, STxt* name) const
{
	std::map<i32, STxt>::const_iterator it = m_index_to_body_name.find(index);
	if (it == m_index_to_body_name.end())
	{
		throw drx::Exc(drx::Format("индекс %d неизвестен\n", index));
		return -1;
	}
	*name = it->second;
	return 0;
}

i32 MultiBodyNameMap::getJointName(i32k index, STxt* name) const
{
	std::map<i32, STxt>::const_iterator it = m_index_to_joint_name.find(index);
	if (it == m_index_to_joint_name.end())
	{
		throw drx::Exc(drx::Format("индекс %d неизвестен\n", index));
		return -1;
	}
	*name = it->second;
	return 0;
}

i32 MultiBodyNameMap::getBodyIndex(const STxt& name, i32* index) const
{
	std::map<STxt, i32>::const_iterator it = m_body_name_to_index.find(name);
	if (it == m_body_name_to_index.end())
	{
		throw drx::Exc(drx::Format("имя %s неизвестно\n", name.c_str()));
		return -1;
	}
	*index = it->second;
	return 0;
}

i32 MultiBodyNameMap::getJointIndex(const STxt& name, i32* index) const
{
	std::map<STxt, i32>::const_iterator it = m_joint_name_to_index.find(name);
	if (it == m_joint_name_to_index.end())
	{
		throw drx::Exc(drx::Format("имя %s неизвестно\n", name.c_str()));
		return -1;
	}
	*index = it->second;
	return 0;
}
}  // namespace drx3d_inverse
