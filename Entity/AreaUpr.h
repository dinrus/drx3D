// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/Entity/AreaGrid.h>

//#define DEBUG_AREAMANAGER

class CEntitySystem;
class CArea;
struct IVisArea;
struct IAreaUprEventListener;

typedef std::vector<IAreaUprEventListener*> AreaUprEventListenerVector;
typedef std::vector<SAudioAreaInfo>             AreaEnvironments;

//Areas manager
class CAreaUpr : public IAreaUpr, public ISystemEventListener
{
	struct SAreaCacheEntry
	{
		SAreaCacheEntry() = default;

		SAreaCacheEntry(CArea* const pPassedArea, bool const bIsNear, bool const bIsInside)
			: pArea(pPassedArea),
			bNear(bIsNear),
			bInside(bIsInside),
			bInGrid(true) {}

		CArea* pArea = nullptr;
		bool   bInGrid = true;
		bool   bInside = false;
		bool   bNear = false;
	};

	typedef std::vector<SAreaCacheEntry> TAreaCacheVector;

	struct SAreasCache
	{
		SAreasCache()
			: lastUpdatePos(ZERO)
		{}

		bool GetCacheEntry(CArea const* const pAreaToFind, SAreaCacheEntry** const ppAreaCacheEntry)
		{
			bool bSuccess = false;
			* ppAreaCacheEntry = nullptr;

			for (auto& entry : entries)
			{
				if (entry.pArea == pAreaToFind)
				{
					* ppAreaCacheEntry = &entry;
					bSuccess = true;

					break;
				}
			}

			return bSuccess;
		}

		TAreaCacheVector entries;
		Vec3             lastUpdatePos;
	};

	typedef VectorMap<EntityId, SAreasCache> TAreaCacheMap;
	typedef VectorMap<EntityId, size_t>      TEntitiesToUpdateMap;

public:

	explicit CAreaUpr();
	~CAreaUpr(void);

	//IAreaUpr
	virtual size_t             GetAreaAmount() const override { return m_areas.size(); }
	virtual IArea const* const GetArea(size_t const nAreaIndex) const override;
	virtual size_t             GetOverlappingAreas(const AABB& bb, PodArray<IArea*>& list) const override;
	virtual void               SetAreasDirty() override;
	virtual void               SetAreaDirty(IArea* pArea) override;
	virtual void               ExitAllAreas(EntityId const entityId) override;
	//~IAreaUpr

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// Makes a new area.
	CArea* CreateArea();

	// Puts the passed entity ID into the update list for the next update.
	virtual void MarkEntityForUpdate(EntityId const entityId) override;

	virtual void TriggerAudioListenerUpdate(IArea const* const _pArea) override;

	// Called every frame.
	void Update();

	bool ProceedExclusiveUpdateByHigherArea(
	  SAreasCache* const pAreaCache,
	  EntityId const entityId,
	  Vec3 const& entityPos,
	  CArea* const pArea,
	  Vec3 const& onLowerHull,
	  AreaEnvironments& areaEnvironments);

	virtual void DrawLinkedAreas(EntityId linkedId) const override;
	size_t       GetLinkedAreas(EntityId linkedId, i32 areaId, std::vector<CArea*>& areas) const;

	virtual bool GetLinkedAreas(EntityId linkedId, EntityId* pOutArray, i32& outAndMaxResults) const override;

	void         DrawAreas(const ISystem* const pSystem);
	void         DrawGrid();
	size_t       MemStat();
	void         ResetAreas();
	void         UnloadAreas();

	virtual bool QueryAudioAreas(Vec3 const& pos, SAudioAreaInfo* const pResults, size_t const numMaxResults, size_t& outNumResults) override;

	// Register EventListener to the AreaUpr.
	virtual void AddEventListener(IAreaUprEventListener* pListener) override;
	virtual void RemoveEventListener(IAreaUprEventListener* pListener) override;

	//! Fires event for all listeners to this sound.
	void OnEvent(EEntityEvent event, EntityId TriggerEntityID, IArea* pArea);

	i32  GetNumberOfPlayersNearOrInArea(CArea const* const pArea);

protected:

	friend class CArea;
	void Unregister(CArea const* const pArea);

	// Holds all registered areas.
	TAreaPointers m_areas;

	bool          m_bAreasDirty;
	CAreaGrid     m_areaGrid;

private:

	AreaUprEventListenerVector m_EventListeners;

	//////////////////////////////////////////////////////////////////////////
	SAreasCache* GetAreaCache(EntityId const nEntityId)
	{
		SAreasCache* pAreaCache = nullptr;
		TAreaCacheMap::iterator const Iter(m_mapAreaCache.find(nEntityId));

		if (Iter != m_mapAreaCache.end())
		{
			pAreaCache = &(Iter->second);
		}

		return pAreaCache;
	}

	//////////////////////////////////////////////////////////////////////////
	SAreasCache* MakeAreaCache(EntityId const nEntityID)
	{
		SAreasCache* pAreaCache = &m_mapAreaCache[nEntityID];
		return pAreaCache;
	}

	//////////////////////////////////////////////////////////////////////////
	void DeleteAreaCache(EntityId const nEntityId)
	{
		m_mapAreaCache.erase(nEntityId);
	}

	void UpdateEntity(Vec3 const& position, CEntity* const pIEntity);
	void UpdateDirtyAreas();
	void ProcessArea(CArea* const pArea, SAreaCacheEntry& areaCacheEntry, SAreasCache* const pAreaCache, Vec3 const& pos, CEntity const* const pIEntity, AreaEnvironments& areaEnvironments);
	void ExitArea(EntityId const entityId, CArea const* const _pArea);
	bool GetEnvFadeValue(SAreasCache const& areaCache, SAreaCacheEntry& areaCacheEntry, Vec3 const& entityPos, EntityId const envProvidingEntityId, AreaEnvironments& areaEnvironments);
	bool GetEnvFadeValueInner(SAreasCache const& areaCache, SAreaCacheEntry const& areaCacheEntry, Vec3 const& entityPos, Vec3 const& posOnLowerArea, EntityId const envProvidingEntityId, AreaEnvironments& areaEnvironments);
	bool RetrieveEnvironmentAmount(CArea const* const pArea, float const amount, float const distance, EntityId const envProvidingEntityId, AreaEnvironments& areaEnvironments);

	// Unary predicates for conditional removing
	static inline bool IsDoneUpdating(std::pair<EntityId, size_t> const& entry)
	{
		return entry.second == 0;
	}

	struct SIsNotInGrid
	{
		explicit SIsNotInGrid(
		  EntityId const _entityId,
		  std::vector<CArea*> const& _areas,
		  size_t const _numAreas)
			: entityId(_entityId),
			areas(_areas),
			numAreas(_numAreas)
		{}

		bool operator()(SAreaCacheEntry const& cacheEntry) const;

		EntityId const             entityId;
		std::vector<CArea*> const& areas;
		size_t const               numAreas;
	};

	struct SRemoveIfNoAreasLeft
	{
		explicit SRemoveIfNoAreasLeft(
		  CArea const* const _pArea,
		  std::vector<CArea*> const& _areas,
		  size_t const _numAreas)
			: pArea(_pArea),
			areas(_areas),
			numAreas(_numAreas)
		{}

		template<typename K, typename V>
		bool operator()(std::pair<K, V>& cacheEntry) const;

		CArea const* const         pArea;
		std::vector<CArea*> const& areas;
		size_t const               numAreas;
	};

	TAreaCacheMap        m_mapAreaCache;                      // Area cache per entity id.
	TEntitiesToUpdateMap m_mapEntitiesToUpdate;

	// We need two lists, one for the main thread access and one for the audio thread access.
	enum Threads : u8 { Main = 0, Audio = 1, Num };
	TAreaPointers                  m_areasAtPos[Threads::Num];
	DrxCriticalSectionNonRecursive m_accessAreas;

#if defined(DEBUG_AREAMANAGER)
	//////////////////////////////////////////////////////////////////////////
	void CheckArea(CArea const* const pArea)
	{
		if (!stl::find(m_areas, pArea))
		{
			DrxFatalError("<AreaUpr>: area not found in overall areas list!");
		}
	}
#endif // DEBUG_AREAMANAGER
};
