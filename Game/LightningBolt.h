// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIGHTNING_BOLT_H_
#define _LIGHTNING_BOLT_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/StickyProjectile.h>
#include <drx3D/Game/Projectile.h>
#include <drx3D/Game/Effects/LightningGameEffect.h>



class CLightningBolt : public CProjectile
{
private:
	typedef CProjectile Parent;

public:
	CLightningBolt();
	~CLightningBolt();

	virtual void Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale/* =1.0f */);
	virtual void Update(SEntityUpdateContext &ctx, i32 updateSlot);
	virtual void HandleEvent(const SGameObjectEvent &);
	virtual void ReInitFromPool();
	
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags);
	virtual NetworkAspectType GetNetSerializeAspects();

	virtual void OnHit(const HitInfo&);

private:
	void UpdateSparkSize();
	void Strike();
	void OnStrike();
	void ProcessHit(CGameRules* gameRules, EntityId target, float damage, Vec3 position, Vec3 direction, i32 partId);
	void CreateSpark(const Vec3& start, const Vec3& end, EntityId targetId);
	void HandleArcing(EntityId target, const Vec3& collisionNormal, const SLightningBoltParams& boltParams);

	CStickyProjectile m_stuckProjectile;
	CLightningGameEffect::TIndex m_lightningId;
	float m_strikeCountDown;
};


#endif
