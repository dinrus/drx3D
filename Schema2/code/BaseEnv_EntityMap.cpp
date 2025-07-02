// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityMap.h>

#include <drx3D/Entity/IEntitySystem.h>

namespace SchematycBaseEnv
{
	void CEntityMap::MarkEntityForDestruction(EntityId entityId)
	{
		m_entitiesMarkedForDestruction.push_back(entityId);
	}

	void CEntityMap::DestroyMarkedEntites()
	{
		EntityIds entitiesMarkedForDestruction = m_entitiesMarkedForDestruction;
		m_entitiesMarkedForDestruction.clear();
		for(EntityId entityId : entitiesMarkedForDestruction)
		{
			gEnv->pEntitySystem->RemoveEntity(entityId);
		}
	}

	void CEntityMap::AddObject(EntityId entityId, const sxema2::ObjectId& objectId)
	{
		m_entityObjectIds.insert(EntityObjectIds::value_type(entityId, objectId));
	}

	void CEntityMap::RemoveObject(EntityId entityId)
	{
		m_entityObjectIds.erase(entityId);
	}

	sxema2::ObjectId CEntityMap::FindObjectId(EntityId entityId) const
	{
		EntityObjectIds::const_iterator itObjectId = m_entityObjectIds.find(entityId);
		return itObjectId != m_entityObjectIds.end() ? itObjectId->second : sxema2::ObjectId();
	}

	sxema2::IObject* CEntityMap::FindObject(EntityId entityId) const
	{
		return gEnv->pSchematyc2->GetObjectUpr().GetObjectById(FindObjectId(entityId));
	}

	void CEntityMap::VisitEntities(const EntityMapVisitor& visitor)
	{
		DRX_ASSERT(visitor);
		if(visitor)
		{
			for(EntityObjectIds::value_type& entityObjectId : m_entityObjectIds)
			{
				visitor(entityObjectId.first);
			}
		}
	}
}