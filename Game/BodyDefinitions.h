// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Common definitions for all body damage related classes

-------------------------------------------------------------------------
История:
- 27:07:2010   Created by Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef _BODY_DEFINITIONS_H_
#define _BODY_DEFINITIONS_H_

#include <drx3D/Animation/IDrxAnimation.h>

struct ISkeletonPose;
struct IAttachmentUpr;
struct IPhysicalEntity;

struct SBodyCharacterInfo
{
	ISkeletonPose* pSkeletonPose;
	IDefaultSkeleton* pIDefaultSkeleton;
	IAttachmentUpr* pAttachmentUpr;
	IPhysicalEntity* pPhysicalEntity;

	SBodyCharacterInfo() : pIDefaultSkeleton(), pSkeletonPose(), pAttachmentUpr(), pPhysicalEntity() {}
};

struct SBodyDamageImpulseFilter
{
	float multiplier;
	float passOnMultiplier;
	i32 passOnPartId;
	u16 projectileClassID;

	SBodyDamageImpulseFilter() : multiplier(1.0f), passOnMultiplier(0.0f), passOnPartId(-1), projectileClassID(u16(~0)) {}
};

enum EBodyDamagePIDFlags
{
	eBodyDamage_PID_None = 0,
	eBodyDamage_PID_Headshot = BIT(0),
	eBodyDamage_PID_Foot = BIT(1),
	eBodyDamage_PID_Groin = BIT(2),
	eBodyDamage_PID_Knee = BIT(3),
	eBodyDamage_PID_WeakSpot = BIT(4),
	eBodyDamage_PID_Helmet = BIT(5)
};

typedef u32 TBodyDamageProfileId;
#define INVALID_BODYDAMAGEPROFILEID ((TBodyDamageProfileId)(~0))

struct SBodyDamageDef
{
	string bodyPartsFileName;
	string bodyDamageFileName;
	u32 crc32BodyPartsFileName;
	u32 crc32BodyDamageFileName;
	TBodyDamageProfileId bodyDamageProfileId;

	SBodyDamageDef()
		: bodyDamageProfileId(INVALID_BODYDAMAGEPROFILEID)
		, crc32BodyPartsFileName(0)
		, crc32BodyDamageFileName(0)
	{
	}

	bool operator ==(const SBodyDamageDef& other) const
	{
		assert(crc32BodyPartsFileName > 0);
		assert(crc32BodyDamageFileName > 0);

		return (crc32BodyPartsFileName == other.crc32BodyPartsFileName &&
			crc32BodyDamageFileName == other.crc32BodyDamageFileName);
	}
};

typedef u32 TBodyDestructibilityProfileId;
#define INVALID_BODYDESTRUCTIBILITYPROFILEID ((TBodyDestructibilityProfileId)(~0))

struct SBodyDestructibilityDef
{
	string bodyDestructibilityFileName;
	u32 crc32BodyDestructibilityFileName;
	TBodyDestructibilityProfileId bodyDestructibilityProfileId;

	SBodyDestructibilityDef()
		: bodyDestructibilityProfileId(INVALID_BODYDESTRUCTIBILITYPROFILEID)
		, crc32BodyDestructibilityFileName(0)
	{
	}

	bool operator ==(const SBodyDestructibilityDef& other) const
	{
		DRX_ASSERT(crc32BodyDestructibilityFileName > 0);

		return (crc32BodyDestructibilityFileName == other.crc32BodyDestructibilityFileName);
	}
};

//////////////////////////////////////////////////////////////////////////
///// Per instance destruction status data					  ////////////
//////////////////////////////////////////////////////////////////////////

class CBodyDestrutibilityInstance
{
public:
	struct SBodyDestructiblePartStatus
	{
		SBodyDestructiblePartStatus()
			: m_baseHealth(50.0f)
			, m_minHealthToDestroyOnDeath(10.0f)
			, m_currentHealth(50.0f)
			, m_visibleAtStart(true)
		{

		}

		SBodyDestructiblePartStatus(float baseHealth, float minHealthToDestroyOnDeath, bool visibleAtStart)
			: m_baseHealth(baseHealth)
			, m_minHealthToDestroyOnDeath(minHealthToDestroyOnDeath)
			, m_currentHealth(baseHealth)
			, m_visibleAtStart(visibleAtStart)
		{

		}

		ILINE void Reset()
		{
			m_currentHealth = m_baseHealth;
		}

		ILINE bool IsDestroyed() const
		{
			return (m_currentHealth <= 0.0f);
		}

		ILINE bool CanDestroyOnDeath() const
		{
			return (m_currentHealth < m_minHealthToDestroyOnDeath);
		}

		ILINE void SetHealth(float health)
		{
			m_currentHealth = max(0.0f, health);
		}

		ILINE float GetInitialHealth() const
		{
			return m_baseHealth;
		}

		ILINE float GetHealth() const
		{
			return m_currentHealth;
		}

		ILINE float GetOnDeathHealthThreshold() const
		{
			return m_minHealthToDestroyOnDeath;
		}

		ILINE bool WasInitialyVisible() const
		{
			return m_visibleAtStart;
		}

	private:
		float	m_baseHealth;
		float	m_minHealthToDestroyOnDeath;
		float	m_currentHealth;
		bool	m_visibleAtStart;
	};

	CBodyDestrutibilityInstance()
		: m_id(INVALID_BODYDESTRUCTIBILITYPROFILEID)
		, m_eventsModified(false)
		, m_lastEventForHitReactionsCrc(0)
		, m_currentHealthRatioEventIdx(0)
		, m_instanceInitialHealth(0.0f)
		, m_mikeAttachmentEntityId(0)
		, m_mikeExplodeAlphaTestFadeOutTimer(0)
		, m_mikeExplodeAlphaTestFadeOutScale(0)
		, m_mikeExplodeAlphaTestMax(0)
	{

	}

	~CBodyDestrutibilityInstance();

	void ReserveSlots(i32k totalAttachmentsCount, i32k destructibleAttachmentsCount, i32k destructibleBonesCount, i32k destructionEventsCount);
	void Reset();

	void DeleteMikeAttachmentEntity();
	void InitWithProfileId(const TBodyDestructibilityProfileId profileId);

	ILINE TBodyDestructibilityProfileId GetProfileId() const 
	{ 
		return m_id;
	};

	ILINE void AddAttachment(float baseHealth, float minHealthToDestroyOnDeath, bool visibleAtStart)
	{
		m_attachmentStatus.push_back(SBodyDestructiblePartStatus(baseHealth, minHealthToDestroyOnDeath, visibleAtStart));
	}

	ILINE void AddBone(float baseHealth, float minHealthToDestroyOnDeath)
	{
		m_boneStatus.push_back(SBodyDestructiblePartStatus(baseHealth, minHealthToDestroyOnDeath, false));
	}

	SBodyDestructiblePartStatus* GetAttachmentStatus(i32 idx)
	{
		const bool validIndex = (idx >= 0) && (idx < (i32)m_attachmentStatus.size());
		DRX_ASSERT(validIndex);

		if (validIndex)
		{
			return &m_attachmentStatus[idx];
		}
		return NULL;
	}

	SBodyDestructiblePartStatus* GetBoneStatus(i32 idx)
	{
		const bool validIndex = (idx >= 0) && (idx < (i32)m_boneStatus.size());
		DRX_ASSERT(validIndex);

		if (validIndex)
		{
			return &m_boneStatus[idx];
		}
		return NULL;
	}

	bool CanTriggerEvent(i32 idx) const
	{
		const bool validIndex = (idx >= 0) && (idx < (i32)m_availableDestructionEvents.size());

		if (validIndex)
		{
			return m_availableDestructionEvents[idx];
		}
		return false;
	}

	void DisableEvent(i32 idx)
	{
		const bool validIndex = (idx >= 0) && (idx < (i32)m_availableDestructionEvents.size());
		DRX_ASSERT(validIndex);

		if (validIndex)
		{
			m_availableDestructionEvents[idx] = false;
			m_eventsModified = true;
		}
	}

	void SetDestructibleLastEventForHitReactions(tukk eventName)
	{
		m_lastEventForHitReactionsCrc = (!eventName || eventName[0] == '\0') ? 0 : CCrc32::ComputeLowercase(eventName);
	}

	ILINE u32 GetLastDestructionEventForHitReactions() const
	{
		return m_lastEventForHitReactionsCrc;
	}

	ILINE void SetCurrentHealthRatioIndex(i32k index)
	{
		m_currentHealthRatioEventIdx = index;
	}

	ILINE i32 GetCurrentHealthRatioIndex() const
	{
		return m_currentHealthRatioEventIdx;
	}

	ILINE bool AreInstanceDestructiblesModified() const
	{
		return m_eventsModified;
	}

	ILINE void SetInstanceHealth(const float baseHealth)
	{
		m_instanceInitialHealth = baseHealth;
	}

	ILINE float GetInstanceInitialHealth() const
	{
		return m_instanceInitialHealth;
	}

	void InitializeMikeDeath(const EntityId entityId, float alphaTestFadeOutTime, float alphaTestFadeOutDelay, float alphaTestMax);

	ILINE EntityId GetMikeAttachmentEntityId() const
	{
		return m_mikeAttachmentEntityId;
	}

	void ReplaceMaterial( IEntity& characterEntity, ICharacterInstance& characterInstance, IMaterial& replacementMaterial );
	void ResetMaterials( IEntity& characterEntity, ICharacterInstance& characterInstance );

	void Update(float frameTime);

private:

	void CleanUpOriginalMaterials();

	typedef std::vector<SBodyDestructiblePartStatus>	TDestructiblePartsStatus;
	typedef std::vector<bool>							TAvailableDestructionEvents;
	typedef std::pair<u32, IMaterial*>	TAttachmentMaterialPair;
	typedef std::vector<TAttachmentMaterialPair>	TOriginalMaterials;

	TDestructiblePartsStatus m_attachmentStatus;
	TDestructiblePartsStatus m_boneStatus;
	TAvailableDestructionEvents m_availableDestructionEvents;
	TOriginalMaterials	m_originalMaterials;

	TBodyDestructibilityProfileId	m_id;

	EntityId		m_mikeAttachmentEntityId;
	u32	m_lastEventForHitReactionsCrc;	
	i32				m_currentHealthRatioEventIdx;
	float			m_instanceInitialHealth;
	float			m_mikeExplodeAlphaTestFadeOutTimer;
	float			m_mikeExplodeAlphaTestFadeOutScale;
	float			m_mikeExplodeAlphaTestMax;

	bool			m_eventsModified;
};

#endif