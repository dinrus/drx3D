// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/GroupUpr.h>
#include <drx3D/AI/Group.h>

static Group s_emptyGroup;

CGroupUpr::CGroupUpr()
	: m_notifications(0)
{
}

CGroupUpr::~CGroupUpr()
{
	Group::ClearStaticData();
}

void CGroupUpr::Reset(EObjectResetType type)
{
	Group::ClearStaticData();

	Groups::iterator it = m_groups.begin();
	Groups::iterator end = m_groups.end();

	for (; it != end; ++it)
	{
		Group& group = it->second;

		group.Reset(type);
	}
}

void CGroupUpr::Update(float updateTime)
{
	Groups::iterator it = m_groups.begin();
	Groups::iterator end = m_groups.end();

	for (; it != end; ++it)
	{
		Group& group = it->second;

		group.Update(updateTime);
	}
}

void CGroupUpr::AddGroupMember(const GroupID& groupID, tAIObjectID objectID)
{
	std::pair<Groups::iterator, bool> iresult = m_groups.insert(Groups::value_type(groupID, Group()));
	Group& group = iresult.first->second;

	if (iresult.second)
	{
		Group(groupID).Swap(group);
	}

	group.AddMember(objectID);
}

void CGroupUpr::RemoveGroupMember(const GroupID& groupID, tAIObjectID objectID)
{
	Groups::iterator git = m_groups.find(groupID);

	if (git != m_groups.end())
	{
		Group& group = git->second;

		group.RemoveMember(objectID);
	}
}

Group& CGroupUpr::GetGroup(const GroupID& groupID)
{
	Groups::iterator git = m_groups.find(groupID);
	if (git != m_groups.end())
	{
		Group& group = git->second;

		return group;
	}

	return s_emptyGroup;
}

const Group& CGroupUpr::GetGroup(const GroupID& groupID) const
{
	Groups::const_iterator git = m_groups.find(groupID);
	if (git != m_groups.end())
	{
		const Group& group = git->second;

		return group;
	}

	return s_emptyGroup;
}

u32 CGroupUpr::GetGroupMemberCount(const GroupID& groupID) const
{
	Groups::const_iterator git = m_groups.find(groupID);

	if (git != m_groups.end())
	{
		const Group& group = git->second;

		return group.GetMemberCount();
	}

	return 0;
}

Group::NotificationID CGroupUpr::NotifyGroup(const GroupID& groupID, tAIObjectID senderID, tukk name)
{
	if (groupID > 0)
	{
		Group& group = GetGroup(groupID);

		group.Notify(++m_notifications, senderID, name);
	}

	return m_notifications;
}

void CGroupUpr::Serialize(TSerialize ser)
{
	ser.BeginGroup("CGroupUpr");
	{
		ser.Value("Groups", m_groups);
		ser.Value("m_notifications", m_notifications);
	}
	ser.EndGroup();
}
