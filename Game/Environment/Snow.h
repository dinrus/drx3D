// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Snow entity

-------------------------------------------------------------------------
История:
- 12:12:2012: Created by Stephen Clement

*************************************************************************/
#ifndef __SNOW_H__
#define __SNOW_H__
#pragma once

#include <drx3D/Act/IGameObject.h>

class CSnow : public CGameObjectExtensionHelper<CSnow, IGameObjectExtension>
{
public:
	CSnow();
	virtual ~CSnow();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId) {};
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(i32 channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags) { return true; }
	virtual void FullSerialize(TSerialize ser);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext &ctx, i32 updateSlot);
	virtual void PostUpdate(float frameTime ) {};
	virtual void PostRemoteSpawn() {};
	virtual void HandleEvent( const SGameObjectEvent &);
	virtual void ProcessEvent(SEntityEvent &);
	virtual void SetChannelId(u16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const { pSizer->Add(*this); }
	
	//~IGameObjectExtension

	bool Reset();

protected:
	
	bool	m_bEnabled;
	float	m_fRadius;

	// Surface params.
	float	m_fSnowAmount;
	float	m_fFrostAmount;
	float	m_fSurfaceFreezing;
	
	// Snowfall params.
	i32		m_nSnowFlakeCount;
	float	m_fSnowFlakeSize;
	float	m_fSnowFallBrightness;
	float	m_fSnowFallGravityScale;
	float	m_fSnowFallWindScale;
	float	m_fSnowFallTurbulence;
	float	m_fSnowFallTurbulenceFreq;

private:
	CSnow(const CSnow&);
	CSnow& operator = (const CSnow&);
};

#endif //__RAIN_H__
