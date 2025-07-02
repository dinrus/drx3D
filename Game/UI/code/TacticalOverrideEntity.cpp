// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: HUD Tactical override entity
  
 -------------------------------------------------------------------------
  История:
  - 13:12:2012: Created by Dean Claassen

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/TacticalOverrideEntity.h>

#include <drx3D/Game/TacticalUpr.h>

//------------------------------------------------------------------------
CTacticalOverrideEntity::CTacticalOverrideEntity()
: m_bMappedToParent(false)
{
}

//------------------------------------------------------------------------

CTacticalOverrideEntity::~CTacticalOverrideEntity()
{
}

//------------------------------------------------------------------------
bool CTacticalOverrideEntity::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);
	
	return true;
}

//------------------------------------------------------------------------
void CTacticalOverrideEntity::PostInit(IGameObject *pGameObject)
{
}

//------------------------------------------------------------------------
void CTacticalOverrideEntity::InitClient( i32 channelId )
{
}

//------------------------------------------------------------------------
void CTacticalOverrideEntity::Release()
{
	DRX_ASSERT(m_bMappedToParent == false); // Done from Detach event / from reset

	delete this;
}

//------------------------------------------------------------------------
void CTacticalOverrideEntity::GetMemoryUsage(IDrxSizer *pSizer) const
{
	pSizer->Add(*this);
}

//------------------------------------------------------------------------
bool CTacticalOverrideEntity::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();
	DRX_ASSERT(!"CTacticalOverrideEntity::ReloadExtension not implemented");
	return false;
}

//------------------------------------------------------------------------
bool CTacticalOverrideEntity::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT(!"CTacticalOverrideEntity::GetEntityPoolSignature not implemented");
	return true;
}

//------------------------------------------------------------------------
void CTacticalOverrideEntity::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_ATTACH_THIS:
		{
			const EntityId parentId = (EntityId) event.nParam[0];
			g_pGame->GetTacticalUpr()->AddOverrideEntity(parentId, GetEntityId());
			m_bMappedToParent = true;
		}
		break;
	case ENTITY_EVENT_DETACH_THIS:
		{
			if (m_bMappedToParent)
			{
				g_pGame->GetTacticalUpr()->RemoveOverrideEntity(GetEntityId());
				m_bMappedToParent = false;
			}
		}
		break;
	case ENTITY_EVENT_RESET:
		{
			if(gEnv->IsEditor())
			{
				if (event.nParam[0]) // Entering game mode
				{
					if (!m_bMappedToParent) // Could already be added if just linked the entity, won't be if loaded the level with linked entity
					{
						IEntity* pEntity = GetEntity();
						if (pEntity)
						{
							IEntity* pParent = pEntity->GetParent();
							if (pParent)
							{
								g_pGame->GetTacticalUpr()->AddOverrideEntity(pParent->GetId(), GetEntityId());
								m_bMappedToParent = true;
							}
							else
							{
								GameWarning("CTacticalOverrideEntity::ProcessEvent: EntityId: %u, doesn't have an attached parent, this entity is a waste", GetEntityId());
							}
						}
					}
				}
				else // Exiting game mode
				{
					if (m_bMappedToParent)
					{
						g_pGame->GetTacticalUpr()->RemoveOverrideEntity(GetEntityId());
						m_bMappedToParent = false;
					}
				}
			}
		}
		break;
	}
}
