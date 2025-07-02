// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   PersonalInterestUpr.cpp
   $Id$
   $DateTime$
   Описание: Interest Upr (tracker) for interested individuals

   Notes:       Consider converting interest values float->i32
   -------------------------------------------------------------------------
   История:
   - 15 Feb 2010 : Heavily refactored by Ricardo Pillosu
 *********************************************************************/

#include <drx3D/AI/StdAfx.h>

#include <drx3D/AI/PersonalInterestUpr.h>
#include <drx3D/AI/CentralInterestUpr.h>
#include <drx3D/AI/CAISystem.h>
#include <drx3D/AI/Puppet.h>
#include <drx3D/AI/AIActions.h>
// For persistent debugging
#include <drx3D/CoreX/Game/IGameFramework.h>

CPersonalInterestUpr::CPersonalInterestUpr(CAIActor* pAIActor) :
	m_pCIM(NULL)
{
	Reset();

	if (pAIActor)
	{
		Assign(pAIActor);
	}
}

//------------------------------------------------------------------------------------------------------------------------

// Clear tracking cache, clear assignment
void CPersonalInterestUpr::Reset()
{
	m_refAIActor.Reset();
	m_refInterestDummy.Release();
	m_IdInterestingEntity = 0;
	m_IdLastInterestingEntity = 0;
	m_timeLastInterestingEntity = 0.f;
	m_Settings.Reset();
	m_vOffsetInterestingEntity.zero();
	m_bIsPlayingAction = false;
}

bool CPersonalInterestUpr::ForgetInterestingEntity()
{
	if (m_IdInterestingEntity > 0)
	{
		m_IdLastInterestingEntity = m_IdInterestingEntity;
		m_timeLastInterestingEntity = GetAISystem()->GetFrameStartTime();
		m_pCIM->OnInterestEvent(
		  IInterestListener::eIE_InterestStop,
		  m_refAIActor.GetAIObject()->GetEntity()->GetId(),
		  m_IdLastInterestingEntity);
		gAIEnv.pSmartObjectUpr->RemoveSmartObjectState(gEnv->pEntitySystem->GetEntity(m_IdInterestingEntity), "Busy");
		m_IdInterestingEntity = 0;
		m_pCIM->AddDebugTag(GetAssigned()->GetEntityID(), "Nothing around");
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------

void CPersonalInterestUpr::Assign(CAIActor* pAIActor)
{
	Assign(GetWeakRef(pAIActor));
}

//------------------------------------------------------------------------------------------------------------------------

// You must also ensure the PIM pointer in the CAIActor is set to this object
void CPersonalInterestUpr::Assign(CWeakRef<CAIActor> refAIActor)
{
	// Check for redundant calls
	if (m_refAIActor == refAIActor)
		return;

	CAIActor* pOldAIActor = m_refAIActor.GetAIObject();
	IEntity* pOldEntity = pOldAIActor ? pOldAIActor->GetEntity() : NULL;
	if (pOldEntity)
	{
		gAIEnv.pSmartObjectUpr->RemoveSmartObjectState(pOldEntity, "RegisteredActor");
	}

	CAIActor* pNewAIActor = refAIActor.GetAIObject();
	IEntity* pNewEntity = pNewAIActor ? pNewAIActor->GetEntity() : NULL;
	if (pNewEntity)
	{
		gAIEnv.pSmartObjectUpr->AddSmartObjectState(pNewEntity, "RegisteredActor");
	}

	Reset();

	// Assign
	m_refAIActor = refAIActor;

	// Don't create interest dummy objects during serialization as the AI system itself is serialized later
	//	(meaning the objects get leaked). The objects will be created by CObjectContainer::Serialize with the correct ID.
	if (pNewAIActor && !gEnv->pSystem->IsSerializingFile())
	{
		gAIEnv.pAIObjectUpr->CreateDummyObject(m_refInterestDummy, "InterestDummy");
	}

	m_pCIM = CCentralInterestUpr::GetInstance();

	// Debug draw
	if (m_pCIM->IsDebuggingEnabled())
	{
		if (pOldEntity)
		{
			m_pCIM->AddDebugTag(pOldEntity->GetId(), "No longer interested");
		}

		if (pNewEntity)
		{
			m_pCIM->AddDebugTag(pNewEntity->GetId(), "Interested");
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------

bool CPersonalInterestUpr::Update(bool bCloseToCamera)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (m_bIsPlayingAction)
		return false;

	bool bRet = false;
	IEntity* pUser = m_refAIActor.GetAIObject()->GetEntity();
	const SEntityInterest* pMostInteresting = bCloseToCamera ? PickMostInteresting() : NULL;

	if (pMostInteresting)
	{
		bRet = true;
		EntityId IdMostInteresting = pMostInteresting->GetEntityId();
		if (m_IdInterestingEntity != IdMostInteresting)
		{
			m_vOffsetInterestingEntity = pMostInteresting->m_vOffset;
			ForgetInterestingEntity();
			if (pMostInteresting->m_nbShared == 0)
			{
				gAIEnv.pSmartObjectUpr->AddSmartObjectState(gEnv->pEntitySystem->GetEntity(IdMostInteresting), "Busy");
			}
			m_pCIM->OnInterestEvent(IInterestListener::eIE_InterestStart, pUser->GetId(), IdMostInteresting);
			m_IdInterestingEntity = IdMostInteresting;

			if (!pMostInteresting->m_sActionName.empty() && pMostInteresting->m_sActionName.compareNoCase("none") != 0)
			{
				const IEntity* pMostInterestingEntity = pMostInteresting->GetEntity();
				gAIEnv.pAIActionUpr->ExecuteAIAction(
				  pMostInteresting->m_sActionName.c_str(), pUser, const_cast<IEntity*>(pMostInterestingEntity), 1, 0, this);
			}
		}
	}
	else
	{
		bRet = ForgetInterestingEntity();
	}

	return bRet;
}

void CPersonalInterestUpr::OnActionEvent(IAIAction::EActionEvent eActionEvent)
{
	if (m_IdInterestingEntity)
	{
		CAIActor* pAIActor = m_refAIActor.GetAIObject();
		if (pAIActor)
		{
			IEntity* pUser = pAIActor->GetEntity();

			//If no reference is available then exit without propagating the eActionEvent
			if (pUser)
			{
				switch (eActionEvent)
				{
				case IAIAction::ActionEnd:
					m_pCIM->OnInterestEvent(IInterestListener::eIE_InterestActionComplete, pUser->GetId(), m_IdInterestingEntity);
					m_bIsPlayingAction = false;
					break;

				case IAIAction::ActionStart:
					m_bIsPlayingAction = true;
					break;

				case IAIAction::ActionCancel:
					m_pCIM->OnInterestEvent(IInterestListener::eIE_InterestActionCancel, pUser->GetId(), m_IdInterestingEntity);
					m_bIsPlayingAction = false;
					break;

				case IAIAction::ActionAbort:
					m_pCIM->OnInterestEvent(IInterestListener::eIE_InterestActionAbort, pUser->GetId(), m_IdInterestingEntity);
					m_bIsPlayingAction = false;
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------

void CPersonalInterestUpr::Serialize(TSerialize ser)
{
	ser.Value("m_refAIActor", m_refAIActor);
	ser.Value("m_IdInterestingEntity", m_IdInterestingEntity);
	ser.Value("m_IdLastInterestingEntity", m_IdLastInterestingEntity);
	ser.Value("m_timeLastInterestingEntity", m_timeLastInterestingEntity);
	ser.Value("m_vOffsetInterestingEntity", m_vOffsetInterestingEntity);
	ser.Value("m_refInterestDummy", m_refInterestDummy);
	ser.Value("m_Settings", m_Settings);
	ser.Value("m_bIsPlayingAction", m_bIsPlayingAction);

	if (ser.IsReading())
	{
		m_pCIM = CCentralInterestUpr::GetInstance();
	}
}

//------------------------------------------------------------------------------------------------------------------------

void CPersonalInterestUpr::SetSettings(bool bEnablePIM, float fInterestFilter, float fAngleCos)
{
	m_Settings.m_bEnablePIM = bEnablePIM;

	if (fInterestFilter >= 0.f)
	{
		m_Settings.m_fInterestFilter = fInterestFilter;
	}

	assert((-1.f <= fAngleCos) && (fAngleCos <= 1.f));
	if ((-1.f <= fAngleCos) && (fAngleCos <= 1.f))
	{
		m_Settings.m_fAngleCos = fAngleCos;
	}

	if (m_pCIM->IsDebuggingEnabled())
	{
		DrxFixedStringT<32> sText;
		sText.Format("%s: Filter %0.1f Angle %0.1f",
		             m_Settings.m_bEnablePIM ? "Enabled" : "Disabled",
		             m_Settings.m_fInterestFilter,
		             2.f * (RAD2DEG(acos(m_Settings.m_fAngleCos))));
		m_pCIM->AddDebugTag(GetAssigned()->GetEntityID(), sText);
	}
}

//------------------------------------------------------------------------------------------------------------------------
const SEntityInterest* CPersonalInterestUpr::PickMostInteresting() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIActor* pAIActor = m_refAIActor.GetAIObject();
	if (!pAIActor)
	{
		assert(false);
		return NULL;
	}

	tukk szActorClass = pAIActor->GetEntity()->GetClass()->GetName();

	Vec3 vActorPos = pAIActor->GetPos();
	const SEntityInterest* pRet = NULL;
	float fMostInterest = m_Settings.m_fInterestFilter;

	const float deltaTime = (GetAISystem()->GetFrameStartTime() - m_timeLastInterestingEntity).GetSeconds();

	CCentralInterestUpr::TVecInteresting* pInterestingEntities = m_pCIM->GetInterestingEntities();
	CCentralInterestUpr::TVecInteresting::const_iterator it = pInterestingEntities->begin();
	CCentralInterestUpr::TVecInteresting::const_iterator itEnd = pInterestingEntities->end();
	for (; it != itEnd; ++it)
	{
		if (!(it->IsValid()))
			continue;

		if (it->m_entityId == m_IdLastInterestingEntity)
		{
			if (deltaTime < it->m_fPause)
				continue;
		}

		if (!(it->SupportsActorClass(szActorClass)))
			continue;

		Vec3 vToTarget = it->GetEntity()->GetPos() - vActorPos;
		float fDistToTargetSq = vToTarget.GetLengthSquared();
		float fRadius = it->m_fRadius;

		if (fDistToTargetSq > square(fRadius))
			continue;

		// Distance aspect - close objects have higher interest
		float& fMaxDist = fRadius;  // Use alias
		float fDist = clamp_tpl(sqrt_tpl(fDistToTargetSq), .1f, fMaxDist);
		float fDistanceAspect = (fMaxDist - fDist) / fMaxDist;

		// Basic accumulation of interest
		float fAccumulated = (it->m_fInterest) * fDistanceAspect;

		// Is this the most interesting so far?
		if (fMostInterest < fAccumulated)
		{
			// Check viewing angle
			float fDot = vToTarget.normalized().Dot(pAIActor->GetViewDir());
			if (fDot > m_Settings.m_fAngleCos)
			{
				// Check if it's busy or not
				if (it->m_nbShared ||
				    (m_IdInterestingEntity == it->m_entityId) ||
				    !gAIEnv.pSmartObjectUpr->CheckSmartObjectStates(it->GetEntity(), "Busy"))
				{
					bool bSeen = m_pCIM->CanCastRays() ? CheckVisibility(*it) : true;
					if (bSeen)
					{
						fMostInterest = fAccumulated;
						pRet = &(*it);
					}
				}
			}
		}
	}

	return pRet;
}

//------------------------------------------------------------------------------------------------------------------------

IEntity* CPersonalInterestUpr::GetInterestEntity() const
{
	return m_IdInterestingEntity ? gEnv->pEntitySystem->GetEntity(m_IdInterestingEntity) : NULL;
}

//------------------------------------------------------------------------------------------------------------------------
bool CPersonalInterestUpr::CheckVisibility(const SEntityInterest& interestingRef) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIActor* pAIActor = m_refAIActor.GetAIObject();
	if (pAIActor)
	{
		Vec3 vEyePos = pAIActor->GetPos();  // For an AI actor with a proxy, this is the eye position

		// Select the point represented by the sum of the interesting target's position and the interesting target's offset.
		const IEntity* pTargetEntity = interestingRef.GetEntity();
		if (pTargetEntity)
		{
			//If no offset specified make sure existing entries work, by ensuring a 1 meter z-direction offset
			Vec3 vPoint2;
			Vec3 vOffset = interestingRef.m_vOffset;
			if (vOffset.IsZero())
			{
				vPoint2 = pTargetEntity->GetPos();
				vPoint2.z += 1.f;
			}
			else
			{
				vPoint2 = pTargetEntity->GetWorldTM() * vOffset;
			}

			ray_hit hits;
			i32 intersections = gEnv->pPhysicalWorld->RayWorldIntersection(vEyePos, vPoint2 - vEyePos, ent_all,
			                                                               rwi_stop_at_pierceable | rwi_colltype_any, &hits, 1, pAIActor->GetPhysics());

			if (!intersections)
				return true;

			IEntity* pCollider = gEnv->pEntitySystem->GetEntityFromPhysics(hits.pCollider);
			return (pTargetEntity == pCollider);
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------------------------------
Vec3 CPersonalInterestUpr::GetInterestingPos() const
{
	// It is often appropriate to test against visible range before calling this (depending on interest type)
	CAIActor* pAIActor = m_refAIActor.GetAIObject();
	if (pAIActor)
	{
		IEntity* pEntity = GetInterestEntity();
		if (pEntity)
		{
			return pEntity->GetWorldTM() * m_vOffsetInterestingEntity;
		}
	}

	return ZERO;
}

//------------------------------------------------------------------------------------------------------------------------

CWeakRef<CAIObject> CPersonalInterestUpr::GetInterestDummyPoint()
{
	if (!IsInterested())
		return NILREF;

	CAIObject* pInterestDummy = m_refInterestDummy.GetAIObject();
	if (pInterestDummy)
		pInterestDummy->SetPos(GetInterestingPos());

	return m_refInterestDummy.GetWeakRef();
}

//------------------------------------------------------------------------------------------------------------------------
ELookStyle CPersonalInterestUpr::GetLookingStyle() const
{
	/* When this works we can really use a better selection of those values:
	   LOOKSTYLE_HARD,
	   LOOKSTYLE_HARD_NOLOWER,
	   LOOKSTYLE_SOFT,
	   LOOKSTYLE_SOFT_NOLOWER,
	 */

	return LOOKSTYLE_HARD;
}
