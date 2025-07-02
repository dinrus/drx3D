// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IAIGroupProxy_h__
#define __IAIGroupProxy_h__

#pragma once

#include <drx3D/AI/IAIObject.h>

struct IAIGroupProxyFactory
{
	// <interfuscator:shuffle>
	virtual IAIGroupProxy* CreateGroupProxy(i32 groupID) = 0;
	virtual ~IAIGroupProxyFactory(){}
	// </interfuscator:shuffle>
};

struct IAIGroupProxy :
	public _reference_target_t
{
	// <interfuscator:shuffle>
	virtual void          Reset(EObjectResetType type) = 0;
	virtual void          Serialize(TSerialize ser) = 0;

	virtual tukk   GetCurrentBehaviorName() const = 0;
	virtual tukk   GetPreviousBehaviorName() const = 0;

	virtual void          Notify(u32 notificationID, tAIObjectID senderID, tukk notification) = 0;

	virtual void          SetBehaviour(tukk behaviour, bool callCDtors = true) = 0;

	virtual void          MemberAdded(tAIObjectID memberID) = 0;
	virtual void          MemberRemoved(tAIObjectID memberID) = 0;

	virtual IScriptTable* GetScriptTable() = 0;
	// </interfuscator:shuffle>
};

#endif
