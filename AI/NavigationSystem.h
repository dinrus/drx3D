// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NavigationSystem_h__
#define __NavigationSystem_h__

#pragma once

#include <drx3D/AI/INavigationSystem.h>

#include <drx3D/AI/MNM.h>
#include <drx3D/AI/Tile.h>
#include <drx3D/AI/NavMesh.h>
#include <drx3D/AI/TileGenerator.h>
#include <drx3D/AI/MarkupVolume.h>

#include <drx3D/AI/WorldMonitor.h>
#include <drx3D/AI/OffMeshNavigationUpr.h>
#include <drx3D/AI/VolumesUpr.h>
#include <drx3D/AI/IslandConnectionsUpr.h>
#include <drx3D/AI/NavigationUpdatesUpr.h>
#include <drx3D/AI/AnnotationsLibrary.h>

#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include <drx3D/CoreX/Thread/IThreadUpr.h>

#if DRX_PLATFORM_WINDOWS
	#define NAVIGATION_SYSTEM_EDITOR_BACKGROUND_UPDATE 1
#else
	#define NAVIGATION_SYSTEM_EDITOR_BACKGROUND_UPDATE 0
#endif

#if DRX_PLATFORM_WINDOWS
	#define NAVIGATION_SYSTEM_PC_ONLY 1
#else
	#define NAVIGATION_SYSTEM_PC_ONLY 0
#endif

struct RayCastRequest;

#if DEBUG_MNM_ENABLED || NAVIGATION_SYSTEM_EDITOR_BACKGROUND_UPDATE
class NavigationSystem;
struct NavigationMesh;
#endif

#if DEBUG_MNM_ENABLED
class NavigationSystem;
struct NavigationMesh;

class NavigationSystemDebugDraw
{
	struct NavigationSystemWorkingProgress
	{
		NavigationSystemWorkingProgress()
			: m_initialQueueSize(0)
			, m_currentQueueSize(0)
			, m_timeUpdating(0.0f)
		{

		}

		void Update(const float frameTime, const size_t queueSize);
		void Draw();

	private:

		void BeginDraw();
		void EndDraw();
		void DrawQuad(const Vec2& origin, const Vec2& size, const ColorB& color);

		float               m_timeUpdating;
		size_t              m_initialQueueSize;
		size_t              m_currentQueueSize;
		SAuxGeomRenderFlags m_oldRenderFlags;
	};

	struct DebugDrawSettings
	{
		DebugDrawSettings()
			: meshID(0)
			, selectedX(0)
			, selectedY(0)
			, selectedZ(0)
			, forceGeneration(false)
		{

		}

		inline bool Valid() const { return (meshID != NavigationMeshID(0)); }

		NavigationMeshID meshID;

		size_t           selectedX;
		size_t           selectedY;
		size_t           selectedZ;

		bool             forceGeneration;
	};

public:

	NavigationSystemDebugDraw()
		: m_agentTypeID(0)
	{

	}

	inline void SetAgentType(const NavigationAgentTypeID agentType)
	{
		m_agentTypeID = agentType;
	}

	inline NavigationAgentTypeID GetAgentType() const
	{
		return m_agentTypeID;
	}

	void DebugDraw(NavigationSystem& navigationSystem);
	void UpdateWorkingProgress(const float frameTime, const size_t queueSize);

private:

	MNM::TileID       DebugDrawTileGeneration(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawRayCast(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawPathFinder(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawClosestPoint(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawGroundPoint(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawSnapToNavmesh(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);
	void              DebugDrawIslandConnection(NavigationSystem& navigationSystem, const DebugDrawSettings& settings);

	void              DebugDrawNavigationMeshesForSelectedAgent(NavigationSystem& navigationSystem, MNM::TileID excludeTileID);
	void              DebugDrawNavigationSystemState(NavigationSystem& navigationSystem);
	void              DebugDrawMemoryStats(NavigationSystem& navigationSystem);
	void              DebugDrawTriangleOnCursor(NavigationSystem& navigationSystem);

	DebugDrawSettings GetDebugDrawSettings(NavigationSystem& navigationSystem);

	inline Vec3       TriangleCenter(const Vec3& a, const Vec3& b, const Vec3& c)
	{
		return (a + b + c) / 3.f;
	}

	const INavMeshQueryFilter* GetDebugQueryFilter(tukk szName) const;

	NavigationAgentTypeID           m_agentTypeID;
	NavigationSystemWorkingProgress m_progress;
};
#else
class NavigationSystemDebugDraw
{
public:
	inline void                  SetAgentType(const NavigationAgentTypeID agentType)                  {};
	inline NavigationAgentTypeID GetAgentType() const                                                 { return NavigationAgentTypeID(0); };
	inline void                  DebugDraw(const NavigationSystem& navigationSystem)                  {};
	inline void                  UpdateWorkingProgress(const float frameTime, const size_t queueSize) {};
};
#endif

#if NAVIGATION_SYSTEM_EDITOR_BACKGROUND_UPDATE
class NavigationSystemBackgroundUpdate : public ISystemEventListener
{
	class Thread : public IThread
	{
	public:
		Thread(NavigationSystem& navigationSystem)
			: m_navigationSystem(navigationSystem)
			, m_requestedStop(false)
		{

		}

		// Start accepting work on thread
		virtual void ThreadEntry();

		// Signals the thread that it should not accept anymore work and exit
		void SignalStopWork();

	private:
		NavigationSystem& m_navigationSystem;

		 bool     m_requestedStop;
	};

public:
	NavigationSystemBackgroundUpdate(NavigationSystem& navigationSystem)
		: m_pBackgroundThread(NULL)
		, m_navigationSystem(navigationSystem)
		, m_enabled(gEnv->IsEditor())
		, m_paused(false)
	{
		RegisterAsSystemListener();
	}

	~NavigationSystemBackgroundUpdate()
	{
		RemoveAsSystemListener();
		Stop();
	}

	bool IsRunning() const
	{
		return (m_pBackgroundThread != NULL);
	}

	void Pause(const bool pause)
	{
		if (pause)
		{
			if (Stop())   // Stop and synch if necessary
			{
				DrxLog("NavMesh generation background thread stopped");
			}
		}

		m_paused = pause;
	}

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

private:

	bool Start();
	bool Stop();

	void RegisterAsSystemListener();
	void RemoveAsSystemListener();

	bool IsEnabled() const { return m_enabled; }

	Thread*           m_pBackgroundThread;
	NavigationSystem& m_navigationSystem;

	bool              m_enabled;
	bool              m_paused;
};
#else
class NavigationSystemBackgroundUpdate
{
public:
	NavigationSystemBackgroundUpdate(NavigationSystem& navigationSystem)
	{

	}

	bool IsRunning() const                     { return false; }
	void Pause(const bool readingOrSavingMesh) {}
};
#endif

typedef MNM::BoundingVolume NavigationBoundingVolume;

struct NavigationMesh
{	
	NavigationMesh(NavigationAgentTypeID _agentTypeID)
		: agentTypeID(_agentTypeID)
		, version(0)
	{
	}

#if DEBUG_MNM_ENABLED
	struct ProfileMemoryStats
	{
		ProfileMemoryStats(const MNM::CNavMesh::ProfilerType& _navMeshProfiler)
			: navMeshProfiler(_navMeshProfiler)
			, totalNavigationMeshMemory(0)
		{

		}

		const MNM::CNavMesh::ProfilerType& navMeshProfiler;
		size_t                             totalNavigationMeshMemory;
	};

	ProfileMemoryStats GetMemoryStats(IDrxSizer* pSizer) const;
#endif

	NavigationAgentTypeID agentTypeID;
	size_t                version;

	MNM::CNavMesh         navMesh;
	NavigationVolumeID    boundary;

	typedef std::vector<NavigationVolumeID> Markups;
	Markups markups;

	typedef std::unordered_map<NavigationVolumeID, MNM::SMarkupVolumeData> MarkupsData;
	MarkupsData markupsData;

#ifdef SW_NAVMESH_USE_GUID
	NavigationVolumeGUID boundaryGUID;
#endif
	typedef std::vector<NavigationVolumeID> ExclusionVolumes;
	ExclusionVolumes exclusions;
#ifdef SW_NAVMESH_USE_GUID
	typedef std::vector<NavigationVolumeGUID> ExclusionVolumesGUID;
	ExclusionVolumesGUID exclusionsGUID;
#endif

	string name; // TODO: this is currently duplicated for all agent types
};

struct AgentType
{
	// Copying the AgentType in a multi threaded environment
	// is not safe due to our string class being implemented as a
	// not thread safe Copy-on-Write
	// Using a custom assignment operator and a custom copy constructor
	// gives us thread safety without the usage of code locks

	AgentType() {}

	AgentType(const AgentType& other)
	{
		MakeDeepCopy(other);
	}

	struct Settings
	{
		Settings()
			: voxelSize(Vec3Constants<float>::fVec3_Zero)
		{}

		Vec3 voxelSize;
		MNM::CTileGenerator::SAgentSettings agent;
	};

	struct MeshInfo
	{
#ifdef SW_NAVMESH_USE_GUID
		MeshInfo(NavigationMeshGUID& _guid, const NavigationMeshID& _id, u32 _name)
			: guid(_guid)
			, id(_id)
			, name(_name)
		{
		}
		NavigationMeshGUID guid;
#else
		MeshInfo(const NavigationMeshID& _id, u32 _name)
			: id(_id)
			, name(_name)
		{
		}
#endif

		NavigationMeshID id;
		u32           name;
	};

	AgentType& operator=(const AgentType& other)
	{
		MakeDeepCopy(other);

		return *this;
	}

	void MakeDeepCopy(const AgentType& other)
	{
		settings = other.settings;

		meshes = other.meshes;

		exclusions = other.exclusions;

		callbacks = other.callbacks;

		meshEntityCallback = other.meshEntityCallback;

		smartObjectUserClasses.reserve(other.smartObjectUserClasses.size());
		SmartObjectUserClasses::const_iterator end = other.smartObjectUserClasses.end();
		for (SmartObjectUserClasses::const_iterator it = other.smartObjectUserClasses.begin(); it != end; ++it)
			smartObjectUserClasses.push_back(it->c_str());

		name = other.name.c_str();
	}

	Settings settings;

	typedef std::vector<MeshInfo> Meshes;
	Meshes meshes;

	typedef std::vector<NavigationVolumeID> ExclusionVolumes;
	ExclusionVolumes exclusions;

	//////////////////////////////////////////////////////////////////////////
	// Temporary - need to find better structures
	// potentially there can be many markup volumes, for each agent type
	typedef std::vector<NavigationVolumeID> MarkupVolumes;
	MarkupVolumes markups;
	//////////////////////////////////////////////////////////////////////////

	CFunctorsList<NavigationMeshChangeCallback> callbacks;
	CFunctorsList<NavigationMeshChangeCallback> annotationCallbacks;

	NavigationMeshEntityCallback                meshEntityCallback;

	typedef std::vector<string> SmartObjectUserClasses;
	SmartObjectUserClasses smartObjectUserClasses;

	string                 name;
};

class NavigationSystem : public INavigationSystem
{
	friend class NavigationSystemDebugDraw;
	friend class NavigationSystemBackgroundUpdate;
	friend class CMNMUpdatesUpr;

public:
	// BAI navigation file version history
	// Changes in version 12
	// - Saving Markups entity guids and names
	// Changes in version 11
	// - Fixed saving/loading markups when its' container is dynamic
	// Changes in version 10
	// - Navigation markup volumes and markup volumes data are stored
	// Changes in version 9
	//  - Navigation volumes storage is changed:
	//    * all used navigation volumes are saved (including exclusion volumes, which were missing before);
	//    * navigation area names saved together with volume data;
	//    * volumes stored only onces, instead of storing them together with each mesh.
	// Changes in version 8
	//  - struct MNM::Tile::STriangle layout is changed - now it has triangle flags

	enum eBAINavigationFileVersion : u16
	{
		INCOMPATIBLE = 10,
		FIRST_COMPATIBLE,
		ENTITY_MARKUP_GUIDS,

		// Add new versions before NEXT
		NEXT,
		CURRENT = NEXT - 1,
	};

	NavigationSystem(tukk configName);
	~NavigationSystem();

	virtual NavigationAgentTypeID      CreateAgentType(tukk name, const CreateAgentTypeParams& params) override;
	virtual NavigationAgentTypeID      GetAgentTypeID(tukk name) const override;
	virtual NavigationAgentTypeID      GetAgentTypeID(size_t index) const override;
	virtual tukk                GetAgentTypeName(NavigationAgentTypeID agentTypeID) const override;
	virtual size_t                     GetAgentTypeCount() const override;
	bool                               GetAgentTypeProperties(const NavigationAgentTypeID agentTypeID, AgentType& agentTypeProperties) const;

	virtual MNM::AreaAnnotation        GetAreaTypeAnnotation(const NavigationAreaTypeID areaTypeID) const override;

	virtual const MNM::IAnnotationsLibrary& GetAnnotationLibrary() const override { return m_annotationsLibrary; }
	const MNM::CAnnotationsLibrary&         GetAnnotations()     { return m_annotationsLibrary; }

#ifdef SW_NAVMESH_USE_GUID
	virtual NavigationMeshID CreateMesh(tukk name, NavigationAgentTypeID agentTypeID, const CreateMeshParams& params, NavigationMeshGUID guid) override;
	virtual NavigationMeshID CreateMesh(tukk name, NavigationAgentTypeID agentTypeID, const CreateMeshParams& params, NavigationMeshID requestedId, NavigationMeshGUID guid) override;
#else
	virtual NavigationMeshID CreateMesh(tukk name, NavigationAgentTypeID agentTypeID, const CreateMeshParams& params) override;
	virtual NavigationMeshID CreateMesh(tukk name, NavigationAgentTypeID agentTypeID, const CreateMeshParams& params, NavigationMeshID requestedId) override;
#endif
	virtual NavigationMeshID CreateMeshForVolumeAndUpdate(tukk name, NavigationAgentTypeID agentTypeID, const CreateMeshParams& params, const NavigationVolumeID volumeID) override;

	virtual void             DestroyMesh(NavigationMeshID meshID) override;

	virtual void             SetMeshEntityCallback(NavigationAgentTypeID agentTypeID, const NavigationMeshEntityCallback& callback) override;
	virtual void             AddMeshChangeCallback(NavigationAgentTypeID agentTypeID, const NavigationMeshChangeCallback& callback) override;
	virtual void             RemoveMeshChangeCallback(NavigationAgentTypeID agentTypeID, const NavigationMeshChangeCallback& callback) override;
	virtual void             AddMeshAnnotationChangeCallback(NavigationAgentTypeID agentTypeID, const NavigationMeshChangeCallback& callback) override;
	virtual void             RemoveMeshAnnotationChangeCallback(NavigationAgentTypeID agentTypeID, const NavigationMeshChangeCallback& callback) override;

#ifdef SW_NAVMESH_USE_GUID
	virtual NavigationVolumeID CreateVolume(Vec3* vertices, size_t vertexCount, float height, NavigationVolumeGUID guid) override;
#else
	virtual NavigationVolumeID CreateVolume(Vec3* vertices, size_t vertexCount, float height) override;
#endif
	virtual NavigationVolumeID CreateVolume(Vec3* vertices, size_t vertexCount, float height, NavigationVolumeID requestedID) override;
	virtual void               DestroyVolume(NavigationVolumeID volumeID) override;
	virtual void               SetVolume(NavigationVolumeID volumeID, Vec3* vertices, size_t vertexCount, float height) override;
	virtual bool               ValidateVolume(NavigationVolumeID volumeID) const override;
	virtual NavigationVolumeID GetVolumeID(NavigationMeshID meshID) const override;

	virtual NavigationVolumeID CreateMarkupVolume(NavigationVolumeID requestedID) override;
	virtual void               SetMarkupVolume(const NavigationAgentTypesMask enabledAgentTypesMask, const Vec3* vertices, size_t vertexCount, const NavigationVolumeID volumeID, const MNM::SMarkupVolumeParams& params) override;
	virtual void               DestroyMarkupVolume(NavigationVolumeID volumeID) override;
	virtual void               SetAnnotationForMarkupTriangles(NavigationVolumeID volumeID, const MNM::AreaAnnotation& areaAnotation) override;

#ifdef SW_NAVMESH_USE_GUID
	virtual void SetMeshBoundaryVolume(NavigationMeshID meshID, NavigationVolumeID volumeID, NavigationVolumeGUID volumeGUID) override;
	virtual void SetExclusionVolume(const NavigationAgentTypeID* agentTypeIDs, size_t agentTypeIDCount,
	                                NavigationVolumeID volumeID, NavigationVolumeGUID volumeGUID) override;
#else
	virtual void SetMeshBoundaryVolume(NavigationMeshID meshID, NavigationVolumeID volumeID) override;
	virtual void SetExclusionVolume(const NavigationAgentTypeID* agentTypeIDs, size_t agentTypeIDCount,
	                                NavigationVolumeID volumeID) override;
#endif

	virtual NavigationMeshID      GetMeshID(tukk name, NavigationAgentTypeID agentTypeID) const override;
	virtual tukk           GetMeshName(NavigationMeshID meshID) const override;
	virtual void                  SetMeshName(NavigationMeshID meshID, tukk name) override;

	virtual WorkingState          GetState() const override;
	virtual WorkingState          Update(bool blocking) override;
	virtual void                  PauseNavigationUpdate() override;
	virtual void                  RestartNavigationUpdate() override;

	virtual u32                GetWorkingQueueSize() const override;

	virtual void                  ProcessQueuedMeshUpdates() override;

	virtual void                  Clear() override;
	virtual void                  ClearAndNotify() override;
	virtual bool                  ReloadConfig() override;
	virtual void                  DebugDraw() override;
	virtual void                  Reset() override;

	void                          GetMemoryStatistics(IDrxSizer* pSizer);

	virtual void                  SetDebugDisplayAgentType(NavigationAgentTypeID agentTypeID) override;
	virtual NavigationAgentTypeID GetDebugDisplayAgentType() const override;

	//! deprecated - RequestQueueMeshUpdate(meshID, aabb) should be used instead
	virtual size_t        QueueMeshUpdate(NavigationMeshID meshID, const AABB& aabb) override;

	const NavigationMesh& GetMesh(const NavigationMeshID& meshID) const;
	NavigationMesh&       GetMesh(const NavigationMeshID& meshID);
	NavigationMeshID      GetEnclosingMeshID(NavigationAgentTypeID agentTypeID, const Vec3& location) const override;
	bool                  IsLocationInMesh(NavigationMeshID meshID, const Vec3& location) const;
	MNM::TriangleID       GetClosestMeshLocation(NavigationMeshID meshID, const Vec3& location, float vrange, float hrange, const INavMeshQueryFilter* pFilter,
	                                             Vec3* meshLocation, float* distance) const;

	virtual bool                             GetClosestPointInNavigationMesh(const NavigationAgentTypeID agentID, const Vec3& location, float vrange, float hrange, Vec3* meshLocation, const INavMeshQueryFilter* pFilter, float minIslandArea = 0.f) const override;

	virtual bool                             IsPointReachableFromPosition(const NavigationAgentTypeID agentID, const IEntity* pEntityToTestOffGridLinks, const Vec3& startLocation, const Vec3& endLocation, const INavMeshQueryFilter* pFilter) const override;
	virtual bool                             IsLocationValidInNavigationMesh(const NavigationAgentTypeID agentID, const Vec3& location, const INavMeshQueryFilter* pFilter, float downRange = 1.0f, float upRange = 1.0f) const override;

	virtual size_t                           GetTriangleCenterLocationsInMesh(const NavigationMeshID meshID, const Vec3& location, const AABB& searchAABB, Vec3* centerLocations, size_t maxCenterLocationCount, const INavMeshQueryFilter* pFilter, float minIslandArea = 0.f) const override;

	virtual size_t                           GetTriangleBorders(const NavigationMeshID meshID, const AABB& aabb, Vec3* pBorders, size_t maxBorderCount, const INavMeshQueryFilter* pFilter, float minIslandArea = 0.f) const override;
	virtual size_t                           GetTriangleInfo(const NavigationMeshID meshID, const AABB& aabb, Vec3* centerLocations, u32* islandids, size_t max_count, const INavMeshQueryFilter* pFilter, float minIslandArea = 0.f) const override;
	virtual MNM::GlobalIslandID              GetGlobalIslandIdAtPosition(const NavigationAgentTypeID agentID, const Vec3& location) override;

	virtual bool                             ReadFromFile(tukk fileName, bool bAfterExporting) override;
	virtual bool                             SaveToFile(tukk fileName) const override;

	virtual void                             RegisterListener(INavigationSystemListener* pListener, tukk name = NULL) override { m_listenersList.Add(pListener, name); }
	virtual void                             UnRegisterListener(INavigationSystemListener* pListener) override                        { m_listenersList.Remove(pListener); }

	virtual void                             RegisterUser(INavigationSystemUser* pUser, tukk name = NULL) override             { m_users.Add(pUser, name); }
	virtual void                             UnRegisterUser(INavigationSystemUser* pUser) override                                    { m_users.Remove(pUser); }

	virtual bool                             RegisterArea(tukk shapeName, NavigationVolumeID& outVolumeId) override;
	virtual void                             UnRegisterArea(tukk shapeName) override;
	virtual NavigationVolumeID               GetAreaId(tukk shapeName) const override;
	virtual void                             SetAreaId(tukk shapeName, NavigationVolumeID id) override;
	virtual void                             UpdateAreaNameForId(const NavigationVolumeID id, tukk newShapeName) override;
	virtual void                             RemoveLoadedMeshesWithoutRegisteredAreas() override;

	virtual bool                             RegisterEntityMarkups(const IEntity& owningEntity, tukk* shapeNamesArray, const size_t count, NavigationVolumeID* pOutIdsArray) override;
	virtual void                             UnregisterEntityMarkups(const IEntity& owningEntity) override;

	virtual void                             StartWorldMonitoring() override;
	virtual void                             StopWorldMonitoring() override;

	virtual bool                             IsInUse() const override;
	virtual void                             CalculateAccessibility() override;

	void                                     OffsetBoundingVolume(const Vec3& additionalOffset, const NavigationVolumeID volumeId);
	void                                     OffsetAllMeshes(const Vec3& additionalOffset);

	void                                     ComputeIslands();
	void                                     AddIslandConnectionsBetweenTriangles(const NavigationMeshID& meshID, const MNM::TriangleID startingTriangleID, const MNM::TriangleID endingTriangleID);
	void                                     RemoveIslandsConnectionBetweenTriangles(const NavigationMeshID& meshID, const MNM::TriangleID startingTriangleID, const MNM::TriangleID endingTriangleID = 0);
	void                                     RemoveAllIslandConnectionsForObject(const NavigationMeshID& meshID, u32k objectId);

	void                                     AddOffMeshLinkIslandConnectionsBetweenTriangles(const NavigationMeshID& meshID, const MNM::TriangleID startingTriangleID, const MNM::TriangleID endingTriangleID, const MNM::OffMeshLinkID& linkID);
	void                                     RemoveOffMeshLinkIslandsConnectionBetweenTriangles(const NavigationMeshID& meshID, const MNM::TriangleID startingTriangleID, const MNM::TriangleID endingTriangleID, const MNM::OffMeshLinkID& linkID);

	virtual MNM::TileID                      GetTileIdWhereLocationIsAtForMesh(NavigationMeshID meshID, const Vec3& location, const INavMeshQueryFilter* pFilter) override;
	virtual void                             GetTileBoundsForMesh(NavigationMeshID meshID, MNM::TileID tileID, AABB& bounds) const override;
	virtual MNM::TriangleID                  GetTriangleIDWhereLocationIsAtForMesh(const NavigationAgentTypeID agentID, const Vec3& location, const INavMeshQueryFilter* pFilter) override;
	virtual bool                             SnapToNavMesh(const NavigationAgentTypeID agentID, const Vec3& position, const INavMeshQueryFilter* pFilter, const SSnapToNavMeshRulesInfo& snappingRules, Vec3& snappedPosition, MNM::TriangleID* pTriangleId) const override;


	virtual const MNM::INavMesh*             GetMNMNavMesh(NavigationMeshID meshID) const override;

	virtual bool                             NavMeshTestRaycastHit(NavigationAgentTypeID agentTypeID, const Vec3& startPos, const Vec3& endPos, const INavMeshQueryFilter* pFilter, MNM::SRayHitOutput* pOutHit) const override;

	virtual const IOffMeshNavigationUpr& GetIOffMeshNavigationUpr() const override { return m_offMeshNavigationUpr; }
	virtual IOffMeshNavigationUpr&       GetIOffMeshNavigationUpr() override       { return m_offMeshNavigationUpr; }

	bool                                     AgentTypeSupportSmartObjectUserClass(NavigationAgentTypeID agentTypeID, tukk smartObjectUserClass) const;
	u16                                   GetAgentRadiusInVoxelUnits(NavigationAgentTypeID agentTypeID) const;
	u16                                   GetAgentHeightInVoxelUnits(NavigationAgentTypeID agentTypeID) const;

	virtual TileGeneratorExtensionID         RegisterTileGeneratorExtension(MNM::TileGenerator::IExtension& extension) override;
	virtual bool                             UnRegisterTileGeneratorExtension(const TileGeneratorExtensionID extensionId) override;

	virtual INavigationUpdatesUpr*       GetUpdateUpr() override { return &m_updatesUpr; }

	inline const WorldMonitor*               GetWorldMonitor() const
	{
		return &m_worldMonitor;
	}

	inline WorldMonitor* GetWorldMonitor()
	{
		return &m_worldMonitor;
	}

	inline const OffMeshNavigationUpr* GetOffMeshNavigationUpr() const
	{
		return &m_offMeshNavigationUpr;
	}

	inline OffMeshNavigationUpr* GetOffMeshNavigationUpr()
	{
		return &m_offMeshNavigationUpr;
	}

	inline const IslandConnectionsUpr* GetIslandConnectionsUpr() const
	{
		return &m_islandConnectionsUpr;
	}

	inline IslandConnectionsUpr* GetIslandConnectionsUpr()
	{
		return &m_islandConnectionsUpr;
	}

	struct TileTaskResult
	{
		TileTaskResult()
			: state(Running)
			, hashValue(0)
		{
		};

		enum State
		{
			Running = 0,
			Completed,
			NoChanges,
			Failed,
		};

		JobUpr::SJobState          jobState;
		u32                         hashValue;
		MNM::STile                     tile;
		MNM::CTileGenerator::SMetaData metaData;

		NavigationMeshID               meshID;

		u16                         x;
		u16                         y;
		u16                         z;
		u16                         volumeCopy;

		 u16                state; // communicated over thread boundaries
		u16                         next;  // next free
	};

private:
	struct VolumeDefCopy
	{
		VolumeDefCopy()
			: version(~0ul)
			, refCount(0)
			, meshID(0)
		{
		}

		size_t                           version;
		size_t                           refCount;

		NavigationMeshID                 meshID;

		MNM::BoundingVolume              boundary;
		std::vector<MNM::BoundingVolume> exclusions;
		std::vector<MNM::SMarkupVolume>  markups;
		std::vector<NavigationVolumeID>  markupIds;
	};

#if NAVIGATION_SYSTEM_PC_ONLY
	void UpdateMeshes(const float frameTime, const bool blocking, const bool multiThreaded, const bool bBackground);
	void SetupGenerator(const NavigationMeshID meshID, const MNM::CNavMesh::SGridParams& paramsGrid,
	                    u16 x, u16 y, u16 z, MNM::CTileGenerator::Params& params, const VolumeDefCopy& pDef, bool bMarkupUpdate);
	bool SpawnJob(TileTaskResult& result, NavigationMeshID meshID, const MNM::CNavMesh::SGridParams& paramsGrid,
	              u16 x, u16 y, u16 z, bool bMt, bool bNoHasTest);
	void CommitTile(TileTaskResult& result);
	void CommitMarkupData(const TileTaskResult& result, const MNM::TileID tileId);
#endif

	void ResetAllNavigationSystemUsers();

	void WaitForAllNavigationSystemUsersCompleteTheirReadingAsynchronousTasks();
	void UpdateNavigationSystemUsersForSynchronousWritingOperations();
	void UpdateNavigationSystemUsersForSynchronousOrAsynchronousReadingOperations();
	void UpdateInternalNavigationSystemData(const bool blocking);
	void UpdateInternalSubsystems();

	void ComputeWorldAABB();
	void SetupTasks();
	void StopAllTasks();

	void UpdateAllListener(const ENavigationEvent event);
	void ApplyAnnotationChanges();

#if MNM_USE_EXPORT_INFORMATION
	void ClearAllAccessibility(u8 resetValue);
	void ComputeAccessibility(const Vec3& debugLocation, NavigationAgentTypeID agentTypeId = NavigationAgentTypeID(0));
#endif

	void GatherNavigationVolumesToSave(std::vector<NavigationVolumeID>& usedVolumes) const;

	bool GrowMarkupsIfNeeded();

	// Returns false when the capacity is reached and no new element can be inserted
	template<class IdMap>
	bool GrowIdMapIfNeeded(IdMap& idMap)
	{
		if (idMap.size() >= idMap.capacity())
		{
			return idMap.grow(idMap.capacity()) > 0;
		}
		return true;
	}

	typedef std::vector<u16> RunningTasks;
	RunningTasks m_runningTasks;
	size_t       m_maxRunningTaskCount;
	float        m_cacheHitRate;
	float        m_throughput;

	typedef stl::aligned_vector<TileTaskResult, alignof(TileTaskResult)> TileTaskResults;
	TileTaskResults m_results;
	u16          m_free;
	WorkingState    m_state;

	typedef id_map<u32, NavigationMesh> Meshes;
	Meshes m_meshes;

	typedef id_map<u32, MNM::BoundingVolume> Volumes;
	Volumes m_volumes;

	// Markup volumes
	typedef id_map<u32, MNM::SMarkupVolume> MarkupVolumes;
	MarkupVolumes                                   m_markupVolumes;
	id_map<u32, MNM::SMarkupVolumeData>          m_markupsData;
	std::unordered_map<NavigationVolumeID, MNM::AreaAnnotation> m_markupAnnotationChangesToApply;

#ifdef SW_NAVMESH_USE_GUID
	typedef std::map<NavigationMeshGUID, NavigationMeshID> MeshMap;
	MeshMap m_swMeshes;

	typedef std::map<NavigationVolumeGUID, NavigationVolumeID> VolumeMap;
	VolumeMap m_swVolumes;

	i32       m_nextFreeMeshId;
	i32       m_nextFreeVolumeId;
#endif

	typedef std::vector<AgentType> AgentTypes;
	AgentTypes                        m_agentTypes;

	MNM::CAnnotationsLibrary          m_annotationsLibrary;

	u32                            m_configurationVersion;

	NavigationSystemDebugDraw         m_debugDraw;

	NavigationSystemBackgroundUpdate* m_pEditorBackgroundUpdate;

	AABB                              m_worldAABB;
	WorldMonitor                      m_worldMonitor;

	OffMeshNavigationUpr          m_offMeshNavigationUpr;
	IslandConnectionsUpr          m_islandConnectionsUpr;

	std::vector<VolumeDefCopy>        m_volumeDefCopy;

	string                            m_configName;

	typedef CListenerSet<INavigationSystemListener*> NavigationListeners;
	NavigationListeners m_listenersList;

	typedef CListenerSet<INavigationSystemUser*> NavigationSystemUsers;
	NavigationSystemUsers                  m_users;

	CMNMUpdatesUpr                     m_updatesUpr;
	CVolumesUpr                        m_volumesUpr;
	bool                                   m_isNavigationUpdatePaused;

	MNM::STileGeneratorExtensionsContainer m_tileGeneratorExtensionsContainer;
};

namespace NavigationSystemUtils
{
inline bool IsDynamicObjectPartOfTheMNMGenerationProcess(IPhysicalEntity* pPhysicalEntity)
{
	if (pPhysicalEntity)
	{
		pe_status_dynamics dyn;
		if (pPhysicalEntity->GetStatus(&dyn) && (dyn.mass <= 1e-6f))
			return true;
	}

	return false;
}
}

#endif
