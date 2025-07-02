// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/BodyDefinitions.h>

CBodyDestrutibilityInstance::~CBodyDestrutibilityInstance()
{
	CleanUpOriginalMaterials();
	DeleteMikeAttachmentEntity();
}

void CBodyDestrutibilityInstance::ReserveSlots( i32k totalAttachmentsCount, i32k destructibleAttachmentsCount, i32k destructibleBonesCount, i32k destructionEventsCount )
{
	DRX_ASSERT(destructibleAttachmentsCount >= 0);
	DRX_ASSERT(destructibleBonesCount >= 0);
	DRX_ASSERT(destructionEventsCount >= 0);

	m_attachmentStatus.reserve(destructibleAttachmentsCount);
	m_boneStatus.reserve(destructibleBonesCount);

	m_availableDestructionEvents.resize(destructionEventsCount, true);

	m_originalMaterials.reserve( totalAttachmentsCount );

	DeleteMikeAttachmentEntity();
}

void CBodyDestrutibilityInstance::Reset()
{
	for (u32 i = 0; i < m_attachmentStatus.size(); ++i)
	{
		m_attachmentStatus[i].Reset();
	}

	for (u32 i = 0; i < m_boneStatus.size(); ++i)
	{
		m_boneStatus[i].Reset();
	}

	for (u32 i = 0; i < m_availableDestructionEvents.size(); ++i)
	{
		m_availableDestructionEvents[i] = true;
	}

	m_eventsModified = false;
	m_lastEventForHitReactionsCrc = 0;
	m_currentHealthRatioEventIdx = 0;

	CleanUpOriginalMaterials();
	DeleteMikeAttachmentEntity();
}

void CBodyDestrutibilityInstance::DeleteMikeAttachmentEntity()
{
	if (m_mikeAttachmentEntityId)
	{
		gEnv->pEntitySystem->RemoveEntity(m_mikeAttachmentEntityId);
		m_mikeAttachmentEntityId = 0;
	}
}

void CBodyDestrutibilityInstance::InitWithProfileId( const TBodyDestructibilityProfileId profileId )
{
	m_id = profileId;
	m_attachmentStatus.clear();
	m_boneStatus.clear();
	m_availableDestructionEvents.clear();
}

void CBodyDestrutibilityInstance::InitializeMikeDeath( const EntityId entityId, float alphaTestFadeOutTime, float alphaTestFadeOutDelay, float alphaTestMax )
{
	DRX_ASSERT(m_mikeAttachmentEntityId == 0);

	m_mikeAttachmentEntityId = entityId;
	m_mikeExplodeAlphaTestFadeOutTimer = alphaTestFadeOutTime + alphaTestFadeOutDelay;
	m_mikeExplodeAlphaTestFadeOutScale = -(1.0f / alphaTestFadeOutTime);
	m_mikeExplodeAlphaTestMax = alphaTestMax;
}

void CBodyDestrutibilityInstance::ReplaceMaterial( IEntity& characterEntity, ICharacterInstance& characterInstance, IMaterial& replacementMaterial )
{
	IMaterial* pCurrentMaterial = characterInstance.GetIMaterial();
	if ((pCurrentMaterial != NULL) && stricmp(pCurrentMaterial->GetName(), replacementMaterial.GetName()))
	{
		characterEntity.SetSlotMaterial(0, &replacementMaterial);
	}

	const bool storeOriginalReplacementMaterials = m_originalMaterials.empty();

	IAttachmentUpr *pAttachmentUpr = characterInstance.GetIAttachmentUpr();
	DRX_ASSERT(pAttachmentUpr);

	i32k attachmentCount = pAttachmentUpr->GetAttachmentCount();
	for (i32 attachmentIdx = 0; attachmentIdx < attachmentCount; ++attachmentIdx)
	{
		IAttachmentObject *pAttachmentObject = pAttachmentUpr->GetInterfaceByIndex(attachmentIdx)->GetIAttachmentObject();
		if (pAttachmentObject)
		{
			IMaterial* pAttachMaterial = (IMaterial*)pAttachmentObject->GetBaseMaterial();
			if ((pAttachMaterial != NULL) && stricmp(pAttachMaterial->GetName(), replacementMaterial.GetName()))
			{
				if (storeOriginalReplacementMaterials)
				{
					IMaterial* pOriginalReplacementMaterial = pAttachmentObject->GetReplacementMaterial();
					if(pOriginalReplacementMaterial != NULL)
					{
						pOriginalReplacementMaterial->AddRef();
						m_originalMaterials.push_back(TAttachmentMaterialPair((u32)attachmentIdx, pOriginalReplacementMaterial));
					}
				}

				pAttachmentObject->SetReplacementMaterial(&replacementMaterial);
			}
		}

	}
}

void CBodyDestrutibilityInstance::ResetMaterials( IEntity& characterEntity, ICharacterInstance& characterInstance )
{
	characterEntity.SetSlotMaterial(0, NULL);

	IAttachmentUpr *pAttachmentUpr = characterInstance.GetIAttachmentUpr();
	DRX_ASSERT(pAttachmentUpr);
	
	u32k attachmentCount = (u32)pAttachmentUpr->GetAttachmentCount();

	for(TOriginalMaterials::iterator it = m_originalMaterials.begin(); it != m_originalMaterials.end(); ++it)
	{
		IAttachmentObject *pAttachmentObject = (it->first < attachmentCount) ? pAttachmentUpr->GetInterfaceByIndex(it->first)->GetIAttachmentObject() : NULL;
		if (pAttachmentObject)
		{
			pAttachmentObject->SetReplacementMaterial(it->second);
			it->second->Release();
		}
	}

	m_originalMaterials.clear();
}

void CBodyDestrutibilityInstance::Update( float frameTime )
{
	if (m_mikeAttachmentEntityId && m_mikeExplodeAlphaTestFadeOutTimer > 0.0f)
	{
		m_mikeExplodeAlphaTestFadeOutTimer -= min(frameTime, m_mikeExplodeAlphaTestFadeOutTimer);

		float alphaTestValue = max(min(m_mikeExplodeAlphaTestFadeOutTimer*m_mikeExplodeAlphaTestFadeOutScale+1.0f, 1.0f), 0.0f) * m_mikeExplodeAlphaTestMax;
		IEntity* pJellyEntity = gEnv->pEntitySystem->GetEntity(m_mikeAttachmentEntityId);
		ICharacterInstance* pJellyCharacter = pJellyEntity ? pJellyEntity->GetCharacter(0) : 0;
		IMaterial* pJellyMaterial = pJellyCharacter ? pJellyCharacter->GetIMaterial() : 0;
		if (pJellyMaterial)
		{
			i32 numSubMaterials = pJellyMaterial->GetSubMtlCount();
			for (i32 i = 0; i < numSubMaterials; ++i)
			{
				IMaterial* pSubJellyMaterial = pJellyMaterial->GetSubMtl(i);
				pSubJellyMaterial->SetGetMaterialParamFloat("alpha", alphaTestValue, false);
			}
		}
	}
}

void CBodyDestrutibilityInstance::CleanUpOriginalMaterials()
{
	for(TOriginalMaterials::iterator it = m_originalMaterials.begin(); it != m_originalMaterials.end(); ++it)
	{
		it->second->Release();
	}

	m_originalMaterials.clear();
}