// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Простая реализация Actor

 -------------------------------------------------------------------------
  История:
  - 7:10:2004   14:46 : Created by Márcio Martins

*************************************************************************/
#ifndef __Actor_H__
#define __Actor_H__

#if _MSC_VER > 1000
# pragma once
#endif


//#include <drx3D/Game/Game.h> // for stance enum
#include <drx3D/AI/IAgent.h> // for stance enum
#include <drx3D/AI/IAIActorProxy.h> // for stance enum
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Entity/IEntityComponent.h>

#include <drx3D/Game/ActorDamageEffectController.h>
#include <drx3D/AI/IAIObject.h>
#include <drx3D/Game/ActorTelemetry.h>
#include <drx3D/Game/ActorDefinitions.h>
#include <drx3D/Game/ActorLuaCache.h>

#include <drx3D/Game/BodyDefinitions.h>

#include <drx3D/Game/Health.h>
#include <drx3D/Act/IDrxMannequinDefs.h>

#include <drx3D/Schema2/IEnvRegistry.h>

#define ITEM_SWITCH_TIMER_ID	525
#define REFILL_AMMO_TIMER_ID	526
#define ITEM_SWITCH_THIS_FRAME	527
#define RECYCLE_AI_ACTOR_TIMER_ID 528

struct SHitImpulse;
struct SAutoaimTargetRegisterParams;
class CItem;
class CWeapon;
class CActor;
DECLARE_SHARED_POINTERS(CActor);
class CActorImpulseHandler;
DECLARE_SHARED_POINTERS(CActorImpulseHandler);
class CProceduralContextRagdoll;

namespace PlayerActor
{
	namespace Stumble
	{
		struct StumbleParameters;
	}
};

class CActor :
	public CGameObjectExtensionHelper<CActor, IActor, 40>,
	public IGameObjectView,
	public IGameObjectProfileUpr
{
	friend class CStatsRecordingMgr;

protected:

	enum EActorClass
	{
		eActorClass_Actor = 0,
		eActorClass_Player
	};


public:

	struct ItemIdParam
	{
		ItemIdParam(): itemId(0), pickOnlyAmmo(false), select(true) {};
		ItemIdParam(EntityId item): itemId(item), pickOnlyAmmo(false), select(true) {};
		ItemIdParam(EntityId item, bool onlyAmmo): itemId(item), pickOnlyAmmo(onlyAmmo), select(true) {};
		ItemIdParam(EntityId item, bool onlyAmmo, bool inSelect): itemId(item), pickOnlyAmmo(onlyAmmo), select(inSelect) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("itemId", itemId, 'eid');
			ser.Value("pickOnlyAmmo", pickOnlyAmmo, 'bool');
			ser.Value("select", select, 'bool');
		}
		EntityId itemId;
		bool pickOnlyAmmo;
		bool select;
	};

	struct ExchangeItemParams
	{
		ExchangeItemParams() : dropItemId(0), pickUpItemId(0) {};
		ExchangeItemParams(EntityId drop, EntityId pickup) : dropItemId(drop), pickUpItemId(pickup) {};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("dropId", dropItemId, 'eid');
			ser.Value("pickId", pickUpItemId, 'eid');
		}

		EntityId dropItemId;
		EntityId pickUpItemId;
	};

	struct DropItemParams
	{
		DropItemParams(): itemId(0), selectNext(true), byDeath(false) {};
		DropItemParams(EntityId item, bool next=true, bool death=false): itemId(item), selectNext(next), byDeath(death) {};

		void SerializeWith(TSerialize ser)
		{
			ser.Value("itemId", itemId, 'eid');
			ser.Value("selectNext", selectNext, 'bool');
			ser.Value("byDeath", byDeath, 'bool');
		}

		EntityId itemId;
		bool selectNext;
		bool byDeath;
	};

	struct ReviveParams
	{
		ReviveParams(): teamId(0), spawnPointIdx(0), physCounter(0), modelIndex(MP_MODEL_INDEX_DEFAULT) {};
		ReviveParams(u8 tId, u16k idx, u8 counter, u8 _modelIndex): spawnPointIdx(idx), teamId(tId), physCounter(counter), modelIndex(_modelIndex) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("teamId", teamId, 'team');
			ser.Value("spawnPointId", spawnPointIdx, 'ui9');
			ser.Value("physCounter", physCounter, 'ui2');
			ser.Value("modelIndex", modelIndex, MP_MODEL_INDEX_NET_POLICY);
		};

		i32	teamId;
		u16 spawnPointIdx;
		u8 physCounter;
		u8 modelIndex;
	};

	struct KillParams
	{
		KillParams()
		: shooterId(0),
			targetId(0),
			weaponId(0),
			projectileId(0),
			itemIdToDrop(0),
			weaponClassId(0),
			damage(0.0f),
			material(0),
			hit_type(0),
			hit_joint(0),
			projectileClassId(~u16(0)),
			penetration(0),
			impulseScale(0),
			dir(ZERO),
#if USE_LAGOMETER
			lagOMeterHitId(0),
#endif
			ragdoll(false),
			winningKill(false),
			firstKill(false),
			bulletTimeReplay(false),
			fromSerialize(false),
			killViaProxy(false),
			forceLocalKill(false),
			targetTeam(0)
		{}

		explicit KillParams( const HitInfo& hitInfo )
		:	shooterId					(hitInfo.shooterId),
			targetId					(hitInfo.targetId),
			weaponId					(hitInfo.weaponId),
			projectileId			(hitInfo.projectileId),
			itemIdToDrop			(-1),
			weaponClassId			(hitInfo.weaponClassId),
			damage						(hitInfo.damage),
			impulseScale			(hitInfo.impulseScale),
			dir								(hitInfo.dir),
			material					(-1),
			hit_type					(hitInfo.type),
			targetTeam				(0),
			hit_joint					(hitInfo.partId),
			projectileClassId	(hitInfo.projectileClassId),
			penetration				(hitInfo.penetrationCount),
#if USE_LAGOMETER
			lagOMeterHitId		(0),
#endif
			ragdoll						(false),
			winningKill				(false),
			firstKill					(false),
			bulletTimeReplay	(false),
			killViaProxy			(hitInfo.hitViaProxy),
			forceLocalKill		(hitInfo.forceLocalKill),
			fromSerialize			(false)
		{}


		EntityId shooterId;
		EntityId targetId;
		EntityId weaponId;
		EntityId projectileId;
		EntityId itemIdToDrop;
		i32 weaponClassId;
		float damage;
		float impulseScale;
		Vec3 dir;
		i32 material;
		i32 hit_type;
		i32 targetTeam;
		u16 hit_joint;
		u16 projectileClassId;
		u8 penetration;
#if USE_LAGOMETER
		u8 lagOMeterHitId;
#endif
		bool ragdoll;
		bool winningKill;
		bool firstKill;
		bool bulletTimeReplay;
		bool killViaProxy;
		bool forceLocalKill; // Not serialised. skips prohibit death reaction checks.

		// Special case - used when actor is killed from FullSerialize to supress certain logic from running
		bool fromSerialize;

		void SerializeWith(TSerialize ser)
		{
			ser.Value("shooterId", shooterId, 'eid');
			ser.Value("targetId", targetId, 'eid');
			ser.Value("weaponId", weaponId, 'eid');
			ser.Value("projectileId", projectileId, 'eid');
			ser.Value("itemIdToDrop", itemIdToDrop , 'eid');
			ser.Value("weaponClassId", weaponClassId, 'clas');
			ser.Value("damage", damage, 'dmg');
			ser.Value("material", material, 'mat');
			ser.Value("hit_type", hit_type, 'hTyp');
			ser.Value("hit_joint", hit_joint, 'u16');
			ser.Value("projectileClassId", projectileClassId, 'u16');
			ser.Value("penetration", penetration, 'u8');
			ser.Value("dir", dir, 'dir1');
			ser.Value("impulseScale", impulseScale, 'impS');
			ser.Value("ragdoll", ragdoll, 'bool');
#if USE_LAGOMETER
			ser.Value("lagOMeterHitId", lagOMeterHitId, 'ui4');
#endif
			ser.Value("winningKill", winningKill, 'bool');
			ser.Value("firstKill", firstKill, 'bool');
			ser.Value("bulletTimeReplay", bulletTimeReplay, 'bool');
			ser.Value("proxyKill", killViaProxy, 'bool');
			ser.Value("targetTeam", targetTeam, 'team');
		};
	};
	struct MoveParams
	{
		MoveParams() {};
		MoveParams(const Vec3 &p, const Quat &q): pos(p), rot(q) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("pos", pos, 'wrld');
			ser.Value("rot", rot, 'ori1');
		}
		Vec3 pos;
		Quat rot;
	};

	struct PickItemParams
	{
		PickItemParams(): itemId(0), select(false), sound(false), pickOnlyAmmo(false) {};
		PickItemParams(EntityId item, bool slct, bool snd): itemId(item), select(slct), sound(snd), pickOnlyAmmo(false) {};
		PickItemParams(EntityId item, bool slct, bool snd, bool onlyAmmo): itemId(item), select(slct), sound(snd), pickOnlyAmmo(onlyAmmo) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("itemId", itemId, 'eid');
			ser.Value("select", select, 'bool');
			ser.Value("sound", sound, 'bool');
			ser.Value("pickOnlyAmmo", pickOnlyAmmo, 'bool');
		}

		EntityId	itemId;
		bool			select;
		bool			sound;
		bool      pickOnlyAmmo;
	};

	struct NoParams
	{
		void SerializeWith(const TSerialize& ser) {};
	};


	struct KillCamFPData
	{
		static i32k DATASIZE = 50;
		static i32k UNIQPACKETIDS = 16;
		u8 m_data[DATASIZE];
		u16 m_size;
		EntityId m_victim;
		u8 m_numPacket;
		u8 m_packetId;
		u8 m_packetType;
		bool m_bFinalPacket;
		bool m_bToEveryone;

		KillCamFPData() { }
		KillCamFPData(u8 packetType, u8 packetId, u8 numPacket, EntityId victim, u32 size, uk buffer, bool bLastPacket, bool bToEveryone)
		{
			m_packetType=packetType;
			m_packetId=packetId;
			m_numPacket=numPacket;
			m_victim=victim;
			m_bFinalPacket=bLastPacket;
			m_bToEveryone=bToEveryone;
			if (size>sizeof(m_data))
			{
				DrxFatalError("Trying to send packet of size %d bytes when maximum allowed is %" PRISIZE_T " bytes\n", size, sizeof(m_data));
				size=sizeof(m_data);
			}
			m_size=size;
			memcpy(m_data, buffer, size);
		}

		void SerializeWith(TSerialize ser)
		{
			ser.Value("packetType", m_packetType, 'ui2'); // 0-3
			ser.Value("packetId", m_packetId, 'ui4');     // 0-15
			ser.Value("numPacket", m_numPacket, 'u8');   // 0-255 (PacketSize=50bytes so 0-12.4Kb)
			ser.Value("victim", m_victim, 'eid');					// u16?
			ser.Value("dataSize", m_size, 'ui10');					// 0 - 64
			ser.Value("finalPacket", m_bFinalPacket, 'bool');
			ser.Value("toEveryone", m_bToEveryone, 'bool');
			for (i32 i = 0; i < m_size; ++i)
			{
				char temp[255];
				drx_sprintf(temp, "data%d", i);
				ser.Value(temp, m_data[i], 'u8');
			}
		};
	};

	struct AttachmentsParams
	{
		struct SWeaponAttachment
		{
			u16 m_classId;
			bool m_default;

			SWeaponAttachment()
			{
				m_classId = ~u16(0);
				m_default = false;
			}

			SWeaponAttachment(u16 classId, bool isDefault)
			{
				m_classId = classId;
				m_default = isDefault;
			}

			void SerializeWith(TSerialize ser)
			{
				ser.Value("classId", m_classId, 'clas');
				ser.Value("default", m_default, 'bool');
			}
		};

		AttachmentsParams()
		{
			m_loadoutIdx = 0;
		}

		#define MAX_WEAPON_ATTACHMENTS 31
		DrxFixedArray<SWeaponAttachment, MAX_WEAPON_ATTACHMENTS> m_attachments;
		u8																											m_loadoutIdx;

		void SerializeWith(TSerialize ser)
		{
			i32 numAttachments = m_attachments.size();
			bool isReading = ser.IsReading();

			ser.Value("loadoutIdx", m_loadoutIdx, 'ui4' );
			ser.Value("numAttachments", numAttachments, 'ui5'); // 0-31, needs to be kept in sync with MAX_WEAPON_ATTACHMENTS
			for(i32 i=0; i < numAttachments; i++)
			{
				if(!isReading)
				{
					m_attachments[i].SerializeWith(ser);
				}
				else
				{
					SWeaponAttachment data;
					data.SerializeWith(ser);
					m_attachments.push_back(data);
				}
			}
		}
	};

	struct SBlendRagdollParams
	{
		SBlendRagdollParams()
			: m_blendInTagState(TAG_STATE_EMPTY)
			, m_blendOutTagState(TAG_STATE_EMPTY)
		{
		}

		TagState m_blendInTagState;
		TagState m_blendOutTagState;
	};

	AUTOENUM_BUILDENUMWITHTYPE(EReasonForRevive, ReasonForReviveList);

	enum EActorSpectatorState
	{
		eASS_None = 0,						// Just joined - fixed spectating, no actor.
		eASS_Ingame,							// Ingame, dead ready to respawn, with actor.
		eASS_ForcedEquipmentChange,	// Has actor, showing the equipment screen to choose new loadout, i.e. between rounds or switching race
		eASS_SpectatorMode,				// Is currently viewing the game as a spectator - has an actor, but will not respawn unless they leave this mode
	};

	enum EActorSpectatorMode
	{
		eASM_None = 0,												// normal, non-spectating

		eASM_FirstMPMode,
		eASM_Fixed = eASM_FirstMPMode,				// fixed position camera
		eASM_Free,														// free roaming, no collisions
		eASM_Follow,													// follows an entity in 3rd person
		eASM_Killer,													// Front view of the killer in 3rdperson.
		eASM_LastMPMode = eASM_Killer,

		eASM_Cutscene,												// HUDInterfaceEffects.cpp sets this
	};

	DECLARE_SERVER_RMI_NOATTACH(SvRequestDropItem, DropItemParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvRequestPickUpItem, ItemIdParam, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_NOATTACH(SvRequestExchangeItem, ExchangeItemParams, eNRT_ReliableOrdered);
	DECLARE_SERVER_RMI_URGENT(SvRequestUseItem, ItemIdParam, eNRT_ReliableOrdered);
	// cannot be _FAST - see comment on InvokeRMIWithDependentObject
	DECLARE_CLIENT_RMI_NOATTACH(ClPickUp, PickItemParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClClearInventory, NoParams, eNRT_ReliableOrdered);

	DECLARE_CLIENT_RMI_NOATTACH(ClDrop, DropItemParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClStartUse, ItemIdParam, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClStopUse, ItemIdParam, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClUseRequestProcessed, NoParams, eNRT_ReliableOrdered);

	//virtual void SendRevive(const Vec3& position, const Quat& orientation, i32 team, bool clearInventory);
	DECLARE_CLIENT_RMI_NOATTACH(ClRevive, ReviveParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClSimpleKill, NoParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_URGENT(ClKill, KillParams, eNRT_ReliableOrdered);
	DECLARE_CLIENT_RMI_NOATTACH(ClMoveTo, MoveParams, eNRT_ReliableOrdered);

	DECLARE_SERVER_RMI_NOATTACH(SvKillFPCamData, KillCamFPData, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH(ClKillFPCamData, KillCamFPData, eNRT_ReliableUnordered);

	DECLARE_CLIENT_RMI_NOATTACH(ClAssignWeaponAttachments, AttachmentsParams, eNRT_ReliableOrdered);

	CItem *GetItem(EntityId itemId) const;
	CItem *GetItemByClass(IEntityClass* pClass) const;
	CWeapon *GetWeapon(EntityId itemId) const;
	CWeapon *GetWeaponByClass(IEntityClass* pClass) const;

	void HideLeftHandObject(bool inHide);

	EntityId ComputeNextItem(i32k startSlot, i32k category, i32k delta, bool& inOutKeepHistory, IItem* pCurrentItem, const bool currWeaponExplosive) const;

	virtual void SelectNextItem(i32 direction, bool keepHistory, i32 category=0);
	virtual void SwitchToWeaponWithAccessoryFireMode();
	virtual void HolsterItem(bool holster, bool playSelect = true, float selectSpeedBias = 1.0f, bool hideLeftHandObject = true);
	virtual void SelectLastItem(bool keepHistory, bool forceNext = false);
	virtual void SelectItemByName(tukk name, bool keepHistory, bool forceFastSelect=false);
	virtual void SelectItem(EntityId itemId, bool keepHistory, bool forceSelect);
	virtual bool ScheduleItemSwitch(EntityId itemId, bool keepHistory, i32 category = 0, bool forceFastSelect=false);
	ILINE void CancelScheduledSwitch()
	{
		GetEntity()->KillTimer(ITEM_SWITCH_TIMER_ID);
		GetEntity()->KillTimer(ITEM_SWITCH_THIS_FRAME);
		SActorStats::SItemExchangeStats& exchangeItemStats = GetActorStats()->exchangeItemStats;
		exchangeItemStats.switchingToItemID = 0;
		exchangeItemStats.switchThisFrame = false;
	}
	void ClearItemActionControllers();

	virtual bool UseItem(EntityId itemId);
	virtual bool PickUpItem(EntityId itemId, bool sound, bool select);
	virtual bool DropItem(EntityId itemId, float impulseScale=1.0f, bool selectNext=true, bool byDeath=false);
	virtual void DropAttachedItems();
	void ExchangeItem(CItem* pCurrentItem, CItem* pNewItem);
	void ServerExchangeItem(CItem* pCurrentItem, CItem* pNewItem);
	bool PickUpItemAmmo(EntityId itemId);

	void NetReviveAt(const Vec3 &pos, const Quat &rot, i32 teamId, u8 modelIndex);
	virtual void NetReviveInVehicle(EntityId vehicleId, i32 seatId, i32 teamId);
	virtual void NetSimpleKill();
	virtual void NetKill(const KillParams &killParams);

	void ForceRagdollizeAndApplyImpulse(const HitInfo& hitInfo);

	Vec3 GetWeaponOffsetWithLean(EStance stance, float lean, float peekOver, const Vec3& eyeOffset, const bool useWhileLeanedOffsets = false) const;
	Vec3 GetWeaponOffsetWithLean(CWeapon* pCurrentWeapon, EStance stance, float lean, float peekOver, const Vec3& eyeOffset, const bool useWhileLeanedOffsets = false) const;
	Vec3 GetWeaponOffsetWithLeanForAI(CWeapon* pCurrentWeapon, EStance stance, float lean, float peekOver, const Vec3& eyeOffset, const bool useWhileLeanedOffsets = false) const;

	virtual bool CanRagDollize() const;

	virtual bool IsStillWaitingOnServerUseResponse() const {return m_bAwaitingServerUseResponse;};
	virtual void SetStillWaitingOnServerUseResponse(bool waiting);
	void UpdateServerResponseTimeOut( const float frameTime );

	void OnHostMigrationCompleted();

public:
	CActor();
	virtual ~CActor();

	// IEntityEvent
	virtual	void ProcessEvent( SEntityEvent &event );
	virtual ComponentEventPriority GetEventPriority( i32k eventID ) const;
	// ~IEntityEvent

	// IActor
	virtual void Release();
	virtual void ResetAnimationState();
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags );
	virtual void PostSerialize();
	virtual void SetChannelId(u16 id);
	virtual void  SerializeLevelToLevel( TSerialize &ser );
	virtual IInventory* GetInventory() const;
	virtual void NotifyCurrentItemChanged(IItem* newItem) {};

	virtual bool IsPlayer() const;
	virtual bool IsClient() const;
	virtual bool IsMigrating() const { return m_isMigrating; }
	virtual void SetMigrating(bool isMigrating) { m_isMigrating = isMigrating; }

	virtual bool Init( IGameObject * pGameObject );
	virtual void InitClient( i32 channelId );
	virtual void PostInit( IGameObject * pGameObject );
	virtual void PostInitClient(i32 channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Update(SEntityUpdateContext& ctx, i32 updateSlot);
	virtual void UpdateView(SViewParams &viewParams) {};
	virtual void PostUpdateView(SViewParams &viewParams) {};
	void UpdateBodyDestruction(float frameTime);
	virtual void ReadDataFromXML(bool isReloading = false);

	virtual void InitLocalPlayer() {};

	virtual void SetIKPos(tukk pLimbName, const Vec3& goalPos, i32 priority);

	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void PostUpdate(float frameTime);
	virtual void PostRemoteSpawn() {};

	virtual bool IsThirdPerson() const { return true; };
	virtual void ToggleThirdPerson(){}


	virtual void RequestFacialExpression(tukk pExpressionName /* = NULL */, f32* sequenceLength = NULL);
	virtual void PrecacheFacialExpression(tukk pExpressionName);

	virtual void NotifyInventoryAmmoChange(IEntityClass* pAmmoClass, i32 amount);
	virtual EntityId	GetGrabbedEntityId() const { return 0; }

	virtual void HideAllAttachments(bool isHiding);

	virtual void OnAIProxyEnabled(bool enabled);
	virtual void OnReturnedToPool() {};
	virtual void OnPreparedFromPool() {};

	virtual void OnReused(IEntity *pEntity, SEntitySpawnParams &params);
	// ~IActor

	// IGameObjectProfileUpr
	virtual bool SetAspectProfile( EEntityAspects aspect, u8 profile );
	virtual u8 GetDefaultProfile( EEntityAspects aspect ) { return aspect == eEA_Physics? eAP_NotPhysicalized : 0; }
	// ~IGameObjectProfileUpr

	// IActionListener
	virtual void OnAction(const ActionId& actionId, i32 activationMode, float value);
	// ~IActionListener

	virtual void AddHeatPulse(const float intensity, const float time);

	void SetGrabbedByPlayer(IEntity* pPlayerEntity, bool grabbed);

	//------------------------------------------------------------------------
	ILINE float GetAirControl() const { return m_airControl; };
	ILINE float GetAirResistance() const { return m_airResistance; };
	ILINE float GetInertia() const { return m_inertia; }
	ILINE float GetInertiaAccel() const { return m_inertiaAccel; }
	ILINE float GetTimeImpulseRecover() const { return m_timeImpulseRecover; };

	virtual void SetViewRotation( const Quat &rotation ) {};
	virtual Quat GetViewRotation() const { return GetEntity()->GetRotation(); };
	virtual void EnableTimeDemo( bool bTimeDemo ) {};

	// offset to add to the computed camera angles every frame
	virtual void AddViewAngleOffsetForFrame(const Ang3 &offset);

	//------------------------------------------------------------------------
	virtual void Revive( EReasonForRevive reasonForRevive = kRFR_Spawn );
	virtual void Reset(bool toGame);
	//physicalization
	static bool LoadPhysicsParams(SmartScriptTable pEntityTable, tukk szEntityClassName, SEntityPhysicalizeParams &outPhysicsParams,
		pe_player_dimensions &outPlayerDim, pe_player_dynamics &outPlayerDyn);
	virtual void Physicalize(EStance stance=STANCE_NULL);
	virtual void PostPhysicalize();
	virtual void RagDollize( bool fallAndPlay ) {}
	void ShutDown();
	//
  virtual i32 IsGod(){ return 0; }

	//reset function clearing state and animations for teleported actor
	virtual void OnTeleported() {}

	virtual void SetSpectatorState(u8 state) {}
	virtual EActorSpectatorState GetSpectatorState() const { return eASS_None; }

	virtual float GetSpectatorOrbitYawSpeed() const { return 0.f; }
	virtual void SetSpectatorOrbitYawSpeed(float yawSpeed, bool singleFrame) {}
	virtual bool CanSpectatorOrbitYaw() const { return false; }
	virtual float GetSpectatorOrbitPitchSpeed() const { return 0.f; }
	virtual void SetSpectatorOrbitPitchSpeed(float pitchSpeed, bool singleFrame) {}
	virtual bool CanSpectatorOrbitPitch() const { return false; }
	virtual void ChangeCurrentFollowCameraSettings(bool increment) {}

	virtual void SetSpectatorModeAndOtherEntId(u8k _mode, const EntityId _othEntId, bool isSpawning=false) {};

	virtual u8 GetSpectatorMode() const { return 0; };
	virtual void SetSpectatorTarget(EntityId targetId) {};
	virtual EntityId GetSpectatorTarget() const { return 0; };
	virtual void SetSpectatorFixedLocation(EntityId locId) {};
	virtual EntityId GetSpectatorFixedLocation() const { return 0; };

	//get actor status
	virtual SActorStats *GetActorStats() { return 0; };
	virtual const SActorStats *GetActorStats() const { return 0; };
	SActorParams &GetActorParams() { return m_params; };
	const SActorParams &GetActorParams() const { return m_params; };

	float GetSpeedMultiplier(SActorParams::ESpeedMultiplierReason reason);
	void SetSpeedMultipler(SActorParams::ESpeedMultiplierReason reason, float fSpeedMult);
	void MultSpeedMultiplier(SActorParams::ESpeedMultiplierReason reason, float fSpeedMult);
	void SetStanceMaxSpeed(u32 stance, float fValue);

	virtual void SetStats(SmartScriptTable &rTable);
	virtual ICharacterInstance *GetFPArms(i32 i) const { return GetEntity()->GetCharacter(3+i); };
	//set/get actor params
	static bool LoadGameParams(SmartScriptTable pEntityTable, SActorGameParams &outGameParams);
	static bool LoadDynamicAimPoseElement(CScriptSetGetChain& gameParamsTableChain, tukk szName, string& output);
	void InitGameParams();
	virtual void SetParamsFromLua(SmartScriptTable &rTable);
	//
	virtual void Freeze(bool freeze) {};
	virtual void Fall(Vec3 hitPos = Vec3(0,0,0));
	void Fall(const HitInfo& hitInfo);
	virtual void KnockDown(float backwardsImpulse);

  virtual void SetLookAtTargetId(EntityId targetId, float interpolationTime=1.f);
  virtual void SetForceLookAtTargetId(EntityId targetId, float interpolationTime=1.f);

	virtual void StandUp();
	virtual bool IsFallen() const;
	virtual bool IsDead() const;

	//
	virtual IEntity *LinkToVehicle(EntityId vehicleId);
	virtual void LinkToMountedWeapon(EntityId weaponId) {};
	virtual IEntity *LinkToEntity(EntityId entityId, bool bKeepTransformOnDetach=true);
	virtual void StartInteractiveAction(EntityId entityId, i32 interactionIndex = 0);
	virtual void StartInteractiveActionByName(tukk interaction, bool bUpdateVisibility, float actionSpeed = 1.0f);
	virtual void EndInteractiveAction(EntityId entityId);

	virtual bool	AllowLandingBob() { return true; }

	virtual IEntity *GetLinkedEntity() const
	{
		return m_linkStats.GetLinked();
	}

	virtual IVehicle *GetLinkedVehicle() const
	{
		return m_linkStats.GetLinkedVehicle();
	}

	float GetLookFOV(const SActorParams &actorParams) const
	{
		return GetLinkedVehicle() ? actorParams.lookInVehicleFOVRadians : actorParams.lookFOVRadians;
	}

	u32 GetAimIKLayer(const SActorParams &actorParams) const
	{
		return actorParams.aimIKLayer;
	}

	u32 GetLookIKLayer(const SActorParams &actorParams) const
	{
		return actorParams.lookIKLayer;
	}

	virtual void SetViewInVehicle(Quat viewRotation) {};

	virtual void SupressViewBlending() {};

	ILINE Vec3 GetLBodyCenter()
	{
		const SStanceInfo *pStance(GetStanceInfo(GetStance()));
		return Vec3(0,0,(pStance->viewOffset.z - pStance->heightPivot) * 0.5f);
	}

	ILINE Vec3 GetWBodyCenter()
	{
		return GetEntity()->GetWorldTM() * GetLBodyCenter();
	}

	//for animations
	virtual void PlayAction(tukk action,tukk extension, bool looping=false) {};
	//
	virtual void SetMovementTarget(const Vec3 &position,const Vec3 &looktarget,const Vec3 &up,float speed) {};
	//
	virtual void CreateScriptEvent(tukk event,float value,tukk str = NULL);
	virtual bool CreateCodeEvent(SmartScriptTable &rTable);
	virtual void AnimationEvent(ICharacterInstance *pCharacter, const AnimEventInstance &event);

	virtual void SetTurnAnimationParams(const float turnThresholdAngle, const float turnThresholdTime);

	virtual void CameraShake(float angle,float shift,float duration,float frequency,Vec3 pos,i32 ID,tukk source="") {};
	//
	virtual void SetAngles(const Ang3 &angles) {};
	virtual Ang3 GetAngles() {return Ang3(0,0,0);};
	virtual void AddAngularImpulse(const Ang3 &angular,float deceleration=0.0f,float duration=0.0f){}
	//
	virtual void SetViewLimits(Vec3 dir,float rangeH,float rangeV) {};
	virtual void DamageInfo(EntityId shooterID, EntityId weaponID, IEntityClass *pProjectileClass, float damage, i32 damageType, const Vec3 hitDirection);
	virtual IAnimatedCharacter * GetAnimatedCharacter() { return m_pAnimatedCharacter; }
	virtual const IAnimatedCharacter * GetAnimatedCharacter() const { return m_pAnimatedCharacter; }
	virtual void PlayExactPositioningAnimation( tukk sAnimationName, bool bSignal, const Vec3& vPosition, const Vec3& vDirection, float startWidth, float startArcAngle, float directionTolerance ) {}
	virtual void CancelExactPositioningAnimation() {}
	virtual void PlayAnimation( tukk sAnimationName, bool bSignal ) {}
	virtual EntityId GetCurrentTargetEntityId() const { return 0; }
	virtual const Vec3 * GetCurrentTargetPos() const { return NULL; }

	virtual void  SetMaxHealth( float maxHealth );
	virtual float GetMaxHealth() const { return m_health.GetHealthMax(); }
	virtual void  SetHealth( float health );
	virtual float GetHealth() const { return m_health.GetHealth(); }
	virtual i32   GetHealthAsRoundedPercentage() const { return m_health.GetHealthAsRoundedPercentage(); }
	virtual i32 GetArmor() const { return 0; }
	virtual i32 GetMaxArmor() const { return 0; }
	virtual i32   GetTeamId() const { return m_teamId; }
	virtual void Kill();

	void ImmuneToForbiddenZone(const bool immune);
	const bool ImmuneToForbiddenZone() const;

	void NotifyInventoryAboutOwnerActivation();
	void NotifyInventoryAboutOwnerDeactivation();

	virtual bool IsSwimming() const {	return false; };
	virtual bool IsHeadUnderWater() const { return false; }

	virtual bool IsSprinting() const { return false; }
	virtual bool CanFire() const { return true; }

	//stances
	ILINE EStance GetStance() const
	{
		return m_stance.Value();
	}

	ILINE const SStanceInfo *GetStanceInfo(EStance stance) const
	{
		if (stance < 0 || stance > STANCE_LAST)
			return &m_defaultStance;
		return &m_stances[stance];
	}

	CActorWeakPtr GetWeakPtr() const { return m_pThis; }
	CActorTelemetry *GetTelemetry() { return &m_telemetry; }

	// forces the animation graph to select a state
	void QueueAnimationState( tukk  state );

	//
	ILINE i32 GetBoneID(i32 ID) const
	{
		DRX_ASSERT((ID >= 0) && (ID < BONE_ID_NUM));
		if((ID >= 0) && (ID < BONE_ID_NUM))
		{
			return m_boneIDs[ID];
		}

		return -1;
	}

	ILINE bool HasBoneID(i32 ID) const
	{
		DRX_ASSERT((ID >= 0) && (ID < BONE_ID_NUM));
		return m_boneIDs[ID] >= 0;
	}

	ILINE const QuatT &GetBoneTransform(i32 ID) const
	{
		DRX_ASSERT((ID >= 0) && (ID < BONE_ID_NUM));
		DRX_ASSERT_MESSAGE(m_boneIDs[ID] >= 0, string().Format("Accessing unmapped bone %s in %s", s_BONE_ID_NAME[ID], GetEntity()->GetName()));

		return m_boneTrans[ID];
	}

	virtual Vec3 GetLocalEyePos() const;
	QuatT GetCameraTran() const;
	QuatT GetHUDTran() const;

	ILINE static const SActorAnimationEvents& GetAnimationEventsTable() { return s_animationEventsTable; };

	virtual void UpdateMountedGunController(bool forceIKUpdate);

	virtual void OnPhysicsPreStep(float frameTime){};

	virtual bool CheckInventoryRestrictions(tukk itemClassName);

	//
	void ProcessIKLimbs(float frameTime);

	//IK limbs
	i32 GetIKLimbIndex(tukk limbName);
	ILINE SIKLimb *GetIKLimb(i32 limbIndex)
	{
		return &m_IKLimbs[limbIndex];
	}
	void CreateIKLimb(const SActorIKLimbInfo &limbInfo);

	//
	virtual IMovementController * GetMovementController() const
	{
		return m_pMovementController;
	}

	CDamageEffectController& GetDamageEffectController() { return m_damageEffectController; }
	u8 GetActiveDamageEffects() const { return m_damageEffectController.GetActiveEffects(); }
	u8 GetDamageEffectsResetSwitch() const { return m_damageEffectController.GetEffectResetSwitch(); }
	u8 GetDamageEffectsKilled() const { return m_damageEffectController.GetEffectsKilled(); }
	void SetActiveDamageEffects(u8 active) { m_damageEffectController.SetActiveEffects(active); }
	void SetDamageEffectsResetSwitch(u8 reset) { m_damageEffectController.SetEffectResetSwitch(reset); }
	void SetDamageEffectsKilled(u8 killed) { m_damageEffectController.SetEffectsKilled(killed); }
	CActorImpulseHandlerPtr GetImpulseHander() { return m_pImpulseHandler; }

	//stances
	void OnSetStance( EStance stance );
	virtual void SetStance(EStance stance);
	virtual void OnStanceChanged(EStance newStance, EStance oldStance);
	virtual bool TrySetStance(EStance stance);

	//Cloak material
	enum eFadeRules
	{
		eAllowFades = 0,
		eDisallowFades,
	};
	virtual void SetCloakLayer(bool set, eFadeRules config = eAllowFades);

	IAnimationGraphState * GetAnimationGraphState();
	void SetFacialAlertnessLevel(i32 alertness);

	//weapons
	virtual IItem *GetCurrentItem(bool includeVehicle=false) const;
	EntityId GetCurrentItemId(bool includeVehicle=false) const;
	virtual IItem *GetHolsteredItem() const;
	EntityId GetHolsteredItemId() const;
	void ProceduralRecoil( float duration, float kinematicImpact, float kickIn/*=0.8f*/, i32 arms = 0/*0=both, 1=right, 2=left*/);

	//Net
	EntityId NetGetCurrentItem() const;
	void NetSetCurrentItem(EntityId id, bool forceDeselect);

	EntityId NetGetScheduledItem() const;
	void NetSetScheduledItem(EntityId id);

	virtual void SwitchDemoModeSpectator(bool activate) {};	//this is a player only function

	//Body damage / destruction
	ILINE TBodyDamageProfileId GetCurrentBodyDamageProfileId() const { return (m_OverrideBodyDamageProfileId != INVALID_BODYDAMAGEPROFILEID) ? m_OverrideBodyDamageProfileId : m_DefaultBodyDamageProfileId; }
	void ReloadBodyDestruction();

	const CBodyDestrutibilityInstance& GetBodyDestructibilityInstance() const { return m_bodyDestructionInstance; }

	float GetBodyDamageMultiplier(const HitInfo &hitInfo) const;
	float GetBodyExplosionDamageMultiplier(const HitInfo &hitInfo) const;
	u32 GetBodyDamagePartFlags(i32k partID, i32k materialID) const;
	TBodyDamageProfileId GetBodyDamageProfileID(tukk bodyDamageFileName, tukk bodyDamagePartsFileName) const;
	void OverrideBodyDamageProfileID(const TBodyDamageProfileId profileID);
	bool IsHeadShot(const HitInfo &hitInfo) const;
	bool IsHelmetShot(const HitInfo & hitInfo) const;
	bool IsGroinShot(const HitInfo &hitInfo) const;
	bool IsFootShot(const HitInfo &hitInfo) const;
	bool IsKneeShot(const HitInfo &hitInfo) const;
	bool IsWeakSpotShot(const HitInfo &hitInfo) const;

	void FillHitInfoFromKillParams(const CActor::KillParams& killParams, HitInfo &hitInfo) const;

	void ProcessDestructiblesHit(const HitInfo& hitInfo, const float previousHealth, const float newHealth);
	void ProcessDestructiblesOnExplosion(const HitInfo& hitInfo, const float previousHealth, const float newHealth);

	//misc
	virtual tukk GetActorClassName() const { return "CActor"; };
	const IItemParamsNode* GetActorParamsNode() const;

	static  ActorClass GetActorClassType() { return (ActorClass)eActorClass_Actor; }
	virtual ActorClass GetActorClass() const { return (ActorClass)eActorClass_Actor; };

	virtual tukk GetEntityClassName() const { return GetEntity()->GetClass()->GetName(); }
	const IItemParamsNode* GetEntityClassParamsNode() const;
	static const char DEFAULT_ENTITY_CLASS_NAME[];

	bool IsPoolEntity() const;

	virtual void SetAnimTentacleParams(pe_params_rope& rope, float animBlend) {};

  virtual bool IsCloaked() const { return m_cloakLayerActive; }

  virtual void DumpActorInfo();

	virtual bool IsFriendlyEntity(EntityId entityId, bool bUsingAIIgnorePlayer = true) const;
	virtual float GetReloadSpeedScale() const { return 1.0f; }
	virtual float GetOverchargeDamageScale() const  { return 1.0f; }

	ILINE bool AllowSwitchingItems() { return m_enableSwitchingItems; }
	void EnableSwitchingItems(bool enable);
	void EnableIronSights(bool enable);
	void EnablePickingUpItems(bool enable);

	bool CanUseIronSights() const { return m_enableIronSights; }
	bool CanPickupItems() const { return m_enablePickupItems; }

	virtual void BecomeRemotePlayer();
	virtual bool BecomeAggressiveToAgent(EntityId entityID);

	ILINE u8 GetNetPhysCounter() { return m_netPhysCounter; }

	virtual void GetMemoryUsage( IDrxSizer * pSizer ) const;
	void GetInternalMemoryUsage( IDrxSizer * pSizer ) const;

	static bool LoadFileModelInfo(SmartScriptTable pEntityTable, SmartScriptTable pProperties, SActorFileModelInfo &outFileModelInfo);
	virtual bool SetActorModel(tukk modelName=NULL);
	void UpdateActorModel();
	bool FullyUpdateActorModel();
	void InvalidateCurrentModelName();

	virtual void PrepareLuaCache();

	void LockInteractor(EntityId lockId, bool lock);

	bool AllowPhysicsUpdate(u8 newCounter) const;
	static bool AllowPhysicsUpdate(u8 newCounter, u8 oldCounter);

	virtual bool IsRemote() const;

	void AddLocalHitImpulse(const SHitImpulse& hitImpulse);

	static bool LoadAutoAimParams(SmartScriptTable pEntityTable, SAutoaimTargetRegisterParams &outAutoAimParams);

	float GetLastUnCloakTime(){return m_lastUnCloakTime;}

  virtual void EnableStumbling(PlayerActor::Stumble::StumbleParameters* stumbleParameters) {};
  virtual void DisableStumbling() {};

	tukk GetShadowFileModel();

	void CloakSyncAttachments(bool bFade);
	void CloakSyncEntity(EntityId entityId, bool bFade);

	virtual const float GetCloakBlendSpeedScale();

	EntityId SimpleFindItemIdInCategory(tukk category) const;
	void RegisterInAutoAimUpr();
	void UnRegisterInAutoAimUpr();

	void SetTag(TagID tagId, bool enable);
	void SetTagByCRC(u32 tagId, bool enable);
	ILINE const SActorPhysics& GetActorPhysics() const { return m_actorPhysics; }

	bool IsInMercyTime() const;

	bool CanSwitchSpectatorStatus() const;
	void RequestChangeSpectatorStatus(bool spectate);
	void OnSpectateModeStatusChanged(bool spectate);

	virtual bool ShouldMuteWeaponSoundStimulus() const { return false; }

	void OnFall(const HitInfo& hitInfo);

	void EnableHitReactions() { m_shouldPlayHitReactions = true; }
	void DisableHitReactions() { m_shouldPlayHitReactions = false; }
	bool ShouldPlayHitReactions() const { return m_shouldPlayHitReactions; }

	EntityId GetPendingDropEntityId() const { return m_pendingDropEntityId; }

	// called by script code upon reset to acquire a lip-sync extension from the entity's script table or to release an existing one if none is provided
	void AcquireOrReleaseLipSyncExtension();

protected:

	void GenerateBlendRagdollTags();

	void PhysicalizeLocalPlayerAdditionalParts();

	// SetActorModel helpers
	bool SetActorModelFromScript();
	bool SetActorModelInternal(tukk modelName = NULL);
	bool SetActorModelInternal(const SActorFileModelInfo &fileModelInfo);

	static IAttachment* GetOrCreateAttachment(IAttachmentUpr *pAttachmentUpr, tukk boneName, tukk attachmentName);

	// Helper to sync cloak to given attachment
	void CloakSyncAttachment(IAttachment* pAttachment, bool bFade);

	//movement
	virtual IActorMovementController * CreateMovementController() = 0;
	//

	virtual void InitGameParams(const SActorGameParams &gameParams, const bool reloadCharacterSounds);
	void RebindScript();

	void RegisterInAutoAimUpr(const SAutoaimTargetRegisterParams &autoAimParams);

	void RegisterDBAGroups();
	void UnRegisterDBAGroups();

	CItem * StartRevive(i32 teamId);
	void FinishRevive(CItem * pItem);
	virtual void SetModelIndex(u8 modelIndex) {}

	virtual bool ShouldRegisterAsAutoAimTarget() { return true; }
	void SelectWeaponWithAmmo(EntityId outOfAmmoId, bool keepHistory);

	void AttemptToRecycleAIActor( );

	bool GetRagdollContext( CProceduralContextRagdoll** ppRagdollContext ) const;
	void PhysicalizeBodyDamage();

	bool IsBodyDamageFlag(const HitInfo &hitInfo, EBodyDamagePIDFlags) const;

	void SetupLocalPlayer();

	void UpdateAutoDisablePhys(bool bRagdoll);

	void RequestServerResync()
	{
		if (!IsClient())
			GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameServerDynamic | eEA_GameClientStatic | eEA_GameServerStatic);
	}
	EntityId GetLeftHandObject() const;

	//
	typedef std::vector<SIKLimb> TIKLimbs;

	//
	virtual bool UpdateStance();
	void UpdateLegsColliders();
	void ReleaseLegsColliders();

	static SActorAnimationEvents s_animationEventsTable;

	EntityId m_lastNetItemId;
	bool	m_isClient;
	bool	m_isPlayer;
	bool	m_isMigrating;
	CHealth m_health;

	CActorPtr	m_pThis;

private:
	IInventory * m_pInventory;

protected:
	CCoherentValue<EStance> m_stance;
	EStance m_desiredStance;

	static SStanceInfo m_defaultStance;
	SStanceInfo m_stances[STANCE_LAST];

	SActorParams m_params;

	IAnimatedCharacter *m_pAnimatedCharacter;
	IActorMovementController * m_pMovementController;
	CDamageEffectController m_damageEffectController;
	CActorImpulseHandlerPtr	m_pImpulseHandler;

	static IItemSystem			*m_pItemSystem;
	static IGameFramework		*m_pGameFramework;
	static IGameplayRecorder*m_pGameplayRecorder;

	mutable SLinkStats m_linkStats;

	TIKLimbs m_IKLimbs;

	u8 m_currentPhysProfile;

	float m_airControl;
	float m_airResistance;
	float m_inertia;
	float m_inertiaAccel;
	float m_timeImpulseRecover;

	EntityId m_netLastSelectablePickedUp;
	EntityId m_pendingDropEntityId;

	string m_currModel;
	string m_currShadowModel;

	// Lua cache
	SLuaCache_ActorPhysicsParamsPtr m_LuaCache_PhysicsParams;
	SLuaCache_ActorGameParamsPtr m_LuaCache_GameParams;
	SLuaCache_ActorPropertiesPtr m_LuaCache_Properties;

	float			m_lastUnCloakTime;
	float			m_spectateSwitchTime;
	float			m_fAwaitingServerUseResponse;

	i32				m_teamId;
	bool			m_IsImmuneToForbiddenZone;

	bool m_enableSwitchingItems;
	bool m_enableIronSights;
	bool m_enablePickupItems;
	bool m_cloakLayerActive;

	u8		m_netPhysCounter;				 //	Physics counter, to enable us to throw away old updates

	bool		m_registeredInAutoAimMng;
	bool		m_registeredAnimationDBAs;

	bool		m_bAllowHitImpulses;

	bool		m_bAwaitingServerUseResponse;

	bool		m_shouldPlayHitReactions;

protected:
	i16 m_boneIDs[BONE_ID_NUM];
	QuatT m_boneTrans[BONE_ID_NUM];

	CActorTelemetry m_telemetry;
	SActorPhysics m_actorPhysics;
	SBlendRagdollParams m_blendRagdollParams;

#ifndef _RELEASE
	u8 m_tryToChangeStanceCounter;
#endif

	IPhysicalEntity  *m_pLegsCollider[2],*m_pLegsFrame,*m_pLegsIgnoredCollider;
	i32								m_iboneLeg[2];
	Vec3							m_ptSample[2];
	char							m_bLegActive[2];

private:

	//Body damage/destruction

	// This profile originates from the entity's properties/archetype.
	TBodyDamageProfileId m_DefaultBodyDamageProfileId;
	CBodyDestrutibilityInstance m_bodyDestructionInstance;

	// The override body damage profile ID that can be manipulating through
	// scripting (INVALID_BODYDAMAGEPROFILEID if the default should be used).
	TBodyDamageProfileId m_OverrideBodyDamageProfileId;

	string m_sLipSyncExtensionType;		// represents the type of the lip-sync extension if one was acquired; equals "" if none was acquired
};

#endif //__Actor_H__
