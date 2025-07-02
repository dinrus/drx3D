// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "EntityObjectWithComponent.h"

REGISTER_CLASS_DESC(CEntityWithComponentClassDesc);
IMPLEMENT_DYNCREATE(CEntityObjectWithComponent, CEntityObject)

bool CEntityObjectWithComponent::Init(CBaseObject* prev, const string& file)
{
	if (file.size() > 0)
	{
		m_componentGUID = DrxGUID::FromString(file);
	}

	// Utilize the Default entity class
	return CEntityObject::Init(prev, "Entity");
}

bool CEntityObjectWithComponent::CreateGameObject()
{
	if (!CEntityObject::CreateGameObject())
	{
		return false;
	}

	if (!m_componentGUID.IsNull() && m_pEntity->QueryComponentByInterfaceID(m_componentGUID) == nullptr)
	{
		if (IEntityComponent* pComponent = m_pEntity->CreateComponentByInterfaceID(m_componentGUID, false))
		{
			pComponent->GetComponentFlags().Add(EEntityComponentFlags::UserAdded);

			// Refresh inspector to show new component
			GetIEditorImpl()->GetObjectManager()->EmitPopulateInspectorEvent();
		}
	}

	return true;
}
