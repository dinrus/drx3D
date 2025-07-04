// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MovementBlock_UseSmartObject.h>
#include <drx3D/AI/MovementActor.h>
#include <drx3D/AI/MovementUpdateContext.h>
#include <drx3D/AI/MovementHelpers.h>
#include <drx3D/AI/NavigationSystem.h>
#include <drx3D/AI/OffGridLinks.h>
#include <drx3D/AI/PipeUser.h>
#include <drx3D/AI/AIBubblesSystem.h>

namespace Movement
{
namespace MovementBlocks
{
UseSmartObject::UseSmartObject(
  const CNavPath& path,
  const PathPointDescriptor::OffMeshLinkData& mnmData,
  const MovementStyle& style)
	: Base(path, style)
	, m_smartObjectMNMData(mnmData)
	, m_timeSpentWaitingForSmartObjectToBecomeFree(0.0f)
{

}

void UseSmartObject::Begin(IMovementActor& actor)
{
	Base::Begin(actor);
	m_timeSpentWaitingForSmartObjectToBecomeFree = 0.0f;
}

Movement::Block::Status UseSmartObject::Update(const MovementUpdateContext& context)
{
	const Status baseStatus = Base::Update(context);

	if (m_state == Traverse)
	{
		PathFollowResult result;
		Movement::Helpers::UpdatePathFollowing(result, context, m_upcomingStyle);
	}

	return baseStatus;
}

void UseSmartObject::SetUpcomingPath(const CNavPath& upcomingPath)
{
	m_upcomingPath = upcomingPath;
}

void UseSmartObject::SetUpcomingStyle(const MovementStyle& upcomingStyle)
{
	m_upcomingStyle = upcomingStyle;
}

void UseSmartObject::OnTraverseStarted(const MovementUpdateContext& context)
{
	Movement::Helpers::BeginPathFollowing(context.actor, m_upcomingStyle, m_upcomingPath);
}

UseExactPositioningBase::TryRequestingExactPositioningResult UseSmartObject::TryRequestingExactPositioning(const MovementUpdateContext& context)
{
	MNM::OffMeshLink* pOffMeshLink = gAIEnv.pNavigationSystem->GetOffMeshNavigationUpr()->GetOffMeshLink(m_smartObjectMNMData.offMeshLinkID);
	OffMeshLink_SmartObject* pSOLink = pOffMeshLink ? pOffMeshLink->CastTo<OffMeshLink_SmartObject>() : NULL;

	if (pSOLink)
	{
		CSmartObjectUpr* pSmartObjectUpr = gAIEnv.pSmartObjectUpr;
		CSmartObject* pSmartObject = pSOLink->m_pSmartObject;

		if (pSmartObjectUpr->IsSmartObjectBusy(pSmartObject))
		{
			// Smart object is busy so stop movement and wait for your turn
			context.actor.GetAdapter().ClearMovementState();

			m_timeSpentWaitingForSmartObjectToBecomeFree += gEnv->pTimer->GetFrameTime();
			if (m_timeSpentWaitingForSmartObjectToBecomeFree > 10.0f)
			{
				const EntityId actorEntityId = context.actor.GetEntityId();
				if (IEntity* entity = gEnv->pEntitySystem->GetEntity(actorEntityId))
				{
					const Vec3 teleportDestination = GetSmartObjectEndPosition();

					// Report
					stack_string message;
					message.Format("Agent spent too long waiting for a smart object to become free. UseSmartObject spent too long waiting. Teleported to end of smart object (%f, %f, %f)", teleportDestination.x, teleportDestination.y, teleportDestination.z);
					AIQueueBubbleMessage("WaitForSmartObjectToBecomeFree", actorEntityId, message.c_str(), eBNS_Balloon | eBNS_LogWarning);

					// Teleport
					Matrix34 transform = entity->GetWorldTM();
					transform.SetTranslation(teleportDestination);
					entity->SetWorldTM(transform);

					return RequestFailed_FinishImmediately;
				}
			}

			return RequestDelayed_SkipPathFollowing;
		}
		else if (context.actor.GetAdapter().IsClosestToUseTheSmartObject(*pSOLink))
		{
			// Fill in the actor target request, and figure out the navSO method.
			if (context.actor.GetAdapter().PrepareNavigateSmartObject(pSmartObject, pSOLink))
			{
				return RequestSucceeded;
			}
			else
			{
				// Attention!
				// We could not navigate via the selected smart object.
				//
				// One reason why this happens is because the entrance
				// or exit area is blocked by some dynamic obstacle.
				//
				// Todo:
				// 1. This should be handled before the path is constructed!
				// 2. The smart object should have a physics phantom that
				//    report back to listeners when blocked/unblocked
				//    so that the smart object and paths currently including
				//    it in a planned path can be correctly invalidated.

				context.actor.GetAdapter().ResetActorTargetRequest();
				context.actor.GetAdapter().InvalidateSmartObjectLink(pSmartObject, pSOLink);

				assert(false);

				// TODO: Handle this failure.

				return RequestFailed_FinishImmediately;
			}
		}
		else
		{
			// We are not the closest candidate user
			// Currently simply keeps moving towards the target
			return RequestDelayed_ContinuePathFollowing;
		}
	}
	else
	{
		// TODO: listen to links unregistration and try to handle invalid movement block sooner than here

		// No SOLink - link was unregistered (blocked, deleted...) and cannot be traversed anymore.
		return RequestFailed_CancelImmediately;
	}
}

void UseSmartObject::HandleExactPositioningError(const MovementUpdateContext& context)
{
	const EntityId actorEntityId = context.actor.GetEntityId();
	if (IEntity* entity = gEnv->pEntitySystem->GetEntity(actorEntityId))
	{
		{
			const Vec3 teleportDestination = GetSmartObjectEndPosition();

			// Report
			stack_string message;
			message.Format("Exact positioning failed to get me to the start of the smart object. Teleported to end of smart object (%f, %f, %f)", teleportDestination.x, teleportDestination.y, teleportDestination.z);
			AIQueueBubbleMessage("PrepareForSmartObjectError", actorEntityId, message.c_str(), eBNS_Balloon | eBNS_LogWarning);

			// Teleport
			Matrix34 transform = entity->GetWorldTM();
			transform.SetTranslation(teleportDestination);
			entity->SetWorldTM(transform);

			// Cancel exact positioning
			context.actor.GetAdapter().ResetActorTargetRequest();
		}
	}
}

Vec3 UseSmartObject::GetSmartObjectEndPosition() const
{
	MNM::OffMeshLink* pOffMeshLink = gAIEnv.pNavigationSystem->GetOffMeshNavigationUpr()->GetOffMeshLink(m_smartObjectMNMData.offMeshLinkID);
	OffMeshLink_SmartObject* pSOLink = pOffMeshLink ? pOffMeshLink->CastTo<OffMeshLink_SmartObject>() : NULL;

	assert(pSOLink);
	if (pSOLink)
	{
		CSmartObject* pSmartObject = pSOLink->m_pSmartObject;
		const Vec3 endPosition = pSmartObject->GetHelperPos(pSOLink->m_pToHelper);
		return endPosition;
	}

	return Vec3Constants<float>::fVec3_Zero;
}
}
}
