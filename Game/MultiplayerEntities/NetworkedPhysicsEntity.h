// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Entity that can change it's physicalisation state during a
		game.
  
 -------------------------------------------------------------------------
  История:
  - 19:09:2010: Created by Colin Gulliver

*************************************************************************/

#ifndef __NETWORKEDPHYSICSENTITY_H__
#define __NETWORKEDPHYSICSENTITY_H__

class CNetworkedPhysicsEntity :	public CGameObjectExtensionHelper<CNetworkedPhysicsEntity, IGameObjectExtension, 2>,
																public IGameObjectProfileUpr
{
public:
	enum ePhysicalization
	{
		ePhys_NotPhysicalized,
		ePhys_PhysicalizedRigid,
		ePhys_PhysicalizedStatic,
	};

	CNetworkedPhysicsEntity();
	virtual ~CNetworkedPhysicsEntity();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId) {}
	virtual void PostInit(IGameObject *pGameObject) {}
	virtual void PostInitClient(i32 channelId) {}
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize(TSerialize ser) {};
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext &ctx, i32 updateSlot) {};
	virtual void PostUpdate(float frameTime) {}
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent(const SGameObjectEvent& event) {}
	virtual void ProcessEvent(SEntityEvent& event) {}
	virtual void SetChannelId(u16 id) {}
	virtual void SetAuthority(bool auth);
	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	// ~IGameObjectExtension

	// IGameObjectProfileUpr
	virtual bool SetAspectProfile( EEntityAspects aspect, u8 profile );
	virtual u8 GetDefaultProfile( EEntityAspects aspect );
	// ~IGameObjectProfileUpr

	void Physicalize(ePhysicalization physicsType);

private:
	void ReadPhysicsParams();

	SEntityPhysicalizeParams m_physicsParams;
	ePhysicalization m_physicsType;
	ePhysicalization m_requestedPhysicsType;
};

#endif //__NETWORKEDPHYSICSENTITY_H__
