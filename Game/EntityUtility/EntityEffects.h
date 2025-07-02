// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Utilities to handle particle effect (and lights) spawning, attaching, etc...

-------------------------------------------------------------------------
История:
- 10:02:2011: Benito G.R.		

*************************************************************************/

#pragma once

#ifndef _ENTITY_EFFECTS_H_
#define _ENTITY_EFFECTS_H_

#include <drx3D/Game/ItemString.h>

namespace EntityEffects
{
	struct SEffectSpawnParams
	{
		SEffectSpawnParams(const Vec3& _position)
			: position(_position)
			, direction(FORWARD_DIRECTION)
			, scale(1.0f)
			, speed(-1.0f)
			, prime(false)
		{

		}

		SEffectSpawnParams(const Vec3& _position, const Vec3& _direction, const float _scale, const float _speed, const bool _prime)
			: position(_position)
			, direction(_direction)
			, scale(_scale)
			, speed(_speed)
			, prime(_prime)
		{

		}

		Vec3		position;
		Vec3		direction;
		float		scale;
		float		speed;
		bool		prime;
	};

	IParticleEmitter* SpawnParticleFX(tukk effectName, const SEffectSpawnParams& spawnParams, tukk requester = NULL);
	IParticleEmitter* SpawnParticleFX(IParticleEffect* pParticleEffect, const SEffectSpawnParams& spawnParams);

	void SpawnParticleWithEntity(const EntityId targetEntityId, i32k targetSlot, tukk effectName, tukk helperName, const SEffectSpawnParams& spawnParams);
	void SpawnParticleWithEntity(const IEntity* pTargetEntity, i32k targetSlot, tukk effectName, tukk helperName, const SEffectSpawnParams& spawnParams);
	void SpawnParticleWithEntity(const IEntity* pTargetEntity, i32k targetSlot, IParticleEffect* pParticleEffect, tukk helperName, const SEffectSpawnParams& spawnParams);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	typedef u32 TAttachedEffectId;

	struct SEffectAttachParams
	{
		SEffectAttachParams()
			: offset(ZERO)
			, direction(FORWARD_DIRECTION)
			, scale(1.0f)
			, prime(false)
			, firstSafeSlot(-1)
		{

		}

		SEffectAttachParams(const Vec3& _offset, const Vec3& _direction, const float _scale, const bool _prime, i32k _firstSafeSlot)
			: offset(_offset)
			, direction(_direction)
			, scale(_scale)
			, prime(_prime)
			, firstSafeSlot(_firstSafeSlot)
		{

		}

		Vec3	offset;
		Vec3	direction;
		float	scale;
		i32		firstSafeSlot;
		bool	prime;
	};

	struct SLightAttachParams
	{

		SLightAttachParams()
			:	pCasterException(0)
			,	color(Vec3Constants<float>::fVec3_One)
			, offset(Vec3Constants<float>::fVec3_Zero)
			, direction(Vec3Constants<float>::fVec3_OneY)
			, radius(5.0f)
			, specularMultiplier(1.0f)
			, diffuseMultiplier(1.0f)
			, projectFov(0.0f)
			, hdrDynamic(0.0f)
			, projectTexture(NULL)
			, material(NULL)
			,	style(0)
			,	animSpeed(0.0f)
			, firstSafeSlot(-1)
			, deferred(false)
			,	castShadows(false)
		{

		}

		IRenderNode* pCasterException;
		Vec3	color;
		Vec3	offset;
		Vec3	direction;
		float	radius;
		float	specularMultiplier;
		float diffuseMultiplier;
		float	projectFov;
		float	hdrDynamic;
		const char	*projectTexture; 
		const char	*material;
		i32		style;
		float	animSpeed;
		i32		firstSafeSlot;
		bool  deferred;
		bool	castShadows;
	};

	struct SEffectInfo
	{
		SEffectInfo()
			: id (0)
			, entityEffectSlot(-1)
			, characterEffectSlot(-1)
		{
		}

		bool operator == (const TAttachedEffectId& otherEffectId) const
		{
			return (id == otherEffectId);
		};

		TAttachedEffectId	id;
		i32					entityEffectSlot; 
		i32					characterEffectSlot;
		ItemString			helperName;
		//GameSharedString	helperName;
	};

	class CEffectsController
	{
	
		typedef std::vector<SEffectInfo>	TAttachedEffects;

	public:

		CEffectsController();

		void InitWithEntity(IEntity* pEntity);
		void FreeAllEffects();

		TAttachedEffectId AttachParticleEffect(IParticleEffect* pParticleEffect, const SEffectAttachParams& attachParams);
		TAttachedEffectId AttachParticleEffect(tukk effectName, const SEffectAttachParams& attachParams);
		TAttachedEffectId AttachParticleEffect(IParticleEffect* pParticleEffect, i32k targetSlot, tukk helperName, const SEffectAttachParams& attachParams);
		TAttachedEffectId AttachParticleEffect(tukk effectName, i32k targetSlot, tukk helperName, const SEffectAttachParams& attachParams);
		TAttachedEffectId AttachLight(i32k targetSlot, tukk helperName, const SLightAttachParams& attachParams);
		void DetachEffect(const TAttachedEffectId effectId);

		ILINE const SEffectInfo& GetEffectInfoAt(u32k index) const
		{
			DRX_ASSERT(index < (u32)m_attachedEffects.size());

			return m_attachedEffects[index];
		}

		ILINE u32 GetEffectCount() const
		{
			return (u32)m_attachedEffects.size();
		}

		IParticleEmitter* GetEffectEmitter(const TAttachedEffectId effectId) const;
		ILightSource*			GetLightSource(const TAttachedEffectId effectId) const;
		void SetEffectWorldTM(const TAttachedEffectId effectId, const Matrix34& effectWorldTM);
		void UpdateEntitySlotEffectLocationsFromHelpers();

		void GetMemoryStatistics(IDrxSizer* pSizer) const;

	private:

		i32 FindSafeSlot(i32 firstSafe);

		IEntity*			m_pOwnerEntity;
		TAttachedEffects	m_attachedEffects;
		TAttachedEffectId	m_effectGeneratorId;
	};
};

#endif //_ENTITY_EFFECTS_H_