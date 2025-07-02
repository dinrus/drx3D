// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Описание: Container Entity to be placed in the Editor that gets
// automatically registered with the Entity Container Upr
// - 10/02/2016 Created by Dario Sancho

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/EntityContainerObject.h>

#include <drx3D/Act/EntityContainerMgr.h>


bool CEntityContainerObject::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}


void CEntityContainerObject::PostInit(IGameObject* pGameObject) { }


bool CEntityContainerObject::ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params)
{
	ResetGameObject();

	DRX_ASSERT_MESSAGE(false, "CEntityContainerObject::ReloadExtension not implemented");
	return false;
}


bool CEntityContainerObject::GetEntityPoolSignature(TSerialize signature)
{
	DRX_ASSERT_MESSAGE(false, "CEntityContainerObject::GetEntityPoolSignature not implemented");
	return true;
}


void CEntityContainerObject::Release()
{
	delete this;
}


void CEntityContainerObject::ProcessEvent(const SEntityEvent& entityEvent)
{
	switch (entityEvent.event)
	{
	case ENTITY_EVENT_START_GAME:
		{
			// Process links
			const EntityId id = GetEntityId();
			CEntityContainerMgr& containerUpr = CDrxAction::GetDrxAction()->GetEntityContainerMgr();
			containerUpr.CreateContainer(id, GetEntity()->GetName());
			const IEntityLink* pLink = GetEntity()->GetEntityLinks();
			while (pLink)
			{
				containerUpr.AddEntity(id, pLink->entityId);
				pLink = pLink->next;
			}
		}
		break;
	case ENTITY_EVENT_RESET:
		{
			CEntityContainerMgr& containerUpr = CDrxAction::GetDrxAction()->GetEntityContainerMgr();
			containerUpr.DestroyContainer(GetEntityId());
		}
		break;
	case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED:
		{
			ReadEditorProperties();
		}
		break;
	case ENTITY_EVENT_DONE:
		{
			CEntityContainerMgr& containerUpr = CDrxAction::GetDrxAction()->GetEntityContainerMgr();
			containerUpr.DestroyContainer(GetEntityId());
		}
		break;
	default:
		break;
	}
}

uint64 CEntityContainerObject::GetEventMask() const
{
	return ENTITY_EVENT_BIT(ENTITY_EVENT_START_GAME) | ENTITY_EVENT_BIT(ENTITY_EVENT_RESET) | ENTITY_EVENT_BIT(ENTITY_EVENT_EDITOR_PROPERTY_CHANGED) | ENTITY_EVENT_BIT(ENTITY_EVENT_DONE);
}

bool CEntityContainerObject::ReadEditorProperties()
{
	// extend when adding custom properties
	return true;
}
