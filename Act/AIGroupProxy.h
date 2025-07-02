// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __AIGroupProxy_h__
#define __AIGroupProxy_h__

#pragma once

#include <drx3D/AI/IAIObject.h>
#include <drx3D/AI/IAIGroupProxy.h>

class CAIGroupProxy :
	public IAIGroupProxy
{
	friend class CAIProxyUpr;
public:
	virtual void          Reset(EObjectResetType type);
	virtual void          Serialize(TSerialize ser);

	virtual tukk   GetCurrentBehaviorName() const;
	virtual tukk   GetPreviousBehaviorName() const;

	virtual void          Notify(u32 notificationID, tAIObjectID senderID, tukk notification);
	virtual void          SetBehaviour(tukk behaviour, bool callCDtor = true);

	virtual void          MemberAdded(tAIObjectID memberID);
	virtual void          MemberRemoved(tAIObjectID memberID);

	virtual IScriptTable* GetScriptTable();

protected:
	CAIGroupProxy(i32 groupID);
	virtual ~CAIGroupProxy();

	bool CallScript(IScriptTable* table, tukk funcName);
	bool CallNotification(IScriptTable* table, tukk notification, u32 notificationID, IScriptTable* sender);
	void PopulateMembersTable();

	typedef std::vector<tAIObjectID> Members;
	Members          m_members;

	SmartScriptTable m_script;
	SmartScriptTable m_prevBehavior;
	SmartScriptTable m_behavior;
	SmartScriptTable m_behaviorsTable;
	SmartScriptTable m_membersTable;

	string           m_behaviorName;
};

#endif
