// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/AuditionMapRayCastUpr.h>
#include <drx3D/AI/AuditionMap.h>

#include <drx3D/AI/IAISystem.h>

namespace Perception
{
namespace AuditionHelpers
{

// Although we let a lot of the containers grow to a worst-case size, we
// once in a while sort the free pending stimulus indices so that we
// keep allocating consecutive stretches of memory as much as possible.
static i32k s_amountOfStimuliReleasesBeforeSorting = 32;

// ===========================================================================
// -- CRayCastDebuggerNull -- CRayCastDebuggerNull -- CRayCastDebuggerNull --
// ===========================================================================

// Just a dummy implementation that does nothing so that it can be optimized away.
class CRayCastDebuggerNull
{
public:

	bool IsRecording() const                                                                                                                                                                            { return false; }
	void RecordFurthestPossibleEarDistance(const float furthestPossibleEarDistance)                                                                                                                     {};
	void RecordSampleHit(const Vec3& hitPos, tukk szMaterialName, const float materialSoundObstruction, const float accumulatedObstructionBeforeHit, const float accumulatedObstructionAfterHit) {};
};

// ===========================================================================
// -- SPendingStimulusRayInfo -- SPendingStimulusRayInfo --
// ===========================================================================

SPendingStimulusRayInfo::SPendingStimulusRayInfo()
	: queuedRayId((QueuedRayID)0)
	, targetEarIndex(s_invalidTargetEarIndex)
{
}

SPendingStimulusRayInfo::SPendingStimulusRayInfo(
  const QueuedRayID queuedRayId, const size_t targetEarIndex)
	: queuedRayId(queuedRayId)
	, targetEarIndex(targetEarIndex)
{
}

void SPendingStimulusRayInfo::operator=(const SPendingStimulusRayInfo& source)
{
	queuedRayId = source.queuedRayId;
	targetEarIndex = source.targetEarIndex;
}

// ===========================================================================
// -- SPendingStimulus -- SPendingStimulus -- SPendingStimulus --
// ===========================================================================
SPendingListenerForStimulus::SPendingListenerForStimulus()
	: listenerEntityId(INVALID_ENTITYID)
{
}

SPendingListenerForStimulus::SPendingListenerForStimulus(const SPendingListenerForStimulus& other)
	: listenerEntityId(other.listenerEntityId)
	, queuedRayInfos(other.queuedRayInfos)
{
}

void SPendingListenerForStimulus::Clear()
{
	listenerEntityId = INVALID_ENTITYID;
	queuedRayInfos.clear();
}

// ===========================================================================
// -- CRayCastDebugger -- CRayCastDebugger -- CRayCastDebugger --
// ===========================================================================
SRayCastRequestInfo::SRayCastRequestInfo()
	: rayStartPos(ZERO)
	, rayEndPos(ZERO)
	, pendingStimulusIndex(s_invalidPendingStimulusIndex)
	, soundLinearFallOffFactor(0.0f)
{
}

// ===========================================================================
// -- SPendingRay -- SPendingRay -- SPendingRay -- SPendingRay --
// ===========================================================================
SPendingRay::SPendingRay(const SRayCastRequestInfo& _requestInfo)
	: requestInfo(_requestInfo)
{
}

// ===========================================================================
// -- CAuditionMapRayCastUpr -- CAuditionMapRayCastUpr --
// ===========================================================================
CAuditionMapRayCastUpr::CAuditionMapRayCastUpr(CAuditionMap& owner)
	: m_owner(owner)
{
}

CAuditionMapRayCastUpr::~CAuditionMapRayCastUpr()
{
	CancelAllPendingStimuli();
}

void CAuditionMapRayCastUpr::Update()
{
}

float CAuditionMapRayCastUpr::ComputeSoundLinearFallOffFactor(const float soundRadius)
{
	// The 'strength' of the sound stimulus starts at 1.0f and then dissipates to 0.0f.
	// As an approximation, we simply calculate a fall-off factor per meter such that
	// we hit exactly 0.0f at the furthest distance from the stimulus source.

	if (soundRadius >= FLT_EPSILON)
	{
		return 1.0f / soundRadius;
	}

	return 1.0f;
}

PendingStimulusParamsIndex CAuditionMapRayCastUpr::PreparePendingStimulusParams(const SSoundStimulusParams& params)
{
	PendingStimulusParamsIndex index = m_pendingStimuliParams.Allocate();

	SPendingStimulusParams& pendingStimulusParams = m_pendingStimuliParams[index];
	pendingStimulusParams.soundParams = params;
	
	/*const bool bHasValidSoundTags = (params.pSoundTags != nullptr);
	if (bHasValidSoundTags)
	{
		pendingStimulusParams.soundTags = *params.pSoundTags;
		pendingStimulusParams.soundParams.pSoundTags = &(pDestCopy->soundTags);
	}*/
	return index;
}

void CAuditionMapRayCastUpr::ReleasePendingStimulusParamsIfNotUsed(PendingStimulusParamsIndex stimulusParamsIndex)
{
	if (m_pendingStimuliParams[stimulusParamsIndex].GetRefCount() <= 0)
	{
		m_pendingStimuliParams.Release(stimulusParamsIndex);
	}
}

void CAuditionMapRayCastUpr::QueueRaysBetweenStimulusAndListener(
  const Vec3& rayStartPos,
  const PendingStimulusParamsIndex& stimulusParamsIndex,
  const float soundLinearFallOffFactor,
  const EntityId listenerEntityId,
  const Perception::SListenerParams& listenerParams)
{
	if (listenerParams.ears.empty())
	{
		return;
	}

	PendingListenerForStimulusIndex pendingListenerIndex = m_pendingListenersForStimulus.Allocate();

	SPendingListenerForStimulus& pendingStimulus = m_pendingListenersForStimulus[pendingListenerIndex];
	pendingStimulus.listenerEntityId = listenerEntityId;
	pendingStimulus.stimulusParamsIndex = stimulusParamsIndex;

	SRayCastRequestInfo rayCastRequestInfo;
	rayCastRequestInfo.rayStartPos = rayStartPos;
	rayCastRequestInfo.pendingStimulusIndex = pendingListenerIndex;
	rayCastRequestInfo.soundLinearFallOffFactor = soundLinearFallOffFactor;

	i32k earsCount = listenerParams.ears.size();
	for (i32 earIndex = 0; earIndex < earsCount; ++earIndex)
	{
		m_pendingStimuliParams[stimulusParamsIndex].AddRef();
		
		rayCastRequestInfo.rayEndPos = listenerParams.ears[earIndex].worldPos;

		const QueuedRayID queuedRayId = QueueRay(rayCastRequestInfo);
		pendingStimulus.queuedRayInfos.push_back(SPendingStimulusRayInfo(queuedRayId, earIndex));
	}
}

void CAuditionMapRayCastUpr::CancelAllPendingStimuliDirectedAtListener(const EntityId listenerEntityId)
{
	m_foundPendingStimulusIndices.clear();
	const size_t count = m_pendingListenersForStimulus.Size();
	for (size_t i = 0; i < count; ++i)
	{
		const PendingListenerForStimulusIndex index = m_pendingListenersForStimulus.GetIndex(i);

		if (m_pendingListenersForStimulus[index].listenerEntityId == listenerEntityId)
		{
			m_foundPendingStimulusIndices.push_back(index);
		}
	}

	for (const PendingListenerForStimulusIndex index : m_foundPendingStimulusIndices)
	{
		CancelRemainingRaysForPendingStimulus(index);
		m_pendingListenersForStimulus.Release(index);
	}
}

QueuedRayID CAuditionMapRayCastUpr::QueueRay(const SRayCastRequestInfo& requestInfo)
{
	static i32 const physicsCheckObjectTypesMask = COVER_OBJECT_TYPES | ent_water;
	static i32 const physicsRayCastConfigMask = (geom_colltype_ray << rwi_colltype_bit) | rwi_colltype_any | rwi_pierceability0;

	const RayCastRequest::Priority rayPriority = GetRayCastRequestPriority();

	PhysSkipList skipList;

	const QueuedRayID queuedRayId = GetAISystem()->GetGlobalRaycaster()->Queue(
	  rayPriority,
	  RayCastRequest(
	    requestInfo.rayStartPos,
	    requestInfo.rayEndPos - requestInfo.rayStartPos,
	    physicsCheckObjectTypesMask,
	    physicsRayCastConfigMask,
	    &skipList.at(0), skipList.size(),
	    (i32)RayCastResult::MaxHitCount),
	  functor(*this, &CAuditionMapRayCastUpr::OnRayCastComplete));
	assert(queuedRayId != 0);

	std::pair<PendingRays::iterator, bool> result = m_pendingRays.insert(
	  PendingRays::value_type(queuedRayId, requestInfo));
	assert(result.second);

	return queuedRayId;
}

void CAuditionMapRayCastUpr::CancelRay(const QueuedRayID queuedRayId)
{
	PendingRays::iterator foundIter = m_pendingRays.find(queuedRayId);
	if (foundIter != m_pendingRays.end())
	{
		m_pendingRays.erase(foundIter);

		GetAISystem()->GetGlobalRaycaster()->Cancel(queuedRayId);
	}
}

void CAuditionMapRayCastUpr::CancelAllRays()
{
	std::vector<QueuedRayID> queuedRayIds;
	RetrieveAllQueuedRayIds(&queuedRayIds);

	for (const QueuedRayID& queuedRayId : queuedRayIds)
	{
		CancelRay(queuedRayId);
	}

	assert(m_pendingRays.empty());
}

void CAuditionMapRayCastUpr::RetrieveAllQueuedRayIds(std::vector<QueuedRayID>* pQueuedRayIds) const
{
	pQueuedRayIds->reserve(m_pendingRays.size());

	for (const PendingRays::value_type& pendingRayValue : m_pendingRays)
	{
		const QueuedRayID queuedRayId = pendingRayValue.first;
		pQueuedRayIds->emplace_back(queuedRayId);
	}
}

RayCastRequest::Priority CAuditionMapRayCastUpr::GetRayCastRequestPriority() const
{
	return RayCastRequest::MediumPriority;
}

void CAuditionMapRayCastUpr::OnRayCastComplete(const QueuedRayID& queuedRayId, const RayCastResult& rayCastResult)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	PendingRays::const_iterator pendingRayIter = m_pendingRays.find(queuedRayId);
	if (pendingRayIter == m_pendingRays.end())
	{
		DRX_ASSERT(0); // We somehow forgot to cancel a pending ray?
		return;
	}

	const SPendingRay& pendingRay = pendingRayIter->second;
	CRayCastDebuggerNull nullDebugger;
	const bool bStimulusReachedEar = HasRayCastReachedEar(pendingRay.requestInfo, rayCastResult, nullDebugger);

	const PendingListenerForStimulusIndex pendingStimulusIndex = pendingRayIter->second.requestInfo.pendingStimulusIndex;

	m_pendingRays.erase(pendingRayIter);
#if !defined(_RELEASE)
	pendingRayIter = m_pendingRays.end(); // Safety code.
#endif

	if (pendingStimulusIndex != s_invalidPendingStimulusIndex)
	{
		ProcessPendingStimulusRayCastResult(pendingStimulusIndex, queuedRayId, bStimulusReachedEar);
	}
	// Else: the ray-cast tester module also shoots rays but these are not actually part of a stimulus,
	//       so we can safely ignore these.
}

// Making this a template function so we can have a clean version, and one that has debug code in it.
template<typename DEBUGGER>
bool CAuditionMapRayCastUpr::HasRayCastReachedEar(
  const SRayCastRequestInfo& rayCastRequestInfo,
  const RayCastResult& rayCastResult,
  DEBUGGER& debugger) const
{
	// When the accumulated obstruction reaches 1.0f, we stop.
	float accumulatedObstruction = 0.0f;

	if (debugger.IsRecording())
	{
		// By default: record the maximum possible distance to the ear if we would not hit anything.
		const float distanceToLastHit = 0.0f;
		const float furthestPossibleEarDistance = ComputeFurthestPossibleEarDistanceBasedOnRemainingObstruction(
		  accumulatedObstruction, distanceToLastHit, rayCastRequestInfo);
		debugger.RecordFurthestPossibleEarDistance(furthestPossibleEarDistance);
	}

	i32k hitsCount = rayCastResult.hitCount;
	if (hitsCount <= 0)
	{
		return true;
	}

	ISurfaceTypeUpr* pSurfaceTypeUpr = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceTypeUpr();

	float distanceStartToLastHit = 0.0f;

	for (i32 hitIndex = 0; hitIndex < hitsCount; ++hitIndex)
	{
		const ray_hit& rayHitInfo = rayCastResult[hitIndex];
		const float distanceStartToHit = rayHitInfo.dist;
		if (distanceStartToHit > 0.0f)
		{
			const float oldAccumulatedObstruction = accumulatedObstruction;
			const float distanceBetweenHits = distanceStartToHit - distanceStartToLastHit;

			accumulatedObstruction += distanceBetweenHits * rayCastRequestInfo.soundLinearFallOffFactor;
			if (accumulatedObstruction >= 1.0f)
			{
				// Early out: Sound obstruction reached maximum before we even reached the hit position.
				if (debugger.IsRecording())
				{
					const float furthestPossibleEarDistance = ComputeFurthestPossibleEarDistanceBasedOnRemainingObstruction(
					  oldAccumulatedObstruction, distanceStartToLastHit, rayCastRequestInfo);
					debugger.RecordFurthestPossibleEarDistance(furthestPossibleEarDistance);
				}
				return false;
			}

			float surfaceSoundObstruction = 0.0f;
			if (DetermineSoundObstruction(pSurfaceTypeUpr, rayCastRequestInfo, rayHitInfo, surfaceSoundObstruction))
			{
				const float accumulatedObstructionJustBeforeHit = accumulatedObstruction;

				accumulatedObstruction += surfaceSoundObstruction;

				debugger.RecordSampleHit(
					rayHitInfo.pt,
					pSurfaceTypeUpr->GetSurfaceType(rayHitInfo.surface_idx)->GetName(), surfaceSoundObstruction,
					accumulatedObstructionJustBeforeHit, accumulatedObstruction);

				if (accumulatedObstruction >= 1.0f)
				{
					// Early out: Sound obstruction reached maximum because of hit with surface.
					if (debugger.IsRecording())
					{
						debugger.RecordFurthestPossibleEarDistance(distanceStartToHit);
					}
					return false;
				}
			}
			distanceStartToLastHit = distanceStartToHit;
		}
	}

	const float rayLength = (rayCastRequestInfo.rayEndPos - rayCastRequestInfo.rayStartPos).GetLength();

	const float remainingFallOffDistance = rayLength - distanceStartToLastHit;

	const float oldAccumulatedObstruction = accumulatedObstruction;

	accumulatedObstruction += (remainingFallOffDistance * rayCastRequestInfo.soundLinearFallOffFactor);

	if (accumulatedObstruction >= 1.0f)
	{
		// Sound obstruction reached maximum before we were able to reach the ear.
		if (debugger.IsRecording())
		{
			const float furthestPossibleEarDistance = ComputeFurthestPossibleEarDistanceBasedOnRemainingObstruction(
			  oldAccumulatedObstruction, distanceStartToLastHit, rayCastRequestInfo);
			debugger.RecordFurthestPossibleEarDistance(furthestPossibleEarDistance);
		}

		return false;
	}

	return true;
}

bool CAuditionMapRayCastUpr::DetermineSoundObstruction(
	ISurfaceTypeUpr* pSurfaceTypeUpr, 
	const SRayCastRequestInfo& rayCastRequestInfo, 
	const ray_hit& rayHitInfo, 
	float& resultSoundObstruction) const
{
	if (m_owner.GetSoundObstructionOnHitCallback())
	{
		const EntityId listenerEntityId = rayCastRequestInfo.pendingStimulusIndex != s_invalidPendingStimulusIndex ?
			m_pendingListenersForStimulus[rayCastRequestInfo.pendingStimulusIndex].listenerEntityId : INVALID_ENTITYID;

		if (m_owner.GetSoundObstructionOnHitCallback()(listenerEntityId, rayHitInfo, resultSoundObstruction))
		{
			return true;
		}
	}
	return DeterminePhysicsSurfaceSoundObstruction(pSurfaceTypeUpr, rayHitInfo.surface_idx, resultSoundObstruction);
}

bool CAuditionMapRayCastUpr::DeterminePhysicsSurfaceSoundObstruction(ISurfaceTypeUpr* pSurfaceTypeUpr, const short surfaceIndex, float& resultSoundObstruction)
{
	ISurfaceType* pSurfaceType = pSurfaceTypeUpr->GetSurfaceType(surfaceIndex);
	if(!pSurfaceType)
	{
		return false;
	}

	const float physicsSoundObstruction = pSurfaceType->GetPhyscalParams().sound_obstruction;
	resultSoundObstruction = physicsSoundObstruction >= 0.0f ? physicsSoundObstruction : 1.0f;
	return true;
}

float CAuditionMapRayCastUpr::ComputeFurthestPossibleEarDistanceBasedOnRemainingObstruction(
  const float oldAccumulatedObstruction, const float distanceStartToLastObstructionComputation,
  const SRayCastRequestInfo& rayCastRequestInfo)
{
	const float furthestPossibleEarDistance = (rayCastRequestInfo.rayEndPos - rayCastRequestInfo.rayStartPos).GetLength();

	if (rayCastRequestInfo.soundLinearFallOffFactor <= FLT_EPSILON)
	{
		return furthestPossibleEarDistance;
	}

	const float remainingDistanceBeforeExhaustion = (1.0f - oldAccumulatedObstruction) / rayCastRequestInfo.soundLinearFallOffFactor;
	const float distanceUntilExhaustion = distanceStartToLastObstructionComputation + remainingDistanceBeforeExhaustion;
	return min(furthestPossibleEarDistance, distanceUntilExhaustion);
}

void CAuditionMapRayCastUpr::CancelRemainingRaysForPendingStimulus(const PendingListenerForStimulusIndex& pendingStimulusIndex)
{
	SPendingListenerForStimulus& pendingStimulus = m_pendingListenersForStimulus[pendingStimulusIndex];

	for (const SPendingStimulusRayInfo& queuedRayInfo : pendingStimulus.queuedRayInfos)
	{
		const QueuedRayID queuedRayId = queuedRayInfo.queuedRayId;
		CancelRay(queuedRayId);

		m_pendingStimuliParams.DecRef(pendingStimulus.stimulusParamsIndex);
	}

	pendingStimulus.queuedRayInfos.clear();
}

bool CAuditionMapRayCastUpr::InterpretPendingStimulusRayCastResult(const PendingListenerForStimulusIndex& pendingStimulusIndex, const QueuedRayID queuedRayId, const bool bStimulusReachedEar, SParamsForListenerEvents& eventsParams)
{
	SPendingListenerForStimulus& pendingStimulus = m_pendingListenersForStimulus[pendingStimulusIndex];

	auto findIt = std::find_if(pendingStimulus.queuedRayInfos.begin(), pendingStimulus.queuedRayInfos.end(), [queuedRayId](const SPendingStimulusRayInfo& rayInfo)
	{
		return rayInfo.queuedRayId == queuedRayId;
	});

	DRX_ASSERT(findIt != pendingStimulus.queuedRayInfos.end());
	const bool bRayFound = findIt != pendingStimulus.queuedRayInfos.end();
	if (bRayFound)
	{
		const PendingStimulusParamsIndex stimulusParamsIndex = pendingStimulus.stimulusParamsIndex;
		SPendingStimulusParams& stimulusParams = m_pendingStimuliParams[stimulusParamsIndex];
		stimulusParams.bReachedOneOrMoreEars |= bStimulusReachedEar;

		// Make copy of the params to make sure, that they aren't changed when sending them further in events
		eventsParams.m_listenerEntityId = pendingStimulus.listenerEntityId;
		eventsParams.m_reachedEarIndex = findIt->targetEarIndex;
		eventsParams.m_soundParams = stimulusParams.soundParams;
		eventsParams.m_bReachedAtLeastOneEar = stimulusParams.bReachedOneOrMoreEars;
		eventsParams.m_bLastRay = stimulusParams.GetRefCount() == 1;
		
		pendingStimulus.queuedRayInfos.erase(findIt);
		m_pendingStimuliParams.DecRef(stimulusParamsIndex);
	}

	const bool bAllRaysDone = pendingStimulus.queuedRayInfos.empty();
	const bool bStimulusDone = bAllRaysDone || bStimulusReachedEar;
	if (bStimulusDone)
	{
		CancelRemainingRaysForPendingStimulus(pendingStimulusIndex);
		m_pendingListenersForStimulus.Release(pendingStimulusIndex);
	}
	return bRayFound;
}

void CAuditionMapRayCastUpr::ProcessPendingStimulusRayCastResult(const PendingListenerForStimulusIndex& pendingStimulusIndex, const QueuedRayID queuedRayId, const bool bStimulusReachedEar)
{
	SParamsForListenerEvents eventsParams;
	if (InterpretPendingStimulusRayCastResult(pendingStimulusIndex, queuedRayId, bStimulusReachedEar, eventsParams))
	{
		if (bStimulusReachedEar)
		{
			m_owner.DeliverStimulusToListener(eventsParams.m_soundParams, eventsParams.m_listenerEntityId, eventsParams.m_reachedEarIndex);
		}
		if (eventsParams.m_bLastRay && eventsParams.m_bReachedAtLeastOneEar)
		{
			m_owner.NotifyGlobalListeners_StimulusProcessedAndReachedOneOrMoreEars(eventsParams.m_soundParams);
		}
	}
}

void CAuditionMapRayCastUpr::CancelAllPendingStimuli()
{
	CancelAllRays();

	m_pendingListenersForStimulus.Clear();
	m_pendingStimuliParams.Clear();
}

} //endns AuditionMapHelpers

} //endns SoundStimulus
