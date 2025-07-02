// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Weapon System

-------------------------------------------------------------------------
История:
- 18:10:2005   17:41 : Created by M�rcio Martins

*************************************************************************/
#ifndef __WEAPONSYSTEM_H__
#define __WEAPONSYSTEM_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/IWeapon.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/TracerUpr.h>
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/Game/AmmoParams.h>
#include <drx3D/Game/GameParameters.h>
#include <drx3D/Game/ItemPackages.h>
#include <drx3D/Game/WeaponAlias.h>
#include <drx3D/Game/FireModePluginParams.h>
#include <drx3D/Game/GameTypeInfo.h>

class CGame;
class CProjectile;
class CFireMode;
struct ISystem;
class CPlayer;
class CMelee;
class IFireModePlugin;
class CIronSight;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct SProjectileQuery
{
	AABB        box;
	tukk ammoName;
	IEntity     **pResults;
	i32         nCount;
	SProjectileQuery()
	{
		pResults = 0;
		nCount = 0;
		ammoName = 0;
	}
};

struct IProjectileListener
{
	// NB: Must be physics-thread safe IF its called from the physics OnPostStep
	virtual void OnProjectilePhysicsPostStep(CProjectile* pProjectile, EventPhysPostStep* pEvent, i32 bPhysicsThread) {}

	// Called from Main thread
	virtual void OnLaunch(CProjectile* pProjectile, const Vec3& pos, const Vec3& velocity) {}

protected:
	// Should never be called
	virtual ~IProjectileListener() {}
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CDelayedDetonationRMIQueue
{
public:
	CDelayedDetonationRMIQueue() 
		: m_updateTimer(0.f)
		, m_numSentThisFrame(0) {};

	~CDelayedDetonationRMIQueue() { while(!m_dataQueue.empty()) { m_dataQueue.pop(); } };

	void AddToQueue(CPlayer* pPlayer, EntityId projectile);
	void Update(float frameTime);
	void SendRMI(CPlayer* pPlayer, EntityId projectile);

protected:

	struct SRMIData
	{
		SRMIData(EntityId player, EntityId proj) 
			: playerId(player)
			, projectileId(proj) {};

		EntityId playerId;
		EntityId projectileId;
	};

	typedef std::queue<SRMIData> TRMIDataQueue;

	i32		m_numSentThisFrame;
	float m_updateTimer;
	TRMIDataQueue m_dataQueue;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CWeaponSystem : public ILevelSystemListener
{
	struct SAmmoTypeDesc
	{
		SAmmoTypeDesc(): params(0) {};
		void GetMemoryUsage( IDrxSizer *pSizer ) const 
		{
			pSizer->AddObject(params);
		}
		const SAmmoParams *params;
	};

	struct SAmmoPoolDesc
	{
		SAmmoPoolDesc(): size(0) {};
		void GetMemoryUsage( IDrxSizer *pSizer ) const; 		
		std::deque<CProjectile *>	frees;
		u16										size;
	};

	typedef std::map<const CGameTypeInfo*, IFireModePlugin*(*)()>				TFireModePluginCreationRegistry;
	typedef std::map<string, CFireMode*(*)()>														TFireModeCreationRegistry;
	typedef std::map<string, IZoomMode*(*)()>														TZoomModeCreationRegistry;
	typedef std::map<const CGameTypeInfo*, void(*)(IZoomMode*)>					TZoomModeDestructionRegistry;
	typedef std::map<const CGameTypeInfo*, void(*)(CFireMode*)>					TFireModeDestructionRegistry;
	typedef std::map<const CGameTypeInfo*, void (*)(IFireModePlugin*)>	TFireModePluginDestructionRegistry;
	typedef std::map<string, void(*)()>																	TWeaponComponentPoolFreeFunctions;
	typedef std::map<string, IGameObjectExtensionCreatorBase *>					TProjectileRegistry;
	typedef std::map<EntityId, CProjectile *>														TProjectileMap;
	typedef VectorMap<const IEntityClass*, SAmmoTypeDesc>								TAmmoTypeParams;
	typedef std::vector<string>																					TFolderList;
	typedef std::vector<IEntity*>																				TIEntityVector;

	typedef VectorMap<IEntityClass *, SAmmoPoolDesc>							TAmmoPoolMap;	

public:
	CWeaponSystem(CGame *pGame, ISystem *pSystem);
	virtual ~CWeaponSystem();

	struct SLinkedProjectileInfo
	{
		SLinkedProjectileInfo()
		{
			weaponId = 0;
			shotId = 0;
		}

		SLinkedProjectileInfo(EntityId _weapon, u8 _shot)
		{
			weaponId = _weapon;
			shotId = _shot;
		}

		EntityId weaponId;
		u8 shotId;
	};

	typedef VectorMap<EntityId, SLinkedProjectileInfo>	TLinkedProjectileMap;

	void Update(float frameTime);
	void Release();

	void GetMemoryStatistics( IDrxSizer * );

	void Reload();
	void LoadItemParams(IItemSystem* pItemSystem);

	// ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName) {};
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) {}
	virtual void OnLoadingStart(ILevelInfo* pLevel);
	virtual void OnLoadingComplete(ILevelInfo* pLevel);
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error) {};
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) {};
	virtual void OnUnloadComplete(ILevelInfo* pLevel);
	//~ILevelSystemListener

	CFireMode *CreateFireMode(tukk name);
	void DestroyFireMode(CFireMode* pObject);
	
	IZoomMode *CreateZoomMode(tukk name);
	void DestroyZoomMode(CIronSight* pObject);

	CMelee* CreateMeleeMode();
	void DestroyMeleeMode(CMelee* pObject);

	IFireModePlugin* CreateFireModePlugin(const CGameTypeInfo* pluginType);
	void DestroyFireModePlugin(IFireModePlugin* pObject);

	IGameSharedParameters *CreateZoomModeData(tukk name);

	IGameSharedParameters *CreateFireModeData(tukk name);

	CProjectile *SpawnAmmo(IEntityClass* pAmmoType, bool isRemote=false);
	const SAmmoParams* GetAmmoParams( const IEntityClass* pAmmoType );
	bool IsServerSpawn(IEntityClass* pAmmoType) const;
	void RegisterProjectile(tukk name, IGameObjectExtensionCreatorBase *pCreator);
	const SAmmoParams* GetAmmoParams( const IEntityClass* pAmmoType ) const;

	void AddProjectile(IEntity *pEntity, CProjectile *pProjectile);
	void RemoveProjectile(CProjectile *pProjectile);

	void AddLinkedProjectile(EntityId projId, EntityId weaponId, u8 shotId);
	void RemoveLinkedProjectile(EntityId projId);

	const TLinkedProjectileMap& GetLinkedProjectiles() { return m_linkedProjectiles; }

	CProjectile  *GetProjectile(EntityId entityId);
	const IEntityClass* GetProjectileClass( const EntityId entityId ) const;
	i32	QueryProjectiles(SProjectileQuery& q);

	CItemPackages &GetItemPackages() { return m_itemPackages; };
	CTracerUpr &GetTracerUpr() { return m_tracerUpr; };
	const CWeaponAlias &GetWeaponAlias() { return m_weaponAlias; };
	CDelayedDetonationRMIQueue& GetProjectileDelayedDetonationRMIQueue() { return m_detonationRMIQueue; }

	void Scan(tukk folderName);
	bool ScanXML(XmlNodeRef &root, tukk xmlFile);

  static void DebugGun(IConsoleCmdArgs *args = 0);
	static void RefGun(IConsoleCmdArgs *args = 0);

	CProjectile *UseFromPool(IEntityClass *pClass, const SAmmoParams *pAmmoParams);
	bool ReturnToPool(CProjectile *pProjectile);
	void RemoveFromPool(CProjectile *pProjectile);
	void DumpPoolSizes();
	void FreePools();

	void OnResumeAfterHostMigration();

	void AddListener(IProjectileListener* pListener);
	void RemoveListener(IProjectileListener* pListener);

	// Used by projectiles to tell the weapon system where they are
	// Must be physics-thread safe IF its called from the physics OnPostStep
	void OnProjectilePhysicsPostStep(CProjectile* pProjectile, EventPhysPostStep* pEvent, i32 bPhysicsThread);
	void OnLaunch(CProjectile* pProjectile, const Vec3& pos, const Vec3& velocity);

private: 

	template<typename PlugInType>
	void RegisterFireModePlugin();

	template<typename FireModeType>
	void RegisterFireMode(tukk componentName);

	template<typename ZoomType>
	void RegisterZoomMode(tukk componentName);

	void CreatePool(IEntityClass *pClass);
	void FreePool(IEntityClass *pClass);
	u16 GetPoolSize(IEntityClass *pClass);
	
	CProjectile *DoSpawnAmmo(IEntityClass* pAmmoType, bool isRemote, const SAmmoParams *pAmmoParams);

	CGame								*m_pGame;
	ISystem							*m_pSystem;
	IItemSystem					*m_pItemSystem;

	CItemPackages			m_itemPackages;
	CTracerUpr			m_tracerUpr;
	CWeaponAlias				m_weaponAlias;

	TFireModeCreationRegistry						m_fmCreationRegistry;
	TFireModeDestructionRegistry				m_fmDestructionRegistry;
	TZoomModeCreationRegistry						m_zmCreationRegistry;
	TZoomModeDestructionRegistry				m_zmDestructionRegistry;
	TFireModePluginCreationRegistry			m_pluginCreationRegistry;
	TFireModePluginDestructionRegistry	m_pluginDestructionRegistry;
	TWeaponComponentPoolFreeFunctions		m_freePoolHandlers;
	
	std::vector<IProjectileListener*> m_listeners;
	 i32 m_listenersLock;
	
	TProjectileRegistry		m_projectileregistry;
	TAmmoTypeParams			m_ammoparams;
	TProjectileMap				m_projectiles;
	TLinkedProjectileMap	m_linkedProjectiles;

	TAmmoPoolMap				m_pools;

	TFolderList					m_folders;
	bool								m_reloading;
	bool								m_recursing;

	ICVar								*m_pPrecache;
	TIEntityVector			m_queryResults;//for caching queries results
	CDelayedDetonationRMIQueue m_detonationRMIQueue;

};



#endif //__WEAPONSYSTEM_H__
