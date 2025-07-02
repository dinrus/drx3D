// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Exposes actor functionality to LUA
  
 -------------------------------------------------------------------------
  История:
  - 7:10:2004   14:19 : Created by Márcio Martins

*************************************************************************/
#ifndef __SCRIPTBIND_ACTOR_H__
#define __SCRIPTBIND_ACTOR_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


struct IGameFramework;
struct IActor;


// <title Actor>
// Syntax: Actor
class CScriptBind_Actor :
	public CScriptableBase
{
public:
	CScriptBind_Actor(ISystem *pSystem);
	virtual ~CScriptBind_Actor();

	void AttachTo(IActor *pActor);

	//------------------------------------------------------------------------
	i32 DumpActorInfo(IFunctionHandler *pH);

	i32 Revive(IFunctionHandler *pH);
	i32 Kill(IFunctionHandler *pH);
	i32 ShutDown(IFunctionHandler *pH);
	i32 SetParams(IFunctionHandler *pH);
	i32 GetHeadDir(IFunctionHandler *pH);
	i32 GetAimDir(IFunctionHandler *pH);

	i32 PostPhysicalize(IFunctionHandler *pH);
	i32 GetChannel(IFunctionHandler *pH);
	i32 IsPlayer(IFunctionHandler *pH);
	i32 IsLocalClient(IFunctionHandler *pH);
	i32 GetLinkedVehicleId(IFunctionHandler *pH);

	i32 LinkToEntity(IFunctionHandler *pH);
	i32 SetAngles(IFunctionHandler *pH,Ang3 vAngles );
	i32 PlayerSetViewAngles( IFunctionHandler *pH,Ang3 vAngles );
	i32 GetAngles(IFunctionHandler *pH);

	i32 SetMovementTarget(IFunctionHandler *pH, Vec3 pos, Vec3 target, Vec3 up, float speed);
	i32 CameraShake(IFunctionHandler *pH,float amount,float duration,float frequency,Vec3 pos);
	i32 SetViewShake(IFunctionHandler *pH, Ang3 shakeAngle, Vec3 shakeShift, float duration, float frequency, float randomness);

	i32 SetExtensionParams(IFunctionHandler* pH, tukk extension, SmartScriptTable params);

	i32 SvRefillAllAmmo(IFunctionHandler* pH, tukk refillType, bool refillAll, i32 grenadeCount, bool bRefillCurrentGrenadeType);
	i32 ClRefillAmmoResult(IFunctionHandler* pH, bool ammoRefilled);

	i32 SvGiveAmmoClips(IFunctionHandler* pH, i32 numClips);

	i32 IsImmuneToForbiddenArea(IFunctionHandler *pH);

	i32 SetHealth(IFunctionHandler *pH, float health);
	i32 SetMaxHealth(IFunctionHandler *pH, float health);
	i32 GetHealth(IFunctionHandler *pH);
	i32 GetMaxHealth(IFunctionHandler *pH);
	i32 DamageInfo(IFunctionHandler *pH, ScriptHandle shooter, ScriptHandle target, ScriptHandle weapon, ScriptHandle projectile, float damage, i32 damageType, Vec3 hitDirection);
	i32 GetLowHealthThreshold(IFunctionHandler *pH);

	i32 SetPhysicalizationProfile(IFunctionHandler *pH, tukk profile);
	i32 GetPhysicalizationProfile(IFunctionHandler *pH);

	i32 QueueAnimationState(IFunctionHandler *pH, tukk animationState);

	i32 CreateCodeEvent(IFunctionHandler *pH,SmartScriptTable params);

	i32 PauseAnimationGraph(IFunctionHandler* pH);
	i32 ResumeAnimationGraph(IFunctionHandler* pH);
	i32 HurryAnimationGraph(IFunctionHandler* pH);
	i32 SetTurnAnimationParams(IFunctionHandler* pH, const float turnThresholdAngle, const float turnThresholdTime);

	i32 SetSpectatorMode(IFunctionHandler *pH, i32 mode, ScriptHandle targetId);
	i32 GetSpectatorMode(IFunctionHandler *pH);
	i32 GetSpectatorState(IFunctionHandler *pH);
	i32 GetSpectatorTarget(IFunctionHandler* pH);

	i32 Fall(IFunctionHandler *pH, Vec3 hitPos);

	i32 StandUp(IFunctionHandler *pH);

	i32 GetExtraHitLocationInfo(IFunctionHandler *pH, i32 slot, i32 partId);

	i32 SetForcedLookDir(IFunctionHandler *pH, CScriptVector dir);
	i32 ClearForcedLookDir(IFunctionHandler *pH);
	i32 SetForcedLookObjectId(IFunctionHandler *pH, ScriptHandle objectId);
	i32 ClearForcedLookObjectId(IFunctionHandler *pH);

	i32 CanSpectacularKillOn(IFunctionHandler *pH, ScriptHandle targetId);
	i32 StartSpectacularKill(IFunctionHandler *pH, ScriptHandle targetId);
	i32 RegisterInAutoAimUpr(IFunctionHandler *pH);
	i32 CheckBodyDamagePartFlags(IFunctionHandler *pH, i32 partID, i32 materialID, u32 bodyPartFlagsMask);
	i32 GetBodyDamageProfileID(IFunctionHandler* pH, tukk bodyDamageFileName, tukk bodyDamagePartsFileName);
	i32 OverrideBodyDamageProfileID(IFunctionHandler* pH, i32k bodyDamageProfileID);
	i32 IsGod(IFunctionHandler* pH);

	//------------------------------------------------------------------------
	// ITEM STUFF
	//------------------------------------------------------------------------
	i32 HolsterItem(IFunctionHandler *pH, bool holster);
	i32 DropItem(IFunctionHandler *pH, ScriptHandle itemId);
	i32 PickUpItem(IFunctionHandler *pH, ScriptHandle itemId);
	i32 IsCurrentItemHeavy( IFunctionHandler* pH );
	i32 PickUpPickableAmmo(IFunctionHandler *pH, tukk ammoName, i32 count);

	i32 SelectItemByName(IFunctionHandler *pH, tukk name);
	i32 SelectItem(IFunctionHandler *pH, ScriptHandle itemId, bool forceSelect);
	i32 SelectLastItem(IFunctionHandler *pH);
	i32 SelectNextItem(IFunctionHandler *pH, i32 direction, bool keepHistory, tukk category);
	i32 SimpleFindItemIdInCategory(IFunctionHandler *pH, tukk category);

	i32 DisableHitReaction(IFunctionHandler *pH);
	i32 EnableHitReaction(IFunctionHandler *pH);

	i32 CreateIKLimb( IFunctionHandler *pH, i32 slot, tukk limbName, tukk rootBone, tukk midBone, tukk endBone, i32 flags);

	i32 PlayAction(IFunctionHandler *pH, tukk action);

	//misc
	i32 RefreshPickAndThrowObjectPhysics( IFunctionHandler *pH );

	i32 AcquireOrReleaseLipSyncExtension(IFunctionHandler *pH);

protected:
	class CActor *GetActor(IFunctionHandler *pH);

	bool IsGrenadeClass(const IEntityClass* pEntityClass) const;
	bool RefillOrGiveGrenades(CActor& actor, IInventory& inventory, IEntityClass* pGrenadeClass, i32 grenadeCount);

	ISystem					*m_pSystem;
	IGameFramework	*m_pGameFW;
};

#endif //__SCRIPTBIND_ACTOR_H__
