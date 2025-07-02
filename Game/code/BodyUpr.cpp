// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Manages body damage/destruction profiles 

-------------------------------------------------------------------------
История:
- 27:07:2010   Extracted from BodyDamage.h/.cpp by Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/BodyUpr.h>

#include <drx3D/Game/BodyDamage.h>
#include <drx3D/Game/BodyDestruction.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/BodyUprCVars.h>

// Reserve size for profile loading
static u32 g_uBodyDamageProfileSize = 8;

/*static */void	CBodyDamageUpr::Warning(tukk szFormat, ...)
{
#ifndef _RELEASE
	if (gEnv && gEnv->pSystem && szFormat)
	{
		va_list	args;
		va_start(args, szFormat);
		GetISystem()->WarningV(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, 0, 0, (string("[BodyDamage] ") + szFormat).c_str(), args);
		va_end(args);
	}
#endif
}

CBodyDamageUpr::CBodyDamageUpr()
: m_bodyDamageIdGen(0)
, m_bodyDestructibilityIdGen(0)
{
	m_bodyDamageProfiles.reserve(g_uBodyDamageProfileSize);
	m_bodyDamageDefinitions.reserve(g_uBodyDamageProfileSize);

	m_bodyDestructibilityProfiles.reserve(g_uBodyDamageProfileSize);
	m_bodyDestructibilityDefinitions.reserve(g_uBodyDamageProfileSize);
}

void CBodyDamageUpr::GetMemoryUsage(IDrxSizer *pSizer) const
{
	pSizer->AddContainer(m_bodyDamageProfiles);
	pSizer->AddContainer(m_bodyDamageDefinitions);

	pSizer->AddContainer(m_bodyDestructibilityDefinitions);
	pSizer->AddContainer(m_bodyDestructibilityProfiles);

	pSizer->AddContainer(m_bodyDamageProfileIdEntityBindings);

	{
		SIZER_COMPONENT_NAME(pSizer, "Body Damage Profiles");
		TBodyDamageProfiles::const_iterator itProfile = m_bodyDamageProfiles.begin();
		TBodyDamageProfiles::const_iterator itProfileEnd = m_bodyDamageProfiles.end();
		for (; itProfile != itProfileEnd; ++itProfile)
		{
			const CBodyDamageProfile *pBodyDamageProfile = *itProfile;
			DRX_ASSERT(pBodyDamageProfile);

			pBodyDamageProfile->GetMemoryUsage(pSizer);
		}
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "Body Destruction Profiles");
		TBodyDestructibilityProfiles::const_iterator itProfile = m_bodyDestructibilityProfiles.begin();
		TBodyDestructibilityProfiles::const_iterator itProfileEnd = m_bodyDestructibilityProfiles.end();
		for (; itProfile != itProfileEnd; ++itProfile)
		{
			const CBodyDestructibilityProfile *pBodyDestructibilityProfile = *itProfile;
			DRX_ASSERT(pBodyDestructibilityProfile);

			pBodyDestructibilityProfile->GetMemoryUsage(pSizer);
		}
	}
}

void CBodyDamageUpr::ReloadBodyDamage()
{
	TBodyDamageProfileIdEntityBindings::const_iterator citEnd = m_bodyDamageProfileIdEntityBindings.end();
	for (TBodyDamageProfileIdEntityBindings::const_iterator cit = m_bodyDamageProfileIdEntityBindings.begin(); cit != citEnd; ++cit)
	{
		const EntityId entityId = cit->first;
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
		const TBodyDamageProfileId profileId = cit->second;
		ReloadBodyDamage(profileId, *pEntity);
	}
}

void CBodyDamageUpr::ReloadBodyDamage(const CActor& actor)
{
	const TBodyDamageProfileId profileId = actor.GetCurrentBodyDamageProfileId();
	IEntity* pEntity = actor.GetEntity();

	ReloadBodyDamage(profileId, *pEntity);
}

void CBodyDamageUpr::ReloadBodyDamage(TBodyDamageProfileId profileId, IEntity& entity)
{
	SBodyCharacterInfo characterInfo;
	SBodyDamageDef bodyDamageDef;
	if (IsValidBodyDamageProfileID(profileId) &&
		GetCharacterInfo(entity, characterInfo) &&
		GetBodyDamageDef(entity, bodyDamageDef, NULL))
	{
		CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		assert(pBodyDamageProfile && pBodyDamageProfile->GetId() == profileId);

		if (pBodyDamageProfile)
			pBodyDamageProfile->Reload(characterInfo, bodyDamageDef, profileId);
	}
}

void CBodyDamageUpr::ReloadBodyDestruction()
{
	DRX_ASSERT(m_bodyDestructibilityDefinitions.size() == m_bodyDestructibilityProfiles.size());

	const size_t profileCount = m_bodyDestructibilityProfiles.size();
	for (size_t profileIdx = 0; profileIdx < profileCount; ++profileIdx)
	{
		m_bodyDestructibilityProfiles[profileIdx]->Reload(m_bodyDestructibilityDefinitions[profileIdx]);
	}
}

TBodyDamageProfileId CBodyDamageUpr::GetBodyDamage(IEntity& characterEntity, tukk damageTable /* = NULL */ )
{
	TBodyDamageProfileId result = INVALID_BODYDAMAGEPROFILEID;

	SBodyDamageDef bodyDamageDef;
	if (GetBodyDamageDef(characterEntity, bodyDamageDef, damageTable))
	{
		result = GetBodyDamage(characterEntity, bodyDamageDef);
	}

	return result;
}


TBodyDamageProfileId CBodyDamageUpr::GetBodyDamage(
	IEntity& characterEntity, 
	tukk bodyDamageFileName, tukk bodyDamagePartsFileName)
{
	SBodyDamageDef bodyDamageDef;
	GetBodyDamageDef(bodyDamageFileName, bodyDamagePartsFileName, bodyDamageDef);
	return GetBodyDamage(characterEntity, bodyDamageDef);
}


TBodyDamageProfileId CBodyDamageUpr::GetBodyDamage(IEntity& characterEntity, SBodyDamageDef& bodyDamageDef)
{
	TBodyDamageProfileId result = INVALID_BODYDAMAGEPROFILEID;

	TBodyDamageDefinitions::const_iterator itDefinition = std::find(m_bodyDamageDefinitions.begin(), m_bodyDamageDefinitions.end(), bodyDamageDef);
	IF_LIKELY (itDefinition != m_bodyDamageDefinitions.end())
	{
		result = itDefinition->bodyDamageProfileId;
	}

	if (result == INVALID_BODYDAMAGEPROFILEID)
	{
		result = LoadBodyDamageDefinition(bodyDamageDef);

		GameWarning("BodyDamage: Loading body damage profile at runtime - Damage: \'%s\' - Parts: \'%s\'", 
			bodyDamageDef.bodyDamageFileName.c_str(),
			bodyDamageDef.bodyPartsFileName.c_str());
	}

	if (!InitializeBodyDamageProfile(characterEntity, result))
	{
		result = INVALID_BODYDAMAGEPROFILEID;
	}

	return result;
}

bool CBodyDamageUpr::CacheBodyDamage(SmartScriptTable pProperties, tukk damageTable /*= NULL*/)
{
	assert(pProperties.GetPtr());

	bool bResult = false;

	if (pProperties.GetPtr())
	{
		// Load definition in if body damage info was found
		SBodyDamageDef bodyDamageDef;
		if (GetBodyDamageDef(pProperties, bodyDamageDef, damageTable))
		{
			bResult = CacheBodyDamage(bodyDamageDef);
		}
	}

	return bResult;
}

bool CBodyDamageUpr::CacheBodyDamage( SBodyDamageDef &bodyDamageDef )
{
	const bool bFound = m_bodyDamageDefinitions.end() != std::find(m_bodyDamageDefinitions.begin(), m_bodyDamageDefinitions.end(), bodyDamageDef);
	bool bResult = (bFound || (INVALID_BODYDAMAGEPROFILEID != LoadBodyDamageDefinition(bodyDamageDef)));
	return bResult;
}


TBodyDamageProfileId CBodyDamageUpr::LoadBodyDamageDefinition(SBodyDamageDef &bodyDamageDef)
{
	// Create a new one
	TBodyDamageProfileId result = m_bodyDamageIdGen++;
	bodyDamageDef.bodyDamageProfileId = result;

	m_bodyDamageDefinitions.push_back(bodyDamageDef);
	assert(m_bodyDamageDefinitions.size() == result+1);

	// Create the profile and load the Xml data in now
	CBodyDamageProfile *pProfile = new CBodyDamageProfile(result);
	assert(pProfile);
	pProfile->LoadXmlInfo(bodyDamageDef);
	m_bodyDamageProfiles.push_back(pProfile);
	assert(m_bodyDamageProfiles.size() == result+1);

	return result;
}

bool CBodyDamageUpr::PhysicalizePlayer(TBodyDamageProfileId profileId, IEntity& characterEntity) const
{
	bool bResult = false;

	SBodyCharacterInfo characterInfo;
	if (IsValidBodyDamageProfileID(profileId) && GetCharacterInfo(characterEntity, characterInfo) && characterInfo.pPhysicalEntity)
	{
		const CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		assert(pBodyDamageProfile && pBodyDamageProfile->GetId() == profileId);

		if (pBodyDamageProfile)
			bResult = pBodyDamageProfile->PhysicalizeEntity(characterInfo.pPhysicalEntity, characterInfo.pIDefaultSkeleton);
	}

	return bResult;
}

float CBodyDamageUpr::GetDamageMultiplier(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo) const
{
	float fMultiplier = 1.0f;

	if (IsValidBodyDamageProfileID(profileId))
	{
		const CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		DRX_ASSERT(pBodyDamageProfile && (pBodyDamageProfile->GetId() == profileId));

		if (pBodyDamageProfile)
			fMultiplier = pBodyDamageProfile->GetDamageMultiplier(characterEntity, hitInfo);
	}

	return fMultiplier;
}

float CBodyDamageUpr::GetExplosionDamageMultiplier(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo) const
{
	float fMultiplier = 1.0f;

	if (IsValidBodyDamageProfileID(profileId))
	{
		const CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		DRX_ASSERT(pBodyDamageProfile && (pBodyDamageProfile->GetId() == profileId));

		if (pBodyDamageProfile)
			fMultiplier = pBodyDamageProfile->GetExplosionDamageMultiplier(characterEntity, hitInfo);
	}

	return fMultiplier;
}

u32 CBodyDamageUpr::GetPartFlags(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo& hitInfo ) const
{
	u32 partFlags = 0;

	if (IsValidBodyDamageProfileID(profileId))
	{
		const CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		assert(pBodyDamageProfile && pBodyDamageProfile->GetId() == profileId);

		if (pBodyDamageProfile)
			partFlags = pBodyDamageProfile->GetPartFlags( characterEntity, hitInfo );
	}

	return partFlags;
}

bool CBodyDamageUpr::GetHitImpulseFilter(TBodyDamageProfileId profileId, IEntity& characterEntity, const HitInfo &hitInfo, SBodyDamageImpulseFilter &impulseFilter) const
{
	bool bResult = false;

	if (IsValidBodyDamageProfileID(profileId))
	{
		const CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileId];
		assert(pBodyDamageProfile && pBodyDamageProfile->GetId() == profileId);

		if (pBodyDamageProfile)
		{
			pBodyDamageProfile->GetHitImpulseFilter( characterEntity, hitInfo, impulseFilter );
			bResult = true;
		}
	}

	return bResult;
}


bool CBodyDamageUpr::GetCharacterInfo(IEntity& characterEntity, SBodyCharacterInfo &outCharacterInfo)
{
	ICharacterInstance* pCharacterInstance = characterEntity.GetCharacter(0);
	IF_UNLIKELY (!pCharacterInstance)
	{
		Warning("Can't calculate body damage for %s '%s' - entity has no character instance", characterEntity.GetClass()->GetName(), characterEntity.GetName());
		return false;
	}

	outCharacterInfo.pSkeletonPose = pCharacterInstance->GetISkeletonPose();
	outCharacterInfo.pIDefaultSkeleton = &pCharacterInstance->GetIDefaultSkeleton();
	IF_UNLIKELY (!outCharacterInfo.pSkeletonPose)
	{
		Warning("Can't calculate body damage for %s '%s' - entity has no skeleton pose", characterEntity.GetClass()->GetName(), characterEntity.GetName());
		return false;
	}

	outCharacterInfo.pAttachmentUpr = pCharacterInstance->GetIAttachmentUpr();
	IF_UNLIKELY (!outCharacterInfo.pAttachmentUpr)
	{
		Warning("Can't calculate body damage for %s '%s' - entity character has no attachment manager", characterEntity.GetClass()->GetName(), characterEntity.GetName());
		return false;
	}

	outCharacterInfo.pPhysicalEntity = outCharacterInfo.pSkeletonPose->GetCharacterPhysics();
	IF_UNLIKELY (!outCharacterInfo.pPhysicalEntity)
	{
		Warning("Can't calculate body damage for %s '%s' - entity has no character physics", characterEntity.GetClass()->GetName(), characterEntity.GetName());
		return false;
	}

	return true;
}

bool CBodyDamageUpr::GetBodyDamageDef(IEntity& characterEntity, SBodyDamageDef &outDef, tukk damageTable)
{
	bool result = false;

	SmartScriptTable propertiesTable;
	IScriptTable *pScriptTable = characterEntity.GetScriptTable();
	if (pScriptTable && pScriptTable->GetValue("Properties", propertiesTable))
	{
		result = GetBodyDamageDef(propertiesTable, outDef, damageTable);
	}

	return result;
}

bool CBodyDamageUpr::GetBodyDamageDef(SmartScriptTable pProperties, SBodyDamageDef &outDef, tukk damageTable)
{
	assert(pProperties.GetPtr());

	bool result = false;

	SmartScriptTable damageScriptTable;
	if (pProperties.GetPtr() && pProperties->GetValue( (damageTable == NULL) ? "Damage" : damageTable, damageScriptTable))
	{
		tukk bodyPartsFileName = 0;
		tukk bodyDamageFileName = 0;
		if (damageScriptTable->GetValue("fileBodyDamageParts", bodyPartsFileName) && 
			damageScriptTable->GetValue("fileBodyDamage", bodyDamageFileName))
		{
			GetBodyDamageDef(bodyDamageFileName, bodyPartsFileName, outDef);
			result = true;
		}
	}

	return result;
}

void CBodyDamageUpr::GetBodyDamageDef( tukk pBodyDamageFile, tukk pBodyPartsFile, SBodyDamageDef &outDef )
{
	IF_UNLIKELY ( (pBodyDamageFile == NULL) || (pBodyPartsFile == NULL) )
	{
		outDef = SBodyDamageDef();
		return;
	}

	outDef.bodyPartsFileName = PathUtil::ToUnixPath(string(pBodyPartsFile));
	outDef.bodyPartsFileName.MakeLower();

	outDef.bodyDamageFileName = PathUtil::ToUnixPath(string(pBodyDamageFile));
	outDef.bodyDamageFileName.MakeLower();

	outDef.crc32BodyPartsFileName = CCrc32::Compute(outDef.bodyPartsFileName.c_str());
	outDef.crc32BodyDamageFileName = CCrc32::Compute(outDef.bodyDamageFileName.c_str());	
}

void CBodyDamageUpr::FlushLevelResourcesCache()
{
	for (TBodyDestructibilityProfiles::iterator profileCit = m_bodyDestructibilityProfiles.begin(); profileCit != m_bodyDestructibilityProfiles.end(); ++profileCit)
	{
		(*profileCit)->FlushLevelResourceCache();
	}
}

TBodyDestructibilityProfileId CBodyDamageUpr::GetBodyDestructibility( IEntity& characterEntity, CBodyDestrutibilityInstance& instance, tukk damageTable /* = NULL */ )
{
	TBodyDestructibilityProfileId result = INVALID_BODYDESTRUCTIBILITYPROFILEID;

	SBodyCharacterInfo characterInfo;
	float characterHealth = 0.0f;

	SBodyDestructibilityDef bodyDestructibilityDef;
	if (GetBodyDestructibilityDef(characterEntity, bodyDestructibilityDef, characterHealth, damageTable))
	{
		TBodyDestructibilityDefinitions::iterator itDefinition = std::find(m_bodyDestructibilityDefinitions.begin(), m_bodyDestructibilityDefinitions.end(), bodyDestructibilityDef);
		if (itDefinition != m_bodyDestructibilityDefinitions.end())
		{
			result = itDefinition->bodyDestructibilityProfileId;
		}

		if (result == INVALID_BODYDESTRUCTIBILITYPROFILEID)
		{
			result = LoadBodyDestructibilityDefinition(bodyDestructibilityDef);
		}
	}

	instance.InitWithProfileId(result);

	if ((result != INVALID_BODYDESTRUCTIBILITYPROFILEID) && (result < m_bodyDestructibilityProfiles.size()) && (GetCharacterInfo(characterEntity, characterInfo)))
	{
		CBodyDestructibilityProfile& bodyDestructionProfile = *m_bodyDestructibilityProfiles[result];
		
		DRX_ASSERT(bodyDestructionProfile.IsInitialized());

		bodyDestructionProfile.PrepareInstance(instance, characterHealth, characterInfo);
	}

	return result;
}

bool CBodyDamageUpr::CacheBodyDestruction( SmartScriptTable pProperties, tukk damageTable /*= NULL*/ )
{
	DRX_ASSERT(pProperties.GetPtr());

	if (pProperties.GetPtr())
	{
		SBodyDestructibilityDef bodyDestructionDef;
		float maxHealth;
		if (GetBodyDestructibilityDef(pProperties, bodyDestructionDef, maxHealth, damageTable))
		{
			CacheBodyDestruction(bodyDestructionDef);
		}
	}

	return false;
}

void CBodyDamageUpr::CacheBodyDestruction( SBodyDestructibilityDef &bodyDestructionDef )
{
	TBodyDestructibilityProfileId profileId = INVALID_BODYDESTRUCTIBILITYPROFILEID;
	TBodyDestructibilityDefinitions::const_iterator definitionCit = std::find(m_bodyDestructibilityDefinitions.begin(), m_bodyDestructibilityDefinitions.end(), bodyDestructionDef);
	if(definitionCit != m_bodyDestructibilityDefinitions.end())
	{
		profileId = definitionCit->bodyDestructibilityProfileId;
	}
	else
	{
		profileId = LoadBodyDestructibilityDefinition(bodyDestructionDef);
	}

	if ((profileId != INVALID_BODYDESTRUCTIBILITYPROFILEID) && (profileId < m_bodyDestructibilityProfiles.size()))
	{
		m_bodyDestructibilityProfiles[profileId]->CacheLevelResources();
	}
}

bool CBodyDamageUpr::GetBodyDestructibilityDef(IEntity& characterEntity, SBodyDestructibilityDef &outDef, float& maxHealth, tukk damageTable)
{
	bool result = false;

	SmartScriptTable propertiesTable;
	IScriptTable *pScriptTable = characterEntity.GetScriptTable();
	if (pScriptTable && pScriptTable->GetValue("Properties", propertiesTable))
	{
		result = GetBodyDestructibilityDef(propertiesTable, outDef, maxHealth, damageTable);
	}

	return result;
}

bool CBodyDamageUpr::GetBodyDestructibilityDef(SmartScriptTable pProperties, SBodyDestructibilityDef &outDef, float& maxHealth, tukk damageTable)
{
	DRX_ASSERT(pProperties.GetPtr());

	bool result = false;

	SmartScriptTable damageScriptTable;
	if (pProperties.GetPtr() && pProperties->GetValue( (damageTable == NULL) ? "Damage" : damageTable, damageScriptTable))
	{
		tukk bodyDestructibilityFileName = 0;
		if (damageScriptTable->GetValue("fileBodyDestructibility", bodyDestructibilityFileName))
		{
			damageScriptTable->GetValue("health", maxHealth);

			GetBodyDestructibilityDef(bodyDestructibilityFileName, outDef);

			result = true;
		}
	}

	return result;
}

void CBodyDamageUpr::GetBodyDestructibilityDef( tukk pBodyDestructibilityFile, SBodyDestructibilityDef &outDef )
{
	outDef.bodyDestructibilityFileName = PathUtil::ToUnixPath(string(pBodyDestructibilityFile));
	outDef.bodyDestructibilityFileName.MakeLower();

	outDef.crc32BodyDestructibilityFileName = CCrc32::Compute(outDef.bodyDestructibilityFileName.c_str());
}

TBodyDamageProfileId CBodyDamageUpr::LoadBodyDestructibilityDefinition( SBodyDestructibilityDef &bodyDestructibilityDef )
{
	// Create a new one
	TBodyDestructibilityProfileId result = m_bodyDestructibilityIdGen++;
	bodyDestructibilityDef.bodyDestructibilityProfileId = result;

	m_bodyDestructibilityDefinitions.push_back(bodyDestructibilityDef);
	DRX_ASSERT(m_bodyDestructibilityDefinitions.size() == (result+1));

	// Create the profile and load the Xml data in now
	CBodyDestructibilityProfile *pProfile = new CBodyDestructibilityProfile(result);
	DRX_ASSERT(pProfile);
	pProfile->LoadXmlInfo(bodyDestructibilityDef);
	m_bodyDestructibilityProfiles.push_back(pProfile);
	DRX_ASSERT(m_bodyDestructibilityProfiles.size() == (result+1));

	return result;
}

void CBodyDamageUpr::ResetInstance( IEntity& characterEntity, CBodyDestrutibilityInstance& instance )
{
	const TBodyDestructibilityProfileId profileId = instance.GetProfileId();

	if (profileId < (TBodyDestructibilityProfileId)m_bodyDestructibilityProfiles.size())
	{
		m_bodyDestructibilityProfiles[profileId]->ResetInstance(characterEntity, instance);
	}
}

void CBodyDamageUpr::ProcessDestructiblesHit( IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth )
{
	const TBodyDestructibilityProfileId profileId = instance.GetProfileId();

	if (profileId < (TBodyDestructibilityProfileId)m_bodyDestructibilityProfiles.size())
	{
		m_bodyDestructibilityProfiles[profileId]->ProcessDestructiblesHit(characterEntity, instance, hitInfo, previousHealth, newHealth);
	}
}

void CBodyDamageUpr::ProcessDestructiblesOnExplosion( IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo, const float previousHealth, const float newHealth )
{
	const TBodyDestructibilityProfileId profileId = instance.GetProfileId();

	if (profileId < (TBodyDestructibilityProfileId)m_bodyDestructibilityProfiles.size())
	{
		m_bodyDestructibilityProfiles[profileId]->ProcessDestructiblesOnExplosion(characterEntity, instance, hitInfo, previousHealth, newHealth);
	}
}

void CBodyDamageUpr::ProcessDestructionEventByName( tukk eventName, tukk referenceBone, IEntity& characterEntity, CBodyDestrutibilityInstance& instance, const HitInfo& hitInfo )
{
	const TBodyDestructibilityProfileId profileId = instance.GetProfileId();

	if (profileId < (TBodyDestructibilityProfileId)m_bodyDestructibilityProfiles.size())
	{
		m_bodyDestructibilityProfiles[profileId]->ProcessDestructionEventByName(eventName, referenceBone, characterEntity, instance, hitInfo);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#if !defined(_RELEASE)

void CBodyDamageUpr::DebugBodyDestructionInstance( IEntity& characterEntity, CBodyDestrutibilityInstance& instance )
{
	if (!CBodyUprCVars::IsBodyDestructionDebugFilterFor(characterEntity.GetName()))
		return;


	const TBodyDestructibilityProfileId profileId = instance.GetProfileId();

	if (profileId < (TBodyDestructibilityProfileId)m_bodyDestructibilityProfiles.size())
	{
		m_bodyDestructibilityProfiles[profileId]->DebugInstance(characterEntity, instance);
	}
}

#endif

//////////////////////////////////////////////////////////////////////////
void CBodyDamageUpr::RegisterBodyDamageProfileIdBinding(EntityId entityId, TBodyDamageProfileId profileId)
{
	m_bodyDamageProfileIdEntityBindings[entityId] = profileId;
}

void CBodyDamageUpr::UnregisterBodyDamageProfileIdBinding(EntityId entityId)
{
	m_bodyDamageProfileIdEntityBindings.erase(entityId);
}

TBodyDamageProfileId CBodyDamageUpr::FindBodyDamageProfileIdBinding(EntityId entityId) const
{
	TBodyDamageProfileIdEntityBindings::const_iterator cit = m_bodyDamageProfileIdEntityBindings.find(entityId);
	if (cit != m_bodyDamageProfileIdEntityBindings.end())
	{
		return cit->second;
	}
	return INVALID_BODYDAMAGEPROFILEID;
}


bool CBodyDamageUpr::InitializeBodyDamageProfile(IEntity& characterEntity, const TBodyDamageProfileId profileID)
{
	IF_UNLIKELY (!IsValidBodyDamageProfileID((profileID)))
	{
		return false;
	}

	CBodyDamageProfile *pBodyDamageProfile = m_bodyDamageProfiles[profileID];
	assert(pBodyDamageProfile && pBodyDamageProfile->GetId() == profileID);
	PREFAST_ASSUME(pBodyDamageProfile);
	bool bInitialized = (pBodyDamageProfile && pBodyDamageProfile->IsInitialized());

	if (!bInitialized)
	{
		SBodyCharacterInfo characterInfo;
		if (GetCharacterInfo(characterEntity, characterInfo))
		{
			bInitialized = pBodyDamageProfile->Init(characterInfo);
		}
	}

	if (!bInitialized)
	{
		GameWarning("BodyDamage: Failed to initialize body damage profile using player \'%s\'", characterEntity.GetName());
	}

	return bInitialized;
}
