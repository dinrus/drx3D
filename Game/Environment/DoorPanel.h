// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Flash animated door panel

-------------------------------------------------------------------------
История:
- 03:04:2012: Created by Dean Claassen

*************************************************************************/

#pragma once

#ifndef _DOOR_PANEL_H_
#define _DOOR_PANEL_H_

#include <drx3D/Act/IGameObject.h>
#include #include <drx3D/Game/State.h>
#include <drx3D/Sys/Scaleform/IFlashPlayer.h>

#define DOOR_PANEL_MAIN_UPDATE_SLOT   0

#if defined(USER_dean) && !defined(_RELEASE)
	#define DEBUG_DOOR_PANEL
#endif

#define DOOR_PANEL_MODEL_NORMAL_SLOT					0
#define DOOR_PANEL_MODEL_DESTROYED_SLOT				1

enum EDoorPanelBehaviorState
{
	eDoorPanelBehaviorState_Idle = 0,
	eDoorPanelBehaviorState_Scanning,
	eDoorPanelBehaviorState_ScanSuccess,
	eDoorPanelBehaviorState_ScanComplete,
	eDoorPanelBehaviorState_ScanFailure,
	eDoorPanelBehaviorState_Destroyed,

	eDoorPanelBehaviorState_Count,
	eDoorPanelBehaviorState_Invalid = eDoorPanelBehaviorState_Count,
};

namespace DoorPanelBehaviorStateNames
{
	tukk* GetNames();
	EDoorPanelBehaviorState FindId( tukk const szName );
}

enum EDoorPanelGameObjectEvent
{
	eDoorPanelGameObjectEvent_StartShareScreen = eGFE_Last,
	eDoorPanelGameObjectEvent_StopShareScreen,
};

//////////////////////////////////////////////////////////////////////////
/// FSM defines

enum EDoorPanelStates
{
	eDoorPanelState_Behavior = STATE_FIRST,
};

enum EDoorPanelBehaviorEvent
{
	STATE_EVENT_DOOR_PANEL_FORCE_STATE = STATE_EVENT_CUSTOM,
	STATE_EVENT_DOOR_PANEL_VISIBLE,
	STATE_EVENT_DOOR_PANEL_RESET,
};

struct SDoorPanelStateEventForceState
	: public SStateEvent
{
	SDoorPanelStateEventForceState( EDoorPanelBehaviorState forcedState )
		: SStateEvent( STATE_EVENT_DOOR_PANEL_FORCE_STATE )
	{
		AddData( forcedState );
	}

	EDoorPanelBehaviorState GetForcedState() const { return static_cast< EDoorPanelBehaviorState >( GetData( 0 ).GetInt() ); }
};

struct SDoorPanelVisibleEvent
	: public SStateEvent
{
	SDoorPanelVisibleEvent( const bool bVisible )
		: SStateEvent( STATE_EVENT_DOOR_PANEL_VISIBLE )
	{
		AddData( bVisible );
	}

	bool IsVisible() const { return GetData( 0 ).GetBool(); }
};

struct SDoorPanelResetEvent
	: public SStateEvent
{
	SDoorPanelResetEvent( const bool bEnteringGameMode )
		: SStateEvent( STATE_EVENT_DOOR_PANEL_RESET )
	{
		AddData( bEnteringGameMode );
	}

	bool IsEnteringGameMode() const { return GetData( 0 ).GetBool(); }
};

//////////////////////////////////////////////////////////////////////////
/// DOOR PANEL

class CDoorPanel : public CGameObjectExtensionHelper<CDoorPanel, IGameObjectExtension>, public IFSCommandHandler
{
	friend class CDoorPanelBehavior;

public:
	CDoorPanel();
	virtual ~CDoorPanel();

	// IGameObjectExtension
	virtual bool Init( IGameObject * pGameObject );
	virtual void InitClient( i32 channelId ) {};
	virtual void PostInit( IGameObject * pGameObject );
	virtual void PostInitClient( i32 channelId ) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) { return false; };
	virtual void PostSerialize() {};
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext& ctx, i32 slot );
	virtual void HandleEvent( const SGameObjectEvent& gameObjectEvent );
	virtual void ProcessEvent( SEntityEvent& entityEvent );
	virtual void SetChannelId( u16 id ) {};
	virtual void SetAuthority( bool auth ) {};
	virtual void PostUpdate( float frameTime ) { DRX_ASSERT(false); }
	virtual void PostRemoteSpawn() {};
	virtual void GetMemoryUsage( IDrxSizer *pSizer ) const;
	// ~IGameObjectExtension

	// IFSCommandHandler
	virtual void HandleFSCommand(tukk pCommand, tukk pArgs, uk pUserData = 0);
	// ~IFSCommandHandler

	EDoorPanelBehaviorState GetInitialBehaviorStateId() const;
	void SetStateById( const EDoorPanelBehaviorState stateId );
	ILINE EDoorPanelBehaviorState GetStateId() const { return m_currentState; }
	void NotifyBehaviorStateEnter( const EDoorPanelBehaviorState stateId );

	void AddToTacticalUpr();
	void RemoveFromTacticalUpr();
	void NotifyScreenSharingEvent(const EDoorPanelGameObjectEvent event);

private:
	void Reset( const bool bEnteringGameMode );
	void SetDelayedStateChange(const EDoorPanelBehaviorState stateId, const float fTimeDelay);
	void AssignAsFSCommandHandler();


	DECLARE_STATE_MACHINE( CDoorPanel, Behavior );

	typedef std::vector<EntityId> TScreenSharingEntities;
	TScreenSharingEntities	m_screenSharingEntities; // Entities which are sharing this materials screen

	float										m_fLastVisibleDistanceCheckTime;
	float										m_fVisibleDistanceSq;
	float										m_fDelayedStateEventTimeDelay;
	EDoorPanelBehaviorState m_currentState;
	EDoorPanelBehaviorState m_delayedState;
	EntityId								m_sharingMaterialEntity; // Entity which is sharing to this entity
	bool										m_bHasDelayedStateEvent; // State changes can't be triggered immediately from FS commands since states modify flash data
	bool										m_bInTacticalUpr;
	bool										m_bFlashVisible;
};

#endif //_DOOR_PANEL_H_