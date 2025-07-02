// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 3:9:2004   11:24 : Created by Márcio Martins

*************************************************************************/
#ifndef __GameContext_H__
#define __GameContext_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Act/ClassRegistryReplicator.h>
#include <drx3D/Network/INetworkService.h>

// FIXME: Cell SDK GCC bug workaround.
#ifndef __GAMEOBJECT_H__
	#include <drx3D/Act/GameObject.h>
#endif

class CScriptRMI;
class CPhysicsSync;
class CGameServerChannel;
class CActionGame;
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
class CVoiceController;
#endif

#if ENABLE_NETDEBUG
class CNetDebug;
#endif

struct SBreakEvent;
class CBreakReplicator;

struct SParsedConnectionInfo
{
	bool                allowConnect;
	bool                isMigrating;
	EDisconnectionCause cause;
	string              gameConnectionString;
	string              errmsg;
	string              playerName;
};

#ifdef USE_NETWORK_STALL_TICKER_THREAD
struct SScopedTickerLock
{
	static DrxLockT<DRXLOCK_RECURSIVE> s_tickerMutex;

	SScopedTickerLock(bool bForceLock) : m_bHasLock(false)
	{
		if (bForceLock)
		{
			s_tickerMutex.Lock();
			m_bHasLock = true;
		}
		else
		{
			m_bHasLock = s_tickerMutex.TryLock();
		}
	}
	~SScopedTickerLock()
	{
		if (m_bHasLock)
		{
			s_tickerMutex.Unlock();
		}
	}

	bool m_bHasLock;
};

	#define SCOPED_TICKER_LOCK     SScopedTickerLock tickerLock(true);
	#define SCOPED_TICKER_TRY_LOCK SScopedTickerLock tickerLock(false);
	#define SCOPED_TICKER_HAS_LOCK tickerLock.m_bHasLock
#else // #if USE_NETWORK_STALL_TICKER_THREAD
	#define SCOPED_TICKER_LOCK     {}
	#define SCOPED_TICKER_TRY_LOCK {}
	#define SCOPED_TICKER_HAS_LOCK true
#endif // #if USE_NETWORK_STALL_TICKER_THREAD

class CDrxAction;
class CGameContext :
	public IGameContext,
	public IGameQuery,
	public IEntitySystemSink,
	public IConsoleVarSink,
	public IHostMigrationEventListener
{
public:
	CGameContext(CDrxAction* pFramework, CScriptRMI*, CActionGame* pGame);
	virtual ~CGameContext();

	void Init(INetContext* pNetContext);

	// IGameContext
	virtual bool                InitGlobalEstablishmentTasks(IContextEstablisher* pEst, i32 establishedToken);
	virtual bool                InitChannelEstablishmentTasks(IContextEstablisher* pEst, INetChannel* pChannel, i32 establishedToken);
	virtual ESynchObjectResult  SynchObject(EntityId id, NetworkAspectType nAspect, u8 currentProfile, TSerialize pSerialize, bool verboseLogging);
	virtual INetSendableHookPtr CreateObjectSpawner(EntityId id, INetChannel* pChannel);
	virtual void                ObjectInitClient(EntityId id, INetChannel* pChannel);
	virtual bool                SendPostSpawnObject(EntityId id, INetChannel* pChannel);
	virtual void                UnboundObject(EntityId id);
	virtual INetAtSyncItem*     HandleRMI(bool bClient, EntityId objID, u8 funcID, TSerialize ser, INetChannel* pChannel);
	virtual void                ControlObject(EntityId id, bool bHaveControl);
	virtual void                PassDemoPlaybackMappedOriginalServerPlayer(EntityId id);
	virtual void                OnStartNetworkFrame();
	virtual void                OnEndNetworkFrame();
	virtual void                ReconfigureGame(INetChannel* pNetChannel);
	virtual CTimeValue          GetPhysicsTime();
	virtual void                BeginUpdateObjects(CTimeValue physTime, INetChannel* pChannel);
	virtual void                EndUpdateObjects();
	virtual void                PlaybackBreakage(i32 breakId, INetBreakagePlaybackPtr pBreakage);
	virtual uk               ReceiveSimpleBreakage(TSerialize ser);
	virtual void                PlaybackSimpleBreakage(uk userData, INetBreakageSimplePlaybackPtr pBreakage);
	virtual void                CompleteUnbind(EntityId id);
	// ~IGameContext

	// IGameQuery
	XmlNodeRef GetGameState();
	// ~IGameQuery

	// IEntitySystemSink
	virtual bool OnBeforeSpawn(SEntitySpawnParams& params);
	virtual void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params);
	virtual bool OnRemove(IEntity* pEntity);
	virtual void OnReused(IEntity* pEntity, SEntitySpawnParams& params);
	// ~IEntitySystemSink

	// IConsoleVarSink
	virtual bool OnBeforeVarChange(ICVar* pVar, tukk sNewValue);
	virtual void OnAfterVarChange(ICVar* pVar);
	virtual void OnVarUnregister(ICVar* pVar) {}
	// ~IConsoleVarSink

	// IHostMigrationEventListener
	virtual EHostMigrationReturn OnInitiate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnDisconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnDemoteToClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnPromoteToServer(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnReconnectClient(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnFinalise(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual EHostMigrationReturn OnTerminate(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	virtual void                 OnComplete(SHostMigrationInfo& hostMigrationInfo) {}
	virtual EHostMigrationReturn OnReset(SHostMigrationInfo& hostMigrationInfo, HMStateType& state);
	// ~IHostMigrationEventListener

	INetContext*          GetNetContext()           { return m_pNetContext; }
	CDrxAction*           GetFramework()            { return m_pFramework; }
	bool                  IsGameStarted() const     { return m_bStarted; }
	bool                  IsInLevelLoad() const     { return m_isInLevelLoad; }
	bool                  IsLoadingSaveGame() const { return m_bIsLoadingSaveGame; };

	SParsedConnectionInfo ParseConnectionInfo(const string& s);

	void                  SetLoadingSaveGame() { m_bIsLoadingSaveGame = true; }

	void                  OnBrokeSomething(const SBreakEvent& be, EventPhysMono* pMono, bool isPlane);
	void                  PhysSimulateExplosion(pe_explosion* pexpl, IPhysicalEntity** pSkipEnts, i32 nSkipEnts, i32 iTypes);

	void                  ResetMap(bool isServer);
	bool                  SetImmersive(bool immersive);

	string                GetLevelName()                      { return m_levelName; }
	string                GetRequestedGameRules()             { return m_gameRules; }

	void                  SetLevelName(tukk levelName) { m_levelName = levelName; }
	void                  SetGameRules(tukk gameRules) { m_gameRules = gameRules; }

	void                  SetPhysicsSync(CPhysicsSync* pSync) { m_pPhysicsSync = pSync; }

	// public so that GameClientChannel can call it
	// -- registers a class in our class name <-> id database
	bool   RegisterClassName(const string& name, u16 id);
	u32 GetClassesHash();
	void   DumpClasses();

	bool   Update();

	bool   ClassNameFromId(string& name, u16 id) const;
	bool   ClassIdFromName(u16& id, const string& name) const;

	void   ChangedScript(EntityId id)
	{
		m_pNetContext->ChangedAspects(id, eEA_Script);
	}

	bool   ChangeContext(bool isServer, const SGameContextParams* pParams);

	void   SetContextInfo(unsigned flags, u16 port, tukk connectionString);
	bool   HasContextFlag(EGameStartFlags flag) const { return (m_flags & flag) != 0; }
	u16 GetServerPort() const                      { return m_port; }
	string GetConnectionString(DrxFixedStringT<HOST_MIGRATION_MAX_PLAYER_NAME_SIZE>* pNameOverride, bool fake) const;

	void   DefineContextProtocols(IProtocolBuilder* pBuilder, bool server);

	void        AddControlObjectCallback(EntityId id, bool willHaveControl, HSCRIPTFUNCTION func);

	static void RegisterExtensions(IGameFramework* pFW);

	void        AllowCallOnClientConnect();

	void        GetMemoryUsage(IDrxSizer* pSizer) const;
	void        GetMemoryStatistics(IDrxSizer* pSizer) { GetMemoryUsage(pSizer); /*dummy till network module is updated*/ }

#if ENABLE_NETDEBUG
	CNetDebug* GetNetDebug() { return m_pNetDebug; }
#endif

private:
	static IEntity* GetEntity(i32 i, uk p)
	{
		if (i == PHYS_FOREIGN_ID_ENTITY)
			return (IEntity*)p;
		return NULL;
	}

	IGameRules* GetGameRules();

	void        BeginChangeContext(const string& levelName);
	void        CallOnClientEnteredGame(CGameServerChannel*);
	void        CallOnSpawnComplete(IEntity* pEntity);
	bool        HasSpawnPoint();

	void        AddLoadLevelTasks(IContextEstablisher* pEst, bool isServer, i32 flags, bool** ppLoadingStarted, i32 establishedToken, bool bChannelIsMigrating);
	void        AddLoadingCompleteTasks(IContextEstablisher* pEst, i32 flags, bool* pLoadingStarted, bool bChannelIsMigrating);
	std::set<INetChannel*> m_resetChannels;

	enum EEstablishmentFlags
	{
		eEF_LevelLoaded  = 1 << 0,
		eEF_LoadNewLevel = 1 << 1,
		eEF_DemoPlayback = 1 << 2
	};

	static i32k EstablishmentFlags_InitialLoad = eEF_LoadNewLevel;
	static i32k EstablishmentFlags_LoadNextLevel = eEF_LoadNewLevel | eEF_LevelLoaded;
	static i32k EstablishmentFlags_ResetMap = eEF_LevelLoaded;

	i32              m_loadFlags;

	static void OnCollision(const EventPhys* pEvent, uk );

	// singleton
	static CGameContext* s_pGameContext;

	IEntitySystem*       m_pEntitySystem;
	IPhysicalWorld*      m_pPhysicalWorld;
	IActorSystem*        m_pActorSystem;
	INetContext*         m_pNetContext;
	CDrxAction*          m_pFramework;
	CActionGame*         m_pGame;
	CScriptRMI*          m_pScriptRMI;
	CPhysicsSync*        m_pPhysicsSync;
#ifndef OLD_VOICE_SYSTEM_DEPRECATED
	CVoiceController*    m_pVoiceController;
#endif
#if ENABLE_NETDEBUG
	CNetDebug* m_pNetDebug;
#endif

	CClassRegistryReplicator     m_classRegistry;

	// context parameters
	string   m_levelName;
	string   m_gameRules;

	unsigned m_flags;
	u16   m_port;
	string   m_connectionString;
	bool     m_isInLevelLoad;
	bool     m_bStarted;
	bool     m_bIsLoadingSaveGame;
	bool     m_bHasSpawnPoint;
	bool     m_bAllowSendClientConnect;
	i32      m_resourceLocks;
	i32      m_removeObjectUnlocks;
	i32      m_broadcastActionEventInGame;
	friend class CScopedRemoveObjectUnlock;

	struct SDelegateCallbackIndex
	{
		EntityId id;
		bool     bHaveControl;
		bool operator<(const SDelegateCallbackIndex& rhs) const
		{
			return bHaveControl < rhs.bHaveControl || (id < rhs.id);
		}
		bool operator==(const SDelegateCallbackIndex& rhs) const
		{
			return (bHaveControl == rhs.bHaveControl) && (id == rhs.id);
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
	};
	typedef std::multimap<SDelegateCallbackIndex, HSCRIPTFUNCTION> DelegateCallbacks;
	DelegateCallbacks                    m_delegateCallbacks;

	std::unique_ptr<IContextEstablisher> m_pContextEstablisher;
	std::unique_ptr<CBreakReplicator>    m_pBreakReplicator;
};

class CScopedRemoveObjectUnlock
{
public:
	CScopedRemoveObjectUnlock(CGameContext* pCtx) : m_pCtx(pCtx) { pCtx->m_removeObjectUnlocks++; }
	~CScopedRemoveObjectUnlock() { m_pCtx->m_removeObjectUnlocks--; }

private:
	CScopedRemoveObjectUnlock(const CScopedRemoveObjectUnlock&);
	CScopedRemoveObjectUnlock& operator=(const CScopedRemoveObjectUnlock&);

	CGameContext* m_pCtx;
};

#endif //__GameContext_H__
