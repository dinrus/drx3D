// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   DrxGame Source File.
   -------------------------------------------------------------------------
   Имя файла:   AIProxy.h
   Version:     v1.00
   Описание:

   -------------------------------------------------------------------------
   История:
   - 7:10:2004   18:54 : Created by Kirill Bulatsev
   - 2 Mar 2009				: Evgeny Adamenkov: Removed parameter of type IRenderer in DebugDraw

 *********************************************************************/

#ifndef __AIProxy_H__
#define __AIProxy_H__

#pragma once

#include <drx3D/AI/IAIActorProxy.h>
#include "IWeapon.h"
#include "IGameObject.h"

struct IAISignalExtraData;
struct IEntity;
class CAIHandler;
class CommunicationHandler;

//! AIProxy listener
struct IAIProxyListener
{
	virtual ~IAIProxyListener() {}
	virtual void OnAIProxyEnabled(bool bEnabled) = 0;
};

struct IFireController
{
	virtual ~IFireController(){}
	virtual bool RequestFire(bool bFire) = 0;
	virtual void UpdateTargetPosAI(const Vec3& pos) = 0;
};

class CAIProxy
	: public IAIActorProxy
	  , public IWeaponEventListener
{
	friend class CAIProxyUpr;

public:
	//------------------  IWeaponEventListener
	virtual void OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType,
	                     const Vec3& pos, const Vec3& dir, const Vec3& vel);
	virtual void OnStartFire(IWeapon* pWeapon, EntityId shooterId)                            {};
	virtual void OnFireModeChanged(IWeapon* pWeapon, i32 currentFireMode)                     {};
	virtual void OnStopFire(IWeapon* pWeapon, EntityId shooterId)                             {};
	virtual void OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {};
	virtual void OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType)   {};
	virtual void OnSetAmmoCount(IWeapon* pWeapon, EntityId shooterId)                         {};
	virtual void OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType)                       {};
	virtual void OnReadyToFire(IWeapon* pWeapon)                                              {};
	virtual void OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed)               {};
	virtual void OnDropped(IWeapon* pWeapon, EntityId actorId);
	virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId);
	virtual void OnStartTargetting(IWeapon* pWeapon) {}
	virtual void OnStopTargetting(IWeapon* pWeapon)  {}
	virtual void OnSelected(IWeapon* pWeapon, bool selected);
	virtual void OnEndBurst(IWeapon* pWeapon, EntityId shooterId);
	//------------------  ~IWeaponEventListener

	//------------------  IAIActorProxy
	virtual i32                      Update(SOBJECTSTATE& state, bool bFullUpdate);
	virtual bool                     CheckUpdateStatus();
	virtual void                     EnableUpdate(bool enable);
	virtual bool                     IsEnabled() const;
	virtual i32                      GetAlertnessState() const;
	virtual void                     SetAlertnessState(i32 alertness);
	virtual bool                     IsCurrentBehaviorExclusive() const;
#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	virtual bool                     SetCharacter(tukk character, tukk behaviour = NULL);
	virtual tukk              GetCharacter();
#endif
	virtual bool                     QueryBodyInfo(SAIBodyInfo& bodyInfo);
	virtual bool                     QueryBodyInfo(const SAIBodyInfoQuery& query, SAIBodyInfo& bodyInfo);
	virtual void                     QueryWeaponInfo(SAIWeaponInfo& weaponInfo);
	virtual EntityId                 GetLinkedDriverEntityId();
	virtual bool                     IsDriver();
	virtual EntityId                 GetLinkedVehicleEntityId();
	virtual bool                     GetLinkedVehicleVisionHelper(Vec3& outHelperPos) const;
	virtual void                     Reset(EObjectResetType type);

	virtual IAICommunicationHandler* GetCommunicationHandler();

	virtual bool                     BecomeAggressiveToAgent(EntityId agentID);

	// This will internally keep a counter to allow stacking of such commands
	virtual void                      SetForcedExecute(bool forced);
	virtual bool                      IsForcedExecute() const;

	virtual void                      Serialize(TSerialize ser);
	virtual IPhysicalEntity*          GetPhysics(bool wantCharacterPhysics = false);
	virtual void                      DebugDraw(i32 iParam = 0);
	virtual void                      GetWorldBoundingRect(Vec3& FL, Vec3& FR, Vec3& BL, Vec3& BR, float extra = 0) const;
	virtual bool                      SetAGInput(EAIAGInput input, tukk value, const bool isUrgent = false);
	virtual bool                      ResetAGInput(EAIAGInput input);
	virtual bool                      IsSignalAnimationPlayed(tukk value);
	virtual bool                      IsActionAnimationStarted(tukk value);
	virtual bool                      IsAnimationBlockingMovement() const;
	virtual EActorTargetPhase         GetActorTargetPhase() const;
	virtual void                      PlayAnimationAction(const struct IAIAction* pAction, i32 goalPipeId);
	virtual void                      AnimationActionDone(bool succeeded);
	virtual bool                      IsPlayingSmartObjectAction() const;
	virtual i32                       GetAndResetShotBulletCount() { i32 ret = m_shotBulletCount; m_shotBulletCount = 0; return ret; } // Only used in firecommand
	virtual void                      EnableWeaponListener(const EntityId weaponId, bool needsSignal);
	virtual void                      UpdateMind(SOBJECTSTATE& state);
	virtual bool                      IsDead() const;
	virtual float                     GetActorHealth() const;
	virtual float                     GetActorMaxHealth() const;
	virtual i32                       GetActorArmor() const;
	virtual i32                       GetActorMaxArmor() const;
	virtual bool                      GetActorIsFallen() const;
	virtual IWeapon*                  QueryCurrentWeapon(EntityId& currentWeaponId);
	virtual IWeapon*                  GetCurrentWeapon(EntityId& currentWeaponId) const;
	virtual const AIWeaponDescriptor& GetCurrentWeaponDescriptor() const;
	virtual IWeapon*                  GetSecWeapon(const ERequestedGrenadeType prefGrenadeType = eRGT_ANY, ERequestedGrenadeType* pReturnedGrenadeType = NULL, EntityId* const pSecondaryWeaponId = NULL) const;
	virtual bool                      GetSecWeaponDescriptor(AIWeaponDescriptor& outDescriptor, ERequestedGrenadeType prefGrenadeType = eRGT_ANY) const;
	virtual void                      SetUseSecondaryVehicleWeapon(bool bUseSecondary);
	virtual bool                      IsUsingSecondaryVehicleWeapon() const { return m_UseSecondaryVehicleWeapon; }
	virtual IEntity*                  GetGrabbedEntity() const;
	virtual bool                      IsUpdateAlways() const                { return m_UpdateAlways; }
	virtual bool                      IfShouldUpdate()                      { return m_pGameObject->ShouldUpdate(); }
	virtual bool                      IsAutoDeactivated() const             { return m_autoDeactivated; };
	virtual void                      NotifyAutoDeactivated()               { m_autoDeactivated = true; };

	virtual tukk               GetVoiceLibraryName(const bool useForcedDefaultName = false) const;
	virtual tukk               GetCommunicationConfigName() const;
	virtual const float               GetFmodCharacterTypeParam() const;
	virtual tukk               GetNavigationTypeName() const;

	virtual bool                      PredictProjectileHit(float vel, Vec3& posOut, Vec3& dirOut, float& speedOut, Vec3* pTrajectoryPositions = 0, u32* trajectorySizeInOut = 0, Vec3* pTrajectoryVelocities = 0);
	virtual bool                      PredictProjectileHit(const Vec3& throwDir, float vel, Vec3& posOut, float& speedOut, ERequestedGrenadeType prefGrenadeType = eRGT_ANY,
	                                                       Vec3* pTrajectoryPositions = 0, u32* trajectorySizeInOut = 0, Vec3* pTrajectoryVelocities = 0);
	virtual void                      GetReadabilityBlockingParams(tukk text, float& time, i32& id);
	virtual tukk               GetCurrentBehaviorName() const;
	virtual tukk               GetPreviousBehaviorName() const;
	virtual void                      UpdateMeAlways(bool doUpdateMeAlways);
	virtual void                      ResendTargetSignalsNextFrame();

	virtual void                      OnActorRemoved();
	//------------------  ~IAIActorProxy

	void        SetMinFireTime(float fTime) { m_fMinFireTime = fTime; }
	CAIHandler* GetAIHandler()              { return m_pAIHandler; }
	void        AddListener(IAIProxyListener* pListener);
	void        RemoveListener(IAIProxyListener* pListener);
	CTimeValue  GetEstimatedAGAnimationLength(EAIAGInput input, tukk value);

	void        SetBehaviour(tukk szBehavior, const IAISignalExtraData* pData = 0);

	void        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	enum AimQueryMode
	{
		QueryAimFromMovementController,
		OverriddenAndAiming,
		OverriddenAndNotAiming,

		AimQueryModeCount,
		FirstAimQueryMode = 0,
	};

	void SetAimQueryMode(const AimQueryMode mode) { m_aimQueryMode = mode; }

protected:
	// (MATT) We want to keep these in sync with the m_aiProxyMap structure, so keep access restricted {2009/04/06}
	CAIProxy(IGameObject* pGameObject);
	virtual ~CAIProxy(void);

	void             OnReused(IEntity* pEntity, SEntitySpawnParams& params);

	void             ReloadScriptProperties();

	void             RemoveWeaponListener(IWeapon* pWeapon);
	IWeapon*         GetWeaponFromId(EntityId entityId);

	IActor*          GetActor() const;
	void             SendSignal(i32 signalID, tukk szText, IEntity* pSender, const IAISignalExtraData* pData, u32 crc = 0u);
	void             UpdateAuxSignal(SOBJECTSTATE& state);
	IFireController* GetFireController(u32 controllerNum = 0);
	void             UpdateShooting(const SOBJECTSTATE& state, bool isAlive);

	bool             IsInCloseMeleeRange(IAIObject* aiObject, IAIActor* aiActor) const;
	void             UpdateCurrentWeapon();

	static float     LinearInterp(float x, float k, float A, float B, float C);
	static float     QuadraticInterp(float x, float k, float A, float B, float C);
	static float     GetTofPointOnParabula(const Vec3& A, const Vec3& V, const Vec3& P, float g);
	static Vec3      GetPointOnParabula(const Vec3& A, const Vec3& V, float t, float g);
	static bool      IntersectSweptSphereWrapper(Vec3* hitPos, float& hitDist, const Lineseg& lineseg, float radius, IPhysicalEntity** pSkipEnts = 0, i32 nSkipEnts = 0, i32 additionalFilter = 0);

	// (MATT) We currently have no way to serialise this. It may be possible to recovery from the entity. Currently must keep between serialisations {2009/04/30}
	IGameObject* m_pGameObject;

	// (MATT) Is deleted/recreated across serialisations {2009/04/30}
	CAIHandler* m_pAIHandler;

	bool        m_firing;
	bool        m_firingSecondary;

	string      m_commConfigName;
	string      m_voiceLibrary;
	string      m_voiceLibraryWhenForcingTest;
	float       m_FmodCharacterTypeParam;
	string      m_agentTypeName;

	int8        m_forcedExecute;

	// weapon/shooting related
	float m_fMinFireTime;     //forces a minimum time for fire trigger to stay on
	bool  m_WeaponShotIsDone; //if weapon did shot during last update
	bool  m_NeedsShootSignal;
	bool  m_CurrentWeaponCanFire;

	// vehicle weapon fire related
	bool               m_UseSecondaryVehicleWeapon;

	i32                m_shotBulletCount;
	CTimeValue         m_lastShotTime;

	bool               m_IsDisabled;
	bool               m_UpdateAlways;
	bool               m_autoDeactivated;

	EntityId           m_currentWeaponId;
	i32                m_currentWeaponFiremodeIndex;
	AIWeaponDescriptor m_currentWeaponDescriptor;

	// (MATT) Mutable for cached lookup {2009/01/27}
	mutable IActor* m_pIActor;

	// (MATT) Not serialised, which might be a source of bugs. {2009/04/30}
	i32      m_animActionGoalPipeId;
	EntityId m_weaponListenerId;

	// Listeners
	typedef std::vector<IAIProxyListener*> TListeners;
	TListeners                            m_listeners;

	std::unique_ptr<CommunicationHandler> m_commHandler;

	AimQueryMode                          m_aimQueryMode;
};

#endif // __AIProxy_H__
