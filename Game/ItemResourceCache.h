// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/***********************************************************************************
------------------------------------------------------------------------------------
$Id$
$DateTime$
Описание:	Caches geometry, particles etc. used by items to prevent repeated file 
							access

------------------------------------------------------------------------------------
История:
- 06:05:2010   10:54 : Moved from ItemSharedParams and expanded by Claire Allan

************************************************************************************/

#pragma once

#ifndef __ITEMRESOURCECACHE_H__
#define __ITEMRESOURCECACHE_H__

#include <drx3D/Game/ItemString.h>
#include <drx3D/Animation/IDrxAnimation.h>

struct ICharacterInstance;
struct IMaterial;

class CItemGeometryCache
{
	typedef _smart_ptr<ICharacterInstance> TCharacterInstancePtr;
	typedef std::map<u32, TCharacterInstancePtr> TEditorCacheCharacterMap;

	typedef _smart_ptr<IStatObj>	TStatObjectPtr;
	typedef std::map<u32, TStatObjectPtr> TCacheStaticObjectMap;

public:

	~CItemGeometryCache()
	{
		//Benito: Cached are already flushed at this time, but this should not 'hurt'
		FlushCaches();
	}

	void FlushCaches()
	{
		m_editorCachedCharacters.clear();
		m_cachedStaticObjects.clear();
	}

	void CacheGeometry(tukk objectFileName, bool useCgfStreaming, u32 nLoadingFlags=0);
	void CacheGeometryFromXml(XmlNodeRef node, bool useCgfStreaming, u32 nLoadingFlags=0);

	void GetMemoryStatistics(IDrxSizer *s);

private:

	void CheckAndCacheGeometryFromXmlAttr( tukk pAttr, bool useCgfStreaming, u32 nLoadingFlags );

	bool IsCharacterCached( u32k fileNameHash ) const;
	bool IsStaticObjectCached( u32k fileNameHash ) const;
	
	TEditorCacheCharacterMap	m_editorCachedCharacters;
	TCacheStaticObjectMap m_cachedStaticObjects;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CItemParticleEffectCache
{
	typedef _smart_ptr<IParticleEffect> TParticleEffectPtr;
	typedef std::map<i32, TParticleEffectPtr> TCacheParticleMap;

public:

	~CItemParticleEffectCache()
	{
		FlushCaches();
	}

	void FlushCaches()
	{
		m_cachedParticles.clear();
	}

	void CacheParticle(tukk particleFileName);
	IParticleEffect* GetCachedParticle(tukk particleFileName) const;
	void GetMemoryStatistics(IDrxSizer *s);

private:

	bool IsParticleCached(tukk particleFileName) const
	{
		return (GetCachedParticle(particleFileName) != NULL);
	}

	TCacheParticleMap	m_cachedParticles;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CItemMaterialAndTextureCache
{
	typedef _smart_ptr<ITexture> TTexturePtr;
	typedef std::map<u32, TTexturePtr> TCacheTextureMap;

	typedef _smart_ptr<IMaterial> TMaterialPtr;
	typedef std::map<u32, TMaterialPtr> TCacheMaterialMap;

public:

	~CItemMaterialAndTextureCache()
	{
		FlushCaches();
	}

	void FlushCaches();

	void CacheTexture(tukk textureFileName, bool noStreaming = false);
	void CacheMaterial(tukk materialFileName);
	void GetMemoryStatistics(IDrxSizer *s);

private:

	ITexture* GetCachedTexture(tukk textureFileName) const;
	bool IsTextureCached(tukk textureFileName) const
	{
		return (GetCachedTexture(textureFileName) != NULL);
	}

	IMaterial* GetCachedMaterial(tukk materialFileName) const;
	bool IsMaterialCached(tukk materialFileName) const
	{
		return (GetCachedMaterial(materialFileName) != NULL);
	}

	TCacheTextureMap	m_cachedTextures;
	TCacheMaterialMap	m_cachedMaterials;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CItemPrefetchCHRUpr
{
public:
	CItemPrefetchCHRUpr();
	~CItemPrefetchCHRUpr();

	void Reset();

	void Update(float fCurrTime);

	void Prefetch(const ItemString& geomName);

private:
	struct PrefetchSlot
	{
		ItemString geomName;
		float requestTime;
	};

	typedef std::vector<PrefetchSlot> PrefetchSlotVec;

private:
	CItemPrefetchCHRUpr(const CItemPrefetchCHRUpr&);
	CItemPrefetchCHRUpr& operator = (const CItemPrefetchCHRUpr&);

private:
	float m_fTimeout;

	PrefetchSlotVec m_prefetches;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CItemSharedParams;

class CItemAnimationDBAUpr
{
private:
	struct SItemDBAInfo
	{
		SItemDBAInfo()
			: m_userCount(0)
			, m_requestedTime(0.0f)
		{
			m_dbas.reserve(2);
#if defined(_DEBUG)
			m_userIds.reserve(16);
#endif	
		}

		void AddUser(const EntityId userId)
		{
#if defined(_DEBUG)
			DRX_ASSERT_MESSAGE(ValidateAddForEntity(userId), "Adding item dba user multiple times!");
#endif	
			m_userCount++;
		}

		void RemoveUser(const EntityId userId)
		{
#if defined(_DEBUG)
			DRX_ASSERT_MESSAGE(ValidateRemoveForEntity(userId), "Removing item dba user which is not!");
#endif	
			m_userCount--;

			DRX_ASSERT(m_userCount >= 0);
		}

		ILINE i32 GetUserCount() const
		{
			return m_userCount;
		}

		ILINE void AddDbaPath(const ItemString& dbaPath)
		{
			m_dbas.push_back(dbaPath);
		}

		ILINE i32 GetDBACount() const 
		{
			return m_dbas.size();
		}

		ILINE tukk GetDBA(i32 i) const
		{
			DRX_ASSERT((i >= 0) && (i < (i32)m_dbas.size()));

			return m_dbas[i].c_str();
		}

		ILINE void SetRequestedTime(const float requestTime)
		{
			m_requestedTime = requestTime;
		}

		ILINE float GetRequestedTime() const
		{
			return m_requestedTime;
		}

#if defined(_DEBUG)
		bool ValidateAddForEntity(const EntityId userId)
		{
			bool wasRegistered = stl::find(m_userIds, userId);
			if (!wasRegistered)
			{
				m_userIds.push_back(userId);
			}
			return !wasRegistered;
		}

		bool ValidateRemoveForEntity(const EntityId userId)
		{
			return stl::find_and_erase(m_userIds, userId);
		}
#endif	

	private:

		i32 m_userCount;
		float m_requestedTime;
		std::vector<ItemString> m_dbas;

#if defined(_DEBUG)
		std::vector<EntityId> m_userIds;
#endif
	};

	typedef std::pair<ItemString, SItemDBAInfo>	TItemDBAPair;
	typedef std::vector<TItemDBAPair> TPreloadDBAArray;

public:

	CItemAnimationDBAUpr();

	void Reset();

	void AddDBAUser(const ItemString& animationGroup, const CItemSharedParams* pItemParams, const EntityId itemUserId);
	void RemoveDBAUser(const ItemString& animationGroup, const EntityId itemUserId);
	void RequestDBAPreLoad(const ItemString& animationGroup, const CItemSharedParams* pItemParams);

	void Update(const float currentTime);

	void Debug();

	void GetMemoryStatistics(IDrxSizer *s);

private:

	bool IsValidDBAPath(tukk dbaPath) const;
	i32 IsDbaAlreadyInUse(const ItemString& animationGroup) const; 
	SItemDBAInfo& GetDbaInUse(i32k index);
	i32 IsDbaAlreadyInPreloadList(const ItemString& animationGroup) const;
	SItemDBAInfo& GetDbaInPreloadList(i32k index);

	void FreeSlotInPreloadListIfNeeded();
	void RemoveFromPreloadListAndDoNotUnload(const ItemString& animationGroup);

	bool IsDbaManagementEnabled() const;

	TPreloadDBAArray m_inUseDBAs;
	TPreloadDBAArray m_preloadedDBASlots;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CItemResourceCache
{
	typedef std::set<const IEntityClass*> TCachedResourcesClassesVector;

public:

	void FlushCaches()
	{
		m_cachedClasses.clear();
		m_itemGeometryCache.FlushCaches();
		m_ammoGeometryCache.FlushCaches();
		m_particleEffectCache.FlushCaches();
		m_materialsAndTextureCache.FlushCaches();
		m_1pDBAUpr.Reset();
		m_pfCHRUpr.Reset();
	}

	void CachedResourcesForClassDone(const IEntityClass* pItemClass)
	{
		m_cachedClasses.insert(pItemClass);
	}

	bool AreClassResourcesCached(const IEntityClass* pItemClass) const
	{
		return (m_cachedClasses.find(pItemClass) != m_cachedClasses.end());
	}

	ILINE CItemGeometryCache& GetItemGeometryCache() 
	{ 
		return m_itemGeometryCache; 
	} 

	ILINE CItemGeometryCache& GetAmmoGeometryCache()
	{
		return m_ammoGeometryCache;
	}

	ILINE CItemParticleEffectCache& GetParticleEffectCache()
	{
		return m_particleEffectCache;
	}

	ILINE CItemMaterialAndTextureCache& GetMaterialsAndTextureCache()
	{
		return m_materialsAndTextureCache;
	}

	ILINE CItemAnimationDBAUpr& Get1pDBAUpr()
	{
		return m_1pDBAUpr;
	}

	ILINE CItemPrefetchCHRUpr& GetPrefetchCHRUpr()
	{
		return m_pfCHRUpr;
	}

	void GetMemoryStatistics(IDrxSizer *s)
	{
		s->AddObject(m_cachedClasses);
		m_itemGeometryCache.GetMemoryStatistics(s);
		m_ammoGeometryCache.GetMemoryStatistics(s);
		m_particleEffectCache.GetMemoryStatistics(s);
		m_materialsAndTextureCache.GetMemoryStatistics(s);
		m_1pDBAUpr.GetMemoryStatistics(s);
	}

private:
	CItemGeometryCache				m_itemGeometryCache;
	CItemGeometryCache				m_ammoGeometryCache;
	CItemParticleEffectCache		m_particleEffectCache;
	CItemMaterialAndTextureCache	m_materialsAndTextureCache;
	CItemAnimationDBAUpr	m_1pDBAUpr;
	CItemPrefetchCHRUpr m_pfCHRUpr;

	TCachedResourcesClassesVector m_cachedClasses;
};

#endif //__ITEMRESOURCECACHE_H__
