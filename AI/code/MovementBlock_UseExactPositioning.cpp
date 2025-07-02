// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MovementBlock_UseExactPositioning.h>
#include <drx3D/AI/MovementActor.h>
#include <drx3D/AI/MovementUpdateContext.h>
#include <drx3D/AI/MovementHelpers.h>
#include <drx3D/AI/NavigationSystem.h>
#include <drx3D/AI/PipeUser.h>
#include <drx3D/AI/AIBubblesSystem.h>

namespace Movement
{
namespace MovementBlocks
{
UseExactPositioning::UseExactPositioning(
  const CNavPath& path,
  const MovementStyle& style)
	: Base(path, style)
{

}

UseExactPositioningBase::TryRequestingExactPositioningResult UseExactPositioning::TryRequestingExactPositioning(const MovementUpdateContext& context)
{
	DRX_ASSERT(m_style.GetExactPositioningRequest());
	if (m_style.GetExactPositioningRequest() == 0)
	{
		return RequestFailed_FinishImmediately;
	}

	const bool useLowerPrecision = false;
	context.actor.GetAdapter().RequestExactPosition(m_style.GetExactPositioningRequest(), useLowerPrecision);

	return RequestSucceeded;
}

void UseExactPositioning::HandleExactPositioningError(const MovementUpdateContext& context)
{
	const EntityId actorEntityId = context.actor.GetEntityId();
	if (IEntity* entity = gEnv->pEntitySystem->GetEntity(actorEntityId))
	{
		// Consider teleporting to the startpoint here, with code like the following
		// (needs to be discussed further)
		//
		//if (m_style.GetExactPositioningRequest() != 0)
		//{
		//Vec3 newForwardDir = m_style.GetExactPositioningRequest()->animDirection.GetNormalizedSafe(FORWARD_DIRECTION);

		//QuatT teleportDestination(
		//	m_style.GetExactPositioningRequest()->animLocation,
		//	Quat::CreateRotationV0V1(FORWARD_DIRECTION, newForwardDir));

		//entity->SetWorldTM(Matrix34(teleportDestination));
		//}

		// Report
		stack_string message;
		message.Format("Exact positioning failed to get me to the start of the actor target or was canceled incorrectly.");
		AIQueueBubbleMessage("PrepareForExactPositioningError", actorEntityId, message.c_str(), eBNS_Balloon | eBNS_LogWarning);
	}
}
}
}
