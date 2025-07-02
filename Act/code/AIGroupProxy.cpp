// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/AIGroupProxy.h>

#include <drx3D/AI/IAIObjectUpr.h>

CAIGroupProxy::CAIGroupProxy(i32 groupID)
{
	SmartScriptTable group;

	if (gEnv->pScriptSystem->GetGlobalValue("AIGroup", group))
	{
		m_script.Create(gEnv->pScriptSystem, false);
		m_script->Delegate(group);

		m_script->SetValue("id", groupID);

		m_membersTable.Create(gEnv->pScriptSystem, false);
		m_script->SetValue("members", m_membersTable);
	}

	gEnv->pScriptSystem->GetGlobalValue("AIGroupBehavior", m_behaviorsTable);
}

CAIGroupProxy::~CAIGroupProxy()
{
}

void CAIGroupProxy::Reset(EObjectResetType type)
{
	if (type == AIOBJRESET_SHUTDOWN)
	{
		m_behavior = 0;
		m_prevBehavior = 0;
		m_behaviorsTable = 0;
	}
	else
	{
		gEnv->pScriptSystem->GetGlobalValue("AIGroupBehavior", m_behaviorsTable);

		m_behavior = 0;
		m_prevBehavior = 0;
	}
}

void CAIGroupProxy::Serialize(TSerialize ser)
{
	if (m_script && m_script->HaveValue("OnSaveAI") && m_script->HaveValue("OnLoadAI"))
	{
		SmartScriptTable saved(m_script->GetScriptSystem());
		if (ser.IsWriting())
			Script::CallMethod(m_script, "OnSaveAI", saved);
		ser.Value("ScriptData", saved.GetPtr());
		if (ser.IsReading())
			Script::CallMethod(m_script, "OnLoadAI", saved);
	}

	string behaviorName = GetCurrentBehaviorName();
	ser.Value("BehaviorName", behaviorName);

	if (ser.IsReading())
		SetBehaviour(behaviorName.c_str(), false);
}

tukk CAIGroupProxy::GetCurrentBehaviorName() const
{
	// @Marcio: Maybe you wanted the name to be in the script?
	// If you didn't, just remove this. /Jonas 2010-08-25

	//  tukk name = 0;
	//  if (m_behavior)
	//    m_behavior->GetValue("Name", name);
	//
	//  return name;

	return m_behaviorName.c_str();
}

tukk CAIGroupProxy::GetPreviousBehaviorName() const
{
	//  tukk name = 0;
	//  if (m_prevBehavior)
	//    m_prevBehavior->GetValue("Name", name);
	//
	//  return name;

	return 0;
}

void CAIGroupProxy::Notify(u32 notificationID, tAIObjectID senderID, tukk notification)
{
	IScriptTable* senderScript = 0;
	if (IAIObject* sender = senderID ? gEnv->pAISystem->GetAIObjectUpr()->GetAIObject(senderID) : 0)
		if (IEntity* senderEntity = sender->GetEntity())
			senderScript = senderEntity->GetScriptTable();

	CallNotification(m_behavior, notification, notificationID, senderScript);

	Members::iterator it = m_members.begin();
	Members::iterator end = m_members.end();

	for (u32 k = 1; it != end; ++it)
	{
		if (IAIObject* aiObject = gEnv->pAISystem->GetAIObjectUpr()->GetAIObject(*it))
			gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 0, notification, aiObject);
	}
}

void CAIGroupProxy::SetBehaviour(tukk behaviour, bool callCDtors)
{
	if (m_behavior && callCDtors)
		CallScript(m_behavior, "Destructor");

	m_prevBehavior = m_behavior;
	m_behavior = 0;
	m_behaviorName.clear();

	if (m_behaviorsTable)
	{
		if (!m_behaviorsTable->GetValue(behaviour, m_behavior))
			m_behaviorsTable->GetValue("DEFAULT", m_behavior);
	}

	if (m_behavior)
	{
		m_behaviorName = behaviour;
		m_script->SetValue("Behavior", m_behavior);

		if (callCDtors)
			CallScript(m_behavior, "Constructor");
	}
	else
		m_script->SetToNull("Behavior");
}

void CAIGroupProxy::MemberAdded(tAIObjectID memberID)
{
	stl::push_back_unique(m_members, memberID);

	PopulateMembersTable();
}

void CAIGroupProxy::MemberRemoved(tAIObjectID memberID)
{
	stl::find_and_erase(m_members, memberID);

	PopulateMembersTable();
}

bool CAIGroupProxy::CallScript(IScriptTable* table, tukk funcName)
{
	if (table)
	{
		HSCRIPTFUNCTION functionToCall = 0;
		if (table->GetValue(funcName, functionToCall) && functionToCall)
		{
			gEnv->pScriptSystem->BeginCall(functionToCall);
			gEnv->pScriptSystem->PushFuncParam(table);
			gEnv->pScriptSystem->PushFuncParam(m_script);
			gEnv->pScriptSystem->EndCall();

			gEnv->pScriptSystem->ReleaseFunc(functionToCall);

			return true;
		}
	}

	return false;
}

bool CAIGroupProxy::CallNotification(IScriptTable* table, tukk notification, u32 notificationID,
                                     IScriptTable* sender)
{
	if (table)
	{
		HSCRIPTFUNCTION functionToCall = 0;
		if (table->GetValue(notification, functionToCall) && functionToCall)
		{
			gEnv->pScriptSystem->BeginCall(functionToCall);
			gEnv->pScriptSystem->PushFuncParam(table);
			gEnv->pScriptSystem->PushFuncParam(m_script);
			gEnv->pScriptSystem->PushFuncParam(ScriptHandle(notificationID));
			gEnv->pScriptSystem->PushFuncParam(sender);
			gEnv->pScriptSystem->EndCall();

			gEnv->pScriptSystem->ReleaseFunc(functionToCall);

			return true;
		}
	}

	return false;
}

void CAIGroupProxy::PopulateMembersTable()
{
	if (m_membersTable.GetPtr())
	{
		m_membersTable->Clear();

		Members::iterator it = m_members.begin();
		Members::iterator end = m_members.end();

		for (u32 k = 1; it != end; ++it)
		{
			if (IAIObject* aiObject = gEnv->pAISystem->GetAIObjectUpr()->GetAIObject(*it))
				if (IEntity* entity = aiObject->GetEntity())
					if (IScriptTable* scriptTable = entity->GetScriptTable())
						m_membersTable->SetAt(k++, scriptTable);
		}
	}
}

IScriptTable* CAIGroupProxy::GetScriptTable()
{
	return m_script;
};
