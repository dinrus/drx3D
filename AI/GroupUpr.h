// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GroupUpr_h__
#define __GroupUpr_h__

#include <drx3D/AI/Group.h>

class CGroupUpr
{
public:
	CGroupUpr();
	~CGroupUpr();

	void                  Reset(EObjectResetType type);
	void                  Update(float updateTime);

	void                  AddGroupMember(const GroupID& groupID, tAIObjectID objectID);
	void                  RemoveGroupMember(const GroupID& groupID, tAIObjectID objectID);

	Group&                GetGroup(const GroupID& groupID);
	const Group&          GetGroup(const GroupID& groupID) const;

	u32                GetGroupMemberCount(const GroupID& groupID) const;

	Group::NotificationID NotifyGroup(const GroupID& groupID, tAIObjectID senderID, tukk name);

	void                  Serialize(TSerialize ser);

private:
	typedef std::unordered_map<GroupID, Group, stl::hash_uint32> Groups;
	Groups                m_groups;

	Group::NotificationID m_notifications;
};

#endif //__GroupUpr_h__
