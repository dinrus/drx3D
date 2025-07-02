// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Object for ledge placement in the levels

-------------------------------------------------------------------------
История:
- 25:02:2012   Created by Benito Gangoso Rodriguez

*************************************************************************/
#pragma once

#ifndef _LEDGE_H_
#define _LEDGE_H_

#include <drx3D/Act/IGameObject.h>

class CLedgeObject : public CGameObjectExtensionHelper<CLedgeObject, IGameObjectExtension>
{
	struct LedgeProperties
	{
		LedgeProperties(const IEntity& entity);

		float ledgeCornerMaxAngle;
		float ledgeCornerEndAdjustAmount;
		bool ledgeFlipped;
		u16 ledgeFlags_MainSide;
		u16 ledgeFlags_OppositeSide;


	private:
		LedgeProperties();
	};

public:

	CLedgeObject();
	virtual ~CLedgeObject();

	// IGameObjectExtension
	virtual bool Init( IGameObject * pGameObject );
	virtual void InitClient( i32 channelId ) {};
	virtual void PostInit( IGameObject * pGameObject );
	virtual void PostInitClient( i32 channelId ) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser ) {};
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) { return false; };
	virtual void PostSerialize() {};
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext& ctx, i32 slot ) { };
	virtual void HandleEvent( const SGameObjectEvent& gameObjectEvent );
	virtual void ProcessEvent( SEntityEvent& entityEvent );
	virtual void SetChannelId( u16 id ) {};
	virtual void SetAuthority( bool auth ) {};
	virtual void PostUpdate( float frameTime ) { DRX_ASSERT(false); }
	virtual void PostRemoteSpawn() {};
	virtual void GetMemoryUsage( IDrxSizer *pSizer ) const;
	// ~IGameObjectExtension

protected:

	virtual bool IsStatic() const
	{
		return false;
	}

private:

	void UpdateLocation();
	void ComputeLedgeMarkers();
	
	ILINE bool IsFlipped() const { return m_flipped; };

	bool m_flipped;
};

//////////////////////////////////////////////////////////////////////////

class CLedgeObjectStatic : public CLedgeObject
{
public:

	CLedgeObjectStatic();
	virtual ~CLedgeObjectStatic();

protected:

	virtual bool IsStatic() const
	{
		return true;
	}

};

#endif //_LEDGE_H_