// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Rain entity

-------------------------------------------------------------------------
История:
- 27:04:2006: Created by Vaclav Kyba

*************************************************************************/
#ifndef __RAIN_H__
#define __RAIN_H__
#pragma once

#include <drx3D/Act/IGameObject.h>

class CRain : public CGameObjectExtensionHelper<CRain, IGameObjectExtension>
{
public:
	CRain();
	virtual ~CRain();

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

	void PreloadTextures();

	SRainParams	m_params;
	bool				m_bEnabled;

	typedef std::vector<ITexture*> TTextureList;
	TTextureList m_Textures;

private:
	CRain(const CRain&);
	CRain& operator = (const CRain&);
};

#endif //__RAIN_H__
