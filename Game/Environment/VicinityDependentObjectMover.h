// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Object that moves based on range to player

-------------------------------------------------------------------------
История:
- 10:11:2012: Created by Dean Claassen

*************************************************************************/

#pragma once

#ifndef _VICINITYDEPENDENTOBJECTMOVER_H_
#define _VICINITYDEPENDENTOBJECTMOVER_H_

#include <drx3D/Act/IGameObject.h>
#include #include <drx3D/Game/State.h>

enum EObjectRangeMoverState
{
	eObjectRangeMoverState_None = 0,
	eObjectRangeMoverState_MovingTo,
	eObjectRangeMoverState_MovingBack,
	eObjectRangeMoverState_Moved,
};

//////////////////////////////////////////////////////////////////////////
/// VICINITYDEPENDENTOBJECTMOVER

class CVicinityDependentObjectMover : public CGameObjectExtensionHelper<CVicinityDependentObjectMover, IGameObjectExtension>
{
public:
	CVicinityDependentObjectMover();
	virtual ~CVicinityDependentObjectMover();

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

private:
	DECLARE_STATE_MACHINE( CVicinityDependentObjectMover, Behavior );

	void Reset( const bool bEnteringGameMode );
	void SetUpdate();
	void ActivateOutputPortBool( tukk szPortName );
	void SetupEntity();
	void DrawSlot( i32k iSlot, const bool bEnable );
	void SetState( const EObjectRangeMoverState state);

	Vec3																m_vOriginalPos;
	Vec3																m_vMoveToPos;
	float																m_fMoveToDistance;
	float																m_fMoveToDistanceSq;
	float																m_fMoveToSpeed;
	float																m_fMoveBackSpeed;
	float																m_fAreaTriggerRange;
	float																m_fAreaTriggerRangeSq;
	float																m_fBackAreaTriggerRange;
	float																m_fBackAreaTriggerRangeSq;
	float																m_fForceMoveCompleteDistanceSq; // Current world pos of entity to player pos distance, if your closer than that, the move finishes immediately to compensate for no physics
	EObjectRangeMoverState							m_currentState;
	bool																m_bUseAreaTrigger;
	bool																m_bDisableAreaTriggerOnMoveComplete;
	bool																m_bForcedReverseMoveToStartPos;
};

#endif //_VICINITYDEPENDENTOBJECTMOVER_H_