// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Manages body damage/destruction profiles 

-------------------------------------------------------------------------
История:
- 27:07:2010   Extracted from BodyDamage.h/.cpp by Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef BODY_DAMAGE_MANAGER_H
#define BODY_DAMAGE_MANAGER_H

#include <drx3D/Game/BodyDefinitions.h>
#include <drx3D/Game/BodyDamage.h>
#include <drx3D/Game/BodyDestruction.h>
#include <drx3D/CoreX/Containers/VectorMap.h>

class CActor;
struct HitInfo;

class CBodyDamageUpr
{

public:
	static void	Warning(tukk szFormat, ...);

	CBodyDamageUpr();

	void GetMemoryUsage(IDrxSizer *pSizer) const;

	void ReloadBodyDamage();
	void ReloadBodyDamage(const CActor& actor);
	void ReloadBodyDamage(TBodyDamageProfileId profileId, IEntity& entity);
	void ReloadBodyDestruction();

	//================== BODY PARTS/DAMAGE ======================================

	// Returns the profile Id for the body damage to be used by this player
	TBodyDamageProfileId GetBodyDamage(IEntity& characterEntity, tukk damageTable = NULL );
	TBodyDamageProfileId GetBodyDamage(IEntity& characterEntity, SBodyDamageDef& bodyDamageDef);
	TBodyDamageProfileId GetBodyDamage(IEntity& characterEntity, tukk bodyDamageFileName, tukk bodyDamagePartsFileName);
	ILINE bool IsValidBodyDamageProfileID(const TBodyDamageProfileId bodyDamageProfileID) const { return (bodyDamageProfileID < m_bodyDamageProfiles.size()); } // Note: INVALID_BODYDAMAGEPROFILEID will most certainly be > m_bodyDamageProfiles.size().

	// Helper to find and cache (pre-load) the body damage in the given properties table
	bool CacheBodyDamage(SmartScriptTable pProperties, tukk damageTable = NULL);
	bool CacheBodyDamage(SBodyDamageDef &bodyDamageDef);

	// Physicalize the player using the given profile
	bool PhysicalizePlayer(TBodyDamageProfileId profileId, IEntity& characterEntity) const;

	// Returns the damage multiplier to be used for this hit info
	float GetDamageMultiplier(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo) const;
	float GetExplosionDamageMultiplier(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo) const;
	u32 GetPartFlags(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo) const;

	bool GetHitImpulseFilter(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo &hitInfo, SBodyDamageImpulseFilter &impulseFilter) const;

	// Optionally, we can ask the body damage manager store the link between an entity and a profileId.
	void RegisterBodyDamageProfileIdBinding(EntityId entityId, TBodyDamageProfileId profileId);
	void UnregisterBodyDamageProfileIdBinding(EntityId entityId);
	TBodyDamageProfileId FindBodyDamageProfileIdBinding(EntityId entityId) const;

	//=================== BODY DESTRUCTION ===========================================

	// Helper to find and cache (pre-load) the body destruction in the given properties table
	bool CacheBodyDestruction(SmartScriptTable pProperties, tukk damageTable = NULL);
	void CacheBodyDestruction(SBodyDestructibilityDef &bodyDestructionDef);

	// Flush level resources (level unload)
	void FlushLevelResourcesCache();

	// Returns the profile Id for the body destructibility to be used by this player
	TBodyDestructibilityProfileId GetBodyDestructibility(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, tukk damageTable = NULL);

	// Reset instance (attachments, health, etc) to initial status
	void ResetInstance(IEntity& characterEntity, CBodyDestrutibilityInstance& instance);

	// On hit/explosion, process destructible attachments/bones
	void ProcessDestructiblesHit(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth);
	void ProcessDestructiblesOnExplosion(IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth);
	void ProcessDestructionEventByName( tukk eventName, tukk referenceBone, IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo );

#if !defined(_RELEASE)
	void DebugBodyDestructionInstance(IEntity& characterEntity, CBodyDestrutibilityInstance& instance);
#else
	ILINE void DebugBodyDestructionInstance(IEntity& characterEntity, CBodyDestrutibilityInstance& instance){};
#endif

	static void GetBodyDamageDef(tukk pBodyDamageFile, tukk pBodyPartsFile, SBodyDamageDef &outDef);
	static void GetBodyDestructibilityDef(tukk pBodyDestructibilityFile, SBodyDestructibilityDef &outDef);

private:
	TBodyDamageProfileId LoadBodyDamageDefinition(SBodyDamageDef &bodyDamageDef);
	TBodyDamageProfileId LoadBodyDestructibilityDefinition(SBodyDestructibilityDef &bodyDestructibilityDef);

	bool InitializeBodyDamageProfile(IEntity& characterEntity, const TBodyDamageProfileId profileID);

	static bool GetCharacterInfo(IEntity& characterEntity, SBodyCharacterInfo &outCharacterInfo);

	static bool GetBodyDamageDef(IEntity& characterEntity, SBodyDamageDef &outDef, tukk damageTable);
	static bool GetBodyDamageDef(SmartScriptTable pProperties, SBodyDamageDef &outDef, tukk damageTable);

	static bool GetBodyDestructibilityDef(IEntity& characterEntity, SBodyDestructibilityDef &outDef, float& maxHealth, tukk damageTable);
	static bool GetBodyDestructibilityDef(SmartScriptTable pProperties, SBodyDestructibilityDef &outDef, float& maxHealth, tukk damageTable);


private:
	typedef std::vector<SBodyDamageDef> TBodyDamageDefinitions;
	typedef std::vector< _smart_ptr<CBodyDamageProfile> > TBodyDamageProfiles;

	typedef std::vector<SBodyDestructibilityDef> TBodyDestructibilityDefinitions;
	typedef std::vector< _smart_ptr<CBodyDestructibilityProfile> > TBodyDestructibilityProfiles;

	typedef VectorMap<EntityId, TBodyDestructibilityProfileId> TBodyDamageProfileIdEntityBindings;

	TBodyDamageDefinitions m_bodyDamageDefinitions;
	TBodyDamageProfiles m_bodyDamageProfiles;
	TBodyDamageProfileId m_bodyDamageIdGen;

	TBodyDestructibilityDefinitions m_bodyDestructibilityDefinitions;
	TBodyDestructibilityProfiles m_bodyDestructibilityProfiles;
	TBodyDestructibilityProfileId m_bodyDestructibilityIdGen;

	TBodyDamageProfileIdEntityBindings m_bodyDamageProfileIdEntityBindings;
};

#endif //BODY_DAMAGE_MANAGER_H