// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: CReplayActor.h$
$DateTime$
Описание: A replay actor spawned during KillCam replay

-------------------------------------------------------------------------
История:
- 03/19/2010 09:15:00: Created by Martin Sherburn

*************************************************************************/

#ifndef __REPLAYACTOR_H__
#define __REPLAYACTOR_H__

#include <drx3D/Game/ReplayObject.h>
#include <drx3D/Game/GameObjects/GameObject.h>
#include <drx3D/Game/DualCharacterProxy.h>
#include <drx3D/Game/Network/Lobby/SessionNames.h>

enum EReplayActorFlags
{
	eRAF_Dead																	= BIT(0),
	eRAF_FirstPerson													= BIT(1),
	eRAF_IsSquadMember												= BIT(2),
	eRAF_Invisible														= BIT(3),
	eRAF_LocalClient													= BIT(4),
	eRAF_HaveSpawnedMyCorpse									= BIT(5),
	eRAF_StealthKilling												= BIT(6),
};

struct SBasicReplayMovementParams
{
	SBasicReplayMovementParams()
		: m_desiredStrafeSmoothQTX(ZERO)
		, m_desiredStrafeSmoothRateQTX(ZERO)
		, m_fDesiredMoveSpeedSmoothQTX(0.f)
		, m_fDesiredMoveSpeedSmoothRateQTX(0.f)
		, m_fDesiredTurnSpeedSmoothQTX(0.f)
		, m_fDesiredTurnSpeedSmoothRateQTX(0.f)
	{
	}

	void SetDesiredLocalLocation2( ISkeletonAnim* pSkeletonAnim, const QuatT& desiredLocalLocation, float lookaheadTime, float fDeltaTime, float turnSpeedMultiplier);

	Vec2 m_desiredStrafeSmoothQTX;
	Vec2 m_desiredStrafeSmoothRateQTX;
	f32 m_fDesiredMoveSpeedSmoothQTX;
	f32 m_fDesiredMoveSpeedSmoothRateQTX;
	f32 m_fDesiredTurnSpeedSmoothQTX;
	f32 m_fDesiredTurnSpeedSmoothRateQTX;
};

class CReplayActor : public CReplayObject
{
public:

	typedef DrxFixedStringT<DISPLAY_NAME_LENGTH> TPlayerDisplayName;
	static i32k ShadowCharacterSlot = 5;

	CReplayActor();
	virtual ~CReplayActor();

	// IEntityEvent
	virtual	void ProcessEvent( SEntityEvent &event );
	// ~IEntityEvent

	virtual void PostInit(IGameObject *pGameObject);
	virtual void Release();

	void OnRemove();

	void SetupActionController(tukk controllerDef, tukk animDB1P, tukk animDB3P);
	class IActionController *GetActionController() 
	{
		return m_pActionController;
	}

	EntityId GetGunId() const { return m_gunId; }
	void SetGunId(EntityId gunId);
	void AddItem( const EntityId itemId );

	bool IsOnGround() const { return m_onGround; }
	void SetOnGround(bool onGround) { m_onGround = onGround; }

	Vec3 GetVelocity() const { return m_velocity; }
	void SetVelocity(const Vec3& velocity) {m_velocity = velocity;}

	i32 GetHealthPercentage() const { return m_healthPercentage; }
	void SetHealthPercentage(i32 health) { m_healthPercentage = health; }

	i32 GetRank() const { return m_rank; }
	void SetRank(i32 rank) { m_rank = rank; }

	i32 GetReincarnations() const { return m_reincarnations; }
	void SetReincarnations(i32 reincarnations) { m_reincarnations = reincarnations; }

	int8 GetTeam() const { return m_team; }
	void SetTeam(int8 team, const bool isFriendly);
	ILINE bool IsFriendly ( ) const { return m_isFriendly; }

	const TPlayerDisplayName& GetName() const { return m_name; }
	void SetName(const TPlayerDisplayName& name) { m_name = name; }

	void SetFirstPerson(bool firstperson);
	bool IsThirdPerson() { return (m_flags & eRAF_FirstPerson) == 0; }

	bool GetIsOnClientsSquad() const { return ((m_flags&eRAF_IsSquadMember)!=0); }
	void SetIsOnClientsSquad(const bool isOnClientsSquad ) { isOnClientsSquad ? m_flags |= eRAF_IsSquadMember : m_flags &= (~eRAF_IsSquadMember); }

	bool IsClient() const { return ((m_flags&eRAF_LocalClient)!=0); }
	void SetIsClient(const bool isClient ) { isClient ? m_flags |= eRAF_LocalClient : m_flags &= (~eRAF_LocalClient); }

	const QuatT& GetHeadPos() const { return m_headPos; }
	const QuatT& GetCameraTran() const { return m_cameraPos; }

	void LoadCharacter(tukk filename);

	void PlayAnimation(i32 animId, const DrxCharAnimationParams& Params, float speedMultiplier, float animTime);
	void PlayUpperAnimation(i32 animId, const DrxCharAnimationParams& Params, float speedMultiplier, float animTime);

	void ApplyMannequinEvent(const struct SMannHistoryItem &mannEvent, float animTime);

	void Ragdollize();
	void ApplyRagdollImpulse( pe_action_impulse& impulse );
	void TransitionToCorpse(IEntity& corpse);

	ICharacterInstance *GetShadowCharacter() const { return GetEntity()->GetCharacter(ShadowCharacterSlot); }

	ILINE SBasicReplayMovementParams& GetBasicMovement() { return m_basicMovement; }

	static CReplayActor *GetReplayActor(IEntity* pEntity)
	{
		if (pEntity)
		{
			if (CGameObject * pGameObject = (CGameObject*)pEntity->GetProxy(ENTITY_PROXY_USER))
			{
				return (CReplayActor*)pGameObject->QueryExtension("ReplayActor");
			}
		}
		return NULL;
	}

	u16 m_flags;		// EReplayActorFlags

	Vec3 m_trackerPreviousPos;
	float m_trackerUpdateTime;

	// These are used for HUD player name tags
	Vec3 m_drawPos;
	float m_size;

private:
	bool GetAdjustedLayerTime(ICharacterInstance* pCharacterInstance, i32 animId, const DrxCharAnimationParams& params, float speedMultiplier, float animTime, float &adjustedLayerTime);
	void UpdateCharacter();
	void UpdateScopeContexts();
	void RemoveGun( const bool bRemoveEntity );
	void OnRagdollized();
	void Physicalize();

	CAnimationProxyDualCharacter m_animationProxy;
	CAnimationProxyDualCharacterUpper m_animationProxyUpper;
	CReplayItemList m_itemList;

	TPlayerDisplayName m_name;
	
	pe_action_impulse m_ragdollImpulse;

	QuatT m_headPos;
	QuatT m_cameraPos;

	Vec3 m_velocity;

	EntityId m_gunId;

	struct SAnimationContext *m_pAnimContext;
	class IActionController *m_pActionController;
	const class IAnimationDatabase *m_pAnimDatabase1P;
	const class IAnimationDatabase *m_pAnimDatabase3P;

	i32 m_lastFrameUpdated;
	i32 m_healthPercentage;
	i32 m_rank;
	i32 m_reincarnations;
	i16 m_headBoneID;
	i16 m_cameraBoneID;
	int8 m_team;
	bool m_onGround;
	bool m_isFriendly;

	SBasicReplayMovementParams m_basicMovement;

	struct GunRemovalListener : public IEntityEventListener
	{
		GunRemovalListener() : pReplayActor(NULL) {}
		virtual ~GunRemovalListener(){}
		virtual void OnEntityEvent( IEntity *pEntity,SEntityEvent &event );

		CReplayActor * pReplayActor;
	} m_GunRemovalListener;
};

#endif //!__REPLAYACTOR_H__
