// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/TacticalUpr.h>

#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/ActorDefinitions.h>

static i32k MAX_TACTICAL_ENTITIES = 256;

//////////////////////////////////////////////////////////////////////////

void CTacticalUpr::STacticalInterestPoint::Reset()
{
	m_scanned = eScanned_None;
	m_overrideIconType = eIconType_NumIcons;
	m_tagged = false;
	m_visible = false;
	m_pinged = false;
}

//////////////////////////////////////////////////////////////////////////

CTacticalUpr::CTacticalUpr()
{
	for (i32 i = 0; i < eTacticalEntity_Last; i++)
		m_allTacticalPoints.push_back(TInterestPoints());
}



CTacticalUpr::~CTacticalUpr()
{
}


void CTacticalUpr::Init()
{
	for (i32 i = 0; i < eTacticalEntity_Last; i++)
		m_allTacticalPoints[i].reserve(MAX_TACTICAL_ENTITIES);
}

void CTacticalUpr::Reset()
{
	for (i32 i = 0; i < eTacticalEntity_Last; i++)
		stl::free_container(m_allTacticalPoints[i]);

	
	ResetClassScanningData();
}

void CTacticalUpr::STacticalInterestPoint::Serialize(TSerialize ser)
{
	ser.Value("m_entityId", m_entityId);
	ser.Value("m_scanned", m_scanned);
	ser.Value("m_overrideIconType", m_overrideIconType);
	ser.Value("m_tagged", m_tagged);
	ser.Value("m_visible", m_visible);
	ser.Value("m_pinged", m_pinged);
}


void CTacticalUpr::Serialize(TSerialize ser)
{
	if (ser.IsReading())
	{
		// Serialize tactical points
		ClearAllTacticalPoints();

		u32 numTacInfoPointsGroups = 0;
		STacticalInterestPoint interestPoint;
		ser.Value("numTacInfoPointsGroups", numTacInfoPointsGroups);
		for (u32 i = 0; i < numTacInfoPointsGroups; i++) // Reads in according to ETacticalEntityType
		{
			ser.BeginGroup("TacInfoPointsGroup");
			u32 numTacInfoPoints = 0;
			ser.Value("numTacInfoPoints", numTacInfoPoints);
			if (numTacInfoPoints > 0)
			{
				// Go through all the tac points
				for (size_t j = 0; j < numTacInfoPoints; j++)
				{
					ser.BeginGroup("TacInfoPoint");
					interestPoint.Serialize(ser);
					ser.EndGroup();
					AddTacticalInfoPointData((ETacticalEntityType)i, interestPoint);
				}
			}

			ser.EndGroup();
		}

		// Serialize tac override entities
		m_tacEntityToOverrideEntities.clear();
		u32 numTacOverrideData = 0;
		ser.Value("numTacOverrideData", numTacOverrideData);
		for (u32 i = 0; i < numTacOverrideData; i++)
		{
			ser.BeginGroup("TacOverrideData");
			EntityId origEntity = 0;
			EntityId overrideEntity = 0;
			ser.Value("origEntity", origEntity);
			ser.Value("overrideEntity", overrideEntity);
			AddOverrideEntity(origEntity, overrideEntity);

			ser.EndGroup();
		}		

		// Serialize class scanned data
		m_classes.clear();
		u32 numInterestClasses = 0;
		string interestClassName;
		ser.Value("numInterestClasses", numInterestClasses);
		for (u32 i = 0; i < numInterestClasses; i++)
		{
			TScanningCount scanningCount;
			ser.BeginGroup("InterestClassData");
			ser.Value("name", interestClassName);
			ser.Value("scanningCount", scanningCount);

			IEntityClassRegistry* pEntityClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
			DRX_ASSERT(pEntityClassRegistry != NULL);
			IEntityClass* pEntityClass = pEntityClassRegistry->FindClass(interestClassName);
			DRX_ASSERT(pEntityClass);
			if (pEntityClass)
			{
				m_classes[pEntityClass] = scanningCount;
			}

			ser.EndGroup();
		}
	}
	else
	{
		// Serialize tactical points
		u32 numTacInfoPointsGroups = eTacticalEntity_Last;
		ser.Value("numTacInfoPointsGroups", numTacInfoPointsGroups);
		for (u32 i = 0; i < numTacInfoPointsGroups; i++) // Writes in according to ETacticalEntityType
		{
			ser.BeginGroup("TacInfoPointsGroup");
			TInterestPoints& interestPoints = m_allTacticalPoints[i];
			u32 numTacInfoPoints = interestPoints.size();
			ser.Value("numTacInfoPoints", numTacInfoPoints);
			if (numTacInfoPoints > 0)
			{
				// Go through all the tac points
				for (u32 j = 0; j < numTacInfoPoints; j++)
				{
					ser.BeginGroup("TacInfoPoint");
					STacticalInterestPoint& interestPoint = interestPoints[j];
					interestPoint.Serialize(ser);
					ser.EndGroup();
				}
			}

			ser.EndGroup();
		}

		// Serialize tac override entities
		u32 numTacOverrideData = m_tacEntityToOverrideEntities.size();
		ser.Value("numTacOverrideData", numTacOverrideData);
		TTacticalEntityToOverrideEntities::iterator tactOverrideDataIter = m_tacEntityToOverrideEntities.begin();
		const TTacticalEntityToOverrideEntities::const_iterator tactOverrideDataIterEnd = m_tacEntityToOverrideEntities.end();
		while (tactOverrideDataIter != tactOverrideDataIterEnd)
		{
			EntityId origEntity = tactOverrideDataIter->first;
			EntityId overrideEntity = tactOverrideDataIter->second;
			ser.BeginGroup("TacOverrideData");
			ser.Value("origEntity", origEntity);
			ser.Value("overrideEntity", overrideEntity);
			ser.EndGroup();
			++tactOverrideDataIter;
		}

		// Serialize class scanned data
		u32 numInterestClasses = m_classes.size();
		ser.Value("numInterestClasses", numInterestClasses);
		TInterestClasses::iterator interestClassesIter = m_classes.begin();
		const TInterestClasses::const_iterator interestClassesIterEnd = m_classes.end();
		while (interestClassesIter != interestClassesIterEnd)
		{
			const IEntityClass* pEntityClass = interestClassesIter->first;
			DRX_ASSERT(pEntityClass);
			TScanningCount scanningCount = interestClassesIter->second;
			ser.BeginGroup("InterestClassData");
			ser.Value("name", pEntityClass->GetName());
			ser.Value("scanningCount", scanningCount);
			ser.EndGroup();
			++interestClassesIter;
		}
	}
}

void CTacticalUpr::PostSerialize()
{
	// Radar needs player client entity to be set otherwise edge entity items won't show
	// ENTITY_EVENT_UNHIDE isn't called when loading, since state is already unhided, that usually triggers CActor::ProcessEvent with event type ENTITY_EVENT_UNHIDE
	// Call the hud event here instead
	const EntityId playerEntityID = gEnv->pGame->GetIGameFramework()->GetClientActorId();
	AddEntity(playerEntityID, CTacticalUpr::eTacticalEntity_Unit);
	SHUDEvent hudevent(eHUDEvent_AddEntity);
	hudevent.AddData(SHUDEventData((i32)playerEntityID));
	CHUDEventDispatcher::CallEvent(hudevent);
}

void CTacticalUpr::AddEntity(const EntityId id, ETacticalEntityType type)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last);
	if (type == eTacticalEntity_Last)
		return;

	TInterestPoints& points = m_allTacticalPoints[type];

	TInterestPoints::const_iterator it = points.begin();
	TInterestPoints::const_iterator end = points.end();
	for(; it!=end; ++it)
	{
		if(it->m_entityId == id)
		{
			return;
		}
	}
	points.push_back(STacticalInterestPoint(id));

	if(!gEnv->bMultiplayer)
	{
		if(IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id))
		{
			SetClassScanned(pEntity->GetClass(), eScanned_None);
		}
	}
}



void CTacticalUpr::RemoveEntity(const EntityId id, ETacticalEntityType type)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last);
	if (type == eTacticalEntity_Last)
		return;

	TInterestPoints& points = m_allTacticalPoints[type];
	TInterestPoints::iterator it = points.begin();
	TInterestPoints::const_iterator end = points.end();
	for(; it!=end; ++it)
	{
		if(it->m_entityId == id)
		{
			//Removes the silhouette in case it's an actor
			SHUDEvent event(eEHUDEvent_OnScannedEnemyRemove);
			event.AddData(SHUDEventData((i32)id));
			CHUDEventDispatcher::CallEvent(event);
			points.erase(it);

			return;
		}
	}
}



void CTacticalUpr::SetEntityScanned(const EntityId id, ETacticalEntityType type)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last);
	if (type == eTacticalEntity_Last)
		return;

	TInterestPoints& points = m_allTacticalPoints[type];
	TInterestPoints::iterator it = points.begin();
	TInterestPoints::const_iterator end = points.end();
	for(; it!=end; ++it)
	{
		if(it->m_entityId == id)
		{
			TScanningCount scanned = min((i32)eScanned_Max, it->m_scanned + 1);

			it->m_scanned = scanned;
			if(IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id))
			{
				SetClassScanned(pEntity->GetClass(), scanned);
			}
			return;
		}
	}
}



void CTacticalUpr::SetEntityScanned(const EntityId id)
{
	for(i32 i=0; i<eTacticalEntity_Last; ++i)
	{
		SetEntityScanned(id, (ETacticalEntityType)i);
	}
}



CTacticalUpr::TScanningCount CTacticalUpr::GetEntityScanned(const EntityId id) const
{
	TScanningCount scanningCount = eScanned_None;
	bool bKeepSearching = true;

	TAllTacticalPoints::const_iterator iter = m_allTacticalPoints.begin();
	const TAllTacticalPoints::const_iterator iterEnd = m_allTacticalPoints.end();

	while (iter != iterEnd && bKeepSearching)
	{
		const CTacticalUpr::TInterestPoints& interestPoints = *iter;

		const size_t numInterestPoints = interestPoints.size();
		for (size_t i = 0; i < numInterestPoints; i++)
		{
			const STacticalInterestPoint& interestPoint = interestPoints[i];

			const EntityId entityId = interestPoint.m_entityId;
			if (entityId == id)
			{
				scanningCount = interestPoint.m_scanned;
				bKeepSearching = false;
				break;
			}
		}

		++iter;
	}

	return scanningCount;
}



void CTacticalUpr::SetEntityTagged(const EntityId id, const bool bTagged)
{
	for(i32 i=0; i<eTacticalEntity_Last; ++i)
	{
		SetEntityTagged(id, (ETacticalEntityType)i, bTagged);
	}
}



void CTacticalUpr::SetEntityTagged(const EntityId id, ETacticalEntityType type, const bool bTagged)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last);
	if (type == eTacticalEntity_Last)
		return;

	TInterestPoints& points = m_allTacticalPoints[type];
	TInterestPoints::iterator it = points.begin();
	TInterestPoints::const_iterator end = points.end();
	for(; it!=end; ++it)
	{
		if(it->m_entityId == id)
		{
			it->m_tagged = bTagged;
			return;
		}
	}
}



bool CTacticalUpr::IsEntityTagged(const EntityId id) const
{
	bool bTagged = false;
	bool bKeepSearching = true;

	TAllTacticalPoints::const_iterator iter = m_allTacticalPoints.begin();
	const TAllTacticalPoints::const_iterator iterEnd = m_allTacticalPoints.end();

	while (iter != iterEnd && bKeepSearching)
	{
		const CTacticalUpr::TInterestPoints& interestPoints = *iter;

		const size_t numInterestPoints = interestPoints.size();
		for (size_t i = 0; i < numInterestPoints; i++)
		{
			const STacticalInterestPoint& interestPoint = interestPoints[i];

			const EntityId entityId = interestPoint.m_entityId;
			if (entityId == id)
			{
				bTagged = interestPoint.m_tagged;
				bKeepSearching = false;
				break;
			}
		}

		++iter;
	}

	return bTagged;
}



void CTacticalUpr::SetClassScanned(IEntityClass* pClass, const TScanningCount scanned)
{
	TInterestClasses::iterator it = m_classes.begin();
	TInterestClasses::iterator end = m_classes.end();
	for(; it!=end; ++it)
	{
		if(it->first == pClass)
		{
			if(it->second < scanned)
			{
				it->second = scanned;
			}
			return;
		}
	}
	m_classes[pClass] = scanned;
}



CTacticalUpr::TScanningCount CTacticalUpr::GetClassScanned(IEntityClass* pClass) const
{
	TInterestClasses::const_iterator it = m_classes.begin();
	TInterestClasses::const_iterator end = m_classes.end();
	for(; it!=end; ++it)
	{
		if(it->first == pClass)
		{
			return it->second;
		}
	}
	return eScanned_None;
}



void CTacticalUpr::ClearAllTacticalPoints()
{
	// Go through all types of tactical points
	CTacticalUpr::TAllTacticalPoints::iterator allPointsIt = m_allTacticalPoints.begin();
	CTacticalUpr::TAllTacticalPoints::iterator allPointsItEnd = m_allTacticalPoints.end();
	for(; allPointsIt!=allPointsItEnd; ++allPointsIt)
	{
		CTacticalUpr::TInterestPoints &points = (*allPointsIt); // Category of points
		points.clear();
	}	
}



void CTacticalUpr::ResetAllTacticalPoints()
{
	// Go through all types of tactical points
	CTacticalUpr::TAllTacticalPoints::iterator allPointsIt = m_allTacticalPoints.begin();
	CTacticalUpr::TAllTacticalPoints::iterator allPointsItEnd = m_allTacticalPoints.end();
	TInterestPoints::iterator interestPointsIt;
	for(; allPointsIt!=allPointsItEnd; ++allPointsIt)
	{
		CTacticalUpr::TInterestPoints &points = (*allPointsIt); // Category of points

		for(interestPointsIt = points.begin(); interestPointsIt!=points.end(); ++interestPointsIt)
		{
			STacticalInterestPoint &entry = (*interestPointsIt);
			entry.Reset();
		}	
	}	
}



void CTacticalUpr::ResetClassScanningData()
{
	stl::free_container(m_classes);
}



CTacticalUpr::TInterestPoints& CTacticalUpr::GetTacticalPoints(const ETacticalEntityType type)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last); // Shouldn't happen
	if (type == eTacticalEntity_Last)
		return m_allTacticalPoints[eTacticalEntity_Custom];

	return m_allTacticalPoints[type];
}



const CTacticalUpr::TInterestPoints& CTacticalUpr::GetTacticalPoints(const ETacticalEntityType type) const
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last); // Shouldn't happen
	if (type == eTacticalEntity_Last)
		return m_allTacticalPoints[eTacticalEntity_Custom];

	return m_allTacticalPoints[type];
}



void CTacticalUpr::Ping(const float fPingDistance)
{
	Ping(eTacticalEntity_Custom, fPingDistance);
}



void CTacticalUpr::Ping(const ETacticalEntityType type, const float fPingDistance)
{
	const Vec3& clientPos = CHUDUtils::GetClientPos();
	AABB entityBox;

	const float pingDistanceSq = fPingDistance*fPingDistance;

	TInterestPoints& points = GetTacticalPoints(type);

	TInterestPoints::iterator pointsIt = points.begin();
	TInterestPoints::iterator pointsItEnd = points.end();

	for(; pointsIt!=pointsItEnd; ++pointsIt)
	{
		STacticalInterestPoint &entry = (*pointsIt);

		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entry.m_entityId);
		if (!pEntity)
			continue;

		if(pEntity->IsHidden())
			continue;

		pEntity->GetWorldBounds(entityBox);

		if(clientPos.GetSquaredDistance(entityBox.GetCenter()) > pingDistanceSq) // Outside ping range
			continue;

		if (entry.m_pinged != true)
		{
			entry.m_pinged = true;
		}
	}
}



void CTacticalUpr::AddOverrideEntity(const EntityId tacticalEntity, const EntityId overrideEntity)
{
	DRX_ASSERT(tacticalEntity != 0);
	DRX_ASSERT(overrideEntity != 0);

	TTacticalEntityToOverrideEntities::iterator iter = m_tacEntityToOverrideEntities.find(tacticalEntity);
	if (iter == m_tacEntityToOverrideEntities.end())
	{
		m_tacEntityToOverrideEntities[tacticalEntity] = overrideEntity;
	}
	else
	{
		GameWarning("CTacticalUpr::AddOverrideEntity: Entity: %u is the parent of more than one TacticalOverride entity, ignoring entity: %u", tacticalEntity, overrideEntity);
	}
}



void CTacticalUpr::RemoveOverrideEntity(const EntityId tacticalEntity, const EntityId overrideEntity)
{
	TTacticalEntityToOverrideEntities::iterator iter = m_tacEntityToOverrideEntities.find(tacticalEntity);
	if (iter != m_tacEntityToOverrideEntities.end())
	{
		const EntityId entityId = iter->second;
		DRX_ASSERT(entityId == overrideEntity);
		m_tacEntityToOverrideEntities.erase(iter);
	}
	else
	{
		GameWarning("CTacticalUpr::RemoveOverrideEntity: Tried to remove override entity but wasn't added");
	}
}



void CTacticalUpr::RemoveOverrideEntity(const EntityId overrideEntity)
{
	TTacticalEntityToOverrideEntities::iterator iter = m_tacEntityToOverrideEntities.begin();
	const TTacticalEntityToOverrideEntities::const_iterator iterEnd = m_tacEntityToOverrideEntities.end();
	while (iter != iterEnd)
	{
		const EntityId curOverrideEntityId = iter->second;
		if (curOverrideEntityId == overrideEntity)
		{
			TTacticalEntityToOverrideEntities::iterator deleteIter = iter;
			++iter;
			m_tacEntityToOverrideEntities.erase(deleteIter);
		}
		else
		{
			++iter;
		}
	}
}



EntityId CTacticalUpr::GetOverrideEntity(const EntityId tacticalEntity) const
{
	TTacticalEntityToOverrideEntities::const_iterator iter = m_tacEntityToOverrideEntities.find(tacticalEntity);
	if (iter == m_tacEntityToOverrideEntities.end())
	{
		return (EntityId)0;
	}
	else
	{
		return iter->second;
	}
}



const Vec3& CTacticalUpr::GetTacticalIconWorldPos(const EntityId tacticalEntityId, IEntity* pTacticalEntity, bool& inOutIsHeadBone)
{
	DRX_ASSERT(tacticalEntityId != 0);
	DRX_ASSERT(pTacticalEntity != NULL);
	if (pTacticalEntity == NULL)
	{
		return m_tempVec3.Set(0.0f,0.0f,0.0f);
	}

	// Try to get pos from headbone if don't have an override pos
	TTacticalEntityToOverrideEntities::const_iterator iter = m_tacEntityToOverrideEntities.find(tacticalEntityId);
	if (iter == m_tacEntityToOverrideEntities.end()) 
	{
		static IEntityClass* s_pTurretEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("Turret");

		IEntityClass* pEntityClass = pTacticalEntity->GetClass();
		if (pEntityClass != NULL && pEntityClass == s_pTurretEntityClass)
		{
			ICharacterInstance* pCharacterInstance = pTacticalEntity->GetCharacter(0);
			if (pCharacterInstance != NULL)
			{
				IDefaultSkeleton& rIDefaultSkeleton = pCharacterInstance->GetIDefaultSkeleton();
				ISkeletonPose* pSkeletonPose = pCharacterInstance->GetISkeletonPose();
				DRX_ASSERT(pSkeletonPose != NULL);

				i16k jointId = rIDefaultSkeleton.GetJointIDByName("arcjoint");
				if (0 <= jointId)
				{
					QuatT boneLocation = pSkeletonPose->GetAbsJointByID(jointId);
					m_tempVec3 = pTacticalEntity->GetWorldTM().TransformPoint(boneLocation.t);
					return m_tempVec3;
				}
			}
		}


		CUICVars* pCVars = g_pGame->GetUI()->GetCVars();
		if (pCVars->hud_InterestPointsAtActorsHeads == 1)
		{
			CActor* pActor = static_cast<CActor*>(g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(tacticalEntityId)); // Only want units to go from headbone, since vehicles have headbone as well
			if(pActor)
			{
				if(pActor->HasBoneID(BONE_HEAD))
				{
					QuatT boneLocation = pActor->GetBoneTransform(BONE_HEAD);
					m_tempVec3 = pTacticalEntity->GetWorldTM().TransformPoint(boneLocation.t) + Vec3(0.0f, 0.0f, 0.25f);
					inOutIsHeadBone = true;
					return m_tempVec3;
				}
				else if(pActor->HasBoneID(BONE_SPINE))
				{
					QuatT boneLocation = pActor->GetBoneTransform(BONE_SPINE);
					m_tempVec3 = pTacticalEntity->GetWorldTM().TransformPoint(boneLocation.t);
					return m_tempVec3;
				}
			}
		}

		return GetTacticalIconCenterBBoxWorldPos(pTacticalEntity);
	}
	else // Override entity exists so it determines position
	{
		IEntity* pOverrideEntity = gEnv->pEntitySystem->GetEntity(iter->second);
		if (pOverrideEntity)
		{
			m_tempVec3 = pOverrideEntity->GetWorldPos();
			return m_tempVec3;
		}
		else
		{
			GameWarning("CTacticalUpr::GetTacticalIconWorldPos: ID exists in mapping but failed to find entity, defaulting to center bounding box position");
			return GetTacticalIconCenterBBoxWorldPos(pTacticalEntity);
		}
	}
}



void CTacticalUpr::SetEntityOverrideIcon(const EntityId id, u8k overrideIconType)
{
	bool bKeepSearching = true;

	TAllTacticalPoints::iterator iter = m_allTacticalPoints.begin();
	const TAllTacticalPoints::const_iterator iterEnd = m_allTacticalPoints.end();

	while (iter != iterEnd && bKeepSearching)
	{
		CTacticalUpr::TInterestPoints& interestPoints = *iter;
		const size_t numInterestPoints = interestPoints.size();
		for (size_t i = 0; i < numInterestPoints; i++)
		{
			STacticalInterestPoint& interestPoint = interestPoints[i];

			const EntityId entityId = interestPoint.m_entityId;
			if (entityId == id)
			{
				interestPoint.m_overrideIconType = overrideIconType;
				bKeepSearching = false;
				break;
			}
		}

		++iter;
	}
}



u8 CTacticalUpr::GetEntityOverrideIcon(const EntityId id) const
{
	u8 overrideIcon = eIconType_NumIcons;
	bool bKeepSearching = true;

	TAllTacticalPoints::const_iterator iter = m_allTacticalPoints.begin();
	const TAllTacticalPoints::const_iterator iterEnd = m_allTacticalPoints.end();

	while (iter != iterEnd && bKeepSearching)
	{
		const CTacticalUpr::TInterestPoints& interestPoints = *iter;

		const size_t numInterestPoints = interestPoints.size();
		for (size_t i = 0; i < numInterestPoints; i++)
		{
			const STacticalInterestPoint& interestPoint = interestPoints[i];

			const EntityId entityId = interestPoint.m_entityId;
			if (entityId == id)
			{
				overrideIcon = interestPoint.m_overrideIconType;
				bKeepSearching = false;
				break;
			}
		}

		++iter;
	}

	return overrideIcon;
}



u8 CTacticalUpr::GetOverrideIconType(tukk szOverrideIconName) const
{
	return eIconType_NumIcons;
}



const Vec3&	CTacticalUpr::GetTacticalIconCenterBBoxWorldPos(IEntity* pTacticalEntity)
{
	DRX_ASSERT(pTacticalEntity != NULL);

	AABB box;
	pTacticalEntity->GetWorldBounds(box);
	m_tempVec3 = box.GetCenter();

	return m_tempVec3;
}



void CTacticalUpr::AddTacticalInfoPointData(const ETacticalEntityType type, const STacticalInterestPoint& point)
{
	DRX_ASSERT(m_allTacticalPoints.size() == eTacticalEntity_Last); // Will always be the case
	DRX_ASSERT(type < eTacticalEntity_Last);
	if (type == eTacticalEntity_Last)
		return;

	const EntityId entityId = point.m_entityId;
	TInterestPoints& points = m_allTacticalPoints[type];

	TInterestPoints::const_iterator it = points.begin();
	TInterestPoints::const_iterator end = points.end();
	for(; it!=end; ++it)
	{
		if(it->m_entityId == entityId)
		{
			return;
		}
	}
	points.push_back(point);
}

//////////////////////////////////////////////////////////////////////////
