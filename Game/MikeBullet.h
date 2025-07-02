// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: MikeBullet

-------------------------------------------------------------------------
История:
- 25:1:2010   : Created by Filipe Amim

*************************************************************************/
#pragma once

#ifndef __MIKE_DRX3D_H__
#define __MIKE_DRX3D_H__

#include <drx3D/Game/Bullet.h>
#include <drx3D/Game/AmmoParams.h>


struct IParticleEmitter;


class CBurnEffectUpr
{
private:

	struct SBurnPoint
	{
		SBurnPoint()
			:	m_pBurnParams(NULL)
			,	m_effect(NULL)
			,	m_position(ZERO)
			,	m_radius(0.f)
			,	m_accumulation(0.f)
			,	m_accumulationDelay(0.0f)
			,	m_surfaceType(-1)
			,	m_attachedEntityId(0)
			,	m_attachType(GeomType_None)
			,	m_attachForm(GeomForm_Surface)
			,	m_hitType(0)
			,	m_shootByPlayer(true) 
			,	m_shooterFactionID(IFactionMap::InvalidFactionID)
		{};

		const SMikeBulletParams::SBurnParams* m_pBurnParams;
		IParticleEmitter* m_effect;
		Vec3 m_position;
		float m_radius;
		float m_accumulation;
		float m_accumulationDelay;
		i32 m_surfaceType;
		EntityId m_attachedEntityId;
		EGeomType m_attachType;
		EGeomForm m_attachForm;
		i32 m_hitType;
		bool m_shootByPlayer;
		uint m_shooterFactionID; // The faction ID of the shooter or IFactionMap::InvalidFactionID if we should always do damage.
	};

	typedef std::vector<SBurnPoint> TBurnPoints;

public:
	CBurnEffectUpr();
	~CBurnEffectUpr();

	void AddBurnPoint(const EventPhysCollision& pCollision, SMikeBulletParams* pBurnBulletParams, i32 hitType, bool shooterIsPlayer, u8k shooterFactionID);
	void Update(float deltaTime);
	void Reset();

private:
	void PushNewBurnPoint(const EventPhysCollision& collision, CBurnEffectUpr::SBurnPoint* burnPoint);
	TBurnPoints::iterator FindBurnPointOnEntity(EntityId entityId, u8k shooterFactionID);
	TBurnPoints::iterator FindClosestBurnPoint(const Vec3& point, i32 surfaceType, u8k shooterFactionID);
	void SpawnImpactEffect(const EventPhysCollision& pCollision, tukk effectName);
	void CreateBurnEffect(const EventPhysCollision& pCollision, SBurnPoint* pBurnPoint);
	void UpdateBurnEffect(SBurnPoint* pBurnPoint);
	void DestroyBurnEffect(SBurnPoint* pBurnPoint);
	void ApplySurroundingDamage(float deltaTime);
	bool ShouldBurnPointInflictDamageOntoEntity(const SBurnPoint& burnPoint, IEntity* targetEntity) const;
	void DebugDraw();

	TBurnPoints m_burnPoints;
	float m_damageTimeOut;
};



class CMikeBullet : public CBullet
{
public:
	typedef CBullet BaseClass;


public:

	CMikeBullet();

	// CProjectile
	virtual void SetParams(const SProjectileDesc& projectileDesc);
	virtual void HandleEvent(const SGameObjectEvent &);
	virtual void CreateBulletTrail(const Vec3& hitPos) {};


private:

	// The faction ID of the owner entity of the weapon that fired this bullet 
	// (IFactionMap::InvalidFactionID if unknown).
	u8 m_ownerFactionID;
};

#endif
