// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Minefield to handle groups of mines

-------------------------------------------------------------------------
История:
- 07:11:2012: Created by Dean Claassen

*************************************************************************/

#pragma once

#ifndef _MINE_FIELD_H_
#define _MINE_FIELD_H_

#include <drx3D/Act/IGameObject.h>

#include <drx3D/Game/SmartMine.h>

//////////////////////////////////////////////////////////////////////////
/// Mine field

class CMineField : public CGameObjectExtensionHelper<CMineField, IGameObjectExtension>
{
public:
	CMineField();
	virtual ~CMineField();

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
	enum EMineFieldState
	{
		eMineFieldState_Showing = 0,
		eMineFieldState_NotShowing,
	};

	enum EMineState
	{
		eMineState_Enabled		= 0x01,
		eMineState_Destroyed	= 0x02,
	};

	struct SMineData
	{
		SMineData()
		: m_state(eMineState_Enabled)
		, m_entityId(0)
		{
		}

		i32					m_state;
		EntityId		m_entityId;
	};

	typedef std::vector<SMineData> TMinesData;

	void SetState( const EMineFieldState state, const bool bForce = false );
	ILINE EMineFieldState GetState() const { return m_currentState; }
	void AddToTacticalUpr();
	void RemoveFromTacticalUpr();
	void NotifyAllMinesEvent( const EMineGameObjectEvent event );
	void NotifyMineEvent( const EntityId targetEntity, const EMineGameObjectEvent event );
	void Reset( const bool bEnteringGameMode );
	SMineData* GetMineData( const EntityId entityId );
	void UpdateState();

	TMinesData							m_minesData;
	EMineFieldState					m_currentState;
};

#endif //_MINE_FIELD_H_