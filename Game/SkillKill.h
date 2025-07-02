// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
static skill kill checks 
- 02:02:2010		Created by Ben Parbury
*************************************************************************/

#ifndef __SKILLKILL_H__
#define __SKILLKILL_H__

#include <drx3D/Game/GameRulesTypes.h>

struct IActor;
class CPlayer;
class CGameRules;

class SkillKill
{
public:
	static bool IsHeadshot(CPlayer* pTargetPlayer, i32 hit_joint, i32 material);
	static bool IsAirDeath(CPlayer* pTargetPlayer);
	static bool IsStealthKill(CPlayer* pTargetPlayer);
	static bool IsMeleeTakedown(CGameRules* pGameRules, i32 hit_type);
	static bool IsBlindKill(CPlayer* pShooterPlayer);
	static bool IsFirstBlood(IActor *pShooter, IActor *pVictim);
	static bool IsRumbled(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static bool IsDefiantKill(CPlayer* pShooterPlayer);
	static bool IsKillJoy(CPlayer* pTargetPlayer);
	static EPPType IsMultiKill(CPlayer* pShooterPlayer);
	static bool IsRecoveryKill(CPlayer* pShooterPlayer);
	static bool IsRetaliationKill(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static bool IsGotYourBackKill(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static bool IsGuardianKill(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static bool IsInterventionKill(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static bool IsPiercing(i32 penetration);
	static bool IsBlinding(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer, u16 weaponClassId);
	static bool IsFlushed(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer, CGameRules* pGameRules, i32 hit_type);
	static bool IsDualWeapon(CPlayer* pShooterPlayer, CPlayer* pTargetPlayer);
	static void	SetFirstBlood(bool firstBlood);
	static bool GetFirstBlood();
	static bool IsKickedCarKill(CPlayer* pKicker, CPlayer* pTargetPlayer, EntityId vehicleId);
	static bool IsSuperChargedKill(CPlayer* pShooter);
	static bool IsIncomingKill(i32 hit_type);
	static bool IsPangKill(i32 hit_type);
	static bool IsAntiAirSupportKill(i32 hit_type);

	static void Reset();

private:
	static bool s_firstBlood;
	static u16 s_flashBangGrenadesClassId;
};

#endif
