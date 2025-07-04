// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class OffMeshNavigationUpr;

namespace MNM
{

class IslandConnections;
class CNavMesh;

struct STile;

class CIslands
{
public:
	typedef uint64 TwoStaticIslandsIDs;

	CIslands();
	
	void           ComputeStaticIslandsAndConnections(CNavMesh& navMesh, const NavigationMeshID meshID, const OffMeshNavigationUpr& offMeshNavigationUpr, MNM::IslandConnections& islandConnections);
	void           UpdateIslandsForTriangles(CNavMesh& navMesh, const NavigationMeshID meshID, const MNM::TriangleID* pTrianglesArray, const size_t trianglesCount, MNM::IslandConnections& islandConnections);

	inline void    SetTotalIslands(u32 totalIslands) { m_islands.resize(totalIslands); }
	inline u32  GetTotalIslands() const { return m_islands.size(); }
	float          GetIslandArea(StaticIslandID islandID) const;

	void           ResetConnectedIslandsIDs(CNavMesh& navMesh);

private:

	struct SIsland
	{
		SIsland()
			: id(0)
			, area(0.f)
			, trianglesCount(0)
		{
		}
		SIsland(StaticIslandID id, AreaAnnotation annotation)
			: id(id)
			, annotation(annotation)
			, area(0.f)
			, trianglesCount(0)
		{}

		StaticIslandID id;
		AreaAnnotation annotation;
		u32         trianglesCount;
		float          area;
	};

	struct SConnectionRequests
	{		
		//! one-directional connection request by off-mesh link
		struct OffmeshConnectionRequest
		{
			OffmeshConnectionRequest(const StaticIslandID _startingIslandID, const TriangleID _startingTriangleID, u16k _offMeshLinkIndex)
				: startingIslandID(_startingIslandID)
				, startingTriangleID(_startingTriangleID)
				, offMeshLinkIndex(_offMeshLinkIndex)
			{
			}

			StaticIslandID startingIslandID;
			TriangleID     startingTriangleID;
			u16         offMeshLinkIndex;
		};

		void AddAreaConnection(const StaticIslandID firstIslandId, const StaticIslandID secondIslandId);
		void RemoveAreaConnection(const StaticIslandID firstIslandId, const StaticIslandID secondIslandId);

		void AddOffmeshConnection(const StaticIslandID startingIslandId, const TriangleID startingTriangleId, u16k offMeshLinkIndex);

		std::vector<OffmeshConnectionRequest> offmeshConnections;
		std::unordered_map<TwoStaticIslandsIDs, i32> areaConnection; //! bi-directional connection request of two islands
	};

	SIsland& GetNewIsland(const AreaAnnotation annotation);
	void    ReleaseIsland(const SIsland& island);
	void    ResolvePendingConnectionRequests(CNavMesh& navMesh, SConnectionRequests& islandConnectionRequests, const NavigationMeshID meshID, const OffMeshNavigationUpr* pOffMeshNavigationUpr, MNM::IslandConnections& islandConnections);
	void    ComputeStaticIslands(CNavMesh& navMesh, SConnectionRequests& islandConnectionRequests);
	
	void    FloodFillOnTriangles(CNavMesh& navMesh, const MNM::TriangleID sourceTriangleId, const size_t reserveCount, 
		std::function<bool(const STile& prevTile, const Tile::STriangle& prevTriangle, const STile& nextTile, Tile::STriangle& nextTriangle)> executeFunc);
	
	template<typename T>
	void    FloodFillOnTrianglesWithBackupValue(CNavMesh& navMesh, const MNM::TriangleID sourceTriangleId, const T sourceValue, const size_t reserveCount, 
		std::function<bool(const STile& prevTile, const Tile::STriangle& prevTriangle, const T prevValue, const STile& nextTile, Tile::STriangle& nextTriangle, T& nextValue)> executeFunc);

	inline size_t GetIslandIndex(const StaticIslandID islandID) const
	{
		DRX_ASSERT(islandID >= MNM::Constants::eStaticIsland_FirstValidIslandID && islandID <= m_islands.size());
		return islandID - 1;
	}

	inline SIsland& GetIsland(const StaticIslandID islandID)
	{
		return m_islands[GetIslandIndex(islandID)];
	}

	std::vector<SIsland> m_islands;
	std::vector<size_t> m_islandsFreeIndices;
};

} // MNM