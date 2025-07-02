// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/EntityComponentsCache.h>
#include <drx3D/Entity/Entity.h>

//////////////////////////////////////////////////////////////////////////
void CEntitiesComponentPropertyCache::StoreEntities()
{
	DRX_PROFILE_FUNCTION(PROFILE_ENTITY);

	IEntityItPtr it = g_pIEntitySystem->GetEntityIterator();
	while (CEntity* pEntity = static_cast<CEntity*>(it->Next()))
	{
		if (!pEntity->IsGarbage())
		{
			StoreEntity(*static_cast<CEntity*>(pEntity));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitiesComponentPropertyCache::RestoreEntities()
{
	DRX_PROFILE_FUNCTION(PROFILE_ENTITY);

	IEntityItPtr it = g_pIEntitySystem->GetEntityIterator();
	while (CEntity* pEntity = static_cast<CEntity*>(it->Next()))
	{
		if (!pEntity->IsGarbage())
		{
			LoadEntity(*static_cast<CEntity*>(pEntity));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitiesComponentPropertyCache::StoreComponent(CEntity& entity, IEntityComponent& component)
{
	DRX_PROFILE_FUNCTION(PROFILE_ENTITY);

	// Only sxema editable and user added components are eligible for storing
	if (!component.GetComponentFlags().Check(EEntityComponentFlags::SchematycEditable) &&
	    !component.GetComponentFlags().Check(EEntityComponentFlags::UserAdded))
	{
		return;
	}

	std::unique_ptr<sxema::CClassProperties> pClassProperties(new sxema::CClassProperties());
	pClassProperties->Read(component.GetClassDesc(), &component);

	SDoubleGUID doubleGuid = { entity.GetGuid(), component.GetGUID() };
	m_componentPropertyCache.insert(std::make_pair(doubleGuid, std::move(pClassProperties)));
}

//////////////////////////////////////////////////////////////////////////
void CEntitiesComponentPropertyCache::LoadComponent(CEntity& entity, IEntityComponent& component)
{
	DRX_PROFILE_FUNCTION(PROFILE_ENTITY);

	// Only sxema editable and user added components are eligible for storing
	if (!component.GetComponentFlags().Check(EEntityComponentFlags::SchematycEditable) &&
	    !component.GetComponentFlags().Check(EEntityComponentFlags::UserAdded))
	{
		return;
	}

	auto iter = m_componentPropertyCache.find({ entity.GetGuid(), component.GetGUID() });
	if (iter != m_componentPropertyCache.end())
	{
		auto& pClassProperties = iter->second;
		bool bMembersChanged = !pClassProperties->Compare(component.GetClassDesc(), &component);
		if (bMembersChanged)
		{
			// Restore members from the previously recorded class properties
			pClassProperties->Apply(component.GetClassDesc(), &component);
			// Notify this component about changed property.
			SEntityEvent entityEvent(ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED);
			component.SendEvent(entityEvent);
		}
	}
}

void CEntitiesComponentPropertyCache::StoreEntity(CEntity& entity)
{
	entity.GetComponentsVector().ForEach([this, &entity](const SEntityComponentRecord& rec) -> EComponentIterationResult
	{
		if (!rec.pComponent->GetClassDesc().GetName().IsEmpty())
		{
			StoreComponent(entity, *rec.pComponent.get());
		}

		return EComponentIterationResult::Continue;
	});
}

void CEntitiesComponentPropertyCache::LoadEntity(CEntity& entity)
{
	entity.GetComponentsVector().ForEach([this, &entity](const SEntityComponentRecord& rec) -> EComponentIterationResult
	{
		if (!rec.pComponent->GetClassDesc().GetName().IsEmpty())
		{
			LoadComponent(entity, *rec.pComponent.get());
		}

		return EComponentIterationResult::Continue;
	});
}

//////////////////////////////////////////////////////////////////////////
void CEntitiesComponentPropertyCache::ClearCache()
{
	DRX_PROFILE_FUNCTION(PROFILE_ENTITY);
	m_componentPropertyCache.clear();
}
