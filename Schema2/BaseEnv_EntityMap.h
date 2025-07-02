// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

namespace SchematycBaseEnv
{
	typedef TemplateUtils::CDelegate<void (const EntityId)> EntityMapVisitor;

	class CEntityMap
	{
	public:

		void MarkEntityForDestruction(EntityId entityId);
		void DestroyMarkedEntites();
		void AddObject(EntityId entityId, const sxema2::ObjectId& objectId);
		void RemoveObject(EntityId entityId);
		sxema2::ObjectId FindObjectId(EntityId entityId) const;
		sxema2::IObject* FindObject(EntityId entityId) const;
		void VisitEntities(const EntityMapVisitor& visitor);

	private:

		typedef std::vector<EntityId>                             EntityIds;
		typedef std::unordered_map<EntityId, sxema2::ObjectId> EntityObjectIds;

		EntityIds       m_entitiesMarkedForDestruction;
		EntityObjectIds m_entityObjectIds;
	};
}