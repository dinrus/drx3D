// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementBlock_Helpers_h
#define MovementBlock_Helpers_h

#include <drx3D/AI/MovementPlan.h>
#include <drx3D/AI/MovementStyle.h>

struct IMovementActor;
class MovementStyle;
class CNavPath;

namespace Movement
{
namespace Helpers
{
class StuckDetector
{
public:
	StuckDetector();
	void Update(const MovementUpdateContext& context);
	void Reset();
	bool IsAgentStuck() const;

private:
	float m_accumulatedTimeAgentIsStuck;
	float m_agentDistanceToTheEndInPreviousUpdate;
};

void BeginPathFollowing(IMovementActor& actor, const MovementStyle& style, const CNavPath& navPath);
bool UpdatePathFollowing(PathFollowResult& result, const MovementUpdateContext& context, const MovementStyle& style);
}
}

#endif // MovementBlock_Helpers_h
