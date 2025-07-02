// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TURRET__H__
#define __TURRET__H__

#include <drx3D/AI/IVisionMap.h>

#include <drx3D/Game/State.h>
#include <drx3D/Game/Health.h>
#include <drx3D/Game/TurretBehaviorParams.h>
#include <drx3D/Game/BodyDefinitions.h>
#include <drx3D/Game/AI/RateOfDeath/RateOfDeathSimple.h>

#include <drx3D/Game/GameRulesModules/IGameRulesKillListener.h>
#include <drx3D/Game/AutoAimUpr.h>
#include <drx3D/Game/AI/AIAwarenessToPlayerHelper.h>

class IActionController;
struct SAnimationContext;
struct HitInfo;
class CLaserBeam;
struct SLaserParams;


enum ETurretBehaviorState
{
	eTurretBehaviorState_Undeployed,
	eTurretBehaviorState_PartiallyDeployed,
	eTurretBehaviorState_Deployed,
	eTurretBehaviorState_Dead,

	eTurretBehaviorState_Count,
	eTurretBehaviorState_Invalid = eTurretBehaviorState_Count,
};

namespace TurretBehaviorStateNames
{
	tukk* GetNames();
	ETurretBehaviorState FindId( tukk const name );
}


typedef CRateOfDeathSimple CRateOfDeath_Turret;


class CTurret
	: public CGameObjectExtensionHelper< CTurret, IGameObjectExtension >
	, public IGameRulesKillListener
	, public CAIAwarenessToPlayerHelper::IAwarenessEntity
{
public:
	CTurret();
	virtual ~CTurret();

	// IEntityEvent
	virtual	void ProcessEvent( SEntityEvent &event );
	virtual ComponentEventPriority GetEventPriority(i32k eventID) const;
	// ~IEntityEvent

	// IGameObjectExtension
	virtual bool Init( IGameObject* pGameObject );
	virtual void InitClient( i32 channelId );
	virtual void PostInit( IGameObject* pGameObject );
	virtual void PostInitClient( i32 channelId );
	virtual bool ReloadExtension( IGameObject* pGameObject, const SEntitySpawnParams& params );
	virtual void PostReloadExtension( IGameObject* pGameObject, const SEntitySpawnParams& params );
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize serializer );
	virtual bool NetSerialize( TSerialize serializer, EEntityAspects aspect, u8 profile, i32 flags );
	virtual void PostSerialize();
	virtual void SerializeSpawnInfo( TSerialize serializer );
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void Update( SEntityUpdateContext& context, i32 updateSlot );
	virtual void PostUpdate( float frameTimeSeconds );
	virtual void PostRemoteSpawn();
	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void SetChannelId( u16 id );
	virtual void SetAuthority( bool authority );
	virtual void GetMemoryUsage( IDrxSizer* pSizer ) const;
	// ~IGameObjectExtension

	// IGameRulesKillListener
	virtual void OnEntityKilledEarly(const HitInfo &hitInfo);
	virtual void OnEntityKilled(const HitInfo &hitInfo);
	// ~IGameRulesKillListener

	// IAwarenessEntity
	virtual i32 GetAwarenessToActor( IAIObject* pAIObject, CActor* pActor ) const;
	// ~IAwarenessEntity

	void Enable();
	void Disable();

	void OnPropertyChange();
	void OnHit( const HitInfo& hitInfo );
	void OnDestroyed();

	bool IsEntityPlayer( IEntity* pTargetEntity ) const;
	bool IsEntityCloaked( IEntity* pTargetEntity ) const;
	bool IsEntityHostileAndThreatening( IEntity* pTargetEntity ) const;
	bool IsEntityInVVV( IEntity* pTargetEntity ) const;
	IEntity* FindTargetEntity( const float maxFindRange ) const;

	void SetTargetEntity( IEntity* pTarget );
	void NotifySelectedTarget( IEntity* targetEntity );
	IEntity* GetTargetEntity() const;
	IEntity* GetValidVisibleTarget() const;

	void SetTargetWorldPosition( const Vec3& targetWorldPosition );
	const Vec3& GetTargetWorldPosition() const;

	bool IsEntityInVisionRange( IEntity* pEntity ) const;
	bool IsVisionIdInVisionRange( const VisionID& otherVisionId ) const;
	bool IsEntityInRange( const IEntity* pEntity, const float range ) const;

	void HandleHit( const HitInfo* pHit );

	float GetHealth() const;
	void SetHealth( const float health, const bool hasBeenHitByPlayer = false );

	float GetMaxHealth() const;
	void SetMaxHealth( const float maxHealth );

	bool IsDead() const;

	IWeapon* GetPrimaryWeapon() const;
	void StartFirePrimaryWeapon();
	void StopFirePrimaryWeapon();
	bool IsInPrimaryWeaponRange( const Vec3& worldPosition ) const;

	IActionController* GetActionController() const;

	void StartFragmentByName( tukk fragmentName );

	void SetStateById( const ETurretBehaviorState forcedStateId );
	ETurretBehaviorState GetStateId() const;
	void NotifyBehaviorStateEnter( const ETurretBehaviorState stateId );

	void AddToTacticalUpr();
	void RemoveFromTacticalUpr();
	void UpdateTacticalIcon();
	void AddToRadar();
	void RemoveFromRadar();
	void SetTargetTrackClassThreat( const float classThreat );

	TurretBehaviorParams::SBehavior& GetBehaviorParams();

	float CalculateYawRadians() const;

	void SetThreateningForHostileFactions( const bool threatening );
	void SetObservable( const bool observable );

	void SetForcedVisibleTarget( const EntityId forcedVisibleTargetEntityId );
	void ClearForcedVisibleTarget();
	EntityId GetForcedVisibleTarget() const;

	void SetAllowFire( const bool allowFire );
	bool GetAllowFire() const;

	void NotifyPreparingToFire( const CTimeValue& nextStartFireTime );
	void NotifyCancelPreparingToFire();

	void NotifyGroupTargetSpotted( const IEntity* pTargetEntity );

	void SetEyeVisionRangeScale( const float scale, const float timeoutSeconds );
	float GetEyeVisionRangeScaleToSeePosition( const Vec3& worldPosition, const float additionalDistanceOffset = 0.0f ) const;
	float GetCachedEyeVisionRangeParamValue() const;

	Vec3 CalculateRadarForwardWorldDirection() const;

	void SetFactionId( u8k factionId );
	u8 GetFactionId() const;

protected:

	void RegisterAsKillListener();
	void UnRegisterAsKillListener();

	void CacheResources();
	void Reset( const bool enteringGameMode );
	void OnPrePhysicsUpdate();

	void Physicalize();
	void UnPhysicalize();

	tukk GetModelName() const;
	tukk GetPrimaryWeaponClassName() const;
	float GetPrimaryWeaponFovDegrees() const;
	float GetPrimaryWeaponRangeCheckOffset() const;
	tukk GetWeaponJointName() const;
	tukk GetDefaultFactionName() const;
	void UpdateWeaponTarget();
	void SetWeaponTargetWorldPosition( const Vec3& targetWorldPosition );

	u8 GetDefaultFactionId() const;

	void InitActionController();
	void ResetActionController();
	void UpdateActionController( const float frameTimeSeconds );
	void UpdateMannequinTags( const float frameTimeSeconds );
	void UpdateMannequinParams( const float frameTimeSeconds );

	void StartInitialMannequinActions();
	void InitMannequinUserParams();
	void InitAimProceduralContext();

	void InitWeapons();
	void RemoveWeapons();

	void InitLaserBeam();
	void RemoveLaserBeam();
	void UpdateLaserBeam( const float frameTimeSeconds );

	void RemoveItemAttachments();
	void CreateItemAttachments();

	bool IsValidTargetEntity( const IEntity* pEntity ) const;

	void ResetTarget();

	void CreateStateMachine();
	void UpdateStateMachine( const float frameTimeSeconds );

	void ResetHealth();
	void OnHealthChanged();

	void ResetVision();
	void RemoveVision();
	void UpdateVision();
	void UpdateRadarVisionLocation();
	void UpdateEyeVisionLocation();
	void UpdateVisionFaction();
	void UpdateEyeVisionScaleExpiration();
	void InitObserverParams( ObserverParams& observerParamsOut, const float fovDegrees );
	void VisionChanged( const VisionID& observerID, const ObserverParams& observerParams, const VisionID& observableID, const ObservableParams& observableParams, bool visible );

	typedef enum
	{
		eIARM_RestorePreviousState,
		eIARM_RebuildFromScratch,
	} EInitAiRepresentationMode;
	void InitAiRepresentation( const EInitAiRepresentationMode mode = eIARM_RestorePreviousState );
	void RemoveAiRepresentation();
	void NotifyAiThatTurretIsRemoved();

	void UpdateAiLocation();

	Vec3 CalculateRadarVisionWorldPosition() const;
	Vec3 CalculateEyeVisionWorldPosition() const;
	Vec3 CalculateEyeVisionWorldDirection() const;

	void AddVisibleTarget( const EntityId targetEntityId );
	void RemoveVisibleTarget( const EntityId targetEntityId );
	void UpdateVisibleTargets();
	void ClearVisibleTargets();


	i16 GetJointIdByName( tukk jointName ) const;

	void ResetBehaviorParams();

	ETurretBehaviorState GetInitialBehaviorStateId() const;
	void SetInitialStateById( const ETurretBehaviorState stateId );

	void CreateRateOfDeathHelper();
	void RemoveRateOfDeathHelper();
	void UpdateRateOfDeathHelper( const float frameTimeSeconds );
	Vec3 GetRateOfDeathTargetOffset() const;

	void CacheBodyDamageResources();
	void CreateBodyDamage();
	void RemoveBodyDamage();

	void UpdateBodyDestructionHit( const HitInfo& hitInfo, const float oldHealth, const float newHealth );
	void UpdateBodyDestruction( const float frameTimeSeconds );

	void InitTurretSoundUpr();
	void UpdateTurretSoundUpr( const float frameTimeSeconds );

	void NotifyDestroyed( const bool hasBeenDestroyedByPlayer = false ) const;
	void OutputFlowEvent( tukk const eventName ) const;

	void SetGroupId( i32k groupId );
	i32 GetGroupId() const;

	void InitAutoAimParams();
	void RegisterInAutoAimSystem();
	void UnRegisterFromAutoAimSystem();

	void RegisterInAwarenessToPlayerHelper();
	void UnRegisterFromAwarenessToPlayerHelper();

	void UpdateFactionChange();

	void ProcessScriptEvent( tukk const eventName );
private:

	IActionController* m_pActionController;
	SAnimationContext* m_pAnimationContext;
	const struct SMannequinTurretParams* m_pUserParams;
	const class CProceduralContextTurretAimPose* m_pAimProceduralContext;

	EntityId m_primaryWeaponId;
	float m_primaryWeaponFovCos;
	float m_primaryWeaponRangeCheckOffset;

	Vec3 m_laserBeamOffset;
	std::unique_ptr<CLaserBeam> m_pLaserBeam;
	std::unique_ptr<SLaserParams> m_pLaserParams;
	i32 m_laserSourceEffectSlot;
	i16 m_laserJointId;

	EntityId m_targetEntityId;
	Vec3 m_targetWorldPosition;

	CHealth m_health;

	VisionID m_radarVisionId;
	float m_visionRadarOffset;

	VisionID m_eyeVisionId;
	i16 m_eyeJointId;
	float m_eyeVisionRangeParameter;
	CTimeValue m_eyeVisionScaleExpireTime;

	struct STrackedTarget
	{
		EntityId entityId;
		bool valid;
		i32 referenceCount;

		STrackedTarget( EntityId entityId_ )
			: entityId( entityId_ )
			, valid( false )
			, referenceCount( 1 )
		{
		}

		bool operator == ( const EntityId otherEntityId ) const
		{
			return ( entityId == otherEntityId );
		}
	};

	typedef std::vector< STrackedTarget > TrackedTargetsList;
	TrackedTargetsList m_trackedTargets;
	EntityId m_validVisbleTargetEntityId;
	EntityId m_forcedVisibleTargetEntityId;
	float m_cloakDetectionDistance;
	float m_updateDistanceSquared;

	bool m_allowFire;
	bool m_registeredAsKillListener;

	TurretBehaviorParams::SBehavior m_behaviorParams;

	ETurretBehaviorState m_stateId;

	std::unique_ptr<struct STurretSerializationInfo> m_pSerializationInfo;

	std::unique_ptr<CRateOfDeath_Turret> m_pRateOfDeathHelper;

	CBodyDestrutibilityInstance m_bodyDestructionInstance;

	std::unique_ptr<class CTurretSoundUpr> m_pSoundUpr;

	SAutoaimTargetRegisterParams m_autoAimParams;
	bool m_registeredInAutoAimSystem;

	u8 m_factionWhenAiRepresentationRemoved;
	u8 m_factionOld;

	DECLARE_STATE_MACHINE( CTurret, Behaviour );
};

#endif
