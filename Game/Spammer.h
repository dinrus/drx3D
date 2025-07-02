// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _SPAMMER_H_
#define _SPAMMER_H_


#include <drx3D/Game/Single.h>


class CSpammerCloudTargets
{
public:
	struct STarget
	{
		EntityId m_target;
		i32 m_numLocks;
	};

public:
	CSpammerCloudTargets();

	bool Empty() const;
	void Clear();

	void LockOn(EntityId target);
	i32 GetNumLockOns() const;
	i32 GetNumLockOns(EntityId target) const;
	const STarget& GetTarget(i32 idx) const;
	i32 GetNumTargets() const;

	EntityId UnlockNext();

private:
	std::vector<STarget> m_targets;
	i32 m_numLockOns;
};



class CSpammerPotentialTargets
{
public:
	struct STarget
	{
		EntityId m_target;
		float m_probability;
	};

public:

	void Clear();
	void AddTarget(EntityId target, float probability);

public:
	std::vector<STarget> m_potentialTargets;
	float m_totalProbability;
};



class CSpammer : public CSingle
{
private:
	enum EState
	{
		EState_None,
		EState_LoadingIn,
		EState_Bursting,
	};

	typedef CSingle BaseFiremode;
public:
	DRX_DECLARE_GTI(CSpammer);

	CSpammer();

	virtual void Activate(bool activate) override;
	virtual void Update(float frameTime, u32 frameId) override;
	virtual void StartFire() override;
	virtual void StopFire() override;

	ILINE const CSpammerPotentialTargets& GetPotentialTargets() { return m_potentialTargets; }
	ILINE const CSpammerCloudTargets& GetLockOnTargets() { return m_targetsAssigned; }

private:
	bool ShootRocket(EntityId target);

	void UpdateLoadIn(float frameTime);
	void UpdateBurst(float frameTime);
	void UpdatePotentialTargets();

	void StartLoadingIn();
	void StartBursting();

	void AddTarget();
	void ShootNextTarget();

	Vec3 GetWeaponPosition(const Vec3& probableHit) const;
	Vec3 GetWeaponDirection(const Vec3& firingPosition, const Vec3& probableHit) const;

	EntityId GetNextLockOnTarget() const;

	EState m_state;
	CSpammerPotentialTargets m_potentialTargets;
	CSpammerCloudTargets m_targetsAssigned;
	float m_timer;
	float m_nextFireTimer;
	i32 m_numLoadedRockets;
	bool m_firingPending;
};


#endif
