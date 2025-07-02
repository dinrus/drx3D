// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/INavigationUpdatesUpr.h>

#include <drx3D/AI/BoundingVolume.h>
#include <drx3D/AI/NavMesh.h>

class NavigationSystem;
struct NavigationMesh;

class CMNMUpdatesUpr : public INavigationUpdatesUpr
{
public:
	enum class EUpdateRequestStatus
	{
		RequestInQueue = 0,
		RequestDelayedAndBuffered,
		RequestIgnoredAndBuffered,
		RequestInvalid,
		Count
	};

	struct TileUpdateRequest
	{
		enum class EFlag : u16
		{
			Aborted      = BIT(0),
			Explicit     = BIT(1),
			MarkupUpdate = BIT(2),
		};

		enum class EState : u16
		{
			Free,
			Active,
			Postponed,
			Ignored,
		};

		TileUpdateRequest()
			: flags(0)
		    , state(EState::Free)
			, idx(-1)
		{}

		bool        CheckFlag(EFlag flag) const { return (flags & u16(flag)) != 0; }
		void        SetFlag(EFlag flag) { flags |= u16(flag); }
		void        ClearFlag(EFlag flag) { flags &= ~u16(flag); }

		bool        IsAborted() const  { return CheckFlag(EFlag::Aborted); }
		bool        IsExplicit() const { return CheckFlag(EFlag::Explicit); }

		inline bool operator==(const TileUpdateRequest& other) const
		{
			return (meshID == other.meshID) && (x == other.x) && (y == other.y) && (z == other.z);
		}

		inline bool operator<(const TileUpdateRequest& other) const
		{
			if (meshID != other.meshID)
				return meshID < other.meshID;

			if (x != other.x)
				return x < other.x;

			if (y != other.y)
				return y < other.y;

			return z < other.z;
		}

		EState           state;
		size_t           idx;

		NavigationMeshID meshID;
		u16           flags;
		u16           x;
		u16           y;
		u16           z;
	};

	static uint64 TileUpdateRequestKey(NavigationMeshID meshID, u16 x, u16 y, u16 z)
	{
		static_assert((MNM::CNavMesh::x_bits + MNM::CNavMesh::y_bits + MNM::CNavMesh::z_bits) <= 32, "Unexpected TileId size!");
		static_assert(sizeof(NavigationMeshID) <= 4, "Unexpected NavigationMeshID size!");

		const uint64 tileName = uint64(MNM::CNavMesh::ComputeTileName(x, y, z));
		return (uint64(meshID) << 32) | tileName;
	};

	class TileUpdateRequestArray
	{
	public:
		TileUpdateRequestArray();
		TileUpdateRequestArray(const TileUpdateRequestArray& rhs);
		~TileUpdateRequestArray();

		void Init(size_t initialCount);
		void Clear();

		const size_t AllocateTileUpdateRequest();
		void         FreeTileUpdateRequest(const size_t requestIdx);
		const size_t GetRequestCount() const;

		TileUpdateRequest&       operator[](size_t requestIdx);
		const TileUpdateRequest& operator[](size_t requestIdx) const;

		void          Swap(TileUpdateRequestArray& other);

	private:
		void          Grow(size_t amount);

		TileUpdateRequest* m_updateRequests;
		size_t         m_count;
		size_t         m_capacity;

		typedef std::vector<size_t> FreeIndexes;
		FreeIndexes m_freeIndexes;
	};

	typedef MNM::BoundingVolume NavigationBoundingVolume;

	CMNMUpdatesUpr() = delete;

	CMNMUpdatesUpr(NavigationSystem* pNavSystem);

	virtual ~CMNMUpdatesUpr() override {}

	virtual void Update() override;

	virtual void EntityChanged(i32 physicalEntityId, const AABB& aabb) override;
	virtual void WorldChanged(const AABB& aabb) override;

	virtual void RequestGlobalUpdate() override;
	virtual void RequestGlobalUpdateForAgentType(NavigationAgentTypeID agentTypeID) override;

	virtual void EnableRegenerationRequestsExecution(bool updateChangedVolumes) override;
	virtual void DisableRegenerationRequestsAndBuffer() override;
	virtual bool AreRegenerationRequestsDisabled() const override   { return m_bIsRegenerationRequestExecutionEnabled; }

	virtual bool WasRegenerationRequestedThisCycle() const override { return m_bWasRegenerationRequestedThisUpdateCycle; }
	virtual void ClearRegenerationRequestedThisCycleFlag() override { m_bWasRegenerationRequestedThisUpdateCycle = false; }

	virtual void EnableUpdatesAfterStabilization() override         { m_bPostponeUpdatesForStabilization = true; }
	virtual void DisableUpdatesAfterStabilization() override        { m_bPostponeUpdatesForStabilization = false; }

	virtual bool HasBufferedRegenerationRequests() const override;
	virtual void ApplyBufferedRegenerationRequests() override;
	virtual void ClearBufferedRegenerationRequests() override;

	//! Request MNM regeneration on a specific AABB for a specific meshID
	//! If MNM regeneration is disabled internally, requests will be stored in a buffer
	//! Return values
	//!   - RequestInQueue: request was successfully validated and is in active execution queue
	//!   - RequestDelayedAndBuffered: request is stored for delayed execution after some time without any changes
	//!   - RequestIgnoredAndBuffered: MNM regeneration is turned off, so request is stored in buffer
	//!	  - RequestInvalid: there was something wrong with the request so it was ignored
	EUpdateRequestStatus     RequestMeshUpdate(NavigationMeshID meshID, const AABB& aabb, bool bImmediateUpdate = true, bool bMarkupUpdate = false);
	EUpdateRequestStatus     RequestMeshDifferenceUpdate(NavigationMeshID meshID, const NavigationBoundingVolume& oldVolume, const NavigationBoundingVolume& newVolume);

	void                     Clear();
	void                     OnMeshDestroyed(NavigationMeshID meshID);

	size_t                   GetRequestQueueSize() const { return m_activeUpdateRequestsQueue.size(); }
	bool                     HasUpdateRequests() const   { return !m_activeUpdateRequestsQueue.empty(); }
	const TileUpdateRequest& GetFrontRequest() const;
	void                     PopFrontRequest();

	void                     DebugDraw();

private:
	struct EntityUpdate
	{
		AABB aabbOld;
		AABB aabbNew;
	};

	struct MeshUpdateBoundaries
	{
		u16 minX;
		u16 maxX;
		u16 minY;
		u16 maxY;
		u16 minZ;
		u16 maxZ;
	};

	typedef std::deque<size_t>                         TileRequestQueue;
	typedef std::vector<size_t>                        TileUpdatesVector;
	typedef std::unordered_map<uint64, size_t>         TileUpdatesMap;
	typedef std::unordered_map<i32, EntityUpdate>      EntityUpdatesMap;

	struct SRequestParams
	{
		bool CheckFlag(TileUpdateRequest::EFlag flag) const { return (flags & u16(flag)) != 0; }
		
		EUpdateRequestStatus status;
		TileUpdateRequest::EState requestState;
		u16 flags;
	};

	void                 RemoveMeshUpdatesFromVector(TileUpdatesVector& tileUpdatesVector, NavigationMeshID meshID);

	void                 UpdatePostponedChanges();

	SRequestParams        GetRequestParams(bool bIsExplicit);
	SRequestParams        GetRequestParamsAfterStabilization(bool bIsExplicit);

	void                 RequestQueueWorldUpdate(const SRequestParams& requestParams, const AABB& aabb);
	bool                 RequestQueueMeshUpdate(const SRequestParams& requestParams, NavigationMeshID meshID, const AABB& aabb);
	bool                 RequestQueueMeshDifferenceUpdate(const SRequestParams& requestParams, NavigationMeshID meshID, const NavigationBoundingVolume& oldVolume, const NavigationBoundingVolume& newVolume);

	MeshUpdateBoundaries ComputeMeshUpdateBoundaries(const NavigationMesh& mesh, const AABB& aabb);
	MeshUpdateBoundaries ComputeMeshUpdateDifferenceBoundaries(const NavigationMesh& mesh, const AABB& oldVolume, const AABB& newVolume);

	void                 SheduleTileUpdateRequests(const SRequestParams& requestParams, NavigationMeshID meshID, const MeshUpdateBoundaries& boundaries);

	void SwitchUpdateRequestState(size_t requestId, TileUpdateRequest::EState newState);

	NavigationSystem* m_pNavigationSystem;

	TileUpdateRequestArray m_updateRequests;

	TileUpdatesMap    m_updateRequestsMap;

	TileRequestQueue  m_activeUpdateRequestsQueue;
	TileUpdatesVector m_pospondedUpdateRequests;
	TileUpdatesVector m_ignoredUpdateRequests;

	EntityUpdatesMap  m_postponedEntityUpdatesMap;

	// map for storing AABBs of navigation areas in the case when navmesh updates are disabled and we will need to update them later after enabling
	std::unordered_map<NavigationMeshID, AABB> m_pendingOldestAabbsOfChangedMeshes;

	bool              m_bIsRegenerationRequestExecutionEnabled;
	bool              m_bWasRegenerationRequestedThisUpdateCycle;
	bool              m_bPostponeUpdatesForStabilization;
	bool              m_bExplicitRegenerationToggle;

	CTimeValue        m_lastUpdateTime;
};
