// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Base class for creating additional optional player functionality
**************************************************************************/

#ifndef __PLAYER_PLUGIN_H__
#define __PLAYER_PLUGIN_H__

#include <drx3D/Game/AutoEnum.h>

struct HitInfo;

class CPlayer;
class CProjectile;
class CPlayerPluginEventDistributor;

// Feel free to turn this on and off locally to make sure code compiles with player plug-in debugging both on and off...
// It's recommended that it's LEFT TURNED ON WHEN COMMITTING - please don't submit this file with it set to 0!
// The "#define PLAYER_PLUGIN_DEBUGGING" line below makes sure the debug code doesn't make it into release builds. [TF]
#define ENABLE_PLAYER_PLUGIN_DEBUGGING			(1)

// Should only use "#if PLAYER_PLUGIN_DEBUGGING" in code, never use ENABLE_PLAYER_PLUGIN_DEBUGGING directly!
#if !defined(_RELEASE)
#define PLAYER_PLUGIN_DEBUGGING             ENABLE_PLAYER_PLUGIN_DEBUGGING
#else
//JAT - has to be like this because other bits of code use _RELEASE test and don't use PLAYER_PLUGIN_DEBUGGING test
#define PLAYER_PLUGIN_DEBUGGING             (0)
#endif

#define PlayerPluginEventList(f)           \
	f(EPE_Reset)                             \
	f(EPE_StampMelee)                        \
	f(EPE_Landed)                            \
	f(EPE_Jump)															 \
	f(EPE_PowerJump)                         \
	f(EPE_ChangedSuitMode)                   \
	f(EPE_ChangedSuitState)                  \
	f(EPE_DeploymentFromMap)                 \
	f(EPE_Spawn)                             \
	f(EPE_Die)                               \
	f(EPE_SetTeam)                           \
	f(EPE_ScopeActive)                       \
	f(EPE_OverrideSwayAmount)                \
	f(EPE_AnimationEvent)                    \
	f(EPE_DamageHandlingTarget)              \
	f(EPE_DamageHandlingFeedback)            \
	f(EPE_SvActivateMissileCountermeasures)  \
	f(EPE_FlashbangEnabled)                    \
	f(EPE_PlayerCloaking)                    \
	f(EPE_SetCloak)                          \
	f(EPE_OverrideStealthEnergyParams)       \
	f(EPE_OverrideFootstepSoundScale)        \
	f(EPE_BulletTrail)                       \
	f(EPE_GrenadeLaunched)                   \
	f(EPE_GrenadeDestroyed)                  \
	f(EPE_EnterSwimming)                     \
	f(EPE_MicrowaveBulletHit)								 \
	f(EPE_ItemPickedUp)                      \
	f(EPE_ItemDropped)                       \
	f(EPE_HostMigrationFinished)             \
	f(EPE_ItemStartUse)									     \
	f(EPE_ItemStopUse)											 \
	f(EPE_RequestItemUse)							 			 \
	f(EPE_RoundEnd)													 \
	f(EPE_NewTarget)												 \
	f(EPE_VisorTag)                          \
	f(EPE_InteractiveEntityRegister)				 \
	f(EPE_InteractiveEntityRegisterShoot)		 \
	f(EPE_InteractiveEntityUnregister)			 \
	f(EPE_InteractiveEntitySetEnabled)			 \
	f(EPE_SuitSupercharge)									 \
	f(EPE_ShotFromCloak)										 \
	f(EPE_ItemSelected)											 \
	f(EPE_ItemDeselected)										 \


AUTOENUM_BUILDENUMWITHTYPE_WITHNUM(EPlayerPlugInEvent, PlayerPluginEventList, EPE_Num);

struct STrailInfo
{
	STrailInfo(CProjectile* _pProj, Vec3 _pos)
	{
		pProjectile = _pProj;
		pos = _pos;
	}
	CProjectile* pProjectile;
	Vec3 pos;
};

struct SDeploymentFromMap
{
	SDeploymentFromMap() {}
	SDeploymentFromMap(Vec3 pos, Vec2 dir)
	{
		m_pos = pos;
		m_dir = dir;
	}

	Vec3 m_pos;
	Vec2 m_dir;
};

//---------------------------------------------------------
enum EPlayerPlugInData
{
	EPD_Stamp,
	EPD_MuteFootsteps,
	EPD_MuteJumping,
	EPD_ProximityDistance,
	EPD_PhantomTimeToKill,
	EPD_SilentFeetRange,
	EPD_ECMRange,
	EPD_CloakAwarenessTimeFromUnCloak,
	EPD_ElectronicInterferenceAmount,
	EPD_CounterMeasuresEffectAmount,
	EPD_SuitBoostActive,
	EPD_IgnoreRadarJammed,
	EPD_GetCloakBlendSpeedScale,
	EPD_GetCloakTimeDelay,
	EPD_SuperChargeTime,
	EPD_SuperChargesCounter,
};

//---------------------------------------------------------
// NB: The uk  data param in HandleEvent points to an instance of this struct when EPlayerPlugInEvent theEvent == EPE_StampMelee etc.
struct SOnActionData
{
	SOnActionData(i32 mode)
	{
		activationMode = mode;
		handled = false;
	}

	i32 activationMode;
	bool handled;
};

struct SDamageHandling
{
	SDamageHandling(const HitInfo *hit, float dm)
	{
		pHitInfo = hit;
		damageMultiplier = dm;
	}
	float damageMultiplier;
	const HitInfo *pHitInfo;
};

#if PLAYER_PLUGIN_DEBUGGING
#define CVAR_IS_ON(n)                 (g_pGameCVars && g_pGameCVars->n)
#define SET_PLAYER_PLUGIN_NAME(name)  tukk  DbgGetPluginName() const { return # name; } ILINE name * CheckCorrectNameGiven_ ## name () { return this; }
#define PlayerPluginLog(...)          do { if (CVAR_IS_ON(pl_debug_log_player_plugins)) DrxLogAlways ("[PLAYER PLUG-IN] <%s %s %s \"%s\"> %s", DbgGetClassDetails().c_str(), m_ownerPlayer->IsClient() ? "Local" : "Remote", m_ownerPlayer->GetEntity()->GetClass()->GetName(), m_ownerPlayer->GetEntity()->GetName(), string().Format(__VA_ARGS__).c_str()); } while(0)
#define PlayerPluginWatch(...)        (CVAR_IS_ON(g_displayDbgText_plugins) && DrxWatch ("[PLAYER PLUG-IN] <%s %s %s \"%s\"> %s", DbgGetClassDetails().c_str(), m_ownerPlayer->IsClient() ? "Local" : "Remote", m_ownerPlayer->GetEntity()->GetClass()->GetName(), m_ownerPlayer->GetEntity()->GetName(), string().Format(__VA_ARGS__).c_str()))
#define PlayerPluginAssert(cond, ...) DRX_ASSERT_TRACE(cond, ("[PLAYER PLUG-IN] <%s %s %s \"%s\"> %s", DbgGetClassDetails().c_str(), m_ownerPlayer->IsClient() ? "Local" : "Remote", m_ownerPlayer->GetEntity()->GetClass()->GetName(), m_ownerPlayer->GetEntity()->GetName(), string().Format(__VA_ARGS__).c_str()))
#define PLAYER_PLUGIN_DETAILS(plugin) plugin ? plugin->DbgGetClassDetails().c_str() : "NULL"
#else
#define SET_PLAYER_PLUGIN_NAME(name)
#define PlayerPluginLog(...)          (void)0
#define PlayerPluginWatch(...)        (void)0
#define PlayerPluginAssert(cond, ...) DRX_ASSERT_TRACE(cond, (__VA_ARGS__))
#define PLAYER_PLUGIN_DETAILS(plugin) ""
#endif

//---------------------------------------------------------
class CPlayerPlugin
{
public:
	CPlayerPlugin();
	virtual ~CPlayerPlugin();

#if PLAYER_PLUGIN_DEBUGGING
	static tukk  s_playerPluginEventNames[];

	virtual tukk  DbgGetPluginName() const = 0;
	virtual string DbgGetClassDetails() const { return DbgGetPluginName(); }
#endif

	ILINE void SetOwnerPlayer(CPlayer * player)
	{
#if PLAYER_PLUGIN_DEBUGGING
		DRX_ASSERT_MESSAGE(!m_entered, string().Format("%s shouldn't change owner while entered!", DbgGetClassDetails().c_str()));
		DRX_ASSERT_MESSAGE((player == NULL) != (m_ownerPlayer == NULL), string().Format("%s shouldn't change owner from %p to %p!", DbgGetClassDetails().c_str(), m_ownerPlayer, player));
#endif
		m_ownerPlayer = player;
	}

	ILINE CPlayer * GetOwnerPlayer()
	{
		return m_ownerPlayer;
	}

	virtual void Enter(CPlayerPluginEventDistributor* pEventDist);
	virtual void Leave();

	virtual void Update(const float dt) {}

	virtual ukk GetData(EPlayerPlugInData) { return NULL; }
	virtual void HandleEvent(EPlayerPlugInEvent theEvent, uk  data = NULL);

	ILINE bool IsEntered() const  { return m_entered; }
	virtual void NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) {}

	static ukk DefaultData(EPlayerPlugInData dataType);

protected:
	CPlayer * m_ownerPlayer;

	virtual void InformActiveHasChanged() {}

private:
	bool m_entered;

	void SetIsNowActive(bool nowOn);
};

#endif // __PLAYER_PLUGIN_H__
