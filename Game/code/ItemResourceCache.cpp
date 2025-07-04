// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/***********************************************************************************
------------------------------------------------------------------------------------
$Id$
$DateTime$
Описание:	Caches geometry, particles etc. used by items to prevent repeated file 
access

------------------------------------------------------------------------------------
История:
- 06:05:2010   10:54 : Part Moved from ItemSharedParams Part Written by Claire Allan

************************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ItemResourceCache.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Animation/IDrxAnimation.h>

//------------------------------------------------------------------------

void CItemGeometryCache::CacheGeometry( tukk objectFileName, bool useCgfStreaming, u32 nLoadingFlags )
{
	bool validName = (objectFileName && objectFileName[0]);

	if (validName)
	{
		u32k fileNameHash = CCrc32::Compute(objectFileName);
		const bool objectAlreadyCached = IsStaticObjectCached(fileNameHash) || IsCharacterCached(fileNameHash);
		if (objectAlreadyCached)
			return;

		stack_string name(objectFileName);
		PathUtil::ToUnixPath(name);
		name.MakeLower();

		IMaterial* pGeometryMaterial = NULL;

		tukk  ext = PathUtil::GetExt(name.c_str());

		if (strcmp(ext, "cdf") == 0 || strcmp(ext, "chr") == 0 || strcmp(ext, "skin") == 0 || strcmp(ext, "cga") == 0)
		{
			if (gEnv->IsEditor())
			{
				ICharacterInstance *pChararacter = gEnv->pCharacterUpr->CreateInstance(objectFileName, nLoadingFlags);
				if (pChararacter)
				{
					m_editorCachedCharacters.insert(TEditorCacheCharacterMap::value_type(fileNameHash, TCharacterInstancePtr(pChararacter)));
					
					pGeometryMaterial = pChararacter->GetIMaterial();
				}
			}
			else
			{
				bool hasLoaded = gEnv->pCharacterUpr->LoadAndLockResources( objectFileName, CA_DoNotStreamStaticObjects );
				if (hasLoaded)
				{
				}
			}
		}
		else
		{
			IStatObj *pStatObj = gEnv->p3DEngine->LoadStatObj(objectFileName, 0, 0, useCgfStreaming, nLoadingFlags);
			if (pStatObj)
			{
				m_cachedStaticObjects.insert(TCacheStaticObjectMap::value_type(CCrc32::Compute(objectFileName), TStatObjectPtr(pStatObj)));
				pGeometryMaterial = pStatObj->GetMaterial();
			}
		}

		if(pGeometryMaterial)
		{
			pGeometryMaterial->RequestTexturesLoading(0.0f);
		}
	}
}

bool CItemGeometryCache::IsCharacterCached( u32k fileNameHash ) const
{
	if (gEnv->IsEditor())
	{
		return (m_editorCachedCharacters.find(fileNameHash) != m_editorCachedCharacters.end());
	}
	else
	{
		return 0;
	}
}

bool CItemGeometryCache::IsStaticObjectCached( u32k fileNameHash ) const
{
	return (m_cachedStaticObjects.find(fileNameHash) != m_cachedStaticObjects.end());
}

void CItemGeometryCache::GetMemoryStatistics( IDrxSizer *s )
{
	s->AddContainer(m_editorCachedCharacters);
	s->AddContainer(m_cachedStaticObjects);
}

void CItemGeometryCache::CacheGeometryFromXml( XmlNodeRef node, bool useCgfStreaming, u32 nLoadingFlags/*=0*/ )
{
	DRX_ASSERT(node != 0);

	tukk attrKey = NULL;
	tukk attrValue = NULL;

	i32k attrCount = node->getNumAttributes();
	for(i32 attrIndex = 0; attrIndex < attrCount; attrIndex++)
	{
		if(node->getAttributeByIndex(attrIndex, &attrKey, &attrValue))
		{
			CheckAndCacheGeometryFromXmlAttr(attrValue, useCgfStreaming, nLoadingFlags);
		}
	}

	i32k childCount = node->getChildCount();
	for(i32 childIndex = 0; childIndex < childCount; childIndex++)
	{
		XmlNodeRef child = node->getChild(childIndex);
		CacheGeometryFromXml(child, useCgfStreaming, nLoadingFlags);
	}
}

void CItemGeometryCache::CheckAndCacheGeometryFromXmlAttr( tukk pAttr, bool useCgfStreaming, u32 nLoadingFlags )
{
	//look for objects/*.ext

	const size_t len = strlen(pAttr);
	if(len > 12)	//longer than objects/*.ext
	{
		if(pAttr[len - 4] == '.')	//check for file type .
		{
			tukk ext = &pAttr[len - 3];
			if(strcmpi("cga", ext) == 0 || strcmpi("cgf", ext) == 0 || strcmpi("chr", ext) == 0 || strcmpi("cdf", ext) == 0)
			{
				CacheGeometry(pAttr, useCgfStreaming, nLoadingFlags);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CItemParticleEffectCache::CacheParticle(tukk particleFileName)
{
	bool validName = (particleFileName && particleFileName[0]);

	if (validName)
	{
		if (IsParticleCached(particleFileName))
			return;

		IParticleEffect* pParticle = gEnv->pParticleUpr->FindEffect(particleFileName, "CItemParticleEffectCache::CacheParticle");
		if (pParticle)
		{
			m_cachedParticles.insert(TCacheParticleMap::value_type(CCrc32::Compute(particleFileName), TParticleEffectPtr(pParticle)));
		}
	}
}

IParticleEffect* CItemParticleEffectCache::GetCachedParticle(tukk particleFileName) const
{
	u32 nameHash = CCrc32::Compute(particleFileName);

	TCacheParticleMap::const_iterator particleCIt = m_cachedParticles.find(nameHash);

	if (particleCIt != m_cachedParticles.end())
	{
		return particleCIt->second;
	}

	return NULL;
}

void CItemParticleEffectCache::GetMemoryStatistics(IDrxSizer *s)
{
	s->AddContainer(m_cachedParticles);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CItemMaterialAndTextureCache::CacheTexture( tukk textureFileName, bool noStreaming /*= false*/ )
{
	const bool validName = (textureFileName && textureFileName[0]);

	if (validName)
	{
		if (IsTextureCached(textureFileName))
			return;

		if (!gEnv->pRenderer)
		{
			return;
		}

		ITexture* pTexture = gEnv->pRenderer->EF_LoadTexture(textureFileName, noStreaming ? FT_DONT_STREAM : 0);
		if (pTexture)
		{
			m_cachedTextures.insert(TCacheTextureMap::value_type(CCrc32::Compute(textureFileName), TTexturePtr(pTexture)));
			
			// Textures have an initial reference count of 1, so we need to release it here to avoid a leak
			pTexture->Release();
		}
	}
}

ITexture* CItemMaterialAndTextureCache::GetCachedTexture( tukk textureFileName ) const
{
	u32k nameHash = CCrc32::Compute(textureFileName);

	TCacheTextureMap::const_iterator textureCIt = m_cachedTextures.find(nameHash);

	if (textureCIt != m_cachedTextures.end())
	{
		return textureCIt->second;
	}

	return NULL;
}

void CItemMaterialAndTextureCache::CacheMaterial( tukk materialFileName )
{
	const bool validName = (materialFileName && materialFileName[0]);

	if (validName)
	{
		if (IsMaterialCached(materialFileName))
			return;

		IMaterial* pMaterial = gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial(materialFileName);
		if (pMaterial)
		{
			m_cachedMaterials.insert(TCacheMaterialMap::value_type(CCrc32::Compute(materialFileName), TMaterialPtr(pMaterial)));
		}
	}
}

IMaterial* CItemMaterialAndTextureCache::GetCachedMaterial( tukk materialFileName ) const
{
	u32 nameHash = CCrc32::Compute(materialFileName);

	TCacheMaterialMap::const_iterator materialCIt = m_cachedMaterials.find(nameHash);

	if (materialCIt != m_cachedMaterials.end())
	{
		return materialCIt->second;
	}

	return NULL;
}

void CItemMaterialAndTextureCache::FlushCaches()
{
	m_cachedTextures.clear();
	m_cachedMaterials.clear();
}

void CItemMaterialAndTextureCache::GetMemoryStatistics( IDrxSizer *s )
{
	s->AddContainer(m_cachedTextures);
	s->AddContainer(m_cachedMaterials);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CItemPrefetchCHRUpr::CItemPrefetchCHRUpr()
	: m_fTimeout(2.0f)
{
	m_prefetches.reserve(8);
}

CItemPrefetchCHRUpr::~CItemPrefetchCHRUpr()
{
	Reset();
}

void CItemPrefetchCHRUpr::Reset()
{
	ICharacterUpr* pCM = gEnv->pCharacterUpr;

	if (pCM)
	{
		for (PrefetchSlotVec::iterator it = m_prefetches.begin(), itEnd = m_prefetches.end(); it != itEnd; ++ it)
		{
			pCM->StreamKeepCharacterResourcesResident(it->geomName.c_str(), 0, false);
		}
	}
	
	stl::free_container(m_prefetches);
}

void CItemPrefetchCHRUpr::Update(float fCurrTime)
{
	ICharacterUpr* pCM = gEnv->pCharacterUpr;

	if (pCM)
	{
		float fTime = fCurrTime - m_fTimeout;

		PrefetchSlotVec::iterator itWrite = m_prefetches.begin();
		for (PrefetchSlotVec::iterator it = itWrite, itEnd = m_prefetches.end(); it != itEnd; ++ it)
		{
			if (it->requestTime <= fTime)
			{
				// Timed out.
				pCM->StreamKeepCharacterResourcesResident(it->geomName.c_str(), 0, false);
			}
			else
			{
				if (itWrite != it)
					*itWrite = *it;

				++ itWrite;
			}
		}

		m_prefetches.erase(itWrite, m_prefetches.end());
	}
}

void CItemPrefetchCHRUpr::Prefetch(const ItemString& geomName)
{
	ICharacterUpr* pCM = gEnv->pCharacterUpr;

	if (pCM)
	{
		float fTime = gEnv->pTimer->GetCurrTime();

		// Don't expect there to be many prefetches in flight, so linear search will be fine
		for (PrefetchSlotVec::iterator it = m_prefetches.begin(), itEnd = m_prefetches.end(); it != itEnd; ++ it)
		{
			if (it->geomName == geomName)
			{
				// Already exists. Reset the timer.
				it->requestTime = fTime;
				return;
			}
		}

		PrefetchSlot ps;
		ps.geomName = geomName;
		ps.requestTime = fTime;
		m_prefetches.push_back(ps);

		pCM->StreamKeepCharacterResourcesResident(geomName.c_str(), 0, true, true);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define INVENTORY_ITEM_DBA_PRELOADED_IN_USE 8	//At max player can have around 6 items at a time in Inventory
#define MAX_ITEM_DBA_PRELOADED	2				//Non used, but left in memory to reduce memory thrashing

CItemAnimationDBAUpr::CItemAnimationDBAUpr()
{
	m_inUseDBAs.reserve(INVENTORY_ITEM_DBA_PRELOADED_IN_USE);	//This one is allowed to grow (in normal circumstances it will never need more entries)
	m_preloadedDBASlots.reserve(MAX_ITEM_DBA_PRELOADED);		//This one is not allowed to grow		
}

void CItemAnimationDBAUpr::AddDBAUser( const ItemString& animationGroup, const CItemSharedParams* pItemParams, const EntityId itemUserId )
{
	DRX_ASSERT(pItemParams);

	if (!IsDbaManagementEnabled())
		return;

	i32 dbaIndex = IsDbaAlreadyInUse(animationGroup);

	if (dbaIndex != -1)
	{
		SItemDBAInfo& dbaInfo = GetDbaInUse(dbaIndex);
		dbaInfo.AddUser(itemUserId);
	}
	else
	{
		m_inUseDBAs.push_back(TItemDBAPair(animationGroup, SItemDBAInfo()));
		SItemDBAInfo& dbaInfo = m_inUseDBAs[(m_inUseDBAs.size() - 1)].second;

		i32k preloadIdx = IsDbaAlreadyInPreloadList(animationGroup);
		bool animationsWerePreloaded = (preloadIdx != -1);

		for (u32 i=0; i < pItemParams->animationPrecache.size(); i++)
		{
			if (!pItemParams->animationPrecache[i].thirdPerson)
			{
				dbaInfo.AddDbaPath(pItemParams->animationPrecache[i].DBAfile);

				if (!animationsWerePreloaded)
				{
					if (!gEnv->pCharacterUpr->DBA_LockStatus(pItemParams->animationPrecache[i].DBAfile.c_str(), 1, ICharacterUpr::eStreamingDBAPriority_Normal))
					{
						DrxLog("Failed to lock DBA %s", pItemParams->animationPrecache[i].DBAfile.c_str());
					}
				}
			}
		}

		if (animationsWerePreloaded)
		{
			//Already loaded remove from preload list
			RemoveFromPreloadListAndDoNotUnload(animationGroup);
		}

		dbaInfo.AddUser(itemUserId);
	}
}

void CItemAnimationDBAUpr::RemoveDBAUser( const ItemString& animationGroup, const EntityId itemUserId )
{
	if (!IsDbaManagementEnabled())
		return;

	i32 dbaIndex = IsDbaAlreadyInUse(animationGroup);

	if (dbaIndex != -1)
	{
		SItemDBAInfo& dbaInfo = GetDbaInUse(dbaIndex);
		dbaInfo.RemoveUser(itemUserId);
		if (dbaInfo.GetUserCount() == 0)
		{
			//Don't unload yet, just put it into the pre-loaded list slots (replacing oldest entry) 
			FreeSlotInPreloadListIfNeeded();

			dbaInfo.SetRequestedTime(gEnv->pTimer->GetCurrTime());
			m_preloadedDBASlots.push_back(TItemDBAPair(animationGroup, dbaInfo));

			//Remove from in use list (swap with last element)
			i32k lastElement = (m_inUseDBAs.size() - 1);
			m_inUseDBAs[dbaIndex] = m_inUseDBAs[lastElement];
			m_inUseDBAs.pop_back();
		}
	}
}

void CItemAnimationDBAUpr::RequestDBAPreLoad( const ItemString& animationGroup, const CItemSharedParams* pItemParams )
{
	DRX_ASSERT(pItemParams);

	if (!IsDbaManagementEnabled())
		return;

	if (IsDbaAlreadyInUse(animationGroup) != -1)
		return;

	const float refreshTime = gEnv->pTimer->GetCurrTime();

	i32 preloadDbaIndex = IsDbaAlreadyInPreloadList(animationGroup);

	if (preloadDbaIndex >= 0)
	{
		SItemDBAInfo& dbaInfo = GetDbaInPreloadList(preloadDbaIndex);
		dbaInfo.SetRequestedTime(refreshTime);
	}
	else
	{
		//Make space for new one (unload oldest one if required)
		FreeSlotInPreloadListIfNeeded();

		//Preload new item animations
		m_preloadedDBASlots.push_back(TItemDBAPair(animationGroup, SItemDBAInfo()));
		SItemDBAInfo& dbaInfo = m_preloadedDBASlots[(m_preloadedDBASlots.size() - 1)].second;
		dbaInfo.SetRequestedTime(refreshTime);

		for (u32 i=0; i < pItemParams->animationPrecache.size(); i++)
		{
			if (!pItemParams->animationPrecache[i].thirdPerson)
			{
				dbaInfo.AddDbaPath(pItemParams->animationPrecache[i].DBAfile);

				//DrxLog("Preload DBA %s", pItemParams->animationPrecache[i].DBAfile.c_str());
			//	gEnv->pCharacterUpr->DBA_PreLoad(pItemParams->animationPrecache[i].DBAfile.c_str());
				gEnv->pCharacterUpr->DBA_LockStatus(pItemParams->animationPrecache[i].DBAfile.c_str(), 1, ICharacterUpr::eStreamingDBAPriority_Urgent);
			}
		}
	}
}


bool CItemAnimationDBAUpr::IsValidDBAPath( tukk dbaPath ) const
{
	return (dbaPath && dbaPath[0]);
}

i32 CItemAnimationDBAUpr::IsDbaAlreadyInUse( const ItemString& animationGroup ) const
{
	for (size_t i = 0; i < m_inUseDBAs.size(); ++i)
	{
		if (m_inUseDBAs[i].first != animationGroup)
			continue;

		return i;
	}

	return -1;
}

i32 CItemAnimationDBAUpr::IsDbaAlreadyInPreloadList( const ItemString& animationGroup ) const
{
	for (size_t i = 0; i < m_preloadedDBASlots.size(); ++i)
	{
		if (m_preloadedDBASlots[i].first != animationGroup)
			continue;

		return i;
	}

	return -1;
}

CItemAnimationDBAUpr::SItemDBAInfo& CItemAnimationDBAUpr::GetDbaInUse( i32k index )
{
	DRX_ASSERT((index >= 0) && (index < (i32)m_inUseDBAs.size()));

	return m_inUseDBAs[index].second;
}

CItemAnimationDBAUpr::SItemDBAInfo& CItemAnimationDBAUpr::GetDbaInPreloadList( i32k index )
{
	DRX_ASSERT((index >= 0) && (index < (i32)m_preloadedDBASlots.size()));

	return m_preloadedDBASlots[index].second;
}

void CItemAnimationDBAUpr::FreeSlotInPreloadListIfNeeded()
{
	if (m_preloadedDBASlots.size() == MAX_ITEM_DBA_PRELOADED)
	{
		float oldestRefreshTime = FLT_MAX;
		i32 oldestIndex = -1;

		for (size_t i = 0; i < m_preloadedDBASlots.size(); ++i)
		{
			if (m_preloadedDBASlots[i].second.GetRequestedTime() < oldestRefreshTime)
			{
				oldestIndex = (i32)i;
				oldestRefreshTime = 0.0f;
			}
		}

		if (oldestIndex >= 0)
		{
			SItemDBAInfo& dbaInfo = m_preloadedDBASlots[oldestIndex].second;
			for (i32 i = 0; i < dbaInfo.GetDBACount(); ++i)
			{
				gEnv->pCharacterUpr->DBA_Unload(dbaInfo.GetDBA(i));
			}

			if (oldestIndex != (MAX_ITEM_DBA_PRELOADED - 1))
			{
				m_preloadedDBASlots[oldestIndex] = m_preloadedDBASlots[(MAX_ITEM_DBA_PRELOADED - 1)];
			}
			m_preloadedDBASlots.pop_back();
		}
	}
}

void CItemAnimationDBAUpr::RemoveFromPreloadListAndDoNotUnload( const ItemString& animationGroup )
{
	i32 removeIdx = -1;
	for (size_t i = 0; i < m_preloadedDBASlots.size(); ++i)
	{
		if (m_preloadedDBASlots[i].first != animationGroup)
			continue;
		
		removeIdx = (i32)i;
		break;
	}

	DRX_ASSERT(removeIdx != -1);

	if (removeIdx != -1)
	{
		if (removeIdx != (m_preloadedDBASlots.size() - 1))
		{
			m_preloadedDBASlots[removeIdx] = m_preloadedDBASlots[(m_preloadedDBASlots.size() - 1)];
		}
		m_preloadedDBASlots.pop_back();
	}
}

void CItemAnimationDBAUpr::Reset()
{
	for (size_t i = 0; i < m_inUseDBAs.size(); ++i)
	{
		const SItemDBAInfo& dbaInfo = m_inUseDBAs[i].second;
		for (i32 j = 0; j < dbaInfo.GetDBACount(); ++j)
		{
			gEnv->pCharacterUpr->DBA_Unload(dbaInfo.GetDBA(j));
		}
	}

	for (size_t i = 0; i < m_preloadedDBASlots.size(); ++i)
	{
		const SItemDBAInfo& dbaInfo = m_preloadedDBASlots[i].second;
		for (i32 j = 0; j < dbaInfo.GetDBACount(); ++j)
		{
			gEnv->pCharacterUpr->DBA_Unload(dbaInfo.GetDBA(j));
		}
	}

	m_inUseDBAs.clear();
	m_preloadedDBASlots.clear();
}

void CItemAnimationDBAUpr::Update( const float currentTime )
{
	const float maxTimeNotUsed = 10.0f;

	TPreloadDBAArray::iterator dbaIt = m_preloadedDBASlots.begin();
	while (dbaIt != m_preloadedDBASlots.end())
	{
		const SItemDBAInfo& dbaInfo = dbaIt->second;

		//Retain...
		if ((currentTime - dbaInfo.GetRequestedTime()) < maxTimeNotUsed)
		{
			++dbaIt;
		}
		else
		{
			//Unload...
			for (i32 j = 0; j < dbaInfo.GetDBACount(); ++j)
			{
				gEnv->pCharacterUpr->DBA_Unload(dbaInfo.GetDBA(j));
			}

			TPreloadDBAArray::iterator nextElement = m_preloadedDBASlots.erase(dbaIt);
			dbaIt = nextElement;
		}
	}
}

void CItemAnimationDBAUpr::GetMemoryStatistics( IDrxSizer *s )
{
	s->AddContainer(m_inUseDBAs);
	s->AddContainer(m_preloadedDBASlots);
}

bool CItemAnimationDBAUpr::IsDbaManagementEnabled() const
{
	return (!gEnv->IsEditor() && (g_pGameCVars->g_fpDbaManagementEnable != 0));
}

void CItemAnimationDBAUpr::Debug()
{
	if (g_pGameCVars->g_fpDbaManagementDebug == 0)
		return;

	const float white[4]	= {1.0f, 10.f, 1.0f, 1.0f};
	const float grey[4]	= {0.6f, 0.6f, 0.6f, 1.0f};

	if (IsDbaManagementEnabled())
	{
		float posY = 50.f;
		float posX = 50.f;

		gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "Currently in use Items and DBAs");
		posY += 15.0f;
		gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "======================================");
		posY += 15.0f;

		for (size_t i = 0; i < m_inUseDBAs.size(); ++i)
		{
			gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "Item: '%s'", m_inUseDBAs[i].first.c_str());
			posY += 15.0f;

			const SItemDBAInfo& dbaInfo = m_inUseDBAs[i].second;
			for (i32 j = 0; j < dbaInfo.GetDBACount(); ++j)
			{
				gEnv->pRenderer->Draw2dLabel(posX + 50.0f, posY, 1.5f, grey, false, "DBA: '%s' - Users: %d", dbaInfo.GetDBA(j), dbaInfo.GetUserCount());
				posY += 15.0f;
			}

			posY += 5.0f;
		}

		gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "Preloaded but not in use Items and DBAs");
		posY += 15.0f;
		gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "======================================");
		posY += 15.0f;


		for (size_t i = 0; i < m_preloadedDBASlots.size(); ++i)
		{
			gEnv->pRenderer->Draw2dLabel(posX, posY, 1.5f, white, false, "Item: '%s'", m_preloadedDBASlots[i].first.c_str());
			posY += 15.0f;

			const SItemDBAInfo& dbaInfo = m_preloadedDBASlots[i].second;
			for (i32 j = 0; j < dbaInfo.GetDBACount(); ++j)
			{
				gEnv->pRenderer->Draw2dLabel(posX + 50.0f, posY, 1.5f, grey, false, "DBA: '%s' - Time: %.2f", dbaInfo.GetDBA(j), dbaInfo.GetRequestedTime());
				posY += 15.0f;
			}

			posY += 5.0f;
		}
	}
	else
	{
		gEnv->pRenderer->Draw2dLabel(50.0f, 50.0f, 1.5f, white, false, "DBA management for 1p animations disabled");

	}
}