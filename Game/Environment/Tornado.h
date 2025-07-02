// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Tornado entity

-------------------------------------------------------------------------
История:
- 21:02:2006: Created by Marco Koegler

*************************************************************************/
#ifndef __TORNADO_H__
#define __TORNADO_H__
#pragma once

#include <drx3D/Act/IGameObject.h>

class CFlowTornadoWander;
struct IGroundEffect;

class CTornado : public CGameObjectExtensionHelper<CTornado, IGameObjectExtension>
{
public:
	CTornado();
	virtual ~CTornado();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId) {};
	virtual void PostInit(IGameObject *pGameObject);
	virtual void PostInitClient(i32 channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
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
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	
	//~IGameObjectExtension

	void	SetTarget(IEntity *pTargetEntity, CFlowTornadoWander *pCallback);

	bool Reset();

protected:

	bool	UseFunnelEffect(tukk effectName);
	void	UpdateParticleEmitters();
	void	UpdateTornadoSpline();
	void	UpdateFlow();
	void	OutputDistance();

protected:
	static const NetworkAspectType POSITION_ASPECT = eEA_GameServerStatic;

	IPhysicalEntity*	m_pPhysicalEntity;

	// appearance of tornado
	IParticleEffect*	m_pFunnelEffect;
	IParticleEffect*	m_pCloudConnectEffect;
	IParticleEffect*	m_pTopEffect;
	IGroundEffect*		m_pGroundEffect;
	float							m_cloudHeight;
	bool							m_isOnWater;
	bool							m_isInAir;
	float							m_radius;
	i32								m_curMatID;

	// spline
	Vec3							m_points[4];
	Vec3							m_oldPoints[4];

	// wandering
	Vec3							m_wanderDir;
	float							m_wanderSpeed;
	Vec3							m_currentPos;

	// target
	IEntity*						m_pTargetEntity;	// the tornado will try to reach this entity
	CFlowTornadoWander*	m_pTargetCallback;

	//
	float m_nextEntitiesCheck;

	float m_spinImpulse;
	float m_attractionImpulse;
	float m_upImpulse;

	std::vector<i32> m_spinningEnts;
};

#endif //__TORNADO_H__
