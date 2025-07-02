// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IAIActorProxy_h__
#define __IAIActorProxy_h__

#pragma once

#include <drx3D/AI/IAIObject.h>

struct IAIActorProxyFactory
{
	virtual IAIActorProxy* CreateActorProxy(EntityId entityID) = 0;
	virtual ~IAIActorProxyFactory(){}
};

struct IAIActorProxy :
	public _reference_target_t
{
	// <interfuscator:shuffle>
	// (MATT) These methods were merged from IUnknownProxy {2009/01/26}
	//----------------------------------------------------------------
	virtual i32                      Update(SOBJECTSTATE& state, bool bFullUpdate) = 0;
	virtual bool                     CheckUpdateStatus() = 0;
	virtual void                     EnableUpdate(bool enable) = 0;
	virtual bool                     IsEnabled() const = 0;
	virtual i32                      GetAlertnessState() const = 0;
	virtual void                     SetAlertnessState(i32 alertness) = 0;
	virtual bool                     IsCurrentBehaviorExclusive() const = 0;
#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	virtual bool                     SetCharacter(tukk character, tukk behaviour = NULL) = 0;
	virtual tukk              GetCharacter() = 0;
#endif
	virtual bool                     QueryBodyInfo(SAIBodyInfo& bodyInfo) = 0;
	virtual bool                     QueryBodyInfo(const SAIBodyInfoQuery& query, SAIBodyInfo& bodyInfo) = 0;
	virtual void                     QueryWeaponInfo(SAIWeaponInfo& weaponInfo) = 0;
	virtual EntityId                 GetLinkedDriverEntityId() = 0;
	virtual bool                     IsDriver() = 0;
	virtual EntityId                 GetLinkedVehicleEntityId() = 0;
	virtual bool                     GetLinkedVehicleVisionHelper(Vec3& outHelperPos) const = 0;
	virtual void                     Reset(EObjectResetType type) = 0;
	virtual void                     Serialize(TSerialize ser) = 0;

	virtual IAICommunicationHandler* GetCommunicationHandler() = 0;

	virtual bool                     BecomeAggressiveToAgent(EntityId agentID) = 0;

	virtual IPhysicalEntity*         GetPhysics(bool wantCharacterPhysics = false) = 0;
	virtual void                     DebugDraw(i32 iParam = 0) = 0;

	//! This will internally keep a counter to allow stacking of such commands.
	virtual void SetForcedExecute(bool forced) = 0;
	virtual bool IsForcedExecute() const = 0;

	//! Gets the corners of the tightest projected bounding rectangle in 2D world space coordinates.
	virtual void              GetWorldBoundingRect(Vec3& FL, Vec3& FR, Vec3& BL, Vec3& BR, float extra = 0) const = 0;
	virtual bool              SetAGInput(EAIAGInput input, tukk value, const bool isUrgent = false) = 0;
	virtual bool              ResetAGInput(EAIAGInput input) = 0;
	virtual bool              IsSignalAnimationPlayed(tukk value) = 0;
	virtual bool              IsActionAnimationStarted(tukk value) = 0;
	virtual bool              IsAnimationBlockingMovement() const = 0;
	virtual EActorTargetPhase GetActorTargetPhase() const = 0;
	virtual void              PlayAnimationAction(const struct IAIAction* pAction, i32 goalPipeId) = 0;
	virtual void              AnimationActionDone(bool succeeded) = 0;
	virtual bool              IsPlayingSmartObjectAction() const = 0;

	//! \return the number of bullets shot since the last call to the method.
	virtual i32                       GetAndResetShotBulletCount() = 0;

	virtual void                      EnableWeaponListener(const EntityId weaponId, bool needsSignal) = 0;
	virtual void                      UpdateMind(SOBJECTSTATE& state) = 0;

	virtual bool                      IsDead() const = 0;
	virtual float                     GetActorHealth() const = 0;
	virtual float                     GetActorMaxHealth() const = 0;
	virtual i32                       GetActorArmor() const = 0;
	virtual i32                       GetActorMaxArmor() const = 0;
	virtual bool                      GetActorIsFallen() const = 0;

	virtual IWeapon*                  QueryCurrentWeapon(EntityId& currentWeaponId) = 0;
	virtual IWeapon*                  GetCurrentWeapon(EntityId& currentWeaponId) const = 0;
	virtual const AIWeaponDescriptor& GetCurrentWeaponDescriptor() const = 0;
	virtual IWeapon*                  GetSecWeapon(const ERequestedGrenadeType prefGrenadeType = eRGT_ANY, ERequestedGrenadeType* pReturnedGrenadeType = NULL, EntityId* const pSecondaryEntityId = NULL) const = 0;
	virtual bool                      GetSecWeaponDescriptor(AIWeaponDescriptor& outDescriptor, ERequestedGrenadeType prefGrenadeType = eRGT_ANY) const = 0;

	//! Sets if the AI should use the secondary weapon when gunning a vehicle
	virtual void        SetUseSecondaryVehicleWeapon(bool bUseSecondary) = 0;
	virtual bool        IsUsingSecondaryVehicleWeapon() const = 0;

	virtual IEntity*    GetGrabbedEntity() const = 0;

	virtual tukk GetVoiceLibraryName(const bool useForcedDefaultName = false) const = 0;
	virtual tukk GetCommunicationConfigName() const = 0;
	virtual const float GetFmodCharacterTypeParam() const = 0;
	virtual tukk GetNavigationTypeName() const = 0;

	//! Needed for debug drawing.
	virtual bool IsUpdateAlways() const = 0;
	virtual bool IfShouldUpdate() = 0;
	virtual bool IsAutoDeactivated() const = 0;
	virtual void NotifyAutoDeactivated() = 0;

	//! Predict the flight path of a projectile launched with the primary weapon.
	virtual bool PredictProjectileHit(float vel, Vec3& posOut, Vec3& dirOut, float& speedOut, Vec3* pTrajectoryPositions = 0, u32* trajectorySizeInOut = 0, Vec3* pTrajectoryVelocities = 0) = 0;

	//! Predict the flight path of a grenade thrown (secondary weapon).
	virtual bool PredictProjectileHit(const Vec3& throwDir, float vel, Vec3& posOut, float& speedOut, ERequestedGrenadeType prefGrenadeType = eRGT_ANY, Vec3* pTrajectoryPositions = 0, u32* trajectorySizeInOut = 0, Vec3* pTrajectoryVelocities = 0) = 0;

	//! \return the amount of time to block the readabilities of same name in a group.
	virtual void GetReadabilityBlockingParams(tukk text, float& time, i32& id) = 0;

	//! Gets name of the running behavior.
	virtual tukk GetCurrentBehaviorName() const { return NULL; }

	//! Gets name of the previous behavior.
	virtual tukk GetPreviousBehaviorName() const { return NULL; }

	virtual void        UpdateMeAlways(bool doUpdateMeAlways) = 0;

	virtual void        SetBehaviour(tukk szBehavior, const IAISignalExtraData* pData = 0) = 0;

	virtual void        OnActorRemoved() {}

	//! The AI signals sent from the AI handler are usually throttled
	//! so they are not sent out all the time. For example, if the
	//! target is standing on the same spot signals will not go out.
	//! In some cases we explicitly want these signals to be sent out
	//! and we can demand that by calling this method.
	virtual void ResendTargetSignalsNextFrame() = 0;
	// </interfuscator:shuffle>
};

#endif
