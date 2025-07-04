// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a wheels based vehicle

   -------------------------------------------------------------------------
   История:
   - 05:10:2004: Created by Mathieu Pinard

*************************************************************************/
#ifndef __CVEHICLE_H__
#define __CVEHICLE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "VehicleSystem.h"
#include <drx3D/Script/IScriptSystem.h>
#include "VehicleDamages.h"
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include <drx3D/Entity/IEntityComponent.h>

#include "VehicleAnimationComponent.h"

class CEntityObject;
class CVehicleSeat;
class CVehicleSeatGroup;
class CVehicleComponent;
struct pe_params_car;
class CVehiclePartBase;
class CVehiclePartWheel;
class CVehicleHelper;
struct ISound;
struct IInventory;
class CVehiclePartAnimated;
class CVehicleSeatActionWeapons;
struct SVehicleWeapon;

typedef std::pair<string, CVehicleSeat*>         TVehicleSeatPair;
typedef std::vector<TVehicleSeatPair>            TVehicleSeatVector;
typedef std::vector<CVehicleSeatGroup*>          TVehicleSeatGroupVector;
typedef std::pair<string, IVehiclePart*>         TVehiclePartPair;
typedef std::vector<TVehiclePartPair>            TVehiclePartVector;
typedef std::vector<CVehicleComponent*>          TVehicleComponentVector;
typedef CListenerSet<IVehicleEventListener*>     TVehicleEventListenerSet;
typedef std::pair<string, IVehicleAnimation*>    TVehicleStringAnimationPair;
typedef std::vector<TVehicleStringAnimationPair> TVehicleAnimationsVector;

struct SActivationInfo
{
	enum EActivationType
	{
		eAT_OnUsed = 0,
		eAT_OnGroundCollision,
		eAT_Last,
	};

	enum EActivationParam1
	{
		eAP1_Part = 0,
		eAP1_Component,
		eAP1_Last,
	};

	SActivationInfo()
		: m_type(eAT_Last)
		, m_param1(eAP1_Last)
		, m_distance(0.f)
	{
	}

	EActivationType   m_type;
	EActivationParam1 m_param1;
	string            m_param2;
	float             m_distance;
};

typedef std::vector<SActivationInfo> TActivationInfoVector;

struct SVehicleActionInfo
{
	TActivationInfoVector activations;
	IVehicleAction*       pAction;
	i32                   useWhenFlipped;
};

typedef std::vector<SVehicleActionInfo> TVehicleActionVector;

struct STransitionInfo
{
	STransitionInfo()
	{
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(waitFor);
	}
	DynArray<TVehicleSeatId> waitFor;
};
typedef DynArray<STransitionInfo> TTransitionInfo;

struct SVehiclePredictionHistory
{
	SVehicleMovementAction m_actions;
	SVehicleNetState       m_state;
	Vec3                   m_pos;
	Quat                   m_rot;
	Vec3                   m_velocity;
	Vec3                   m_angVelocity;
	float                  m_deltaTime;
};

#if ENABLE_VEHICLE_DEBUG
struct SDebugClientPredictData
{
	QuatT recordedPos;
	QuatT correctedPos;
	Vec3  recordedVel;
	Vec3  correctedVel;
};
#endif

class CVehicle :
	public CGameObjectExtensionHelper<CVehicle, IVehicle>,
	public CVehicleDamages
{
public:

	CVehicle();
	~CVehicle();

	static const NetworkAspectType ASPECT_SEAT_PASSENGER = eEA_GameServerStatic;
	static const NetworkAspectType ASPECT_SEAT_ACTION = eEA_GameClientDynamic;
	static const NetworkAspectType ASPECT_COMPONENT_DAMAGE = eEA_GameServerStatic;
	static const NetworkAspectType ASPECT_PART_MATRIX = eEA_GameClientA;

	enum EVehicleTimers
	{
		eVT_Abandoned = 0,
		eVT_AbandonedSound,
		eVT_Flipped,
		eVT_Last
	};

	struct SPartComponentPair
	{
		SPartComponentPair(IVehiclePart* pP, tukk pC) : pPart(pP), component(pC) {}

		IVehiclePart* pPart;
		string        component;
	};

	struct SPartInitInfo
	{
		i32                             index;
		std::vector<SPartComponentPair> partComponentMap;

		SPartInitInfo() : index(-1)
		{}
	};

	virtual void     SetOwnerId(EntityId ownerId) { m_ownerId = ownerId; };
	virtual EntityId GetOwnerId() const           { return m_ownerId; };
	bool             CanEnter(EntityId actorId);

	enum EVehiclePhysicsProfile
	{
		eVPhys_NotPhysicalized = 0,
		eVPhys_Physicalized,
		eVPhys_DemoRecording
	};

	//IEntityEvent
	virtual void                   ProcessEvent(const SEntityEvent& entityEvent) final;
	virtual uint64                 GetEventMask() const final;
	virtual ComponentEventPriority GetEventPriority() const final;
	//~IEntityEvent

	// IVehicle
	virtual bool Init(IGameObject* pGameObject);
	virtual void InitClient(i32 channelId) {};
	virtual void PostInit(IGameObject* pGameObject);
	virtual void PostInitClient(i32 channelId);
	virtual void Reset(bool enterGame);
	virtual bool ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) {}
	virtual void Release()                                                                       { delete this; }
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void SetAmmoCapacity(IEntityClass* pAmmoType, i32 capacity);
	virtual void SetAmmoCount(IEntityClass* pAmmoType, i32 amount);
	virtual i32  GetAmmoCount(IEntityClass* pAmmoType) const;

	// set/get the last weapon created on this vehicle
	// see ClSetupWeapons, SendWeaponSetup and  CVehicleSeatActionWeapons::Init for more info
	virtual void                        SetLastCreatedWeaponId(EntityId lastWeaponId) { m_lastWeaponId = lastWeaponId; };
	virtual EntityId                    GetLastCreatedWeaponId() const                { return m_lastWeaponId; };

	SVehicleWeapon*                     GetVehicleWeaponAllSeats(EntityId weaponId) const;
	virtual void                        SendWeaponSetup(i32 where, i32 channelId = -1);

	virtual SEntityPhysicalizeParams&   GetPhysicsParams() { return m_physicsParams; }

	virtual void                        HandleEvent(const SGameObjectEvent&);
	virtual void                        PostUpdate(float frameTime);
	virtual void                        PostRemoteSpawn() {};

	virtual const SVehicleStatus&       GetStatus() const;

	virtual void                        Update(SEntityUpdateContext& ctx, i32 nSlot);
	virtual void                        UpdateView(SViewParams& viewParams, EntityId playerId = 0);
	virtual void                        UpdatePassenger(float frameTime, EntityId playerId = 0);

	virtual void                        SetChannelId(u16 id) {};

	virtual void                        FullSerialize(TSerialize ser);
	virtual bool                        NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	virtual void                        PostSerialize();
	virtual void                        SerializeSpawnInfo(TSerialize ser);
	virtual ISerializableInfoPtr        GetSpawnInfo();
	virtual void                        OnAction(const TVehicleActionId actionId, i32 activationMode, float value, EntityId callerId);
	virtual void                        OnHit(const HitInfo& hitInfo, IVehicleComponent* pHitComponent = NULL);

	virtual i32                         IsUsable(EntityId userId);
	virtual bool                        OnUsed(EntityId userId, i32 index);

	virtual IVehiclePart*               AddPart(const CVehicleParams& partParams, IVehiclePart* parent, SPartInitInfo& initInfo);
	virtual bool                        AddSeat(const SmartScriptTable& seatParams);
	virtual bool                        AddHelper(tukk pName, Vec3 position, Vec3 direction, IVehiclePart* pPart);
	virtual void                        InitHelpers(const CVehicleParams& table);
	virtual bool                        HasHelper(tukk name);

	virtual bool                        SetMovement(const string& movementName, const CVehicleParams& table);
	virtual IVehicleMovement*           GetMovement() const { return m_pMovement; }

	virtual i32                         GetWheelCount()     { return m_wheelCount; }
	virtual IVehiclePart*               GetWheelPart(i32 idx);

	virtual float                       GetMass() const { return m_mass; }
	virtual float                       GetAltitude();

	virtual u32                GetSeatCount() { return m_seats.size(); }

	virtual TVehicleSeatId              GetLastSeatId();

	virtual IVehicleSeat*               GetSeatForPassenger(EntityId passengerId) const;
	virtual IVehicleSeat*               GetSeatById(const TVehicleSeatId seatId) const;
	virtual IActor*                     GetDriver() const;
	virtual TVehicleSeatId              GetSeatId(tukk pSeatName);

	virtual IVehiclePart*               GetPart(tukk name);
	virtual IVehiclePart*               GetWeaponParentPart(EntityId weaponId);
	IVehicleSeat*                       GetWeaponParentSeat(EntityId weaponId);

	virtual i32                         GetComponentCount() const;
	virtual IVehicleComponent*          GetComponent(i32 index);
	virtual IVehicleComponent*          GetComponent(tukk name);
	virtual float                       GetDamageRatio(bool onlyMajorComponents = false) const;

	virtual void                        SetUnmannedFlippedThresholdAngle(float angle);

	virtual SParticleParams*            GetParticleParams()     { return &m_particleParams; }
	virtual const SVehicleDamageParams& GetDamageParams() const { return m_damageParams; }

	virtual IVehicleAnimation*          GetAnimation(tukk name);

	virtual IMovementController*        GetMovementController();
	virtual IMovementController*        GetPassengerMovementController(EntityId passenger);

	virtual IFireController*            GetFireController(u32 controllerNum = 0);
	virtual EntityId                    GetCurrentWeaponId(EntityId passengerId, bool secondary = false) const;
	virtual bool                        GetCurrentWeaponInfo(SVehicleWeaponInfo& outInfo, EntityId passengerId, bool secondary = false) const;
	virtual i32                         GetWeaponCount() const;
	virtual EntityId                    GetWeaponId(i32 index) const;

	// check if player/client is driving
	virtual bool IsPlayerDriving(bool clientOnly = true);

	// check if any passenger is friendly to pPlayer
	virtual bool HasFriendlyPassenger(IEntity* pPlayer);

	// check if ClientActor is on board
	virtual bool            IsPlayerPassenger();

	virtual void            StartAbandonedTimer(bool force = false, float timer = -1.0f);
	virtual void            KillAbandonedTimer();
	virtual void            Destroy();
	virtual void            EnableAbandonedWarnSound(bool enable);

	virtual void            BroadcastVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params);
	virtual void            RegisterVehicleEventListener(IVehicleEventListener* pEvenListener, tukk name);
	virtual void            UnregisterVehicleEventListener(IVehicleEventListener* pEvenListener);

	void                    SetObjectUpdate(IVehicleObject* pObject, EVehicleObjectUpdate updatePolicy);

	virtual IVehicleHelper* GetHelper(tukk name);
	virtual bool            IsDestroyed() const;
	bool                    IsDestroyable() const;

	virtual void            TriggerEngineSlotBySpeed(bool trigger) { m_engineSlotBySpeed = trigger; }
	virtual i32             SetTimer(i32 timerId, i32 ms, IVehicleObject* pObject);
	virtual i32             KillTimer(i32 timerId);
	virtual void            KillTimers();

	virtual bool            GetExitPositionForActor(IActor* pActor, Vec3& pos, bool extended = false);
	virtual void            ExitVehicleAtPosition(EntityId passengerId, const Vec3& pos);
	virtual void            EvictAllPassengers();
	virtual void            ClientEvictAllPassengers();
	virtual void            ClientEvictPassenger(IActor* pActor);

	virtual bool            IsCrewHostile(EntityId userId);

	virtual tukk     GetActionMap() const { return m_actionMap.c_str(); }

	virtual float           GetSelfCollisionMult(const Vec3& velocity, const Vec3& normal, i32 partId, EntityId colliderId) const;

	// Is vehicle probably distant from the player?
	virtual bool IsProbablyDistant() const;

	virtual void OffsetPosition(const Vec3& delta);

	// ~IVehicle

	void OnSpawnComplete();

	struct RequestUseParams
	{
		EntityId actorId;
		i32      index;
		RequestUseParams() {};
		RequestUseParams(EntityId _actorId, i32 _index)
			: actorId(_actorId),
			index(_index) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("index", index);
			ser.Value("actorId", actorId, 'eid');
		};
	};

	struct RequestChangeSeatParams
	{
		EntityId actorId;
		i32      seatChoice;
		RequestChangeSeatParams()
			: actorId(0)
			, seatChoice(0)
		{}
		RequestChangeSeatParams(EntityId _actorId, i32 _seatChoice)
			: actorId(_actorId)
			, seatChoice(_seatChoice)
		{}
		void SerializeWith(TSerialize ser)
		{
			ser.Value("actorId", actorId, 'eid');
			ser.Value("seatChoice", seatChoice, 'vSit');
		};
	};

	struct RequestLeaveParams
	{
		EntityId actorId;
		Vec3     exitPos;
		RequestLeaveParams() {};
		RequestLeaveParams(EntityId _actorId, Vec3 _exitPos)
			: actorId(_actorId), exitPos(_exitPos) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("actorId", actorId, 'eid');
			ser.Value("exitPos", exitPos, 'wrld');
		};
	};

	struct RequestCompleteParams
	{
		EntityId actorId;
		RequestCompleteParams() {};
		RequestCompleteParams(EntityId _actorId) : actorId(_actorId) {}
		void SerializeWith(TSerialize ser) { ser.Value("actorId", actorId, 'eid'); }
	};

	struct SetupWeaponsParams
	{
		struct SeatWeaponParams
		{
			struct SeatActionWeaponParams
			{
				std::vector<EntityId> weapons;

				void                  SerializeWith(TSerialize ser)
				{
					if (ser.IsReading())
					{
						i32 nweapons = 0;
						ser.Value("NumberWeapons", nweapons, 'vNWp');
						for (i32 i = 0; i < nweapons; i++)
						{
							EntityId id;
							ser.Value("weaponId", id, 'eid');
							weapons.push_back(id);
						}
					}
					else
					{
						i32 nweapons = (i32)weapons.size();
						ser.Value("NumberWeapons", nweapons, 'vNWp');
						for (i32 i = 0; i < nweapons; i++)
						{
							EntityId id = weapons[i];
							ser.Value("weaponId", id, 'eid');
						}
					}
				}
			};

			std::vector<SeatActionWeaponParams> seatactions;

			void                                SerializeWith(TSerialize ser)
			{
				if (ser.IsReading())
				{
					i32 nactions = 0;
					ser.Value("NumberActions", nactions, 'vNWp');
					seatactions.reserve(nactions);
					for (i32 i = 0; i < nactions; i++)
					{
						seatactions.push_back(SeatActionWeaponParams());
						seatactions.back().SerializeWith(ser);
					}
				}
				else
				{
					i32 nactions = (i32)seatactions.size();
					ser.Value("NumberActions", nactions, 'vNWp');
					for (i32 i = 0; i < nactions; i++)
						seatactions[i].SerializeWith(ser);
				}
			};
		};

		std::vector<SeatWeaponParams> seats;

		SetupWeaponsParams() {};
		void SerializeWith(TSerialize ser)
		{
			if (ser.IsReading())
			{
				i32 nseats = 0;
				ser.Value("NumberSeats", nseats, 'vNWp');
				seats.reserve(nseats);
				for (i32 i = 0; i < nseats; i++)
				{
					seats.push_back(SeatWeaponParams());
					seats.back().SerializeWith(ser);
				}
			}
			else
			{
				i32 nseats = (i32)seats.size();
				ser.Value("NumberSeats", nseats, 'vNWp');
				for (i32 i = 0; i < nseats; i++)
					seats[i].SerializeWith(ser);
			}
		};
	};

	struct AmmoParams
	{
		AmmoParams()
			: count(0)
		{}
		AmmoParams(tukk name, i32 amount)
			: ammo(name)
			, count(amount)
		{}
		void SerializeWith(TSerialize ser)
		{
			ser.Value("ammo", ammo);
			ser.Value("amount", count, 'ammo');
		}
		string ammo;
		i32    count;
	};

	struct AbandonWarningParams
	{
		bool enable;

		AbandonWarningParams() : enable(false) {};
		AbandonWarningParams(bool e) : enable(e) {};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("enable", enable, 'bool');
		}
	};

	struct RespawnWeaponParams
	{
		RespawnWeaponParams() : weaponId(0), seatId(0), seatActionId(0), weaponIndex(0) {};
		RespawnWeaponParams(EntityId id, TVehicleSeatId vehicleSeatId, TVehicleObjectId actionId, u8 weaponIdx) : weaponId(id), seatId(vehicleSeatId), seatActionId(actionId), weaponIndex(weaponIdx) {};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("weaponId", weaponId, 'eid');
			i32 seatIdZeroIndex = seatId - 1;
			DRX_ASSERT(seatActionId < 64 && weaponIndex < 2 && seatIdZeroIndex < 4);
			ser.Value("seatActionId", seatActionId, 'ui6');
			ser.Value("seatId", seatIdZeroIndex, 'ui2');
			if (ser.IsReading())
			{
				seatId = seatIdZeroIndex + 1;
			}

			bool isPrimary = weaponIndex == 0;
			ser.Value("weaponIndex", isPrimary, 'bool');
			if (ser.IsReading())
			{
				weaponIndex = isPrimary ? 0 : 1;
			}
		};

		EntityId weaponId;
		u8    seatId;
		u8    seatActionId;
		u8    weaponIndex;
	};

	DECLARE_SERVER_RMI_PREATTACH(SvRequestUse, RequestUseParams);
	DECLARE_CLIENT_RMI_NOATTACH(ClRequestComplete, RequestCompleteParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_PREATTACH(SvRequestChangeSeat, RequestChangeSeatParams);
	DECLARE_SERVER_RMI_PREATTACH(SvRequestLeave, RequestLeaveParams);
	DECLARE_CLIENT_RMI_PREATTACH(ClProcessLeave, RequestLeaveParams);
	DECLARE_CLIENT_RMI_NOATTACH(ClSetAmmo, AmmoParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClSetupWeapons, SetupWeaponsParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClAbandonWarning, AbandonWarningParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClRespawnWeapon, RespawnWeaponParams, eNRT_ReliableOrdered);

public:

	struct SObjectUpdateInfo
	{
		IVehicleObject* pObject;
		i32             updatePolicy;
	};
	typedef std::list<SObjectUpdateInfo>           TVehicleObjectUpdateInfoList;
	typedef std::vector<SVehiclePredictionHistory> TVehiclePredictionHistory;

	// saved hit type ids to avoid strcmp
	static i32 s_repairHitTypeId;
	static i32 s_disableCollisionsHitTypeId;
	static i32 s_collisionHitTypeId;
	static i32 s_normalHitTypeId;
	static i32 s_fireHitTypeId;
	static i32 s_punishHitTypeId;
	static i32 s_vehicleDestructionTypeId;

public:

	bool                     AddComponent(const CVehicleParams& componentParams);

	TVehicleComponentVector& GetComponents() { return m_components; }
	TVehiclePartVector&      GetParts()      { return m_parts; }

	virtual i32              GetPartCount()  { return m_parts.size(); }
	virtual IVehiclePart*    GetPart(u32 index);
	virtual void             GetParts(IVehiclePart** parts, i32 nMax);
	CVehiclePartAnimated*    GetVehiclePartAnimated();

	virtual i32              GetActionCount() { return m_actions.size(); }
	virtual IVehicleAction*  GetAction(i32 index);

	TVehicleSeatId           GetSeatId(CVehicleSeat* pSeat);
	TVehicleSeatId           GetDriverSeatId() const;

	TVehicleSeatId           GetNextSeatId(const TVehicleSeatId seatIdToStart = InvalidVehicleSeatId);
	void                     ChangeSeat(EntityId actorId, i32 seatChoice = 0, TVehicleSeatId newSeatId = InvalidVehicleSeatId);

	void                     OnCollision(EventPhysCollision* pCollision);

#if ENABLE_VEHICLE_DEBUG
	i32  GetDebugIndex() { return m_debugIndex++; }
	void DumpParts() const;
	void DebugFlipOver();
	void DebugReorient();
#endif

	// when using this, take care of static buffer the result points to
	tukk          GetSoundName(tukk eventName, bool isEngineSound);

	bool                 InitRespawn();
	void                 OnDestroyed();
	void                 OnSubmerged(float ratio);
	bool                 SpawnAndDeleteEntities(bool clientBasedOnly = false);

	bool                 TriggerEngineSlotBySpeed() { return m_engineSlotBySpeed; }
	bool                 IsEmpty();
	virtual bool         IsFlipped(float maxSpeed = 0.f);
	bool                 IsSubmerged();

	TVehicleSoundEventId AddSoundEvent(SVehicleSoundInfo& info);
	SVehicleSoundInfo*   GetSoundInfo(TVehicleSoundEventId eventId);
	//ISound* GetSound(TVehicleSoundEventId eventId, bool start=true);
	void                 SetSoundParam(TVehicleSoundEventId eventId, tukk param, float value, bool start = true);
	void                 StopSound(TVehicleSoundEventId eventId);
	inline bool          EventIdValid(TVehicleSoundEventId eventId);

	i32                  GetNextPhysicsSlot(bool high) const;

	void                 RequestPhysicalization(IVehiclePart* pPart, bool request);
	virtual void         NeedsUpdate(i32 flags = 0, bool bThreadSafe = false);

	void                 CheckFlippedStatus(const float deltaTime);

	STransitionInfo&     GetTransitionInfoForSeat(TVehicleSeatId seatId)
	{
		DRX_ASSERT(seatId > 0 && seatId <= m_transitionInfo.size());
		return m_transitionInfo[seatId - 1];
	}

	i32                 GetSkipEntities(IPhysicalEntity** pSkipEnts, i32 nMaxSkip);

	virtual tukk GetModification() const;

	IMaterial*          GetPaintMaterial() const     { return m_pPaintMaterial; }
	IMaterial*          GetDestroyedMaterial() const { return m_pDestroyedMaterial; }

	bool                ExitSphereTest(IPhysicalEntity** pSkipEnts, i32 nskip, Vec3 startPos, Vec3 testPos, EntityId* pBlockingEntity);
	bool                DoExtendedExitTest(Vec3 seatPos, Vec3 firstTestPos, EntityId blockingEntity, Vec3& outPos);

	bool                IsIndestructable() const { return m_indestructible; }

	float               GetPhysicsFrameTime()    { return m_physUpdateTime; }

	//Used in MP for logically linking associated vehicles together across network
	virtual EntityId                  GetParentEntityId() const                  { return m_ParentId; }
	virtual void                      SetParentEntityId(EntityId parentEntityId) { m_ParentId = parentEntityId; };

	ILINE CVehicleAnimationComponent& GetAnimationComponent()
	{
		return m_vehicleAnimation;
	}

	void UpdatePassengerCount();

	void OnActionEvent(EVehicleActionEvent event);
	void OnPrePhysicsTimeStep(float deltaTime);

protected:

	bool                             InitActions(const CVehicleParams& vehicleTable);
	void                             InitPaint(const CVehicleParams& xmlContent);
	bool                             IsActionUsable(const SVehicleActionInfo& actionInfo, const SMovementState* movementState);

	void                             LoadParts(const CVehicleParams& table, IVehiclePart* pParent, SPartInitInfo& initInfo);

	void                             OnPhysPostStep(const EventPhys* pEvent, bool logged);
	void                             OnPhysStateChange(EventPhysStateChange* pEvent);
	void                             OnMaterialLayerChanged(const SEntityEvent& event);
	bool                             InitParticles(const CVehicleParams& table);
	void                             InitModification(const CVehicleParams& data, tukk modification);
	void                             OnTimer(i32 timerId);
	void                             CheckDisableUpdate(i32 slot);
	void                             ProcessFlipped();

	void                             UpdateStatus(const float deltaTime);
	void                             UpdateNetwork(const float deltaTime);
	void                             ReplayPredictionHistory(const float remainderTime);
	void                             SetDestroyedStatus(bool isDestroyed) { m_isDestroyed = isDestroyed; }
	void                             DoRequestedPhysicalization();
	void                             DeleteActionController();

	const CVehicleSeatActionWeapons* GetCurrentSeatActionWeapons(EntityId passengerId, bool secondary) const;

#if ENABLE_VEHICLE_DEBUG
	void DebugDraw(const float frameTime);
	bool IsDebugDrawing();
	void DebugDrawClientPredict();
	void TestClientPrediction(const float deltaTime);
#endif

	void         CreateAIHidespots();

	u32 GetPartChildrenCount(IVehiclePart* pParentPart);

	void         KillPassengersInExposedSeats(bool includePlayers);

	void         Physicalize();

	std::vector<IParticleEffect*> m_pParticleEffects;

	IVehicleSystem*               m_pVehicleSystem;
	IInventory*                   m_pInventory;

	SEntityPhysicalizeParams      m_physicsParams;

	IVehicleMovement*             m_pMovement;
	SVehicleStatus                m_status;

	EntityId                      m_ownerId;

	i32                           m_lastFrameId;

	TVehicleSeatVector            m_seats;
	TVehicleSeatGroupVector       m_seatGroups;
	TTransitionInfo               m_transitionInfo;

	TVehicleComponentVector       m_components;
	TVehiclePartVector            m_parts;

	typedef std::map<string, CVehicleHelper*> TVehicleHelperMap;
	TVehicleHelperMap m_helpers;

	typedef std::map<i32, IVehicleObject*> TVehicleTimerMap;
	TVehicleTimerMap             m_timers;

	pe_params_buoyancy           m_buoyancyParams;
	pe_simulation_params         m_simParams;
	pe_params_flags              m_paramsFlags;

	float                        m_mass;
	i32                          m_wheelCount;

	CParticleParams              m_particleParams;
	TVehicleEventListenerSet     m_eventListeners;
	TVehicleActionVector         m_actions;
	TVehicleAnimationsVector     m_animations;
	TVehicleObjectUpdateInfoList m_objectsToUpdate;

	EntityId                     m_lastWeaponId;

	IEntityAudioComponent*         m_pIEntityAudioComponent;

	typedef std::vector<SVehicleSoundInfo> TVehicleSoundEvents;
	TVehicleSoundEvents m_soundEvents;

	Vec3                m_gravity;

	EntityId            m_ParentId;

#if ENABLE_VEHICLE_DEBUG
	i32 m_debugIndex;
#endif

	Vec3                  m_initialposition;
	string                m_modifications;
	string                m_paintName;
	_smart_ptr<IMaterial> m_pPaintMaterial;
	_smart_ptr<IMaterial> m_pDestroyedMaterial;
	//tSoundID m_abandonedSoundId;

	float  m_damageMax;
	float  m_majorComponentDamageMax;
	float  m_unmannedflippedThresholdDot;

	string m_actionMap; // Move to shared params.

	typedef std::set<IVehiclePart*> TVehiclePartSet;
	TVehiclePartSet           m_physicalizeParts;
	TVehiclePredictionHistory m_predictionHistory;
#if ENABLE_VEHICLE_DEBUG
	typedef std::vector<SDebugClientPredictData> TDebugClientPredictData;
	TDebugClientPredictData    m_debugClientPredictData;
	DrxCriticalSection         m_debugDrawLock;
#endif
	float                      m_smoothedPing;
	QuatT                      m_clientSmoothedPosition;
	QuatT                      m_clientPositionError;

	CVehicleAnimationComponent m_vehicleAnimation;

	float                      m_collisionDisabledTime;

	bool                       m_customEngineSlot;
	bool                       m_engineSlotBySpeed;
	bool                       m_bNeedsUpdate;
	bool                       m_bRetainGravity;
	bool                       m_usesVehicleActionToEnter;
	bool                       m_indestructible;
	bool                       m_isDestroyed;
	bool                       m_isDestroyable;
	bool                       m_bCanBeAbandoned;
	bool                       m_hasAuthority;

	float                      m_physUpdateTime;

	/*
	   struct SSharedParams : public ISharedParams
	   {
	   };

	   typedef drxshared_ptr<const SSharedParams> SharedParamsPtr;

	   SharedParamsPtr GetSharedParams(const string &name, const CVehicleParams &paramsTable) const;

	   SharedParamsPtr	m_pSharedParams;
	 */

	friend class CScriptBind_Vehicle;
	friend class CVehiclePartBase;
	friend class CVehiclePartSubPartWheel;
	friend class CVehiclePartMassBox;
	friend class CVehicleDamageBehaviorDestroy;
};

#endif // __CVEHICLE_H__
