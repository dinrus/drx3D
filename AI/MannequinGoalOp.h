// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MANNEQUIN_GOAL_OP__H__
#define __MANNEQUIN_GOAL_OP__H__

#include <drx3D/AI/EnterLeaveUpdateGoalOp.h>

//////////////////////////////////////////////////////////////////////////
class CMannequinTagGoalOp
	: public EnterLeaveUpdateGoalOp
{
protected:
	CMannequinTagGoalOp();
	CMannequinTagGoalOp(tukk tagName);
	CMannequinTagGoalOp(u32k tagCrc);
	CMannequinTagGoalOp(const XmlNodeRef& node);

	u32 GetTagCrc() const { return m_tagCrc; }

private:
	u32 m_tagCrc;
};

//////////////////////////////////////////////////////////////////////////
class CSetAnimationTagGoalOp
	: public CMannequinTagGoalOp
{
public:
	CSetAnimationTagGoalOp();
	CSetAnimationTagGoalOp(tukk tagName);
	CSetAnimationTagGoalOp(u32k tagCrc);
	CSetAnimationTagGoalOp(const XmlNodeRef& node);

	virtual void Update(CPipeUser& pipeUser);
};

//////////////////////////////////////////////////////////////////////////
class CClearAnimationTagGoalOp
	: public CMannequinTagGoalOp
{
public:
	CClearAnimationTagGoalOp();
	CClearAnimationTagGoalOp(tukk tagName);
	CClearAnimationTagGoalOp(u32k tagCrc);
	CClearAnimationTagGoalOp(const XmlNodeRef& node);

	virtual void Update(CPipeUser& pipeUser);
};

#endif
