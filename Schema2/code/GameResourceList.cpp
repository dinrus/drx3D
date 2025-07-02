// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/GameResourceList.h>

namespace sxema2 
{
	void CGameResourceList::AddResource(tukk szResource, EType type)
	{
		stl::push_back_unique(m_resources, SResource(ResourceName(szResource), type));
	}

	size_t CGameResourceList::GetResourceCount() const
	{
		return m_resources.size();
	}

	IGameResourceList::SItem CGameResourceList::GetResourceItemAt(size_t idx) const
	{
		if (idx < m_resources.size())
			return IGameResourceList::SItem(m_resources[idx].name.c_str(), m_resources[idx].type);
		else
			return IGameResourceList::SItem("", IGameResourceList::EType::Invalid);
	}

	void CGameResourceList::Sort()
	{
		std::sort(m_resources.begin(), m_resources.end());
	}

}
