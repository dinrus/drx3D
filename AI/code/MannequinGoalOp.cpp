// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MannequinGoalOp.h>
#include <drx3D/AI/PipeUser.h>
#include <drx3D/AI/AIBubblesSystem.h>

//////////////////////////////////////////////////////////////////////////
CMannequinTagGoalOp::CMannequinTagGoalOp()
	: m_tagCrc(0)
{
}

CMannequinTagGoalOp::CMannequinTagGoalOp(tukk tagName)
	: m_tagCrc(0)
{
	assert(tagName);
	assert(tagName[0] != 0);

	m_tagCrc = CCrc32::ComputeLowercase(tagName);
}

CMannequinTagGoalOp::CMannequinTagGoalOp(u32k tagCrc)
	: m_tagCrc(tagCrc)
{
}

CMannequinTagGoalOp::CMannequinTagGoalOp(const XmlNodeRef& node)
	: m_tagCrc(0)
{
	assert(node != 0);

	tukk tagName = 0;
	if (node->getAttr("name", &tagName))
	{
		m_tagCrc = CCrc32::ComputeLowercase(tagName);
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_ERROR, "Animation tag GoalOp doesn't have a 'name' attribute.");
	}
}

//////////////////////////////////////////////////////////////////////////
CSetAnimationTagGoalOp::CSetAnimationTagGoalOp()
	: CMannequinTagGoalOp()
{
}

CSetAnimationTagGoalOp::CSetAnimationTagGoalOp(tukk tagName)
	: CMannequinTagGoalOp(tagName)
{
}

CSetAnimationTagGoalOp::CSetAnimationTagGoalOp(u32k tagCrc)
	: CMannequinTagGoalOp(tagCrc)
{
}

CSetAnimationTagGoalOp::CSetAnimationTagGoalOp(const XmlNodeRef& node)
	: CMannequinTagGoalOp(node)
{
}

void CSetAnimationTagGoalOp::Update(CPipeUser& pipeUser)
{
	SOBJECTSTATE& state = pipeUser.GetState();
	u32k tagCrc = GetTagCrc();

	const aiMannequin::SCommand* pCommand = state.mannequinRequest.CreateSetTagCommand(tagCrc);
	if (pCommand != NULL)
	{
		GoalOpSucceeded();
	}
	else
	{
		const EntityId entityId = pipeUser.GetEntityID();
		AIQueueBubbleMessage("CMannequinSetTagGoalOp::Update", entityId, "Could not add a set tag command to the mannequin request this frame.", eBNS_LogWarning);
	}
}

//////////////////////////////////////////////////////////////////////////
CClearAnimationTagGoalOp::CClearAnimationTagGoalOp()
	: CMannequinTagGoalOp()
{
}

CClearAnimationTagGoalOp::CClearAnimationTagGoalOp(tukk tagName)
	: CMannequinTagGoalOp(tagName)
{
}

CClearAnimationTagGoalOp::CClearAnimationTagGoalOp(u32k tagCrc)
	: CMannequinTagGoalOp(tagCrc)
{
}

CClearAnimationTagGoalOp::CClearAnimationTagGoalOp(const XmlNodeRef& node)
	: CMannequinTagGoalOp(node)
{
}

void CClearAnimationTagGoalOp::Update(CPipeUser& pipeUser)
{
	SOBJECTSTATE& state = pipeUser.GetState();
	u32k tagCrc = GetTagCrc();

	const aiMannequin::SCommand* pCommand = state.mannequinRequest.CreateClearTagCommand(tagCrc);
	if (pCommand != NULL)
	{
		GoalOpSucceeded();
	}
	else
	{
		const EntityId entityId = pipeUser.GetEntityID();
		AIQueueBubbleMessage("CMannequinClearTagGoalOp::Update", entityId, "Could not add a clear tag command to the mannequin request this frame.", eBNS_LogWarning);
	}
}
