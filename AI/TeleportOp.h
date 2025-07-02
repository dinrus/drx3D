// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef TeleportOp_h
	#define TeleportOp_h

	#include <drx3D/AI/EnterLeaveUpdateGoalOp.h>
	#include <drx3D/Sys/TimeValue.h>

// Teleports a pipe user to a specific location
class TeleportOp : public EnterLeaveUpdateGoalOp
{
public:
	TeleportOp();
	TeleportOp(const XmlNodeRef& node);
	TeleportOp(const TeleportOp& rhs);

	virtual void Update(CPipeUser& pipeUser) override;
	virtual void Serialize(TSerialize serializer) override;

	void         SetDestination(const Vec3& position, const Vec3& direction);
	void         SetWaitUntilAgentIsNotMovingBeforeTeleport();
private:

	void Teleport(CPipeUser& pipeUser);
	bool IsAgentMoving(CPipeUser& pipeUser);

	Vec3 m_destinationPosition;
	Vec3 m_destinationDirection;
	bool m_waitUntilAgentIsNotMoving;
};

#endif // TeleportOp_h
