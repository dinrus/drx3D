// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Body destructibility for actors (ported from original lua version)

-------------------------------------------------------------------------
История:
- 27:07:2010   Created by Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef BODY_DESTRUCTION_H
#define BODY_DESTRUCTION_H

#include <drx3D/Game/BodyDefinitions.h>
#include <drx3D/Game/Utility/DrxHash.h>

class CBodyDestrutibilityInstance;
struct HitInfo;

//////////////////////////////////////////////////////////////////////////
///// Destructible info (shared between users of this profile ////////////
//////////////////////////////////////////////////////////////////////////

class CBodyDestructibilityProfile : public _reference_target_t
{
private:

	typedef i32	TDestructibleBodyPartId;
	typedef	i32	TDestructionEventId;

	typedef std::vector<TDestructibleBodyPartId>	TDestructibleAttachmentIds;
	typedef std::vector<TDestructionEventId>	TDestructionEventIds;

	struct SBodyHitType
	{
		SBodyHitType()
			: hitTypeId(0)
			, damageMultiplier(1.0f)
		{
			destructionEvents[0] = -1;
			destructionEvents[1] = -1;
		}

		bool operator ==(const SBodyHitType& other) const
		{
			return (hitTypeId == other.hitTypeId);
		}

		bool operator ==(i32k& otherHitTypeId) const
		{
			return (hitTypeId == otherHitTypeId);
		}

		i32		hitTypeId;
		float	damageMultiplier;

		TDestructionEventId		destructionEvents[2];	//[0] normal - [1] on actor death
	};

	typedef std::vector<SBodyHitType>	TBodyHitTypes;

	struct SDestructibleBodyPart
	{
		SDestructibleBodyPart()
			: hashId(0)
			, healthRatio(0.0f)
			, minHealthToDestroyOnDeathRatio(0.0f)
		{
			destructionEvents[0] = -1;
			destructionEvents[1] = -1;
		}

		const SBodyHitType* GetHitTypeModifiers(i32k hitTypeId) const
		{
			TBodyHitTypes::const_iterator hitTypeCit = std::find(hitTypes.begin(), hitTypes.end(), hitTypeId);

			return (hitTypeCit != hitTypes.end()) ? &(*hitTypeCit) : NULL;
		}

		
		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(name);
			pSizer->AddContainer(hitTypes);
		}

		string	name;
		DrxHash	hashId;

		float	healthRatio;
		float	minHealthToDestroyOnDeathRatio;

		TDestructionEventId		destructionEvents[2];

		TBodyHitTypes hitTypes;
	};

	typedef std::vector<SDestructibleBodyPart>	TDestructibleBodyParts;

	struct SBodyPartExplosion
	{
		SBodyPartExplosion()
			: damage(50.0f)
			, minRadius(1.0f)
			, maxRadius(3.0f)
			, pressure(500.0f)
			, hitTypeId(0)
		{

		}

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(effectName);
		}

		float damage;
		float minRadius;
		float maxRadius;
		float pressure;
		i32	hitTypeId;

		string effectName;
	};

	struct SDestructionEvent
	{
		SDestructionEvent()
			: pExplosion(NULL)
		{

		}

		~SDestructionEvent()
		{
			SAFE_DELETE(pExplosion);
		}

		bool operator ==(const SDestructionEvent& other) const
		{
			return (eventId == other.eventId);
		}

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(name);
			pSizer->AddObject(particleFX);
			pSizer->AddObject(scriptCondition);
			pSizer->AddObject(scriptCallback);
		}

		string			name;
		string			particleFX;
		string			scriptCondition;
		string			scriptCallback;

		DrxHash			eventId;

		TDestructibleAttachmentIds	attachmentsToHide;
		TDestructibleAttachmentIds	attachmentsToUnhide;
		TDestructionEventIds	eventsToDisable;
		TDestructionEventIds	eventsToStop;

		SBodyPartExplosion*	pExplosion;
	};

	typedef std::vector<SDestructionEvent> TDestructionEvents;

	struct SHealthRatioEvent
	{
		SHealthRatioEvent()
			: healthRatio(0.5f)
			, destructionEvent(-1)
		{

		}

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(bone);
			pSizer->AddObject(material);
		}

		float healthRatio;
		TDestructionEventId destructionEvent;

		string bone;
		string material;
	};

	//Order from greater to smaller
	struct compareHealthRatios
	{
		bool operator() (const SHealthRatioEvent& lhs, const SHealthRatioEvent& rhs) const
		{
			return (lhs.healthRatio > rhs.healthRatio);
		}
	};

	typedef std::vector<SHealthRatioEvent>	THealthRatioEvents;

	struct SExplosionDeathEvent
	{
		SExplosionDeathEvent()
			: gibProbability(0.5f)
			, minExplosionDamage(200.0f)
			, destructionEvent(-1)
		{

		}

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(bone);
			pSizer->AddObject(material);
		}

		float gibProbability;
		float minExplosionDamage;
		TDestructionEventId destructionEvent;

		string bone;
		string material;
	};

	struct SMikeDeath
	{
		SMikeDeath()
			: attachmentId(-1)
			, destructionEvent(-1)
			, alphaTestFadeOutDelay(0.85f)
			, alphaTestFadeOutTimeOut(0.25f)
			, alphaTestFadeOutMaxAlpha(0.0f)
		{

		}

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(bone);
			pSizer->AddObject(object);
			pSizer->AddObject(animation);
		}

		TDestructibleBodyPartId attachmentId;
		TDestructionEventId destructionEvent;
		
		string	bone;
		string	object;
		string	animation;
		float	alphaTestFadeOutDelay;
		float	alphaTestFadeOutTimeOut;
		float	alphaTestFadeOutMaxAlpha;
	};

	typedef std::map<DrxHash, i32>	TNameHashToIdxMap;

	struct SParsingHelper
	{
		i32 GetAttachmentIndex(tukk attachmentName) const
		{
			TNameHashToIdxMap::const_iterator cit = attachmentIndices.find(DrxStringUtils::HashString(attachmentName));
			
			return (cit != attachmentIndices.end()) ? cit->second : -1;
		}

		i32 GetBoneIndex(tukk boneName) const
		{
			TNameHashToIdxMap::const_iterator cit = boneIndices.find(DrxStringUtils::HashString(boneName));

			return (cit != boneIndices.end()) ? cit->second : -1;
		}

		i32 GetEventIndex(tukk eventName) const
		{
			TNameHashToIdxMap::const_iterator cit = eventIndices.find(DrxStringUtils::HashString(eventName));

			return (cit != eventIndices.end()) ? cit->second : -1;
		}

		TNameHashToIdxMap attachmentIndices;
		TNameHashToIdxMap boneIndices;
		TNameHashToIdxMap eventIndices;
	};

	struct SBodyPartQueryResult
	{
		SBodyPartQueryResult()
			: pPart(NULL)
			, index(-1)
		{

		}

		const SDestructibleBodyPart* pPart;
		i32 index;
	};

	typedef _smart_ptr<IMaterial>	IMaterialSmartPtr;
	typedef std::map<DrxHash, IMaterialSmartPtr> TReplacementMaterials;

	typedef _smart_ptr<ICharacterInstance> ICharacterInstancePtr;
	typedef std::map<DrxHash, ICharacterInstancePtr> TCachedCharacterInstances;

public:
	CBodyDestructibilityProfile(TBodyDestructibilityProfileId id)
		: m_id(id)
		, m_destructibleAttachmentCount(0)
		, m_initialized(false)
		, m_arelevelResourcesCached(false)
	{

	}

	ILINE TBodyDestructibilityProfileId GetId() const { return m_id; }
	ILINE bool IsInitialized() const { return m_initialized; }

	void Reload(SBodyDestructibilityDef& bodyDestructionDef);
	void LoadXmlInfo(SBodyDestructibilityDef& bodyDestructionDef);
	
	void PrepareInstance(CBodyDestrutibilityInstance& instance, float instanceBaseHealth, const SBodyCharacterInfo& bodyInfo);
	void ResetInstance(IEntity& characterEntity, CBodyDestrutibilityInstance& instance);

	ILINE bool AreLevelResourcesCached() const { return m_arelevelResourcesCached; };
	void CacheLevelResources();
	void FlushLevelResourceCache();

	void ProcessDestructiblesHit(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth);
	void ProcessDestructiblesOnExplosion(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth);
	void ProcessDestructionEventByName(tukk eventName, tukk referenceBone, IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo);

	void GetMemoryUsage(IDrxSizer *pSizer) const;

#if !defined(_RELEASE)
	void DebugInstance( IEntity& characterEntity, CBodyDestrutibilityInstance& instance );
	void LogDamage(IEntity& characterEntity, tukk customLog, tukk affectedPart, float inflictedDamage, float hitTypeScale, bool destroyed);
	void LogDestructionEvent(IEntity& characterEntity, tukk eventName);
	void LogHealthRatioEvent(IEntity& characterEntity, i32 ratioEventIndex);
	void LogMessage(IEntity& characterEntity, tukk message);
#else
	ILINE void DebugInstance( IEntity& characterEntity, CBodyDestrutibilityInstance& instance ) {};
	ILINE void LogDamage(IEntity& characterEntity, tukk customLog, tukk affectedPart, float inflictedDamage, float hitTypeScale, bool destroyed) {};
	ILINE void LogDestructionEvent(IEntity& characterEntity, tukk eventName) {};
	ILINE void LogHealthRatioEvent(IEntity& characterEntity, i32 ratioEventIndex) {};
	ILINE void LogMessage(IEntity& characterEntity, tukk message) {};
#endif

private:

	void CacheMaterial(tukk materialName);
	void CacheCharacter(tukk characterName);

	void PreparePartsAndEventsBeforeLoad(const XmlNodeRef& rootNode, SParsingHelper& parsingHelper);

	void LoadDestructibleParts(const XmlNodeRef& destructiblesNode, SParsingHelper& parsingHelper);
	void LoadPart(const XmlNodeRef& partNode, SDestructibleBodyPart& partData, SParsingHelper& parsingHelper);
	void LoadEvents(const XmlNodeRef& eventsNode, SParsingHelper& parsingHelper);
	void LoadEvent(const XmlNodeRef& eventNode, SDestructionEvent& eventData, SParsingHelper& parsingHelper);
	void LoadHealthRatioEvents(const XmlNodeRef& hitRatiosNode, SParsingHelper& parsingHelper);
	void LoadExplosionDeaths(const XmlNodeRef& explosionDeathsNode, SParsingHelper& parsingHelper);
	void LoadMikeDeath(const XmlNodeRef& mikeDeathNode, SParsingHelper& parsingHelper);

	bool GetDestructibleAttachment(DrxHash hashId, SBodyPartQueryResult& bodyPart) const;
	bool GetDestructibleBone(DrxHash hashId, SBodyPartQueryResult& bodyPart) const;

	void ProcessDestructionEvent(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const TDestructionEventId destructionEventId, IAttachmentUpr* pAttachmentUpr, tukk targetEffectBone, const Vec3& noAttachEffectPos = ZERO);
	void ProcessDeathByExplosion(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo);
	void ProcessDeathByPunishHit(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo);
	void ProcessDeathByHit(IEntity& characterEntity, ICharacterInstance& characterInstance, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo);
	void ProcessMikeDeath(IEntity& characterEntity, ICharacterInstance& characterInstance, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo);
	void ProcessHealthRatioEvents(IEntity& characterEntity, ICharacterInstance& characterInstance, IAttachmentUpr* pAttachmentUpr, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float newHealth);

	void HideAttachment(IAttachmentUpr* pAttachmentUpr, tukk attachmentName, u32 hide);

	void ReplaceMaterial(IEntity& characterEntity, ICharacterInstance& characterInstance, CBodyDestrutibilityInstance& instance, tukk material);
	void ResetMaterials(IEntity& characterEntity, ICharacterInstance& characterInstance, CBodyDestrutibilityInstance& instance);
	IMaterial* GetMaterial(tukk materialName) const;

	bool CheckAgainstScriptCondition(IEntity& characterEntity, TDestructionEventId destructionEventId);

	TReplacementMaterials	m_replacementMaterials;
	TCachedCharacterInstances m_cachedCharacterInstaces;

	TDestructibleBodyParts	m_attachments;
	TDestructibleBodyParts	m_bones;

	TDestructionEvents		m_destructionEvents;
	THealthRatioEvents		m_healthRatioEvents;

	SExplosionDeathEvent	m_gibDeath;
	SExplosionDeathEvent	m_nonGibDeath;

	SMikeDeath				m_mikeDeath;

	TBodyDestructibilityProfileId m_id;
	size_t	m_destructibleAttachmentCount;
	bool	  m_initialized;
	bool	  m_arelevelResourcesCached;
};

#endif