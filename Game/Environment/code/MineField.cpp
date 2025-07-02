// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Minefield to handle groups of mines

-------------------------------------------------------------------------
История:
- 07:11:2012: Created by Dean Claassen

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/MineField.h>

#include #include <drx3D/Game/TacticalUpr.h>
#include <drx3D/Game/EntityUtility/EntityScriptCalls.h>
#include <GameObjects/GameObject.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>

namespace MF
{
	void RegisterEvents( IGameObjectExtension& goExt, IGameObject& gameObject )
	{
		i32k events[] = {	eGFE_ScriptEvent,

														eMineEventListenerGameObjectEvent_Enabled, 
														eMineEventListenerGameObjectEvent_Disabled, 
														eMineEventListenerGameObjectEvent_Destroyed};

		gameObject.UnRegisterExtForEvents( &goExt, NULL, 0 );
		gameObject.RegisterExtForEvents( &goExt, events, (sizeof(events) / sizeof(i32)) );
	}
}

//////////////////////////////////////////////////////////////////////////

CMineField::CMineField()
:	m_currentState(eMineFieldState_NotShowing)
{

}

CMineField::~CMineField()
{
	NotifyAllMinesEvent( eMineGameObjectEvent_UnRegisterListener );
	m_minesData.clear();

	RemoveFromTacticalUpr();
}

bool CMineField::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);

	return true;
}

void CMineField::PostInit( IGameObject * pGameObject )
{
	MF::RegisterEvents( *this, *pGameObject );

	Reset( !gEnv->IsEditor() );
}

bool CMineField::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();
	MF::RegisterEvents( *this, *pGameObject );

	DRX_ASSERT_MESSAGE(false, "CMineField::ReloadExtension not implemented");

	return false;
}

bool CMineField::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CMineField::GetEntityPoolSignature not implemented");

	return true;
}

void CMineField::Release()
{
	delete this;
}

void CMineField::FullSerialize( TSerialize serializer )
{
	serializer.Value( "CurrentState", (i32&)m_currentState );
	u32 iNumMinesDataData = m_minesData.size();
	serializer.Value( "iNumMinesDataData", iNumMinesDataData );

	if (serializer.IsReading())
	{
		m_minesData.clear();

		for (size_t i = 0; i < iNumMinesDataData; i++)
		{
			serializer.BeginGroup("MineData");
			SMineData mineData;
			serializer.Value("entityId", (i32&)mineData.m_entityId);
			serializer.Value("state", mineData.m_state);
			m_minesData.push_back(mineData);
			serializer.EndGroup();
		}

		SetState( m_currentState, true );
	}
	else
	{
		TMinesData::iterator iter = m_minesData.begin();
		const TMinesData::const_iterator iterEnd = m_minesData.end();
		while (iter != iterEnd)
		{
			SMineData& mineData = *iter;
			serializer.BeginGroup("MineData");
			serializer.Value("entityId", (i32&)mineData.m_entityId);
			serializer.Value("state", mineData.m_state);
			serializer.EndGroup();

			++iter;
		}
	}
}

void CMineField::Update( SEntityUpdateContext& ctx, i32 slot )
{
}

void CMineField::HandleEvent( const SGameObjectEvent& gameObjectEvent )
{
	u32k eventId = gameObjectEvent.event;
	uk pParam = gameObjectEvent.param;
	if ((eventId == eGFE_ScriptEvent) && (pParam != NULL))
	{
		tukk szEventName = static_cast<tukk >(pParam);
		if (strcmp(szEventName, "OnScanned") == 0)
		{
			// Make all mines scanned and tagged
			TMinesData::iterator iter = m_minesData.begin();
			const TMinesData::const_iterator iterEnd = m_minesData.end();
			while (iter != iterEnd)
			{
				const SMineData& mineData = *iter;
				const EntityId mineEntityId = mineData.m_entityId;
				g_pGame->GetTacticalUpr()->SetEntityScanned(mineEntityId);
				SHUDEvent scannedEvent(eHUDEvent_OnEntityScanned);
				scannedEvent.AddData(SHUDEventData(static_cast<i32>(mineEntityId)));
				CHUDEventDispatcher::CallEvent(scannedEvent);

				g_pGame->GetTacticalUpr()->SetEntityTagged(mineEntityId, true);
				SHUDEvent taggedEvent(eHUDEvent_OnEntityTagged);
				taggedEvent.AddData(SHUDEventData(static_cast<i32>(mineEntityId)));
				CHUDEventDispatcher::CallEvent(taggedEvent);

				if(IEntity* pEntity = gEnv->pEntitySystem->GetEntity(mineEntityId))
				{
					if(IScriptTable *pScriptTable = pEntity->GetScriptTable())
					{
						Script::CallMethod(pScriptTable, "HasBeenScanned");
					}
				}

				++iter;
			}
		}
		else if (strcmp(szEventName, "OnDestroy") == 0)
		{
			NotifyAllMinesEvent( eMineGameObjectEvent_OnNotifyDestroy );
			UpdateState();
		}
	}
	else if ((eventId == 	eMineEventListenerGameObjectEvent_Enabled) && (pParam != NULL))
	{
		const EntityId mineEntity = *((EntityId*)(pParam));
		DRX_ASSERT(mineEntity != 0);
		if (mineEntity != 0)
		{
				SMineData* pMineData = GetMineData( mineEntity );
				if (pMineData)
				{
					pMineData->m_state |= eMineState_Enabled;
					UpdateState();
				}
				else
				{
					GameWarning( "CMineField::HandleEvent: Failed to find mine entity id: %u", mineEntity );
				}
		}
	}
	else if ((eventId == 	eMineEventListenerGameObjectEvent_Disabled) && (pParam != NULL))
	{
		const EntityId mineEntity = *((EntityId*)(pParam));
		DRX_ASSERT(mineEntity != 0);
		if (mineEntity != 0)
		{
			SMineData* pMineData = GetMineData( mineEntity );
			if (pMineData)
			{
				pMineData->m_state &= ~eMineState_Enabled;
				UpdateState();
			}
			else
			{
				GameWarning( "CMineField::HandleEvent: Failed to find mine entity id: %u", mineEntity );
			}
		}
	}
	else if ((eventId == eMineEventListenerGameObjectEvent_Destroyed) && (pParam != NULL))
	{
		const EntityId mineEntity = *((EntityId*)(pParam));
		DRX_ASSERT(mineEntity != 0);
		if (mineEntity != 0)
		{
			SMineData* pMineData = GetMineData( mineEntity );
			if (pMineData)
			{
				pMineData->m_state |= eMineState_Destroyed;
				UpdateState();
			}
			else
			{
				GameWarning( "CMineField::HandleEvent: Failed to find mine entity id: %u", mineEntity );
			}
		}
	}
}

void CMineField::ProcessEvent( SEntityEvent& entityEvent )
{
	switch(entityEvent.event)
	{
	case ENTITY_EVENT_RESET:
		{
			const bool bEnteringGameMode = ( entityEvent.nParam[ 0 ] == 1 );
			Reset( bEnteringGameMode );
		}
		break;
	case ENTITY_EVENT_LEVEL_LOADED:
		{
			// For pure game, Only now are all entity links attached and all those entities initialized
			if ( !gEnv->IsEditor() ) // Performed in reset for editor
			{
				NotifyAllMinesEvent( eMineGameObjectEvent_RegisterListener );
				UpdateState();
			}
		}
		break;
	case ENTITY_EVENT_LINK:
		{
			IEntityLink* pLink = (IEntityLink*)entityEvent.nParam[ 0 ];
			DRX_ASSERT(pLink != NULL);
			if (pLink && (stricmp(pLink->name, "Mine") == 0))
			{
				const EntityId mineEntity = pLink->entityId;
				SMineData* pMineData = GetMineData(mineEntity);
				DRX_ASSERT(pMineData == NULL);
				if (!pMineData)
				{
					SMineData newMineData;
					newMineData.m_entityId = mineEntity;
					m_minesData.push_back(newMineData);
				}
			}
		}
		break;
	case ENTITY_EVENT_DELINK:
		{
			IEntityLink* pLink = (IEntityLink*)entityEvent.nParam[ 0 ];
			DRX_ASSERT(pLink != NULL);
			if (pLink && (stricmp(pLink->name, "Mine") == 0))
			{
				const EntityId mineEntity = pLink->entityId;

				TMinesData::iterator iter = m_minesData.begin();
				const TMinesData::const_iterator iterEnd = m_minesData.end();
				while (iter != iterEnd)
				{
					const SMineData& mineData = *iter;
					if (mineData.m_entityId == mineEntity)
					{
						m_minesData.erase(iter);

						NotifyMineEvent( mineEntity, eMineGameObjectEvent_UnRegisterListener );
						UpdateState();

						break;
					}

					++iter;
				}
			}
		}
		break;
	}
}

void CMineField::GetMemoryUsage( IDrxSizer *pSizer ) const
{

}

void CMineField::SetState( const EMineFieldState state, const bool bForce )
{
	if (state != GetState() || bForce)
	{
		m_currentState = state;

		if (state == eMineFieldState_Showing)
		{
			AddToTacticalUpr();
		}
		else
		{
			RemoveFromTacticalUpr();
		}
	}
}

void CMineField::NotifyAllMinesEvent( const EMineGameObjectEvent event )
{
	TMinesData::iterator iter = m_minesData.begin();
	const TMinesData::const_iterator iterEnd = m_minesData.end();
	while (iter != iterEnd)
	{
		const SMineData& mineData = *iter;
		NotifyMineEvent( mineData.m_entityId, event );

		++iter;
	}
}

void CMineField::NotifyMineEvent( const EntityId targetEntity, const EMineGameObjectEvent event )
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity( targetEntity );
	if (pEntity)
	{
		CGameObject* pGameObject = static_cast<CGameObject*>(pEntity->GetProxy(ENTITY_PROXY_USER));
		if (pGameObject != NULL)
		{
			const EntityId thisEntityId = GetEntityId();
			SGameObjectEvent goEvent( (u32)event, eGOEF_ToExtensions, IGameObjectSystem::InvalidExtensionID, (uk )(&(thisEntityId)) ); // This entity's id is sent as parameter
			pGameObject->SendEvent( goEvent );
		}
	}
}

void CMineField::AddToTacticalUpr()
{
	CTacticalUpr* pTacticalUpr = g_pGame ? g_pGame->GetTacticalUpr() : NULL;
	if ( pTacticalUpr != NULL )
	{
		pTacticalUpr->AddEntity( GetEntityId(), CTacticalUpr::eTacticalEntity_Explosive );
	}
}


void CMineField::RemoveFromTacticalUpr()
{
	CTacticalUpr* pTacticalUpr = g_pGame ? g_pGame->GetTacticalUpr() : NULL;
	if ( pTacticalUpr != NULL )
	{
		pTacticalUpr->RemoveEntity( GetEntityId(), CTacticalUpr::eTacticalEntity_Explosive );
	}
}

void CMineField::Reset( const bool bEnteringGameMode )
{
	if (gEnv->IsEditor()) // Need to clear data otherwise won't register mines as not destroyed if were previously destroyed
	{
		TMinesData::iterator iter = m_minesData.begin();
		const TMinesData::const_iterator iterEnd = m_minesData.end();
		while (iter != iterEnd)
		{
			SMineData& mineData = *iter;
			mineData.m_state = eMineState_Enabled;
			++iter;
		}
	}

	if ( bEnteringGameMode )
	{
		NotifyAllMinesEvent( eMineGameObjectEvent_RegisterListener );
	}

	UpdateState();
}

CMineField::SMineData* CMineField::GetMineData( const EntityId entityId )
{
	SMineData* pFoundData = NULL;

	TMinesData::iterator iter = m_minesData.begin();
	const TMinesData::const_iterator iterEnd = m_minesData.end();
	while (iter != iterEnd)
	{
		SMineData& mineData = *iter;
		if (mineData.m_entityId == entityId)
		{
			pFoundData = &mineData;
			break;
		}

		++iter;
	}

	return pFoundData;
}

void CMineField::UpdateState()
{
	if (gEnv->IsEditor() && !gEnv->IsEditorGameMode()) // Always show in editor
	{
		SetState(eMineFieldState_Showing);
	}
	else
	{
		bool bHasVisibleMine = false;

		TMinesData::iterator iter = m_minesData.begin();
		const TMinesData::const_iterator iterEnd = m_minesData.end();
		while (iter != iterEnd)
		{
			SMineData& mineData = *iter;
			if (mineData.m_state & eMineState_Enabled && !(mineData.m_state & eMineState_Destroyed))
			{
				bHasVisibleMine = true;
				break;
			}

			++iter;
		}

		if (bHasVisibleMine)
		{
			SetState(eMineFieldState_Showing);
		}
		else
		{
			SetState(eMineFieldState_NotShowing);
		}
	}
}
