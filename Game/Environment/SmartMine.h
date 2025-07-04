// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Smart proximity mine

-------------------------------------------------------------------------
История:
- 20:03:2012: Created by Benito G.R.

*************************************************************************/

#pragma once

#ifndef _SMART_MINE_H_
#define _SMART_MINE_H_

#include <drx3D/Act/IGameObject.h>

#include #include <drx3D/Game/State.h>
#include #include <drx3D/Game/EntityUtility/EntityEffects.h>

#define SMART_MINE_MAIN_UPDATE_SLOT   0

// Events sent to mines (i.e. from minefield)
enum EMineGameObjectEvent
{
	eMineGameObjectEvent_RegisterListener = eGFE_Last,
	eMineGameObjectEvent_UnRegisterListener,
	eMineGameObjectEvent_OnNotifyDestroy,
};

// Events sent to the event listeners
enum EMineEventListenerGameObjectEvent
{
	eMineEventListenerGameObjectEvent_Enabled = eGFE_Last,
	eMineEventListenerGameObjectEvent_Disabled,
	eMineEventListenerGameObjectEvent_Destroyed
};

//////////////////////////////////////////////////////////////////////////
/// FSM defines

enum ESmartMineStates
{
	eSmartMineState_Behavior = STATE_FIRST,
};

enum ESmartMineBehaviorEvent
{
	STATE_EVENT_SMARTMINE_ENTITY_ENTERED_AREA = STATE_EVENT_CUSTOM,
	STATE_EVENT_SMARTMINE_ENTITY_LEFT_AREA,
	STATE_EVENT_SMARTMINE_UPDATE,
	STATE_EVENT_SMARTMINE_TRIGGER_DETONATE,
	STATE_EVENT_SMARTMINE_HIDE,
	STATE_EVENT_SMARTMINE_UNHIDE
};

struct SSmartMineEvent_TriggerEntity : public SStateEvent
{
	SSmartMineEvent_TriggerEntity( ESmartMineBehaviorEvent eventType, const EntityId entityId )
		: SStateEvent( eventType )
	{
		AddData( (i32)entityId );
	}

	EntityId GetTriggerEntity() const 
	{
		return static_cast<EntityId>(GetData(0).GetInt());
	}
};

struct SSmartMineEvent_Update : public SStateEvent
{
	SSmartMineEvent_Update( const float frameTime )
		: SStateEvent( STATE_EVENT_SMARTMINE_UPDATE )
	{
		AddData( frameTime );
	}

	float GetFrameTime() const 
	{
		return GetData(0).GetFloat();
	}
};

//////////////////////////////////////////////////////////////////////////
/// Smart Mine

class CSmartMine : public CGameObjectExtensionHelper<CSmartMine, IGameObjectExtension>
{

public:

	typedef DrxFixedArray<EntityId, 4> TrackedEntities; //Up to 4 targets should be more than enough... 

	CSmartMine();
	virtual ~CSmartMine();

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

	void StartTrackingEntity( const EntityId entityId );
	void StopTrackingEntity( const EntityId entityId );

	ILINE bool NeedsToKeepTracking( ) const { return (m_trackedEntities.size() > 0); }
	ILINE u32 GetTrackedEntitiesCount() const { return m_trackedEntities.size(); }
	ILINE EntityId GetTrackedEntityId( u32k idx ) const { DRX_ASSERT( idx < m_trackedEntities.size() ); return m_trackedEntities[idx]; }

	bool IsHostileEntity( const EntityId entityId ) const;
	bool ContinueTrackingEntity( const EntityId entityId ) const;
	bool ShouldStartTrackingEntity( const EntityId entityId ) const;

	void AddToTacticalUpr();
	void RemoveFromTacticalUpr();
	void UpdateTacticalIcon();
	void OnEnabled();
	void OnDisabled();

	void SetFaction( u8k factionId ) { m_factionId = factionId; }

	ILINE EntityEffects::CEffectsController& GetEffectsController() { return m_effectsController; }

	void NotifyMineListenersEvent( const EMineEventListenerGameObjectEvent event );

private:
	typedef std::vector<EntityId> TMineEventListeners;

	void Reset();

	DECLARE_STATE_MACHINE( CSmartMine, Behavior );

	EntityEffects::CEffectsController m_effectsController;

	TrackedEntities	m_trackedEntities; 

	TMineEventListeners	m_mineEventListeners;

	bool  m_enabled;
	bool  m_inTacticalUpr;
	u8 m_factionId;
};

#endif //_SMART_MINE_H_