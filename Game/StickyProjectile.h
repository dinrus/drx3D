// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __STICKYPROJECTILE_H__
#define __STICKYPROJECTILE_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/Projectile.h>



class CStickyProjectile : public IEntityEventListener
{
public:

	static const NetworkAspectType ASPECT_STICK	= eEA_GameServerStatic;

	enum EStickFlags
	{
		eSF_IsStuck									= BIT(0),
		eSF_OrientateToCollNormal		= BIT(1),
		eSF_HandleLocally						= BIT(2),
		eSF_StuckToPlayer						= BIT(3),
		eSF_StuckToAI								= BIT(4),

		eSF_Reset										= (eSF_IsStuck|eSF_StuckToPlayer|eSF_StuckToAI)
	};
	typedef u8 TFlags;

	enum EOrientation
	{
		eO_AlignToSurface = 0,
		eO_AlignToVelocity,
	};

	struct SStickParams
	{
		SStickParams(CProjectile* pProjectile, EventPhysCollision *pCollision, EOrientation orientation );
		SStickParams(CProjectile* pProjectile, const HitInfo& rHitInfo, EOrientation orientation );

		CProjectile* m_pProjectile;
		EntityId m_ownerId;
		IPhysicalEntity* m_pTarget;
		Vec3 m_stickPosition;
		Vec3 m_stickNormal;
		i32 m_targetPartId;
		i32 m_targetMatId;
		bool m_bStickToFriendlies; 
	};

public:
	CStickyProjectile(bool bOrientateToCollisionNormal, bool bHandleLocally = false);
	~CStickyProjectile();

	ILINE bool IsStuck() const { return (m_flags&eSF_IsStuck)!=0; }
	ILINE EntityId GetStuckEntityId() const { return m_parentId; }
	ILINE const Vec3& GetNormal() const { return m_stuckNormal; }
	bool IsStuckToPlayer() const;
	bool IsStuckToAI() const;
	ILINE i32 GetStuckJoint() const {return m_stuckJoint;}
	bool IsValid() const;

	void SwapToCorpse( const EntityId corpseId );

	void Reset();
	void Stick(const SStickParams& stickParams);
	void UnStick();
	void Serialize(TSerialize ser);
	void NetSerialize(CProjectile* pProjectile, TSerialize ser, EEntityAspects aspect);
	NetworkAspectType GetNetSerializeAspects();
	void NetSetStuck(CProjectile* pProjectile, bool stuck);

	// IEntityEventListener
	virtual void OnEntityEvent( IEntity *pEntity,SEntityEvent &event );
	//~IEntityEventListener

private:
	void AttachTo(CProjectile* pProjectile, IEntity * pTargetEntity);
	bool AttachToCharacter(CProjectile* pProjectile, IEntity& pEntity, ICharacterInstance& pCharacter, tukk boneName);
	void SetProjectilePhysics(CProjectile* pProjectile, EPhysicalizationType physics);
	void CalculateLocationForStick(const IEntity& projectile, const Vec3& collPos, const Vec3& collNormal, QuatT& outLocation) const;
	void SendHitInfo(const CProjectile* pProjectile, IEntity* pTargetEntity);

	bool StickToStatic( const SStickParams& stickParams );
	bool StickToEntity( const SStickParams& stickParams, IEntity* pTargetEntity );
	void SetParentId( const EntityId newParentId );

	u32		m_characterAttachmentCrC;
	Vec3			m_stuckPos;
	Quat			m_stuckRot;
	Vec3			m_stuckNormal;
	EntityId	m_parentId;
	EntityId	m_childId;
	i32				m_stuckJoint;
	i32				m_stuckPartId;
	TFlags		m_flags;
};




#endif
