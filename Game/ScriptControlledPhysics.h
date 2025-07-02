// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script Interface for controlling physics.
						 Neccessary when changing physics positions and velocities
						 while keeping visual/physics in sync...

-------------------------------------------------------------------------
История:
- 26:7:2007   17:00 : Created by M�rcio Martins

*************************************************************************/
#ifndef __SCRIPTCONTROLLEDPHYSICS_H__
#define __SCRIPTCONTROLLEDPHYSICS_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Game/IGameObject.h>


class CScriptControlledPhysics: public CGameObjectExtensionHelper<CScriptControlledPhysics, IGameObjectExtension>,
	CScriptableBase
{
public:
	CScriptControlledPhysics();
	virtual ~CScriptControlledPhysics();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;

	// IGameObjectExtension
	virtual bool Init( IGameObject * pGameObject );
	using CScriptableBase::Init;

	virtual void InitClient(i32 channelId) {};
	virtual void PostInit( IGameObject * pGameObject );
	virtual void PostInitClient(i32 channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser );
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext& ctx, i32 slot ) {};
	virtual void HandleEvent( const SGameObjectEvent& );
	virtual void ProcessEvent(SEntityEvent& ) {};
	virtual void SetChannelId(u16 id) {};
	virtual void SetAuthority(bool auth) {}
	virtual void PostUpdate(float frameTime) { assert(false); }
	virtual void PostRemoteSpawn() {};

	//~IGameObjectExtension
	i32 Reset(IFunctionHandler *pH);
	i32 GetSpeed(IFunctionHandler *pH);
	i32 GetAcceleration(IFunctionHandler *pH);

	i32 GetAngularSpeed(IFunctionHandler *pH);
	i32 GetAngularAcceleration(IFunctionHandler *pH);

	i32 MoveTo(IFunctionHandler *pH, Vec3 point, float initialSpeed, float speed, float acceleration, float stopTime);
	i32 RotateTo(IFunctionHandler *pH, Vec3 dir, float roll, float initialSpeed, float speed, float acceleration, float stopSpeed);
	i32 RotateToAngles(IFunctionHandler *pH, Vec3 angles, float initialSpeed, float speed, float acceleration, float stopSpeed);

	i32 HasArrived(IFunctionHandler *pH);

	void OnPostStep(EventPhysPostStep *pPostStep);
private:
	void RegisterGlobals();
	void RegisterMethods();

	bool m_moving;
	Vec3  m_moveTarget;
	Vec3	m_lastVelocity;
	float m_speed;
	float m_maxSpeed;
	float m_acceleration;
	float m_stopTime;

	bool m_rotating;
	Quat m_rotationTarget;
	float m_rotationSpeed;
	float m_rotationMaxSpeed;
	float m_rotationAcceleration;
	float m_rotationStopTime;
};


#endif //__SCRIPTCONTROLLEDPHYSICS_H__
