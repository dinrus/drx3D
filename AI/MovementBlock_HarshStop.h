// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementBlock_HarshStop_h
	#define MovementBlock_HarshStop_h

	#include <drx3D/AI/MovementPlan.h>

namespace Movement
{
namespace MovementBlocks
{
// Clears the movement variables in the pipe user state and waits
// until the ai proxy says we are no longer moving.
class HarshStop : public Movement::Block
{
public:
	virtual void        Begin(IMovementActor& actor);
	virtual Status      Update(const MovementUpdateContext& context);
	virtual tukk GetName() const { return "HarshStop"; }
};
}
}

#endif // MovementBlock_HarshStop_h
