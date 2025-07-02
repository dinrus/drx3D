// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIGHTNING_ARC_H_
#define _LIGHTNING_ARC_H_

#pragma once


#include <drx3D/Game/Effects/LightningGameEffect.h>



class CLightningArc : public CGameObjectExtensionHelper<CLightningArc, IGameObjectExtension>
{
public:
	CLightningArc();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	virtual bool Init(IGameObject* pGameObject);
	virtual void PostInit( IGameObject * pGameObject );
	virtual void InitClient(i32 channelId);
	virtual void PostInitClient(i32 channelId);
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags );
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize ser );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext& ctx, i32 updateSlot );
	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void ProcessEvent( SEntityEvent& event );	
	virtual void SetChannelId(u16 id);
	virtual void SetAuthority( bool auth );
	virtual ukk  GetRMIBase() const;
	virtual void PostUpdate( float frameTime );
	virtual void PostRemoteSpawn();

	void TriggerSpark();
	void Enable(bool enable);
	void ReadLuaParameters();

private:
	void Reset(bool jumpingIntoGame);

	tukk m_lightningPreset;
	float m_delay;
	float m_delayVariation;
	float m_timer;
	bool m_enabled;
	bool m_inGameMode;
};



#endif
