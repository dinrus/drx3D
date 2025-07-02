// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Flash animated door panel

-------------------------------------------------------------------------
История:
- 03:04:2012: Created by Dean Claassen

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/DoorPanel.h>

#include #include <drx3D/Game/AutoAimUpr.h>
#include #include <drx3D/Game/TacticalUpr.h>
#include <drx3D/Game/EntityUtility/EntityScriptCalls.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>
#include <GameObjects/GameObject.h>

namespace DP
{
	void RegisterEvents( IGameObjectExtension& goExt, IGameObject& gameObject )
	{
		gameObject.UnRegisterExtForEvents( &goExt, NULL, 0 );
		i32k iScriptEventID = eGFE_ScriptEvent;
		gameObject.RegisterExtForEvents( &goExt, &iScriptEventID, 1 );
		i32k iStartSharingScreenEventID = eDoorPanelGameObjectEvent_StartShareScreen;
		gameObject.RegisterExtForEvents( &goExt, &iStartSharingScreenEventID, 1 );
		i32k iStopSharingScreenEventID = eDoorPanelGameObjectEvent_StopShareScreen;
		gameObject.RegisterExtForEvents( &goExt, &iStopSharingScreenEventID, 1 );
	}
}


namespace DoorPanelBehaviorStateNames
{
	tukk g_doorPanelStateNames[ eDoorPanelBehaviorState_Count ] =
	{
		"Idle",
		"Scanning",
		"ScanSuccess",
		"ScanComplete",
		"ScanFailure",
		"Destroyed",
	};

	tukk* GetNames()
	{
		return &g_doorPanelStateNames[ 0 ];
	}

	EDoorPanelBehaviorState FindId( tukk const szName )
	{
		for ( size_t i = 0; i < eDoorPanelBehaviorState_Count; ++i )
		{
			tukk const stateName = g_doorPanelStateNames[ i ];
			if ( strcmp( stateName, szName ) == 0 )
			{
				return static_cast< EDoorPanelBehaviorState >( i );
			}
		}

		GameWarning("DoorPanelBehaviorStateNames:FindId: Failed to find door panel name: %s", szName);
		return eDoorPanelBehaviorState_Invalid;
	}

	tukk GetNameFromId( const EDoorPanelBehaviorState stateId )
	{
		if (stateId >= 0 && stateId < eDoorPanelBehaviorState_Count)
		{
			return g_doorPanelStateNames[stateId];
		}

		GameWarning("DoorPanelBehaviorStateNames:GetNameFromId: Failed to get name for id: %d", stateId);
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

DEFINE_STATE_MACHINE( CDoorPanel, Behavior );

CDoorPanel::CDoorPanel()
:	m_fLastVisibleDistanceCheckTime(0.0f)
, m_fVisibleDistanceSq(0.0f)
, m_fDelayedStateEventTimeDelay(-1.0f)
, m_currentState(eDoorPanelBehaviorState_Invalid)
, m_delayedState(eDoorPanelBehaviorState_Invalid)
, m_sharingMaterialEntity(0)
, m_bHasDelayedStateEvent(false)
, m_bInTacticalUpr(false)
, m_bFlashVisible(false)
{

}

CDoorPanel::~CDoorPanel()
{
	StateMachineReleaseBehavior();
}

bool CDoorPanel::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);

	return true;
}

void CDoorPanel::PostInit( IGameObject * pGameObject )
{
	DP::RegisterEvents( *this, *pGameObject );

	StateMachineInitBehavior();

	GetGameObject()->EnableUpdateSlot( this, DOOR_PANEL_MODEL_NORMAL_SLOT );

	Reset( !gEnv->IsEditor() );

	SetStateById( GetInitialBehaviorStateId() );
}

bool CDoorPanel::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();
	DP::RegisterEvents( *this, *pGameObject );

	DRX_ASSERT_MESSAGE(false, "CDoorPanel::ReloadExtension not implemented");

	return false;
}

bool CDoorPanel::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CDoorPanel::GetEntityPoolSignature not implemented");

	return true;
}

void CDoorPanel::Release()
{
	delete this;
}

void CDoorPanel::FullSerialize( TSerialize serializer )
{
	if (serializer.IsReading())
	{
		m_fLastVisibleDistanceCheckTime = 0.0f;
		i32 iCurrentState = (i32)eDoorPanelBehaviorState_Idle;
		serializer.Value( "CurrentState", iCurrentState );
		const EDoorPanelBehaviorState stateId = (EDoorPanelBehaviorState)iCurrentState;
		if (stateId != eDoorPanelBehaviorState_Invalid)
		{
			m_currentState = stateId;
		}
	}
	else
	{
		i32 iCurrentState = (i32)m_currentState;
		serializer.Value( "CurrentState", iCurrentState );
	}
	
	StateMachineSerializeBehavior( SStateEventSerialize(serializer) );
}

void CDoorPanel::Update( SEntityUpdateContext& ctx, i32 slot )
{
	if (m_bHasDelayedStateEvent)
	{
		if (m_fDelayedStateEventTimeDelay > FLT_EPSILON)
		{
			m_fDelayedStateEventTimeDelay -= ctx.fFrameTime;
		}
		else
		{
			m_fDelayedStateEventTimeDelay = -1.0f;
			m_bHasDelayedStateEvent = false;
			const EDoorPanelBehaviorState delayedState = m_delayedState;
			m_delayedState = eDoorPanelBehaviorState_Invalid;
			SetStateById(delayedState);
		}		
	}

	// Check visible distance
	if (m_fVisibleDistanceSq > 0.0f)
	{
		const float fCurTime = ctx.fCurrTime;
		if ((fCurTime - m_fLastVisibleDistanceCheckTime) >= GetGameConstCVar(g_flashdoorpanel_distancecheckinterval))
		{
			m_fLastVisibleDistanceCheckTime = fCurTime;
			const Vec3& clientPos = CHUDUtils::GetClientPos();
			const float fDistSqFromClientToObject = clientPos.GetSquaredDistance(GetEntity()->GetWorldPos());
			const bool bVisible = fDistSqFromClientToObject <= m_fVisibleDistanceSq;
			if (bVisible != m_bFlashVisible)
			{
				m_bFlashVisible = bVisible;
				SDoorPanelVisibleEvent visibleEvent( bVisible );
				StateMachineHandleEventBehavior( visibleEvent );
			}
		}
	}
}

void CDoorPanel::HandleEvent( const SGameObjectEvent& gameObjectEvent )
{
	u32k eventId = gameObjectEvent.event;
	uk pParam = gameObjectEvent.param;
	if ((eventId == eGFE_ScriptEvent) && (pParam != NULL))
	{
		tukk szEventName = static_cast<tukk >(pParam);

		const EDoorPanelBehaviorState stateId = DoorPanelBehaviorStateNames::FindId( szEventName );
		if ( stateId != eDoorPanelBehaviorState_Invalid )
		{
			SetStateById( stateId );
		}
		else
		{
			GameWarning("CDoorPanel::HandleEvent: Failed to handle event: %s", szEventName);
		}
	}
	else if ((eventId == 	eDoorPanelGameObjectEvent_StartShareScreen) && (pParam != NULL))
	{
		const EntityId entityToShareScreen = *((EntityId*)(pParam));
		DRX_ASSERT(entityToShareScreen != 0);
		DRX_ASSERT(m_sharingMaterialEntity == 0);
		if (m_sharingMaterialEntity == 0 && entityToShareScreen != 0)
		{
			IEntity* pEntity = GetEntity();
			if (pEntity && !pEntity->IsHidden())
			{
				// Force flash deinit and init (Much cleaner to do this instead of delaying till after entity links are set)
				StateMachineHandleEventBehavior( SDoorPanelVisibleEvent(false) );
				m_sharingMaterialEntity = entityToShareScreen; // Can't set till after releasing to ensure it doesnt think its already shared
				m_bFlashVisible = true;
				StateMachineHandleEventBehavior( SDoorPanelVisibleEvent(true) );
			}
			else
			{
				m_sharingMaterialEntity = entityToShareScreen;
			}
		}
		else
		{
			GameWarning("CDoorPanel::ProcessEvent: Not valid to have multiple ShareScreen entity links or invalid entity id");
		}
	}
	else if ((eventId == 	eDoorPanelGameObjectEvent_StopShareScreen) && (pParam != NULL))
	{
		if (m_sharingMaterialEntity != 0)
		{
			const EntityId entityToShareScreen = *((EntityId*)(pParam));
			DRX_ASSERT(entityToShareScreen == m_sharingMaterialEntity);
			if (entityToShareScreen == m_sharingMaterialEntity)
			{
				StateMachineHandleEventBehavior( SDoorPanelVisibleEvent(false) );
				m_sharingMaterialEntity = 0;
				m_bFlashVisible = false;
			}
		}
	}
}

void CDoorPanel::ProcessEvent( SEntityEvent& entityEvent )
{
	switch(entityEvent.event)
	{
	case ENTITY_EVENT_RESET:
		{
			const bool bEnteringGameMode = ( entityEvent.nParam[ 0 ] == 1 );
			Reset( bEnteringGameMode );
		}
		break;
	case ENTITY_EVENT_UNHIDE:
		{
			GetGameObject()->EnableUpdateSlot( this, DOOR_PANEL_MODEL_NORMAL_SLOT );
			if (m_fVisibleDistanceSq < 0.0f)
			{
				m_bFlashVisible = true;
				SDoorPanelVisibleEvent visibleEvent( true );
				StateMachineHandleEventBehavior( visibleEvent );
			}
		}
		break;
	case ENTITY_EVENT_HIDE:
		{
			GetGameObject()->DisableUpdateSlot( this, DOOR_PANEL_MODEL_NORMAL_SLOT );
			m_bFlashVisible = false;
			SDoorPanelVisibleEvent visibleEvent( false );
			StateMachineHandleEventBehavior( visibleEvent );
		}
		break;
	case ENTITY_EVENT_LINK:
		{
			IEntityLink* pLink = (IEntityLink*)entityEvent.nParam[ 0 ];
			DRX_ASSERT(pLink != NULL);
			if (pLink && (stricmp(pLink->name, "ShareScreen") == 0))
			{
				const EntityId entityToShareTo = pLink->entityId;
				stl::push_back_unique( m_screenSharingEntities, entityToShareTo );
			}
		}
		break;
	case ENTITY_EVENT_DELINK:
		{
			IEntityLink* pLink = (IEntityLink*)entityEvent.nParam[ 0 ];
			DRX_ASSERT(pLink != NULL);
			if (pLink && (stricmp(pLink->name, "ShareScreen") == 0))
			{
				const EntityId entityToShareTo = pLink->entityId;
				stl::find_and_erase(m_screenSharingEntities, entityToShareTo);
			}
		}
		break;
	}
}

void CDoorPanel::GetMemoryUsage( IDrxSizer *pSizer ) const
{

}

void CDoorPanel::HandleFSCommand(tukk pCommand, tukk pArgs, uk pUserData)
{
	/* Nolonger using fs commands to handle state change due to needing to use pDynTexSrc->EnablePerFrameRendering(true) to prevent state change not being processed when not being rendered
		 using m_fDelayedStateEventTimeDelay instead
	if( strcmp(pCommand, "ScanSuccessDone") == 0)
	{
		SetDelayedStateChange(eDoorPanelBehaviorState_ScanComplete);
	}
	else if( strcmp(pCommand, "ScanFailureDone") == 0)
	{
		SetDelayedStateChange(eDoorPanelBehaviorState_Idle);
	}
	*/
}

EDoorPanelBehaviorState CDoorPanel::GetInitialBehaviorStateId() const
{
	EDoorPanelBehaviorState doorPanelState = eDoorPanelBehaviorState_Invalid;

	IEntity* pEntity = GetEntity();

	tuk szDoorPanelStateName = NULL;
	bool bResult = EntityScripts::GetEntityProperty(GetEntity(), "esDoorPanelState", szDoorPanelStateName);
	if (bResult)
	{
		doorPanelState = DoorPanelBehaviorStateNames::FindId( szDoorPanelStateName );
	}

	if ( doorPanelState != eDoorPanelBehaviorState_Invalid )
	{
		return doorPanelState;
	}

	return eDoorPanelBehaviorState_Idle;
}

void CDoorPanel::SetStateById( const EDoorPanelBehaviorState stateId )
{
	SDoorPanelStateEventForceState stateEventForceState( stateId );
	StateMachineHandleEventBehavior( stateEventForceState );
}

void CDoorPanel::NotifyBehaviorStateEnter( const EDoorPanelBehaviorState stateId )
{
	m_currentState = stateId;

	if ( stateId != eDoorPanelBehaviorState_Invalid ) // Notify state change to script
	{
		IEntity* pEntity = GetEntity();
		if (pEntity)
		{
			AssignAsFSCommandHandler(); // Need this on state change to ensure if a door panel which has a shared screen handles it has command focus

			tukk szDoorPanelStateName = DoorPanelBehaviorStateNames::GetNameFromId( stateId );
			if (szDoorPanelStateName != NULL && szDoorPanelStateName[0] != '\0')
			{
				EntityScripts::CallScriptFunction(pEntity, pEntity->GetScriptTable(), "OnStateChange", szDoorPanelStateName);
			}
		}
	}
}

void CDoorPanel::SetDelayedStateChange(const EDoorPanelBehaviorState stateId, const float fTimeDelay)
{
	DRX_ASSERT(m_bHasDelayedStateEvent == false);
	if (!m_bHasDelayedStateEvent)
	{
		m_bHasDelayedStateEvent = true;
		m_fDelayedStateEventTimeDelay = fTimeDelay;
		m_delayedState = stateId;
	}
	else
	{
		GameWarning("CDoorPanel::AddDelayedStateEvent: Already have a delayed state event, this shouldn't happen");
	}
}

void CDoorPanel::AssignAsFSCommandHandler()
{
	IEntity* pEntity = GetEntity();
	if (pEntity)
	{
		IEntityRenderProxy* pRenderProxy = static_cast<IEntityRenderProxy*>(pEntity->GetProxy(ENTITY_PROXY_RENDER));
		if (pRenderProxy)
		{
			_smart_ptr<IMaterial> pMaterial = pRenderProxy->GetRenderMaterial(DOOR_PANEL_MODEL_NORMAL_SLOT);
			IFlashPlayer* pFlashPlayer = CHUDUtils::GetFlashPlayerFromMaterialIncludingSubMaterials(pMaterial, true);
			if (pFlashPlayer) // Valid to not have a flash player, since will update when flash setup
			{
				pFlashPlayer->SetFSCommandHandler(this);
				pFlashPlayer->Release();
			}
		}
	}
}

void CDoorPanel::NotifyScreenSharingEvent(const EDoorPanelGameObjectEvent event)
{
	if (m_screenSharingEntities.size() > 0)
	{
		const EntityId thisEntityId = GetEntityId();
		for (size_t i = 0; i < m_screenSharingEntities.size(); i++)
		{
			const EntityId entityId = m_screenSharingEntities[i];
			IEntity* pEntity = gEnv->pEntitySystem->GetEntity( entityId );
			if (pEntity)
			{
				CGameObject* pGameObject = static_cast<CGameObject*>(pEntity->GetProxy(ENTITY_PROXY_USER));
				if (pGameObject != NULL)
				{
					SGameObjectEvent goEvent( (u32)event, eGOEF_ToExtensions, IGameObjectSystem::InvalidExtensionID, (uk )(&(thisEntityId)) );
					pGameObject->SendEvent( goEvent );
				}
			}
		}
	}
}

void CDoorPanel::AddToTacticalUpr()
{
	if (m_bInTacticalUpr)
		return;

	CTacticalUpr* pTacticalUpr = g_pGame ? g_pGame->GetTacticalUpr() : NULL;
	if ( pTacticalUpr != NULL )
	{
		pTacticalUpr->AddEntity( GetEntityId(), CTacticalUpr::eTacticalEntity_Item );
		m_bInTacticalUpr = true;;
	}
}


void CDoorPanel::RemoveFromTacticalUpr()
{
	if (!m_bInTacticalUpr)
		return;

	CTacticalUpr* pTacticalUpr = g_pGame ? g_pGame->GetTacticalUpr() : NULL;
	if ( pTacticalUpr != NULL )
	{
		pTacticalUpr->RemoveEntity( GetEntityId(), CTacticalUpr::eTacticalEntity_Item );
		m_bInTacticalUpr = false;
	}
}

void CDoorPanel::Reset( const bool bEnteringGameMode )
{
	m_fLastVisibleDistanceCheckTime = 0.0f;
	m_bHasDelayedStateEvent = false;

	SDoorPanelResetEvent resetEvent( bEnteringGameMode );
	StateMachineHandleEventBehavior( resetEvent );

	SetStateById( GetInitialBehaviorStateId() );
}
