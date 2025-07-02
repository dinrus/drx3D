// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MovementBlock_HarshStop.h>
#include <drx3D/AI/MovementUpdateContext.h>
#include <drx3D/AI/MovementActor.h>
#include <drx3D/AI/PipeUser.h>

namespace Movement
{
namespace MovementBlocks
{
void HarshStop::Begin(IMovementActor& actor)
{
	actor.GetAdapter().ClearMovementState();
}

Movement::Block::Status HarshStop::Update(const MovementUpdateContext& context)
{
	const bool stopped = !context.actor.GetAdapter().IsMoving();

	return stopped ? Finished : Running;
}
}
}
