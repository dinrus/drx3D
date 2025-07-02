// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementBlock_TurnTowardsPosition_h
	#define MovementBlock_TurnTowardsPosition_h

	#include <drx3D/AI/MovementPlan.h>
	#include <drx3D/AI/MovementStyle.h>

namespace Movement
{
namespace MovementBlocks
{
class TurnTowardsPosition : public Movement::Block
{
public:
	TurnTowardsPosition(const Vec3& position);
	virtual void                    End(IMovementActor& actor);
	virtual Movement::Block::Status Update(const MovementUpdateContext& context);
	virtual bool                    InterruptibleNow() const { return true; }
	virtual tukk             GetName() const          { return "TurnTowardsPosition"; }

private:
	Vec3  m_positionToTurnTowards;
	float m_timeSpentAligning;
	float m_correctBodyDirTime;
};
}
}

#endif // MovementBlock_TurnTowardsPosition_h
