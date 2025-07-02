// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Polygon.h"

namespace Designer
{
class BSPTree2D;
class Convexes;

static const BrushFloat kDecomposerEpsilon = 1.0e-5f;

namespace DecomposerComp
{
template<class _Type>
bool IsEquivalent(const _Type& v0, const _Type& v1)
{
	return std::abs(v0 - v1) < kDecomposerEpsilon;
}

template<class _Type>
bool IsGreaterEqual(const _Type& v0, const _Type& v1)
{
	return v0 - v1 >= -kDecomposerEpsilon;
}

template<class _Type>
bool IsGreater(const _Type& v0, const _Type& v1)
{
	return v0 - v1 > -kDecomposerEpsilon;
}

template<class _Type>
bool IsLessEqual(const _Type& v0, const _Type& v1)
{
	return v0 - v1 <= kDecomposerEpsilon;
}

template<class _Type>
bool IsLess(const _Type& v0, const _Type& v1)
{
	return v0 - v1 < kDecomposerEpsilon;
}
}

enum EDecomposerFlag
{
	eDF_SkipOptimizationOfPolygonResults = BIT(0)
};

//! This class is in charge of decomposing a polygon into triangles and convex hulls without adding any vertices.
//! In order to achieve this precise and compact triangulation, the sweep line triangulation algorithm with monotone polygons is used
//! and the triangles made by this triangulation algorithm are used to create convex hulls.
//! This convex hull algorithm presented by Hertel and Mehlhorn(1983) is simple and fast but the result is suboptimal.
class PolygonDecomposer
{
public:
	PolygonDecomposer(i32 nFlag = 0) :
		m_nFlag(nFlag),
		m_bGenerateConvexes(false),
		m_pOutData(NULL)
	{
	}

	void DecomposeToConvexes(PolygonPtr pPolygon, Convexes& outConvexes);
	bool TriangulatePolygon(PolygonPtr pPolygon, FlexibleMesh& outMesh);
	bool TriangulatePolygon(PolygonPtr pPolygon, std::vector<PolygonPtr>& outTrianglePolygons);

private:
	enum EPointType
	{
		ePointType_Invalid,
		ePointType_Start,
		ePointType_End,
		ePointType_Regular,
		ePointType_Split,
		ePointType_Merge
	};

	enum EDirection
	{
		eDirection_Invalid,
		eDirection_Right,
		eDirection_Left
	};

	struct Point
	{
		Point(const BrushVec2& _pos, i32 _prev, i32 _next) :
			pos(_pos),
			prev(_prev),
			next(_next)
		{
		}
		BrushVec2 pos;
		i32       prev;
		i32       next;
	};

	typedef std::vector<i32> IndexList;

private:

	struct less_BrushFloat
	{
		bool operator()(const BrushFloat& v0, const BrushFloat& v1) const
		{
			return DecomposerComp::IsLess(v0, v1);
		}
	};

	struct PointComp
	{
		PointComp(const BrushFloat yPos, i32 xPriority)
		{
			m_yPos = yPos;
			m_XPriority = xPriority;
		}

		BrushFloat m_yPos;
		i32        m_XPriority;

		bool operator<(const PointComp& point) const
		{
			if (DecomposerComp::IsLess(m_yPos, point.m_yPos))
				return true;
			if (DecomposerComp::IsEquivalent(m_yPos, point.m_yPos) && m_XPriority < point.m_XPriority)
				return true;
			return false;
		}

		bool operator>(const PointComp& point) const
		{
			if (DecomposerComp::IsGreater(m_yPos, point.m_yPos))
				return true;
			if (DecomposerComp::IsEquivalent(m_yPos, point.m_yPos) && m_XPriority > point.m_XPriority)
				return true;
			return false;
		}
	};

	typedef std::map<BrushFloat, IndexList, less_BrushFloat> EdgeListMap;

private:
	bool        Decompose();
	bool        DecomposeToTriangules(IndexList* pIndexList, bool bHasInnerHull);
	bool        DecomposeNonplanarQuad();
	bool        DecomposePolygonIntoMonotonePieces(const IndexList& indexList, std::vector<IndexList>& outMonatonePieces);

	bool        TriangulateConvex(const IndexList& indexList, std::vector<SMeshFace>& outFaceList) const;
	bool        TriangulateConcave(const IndexList& indexList, std::vector<SMeshFace>& outFaceList);
	bool        TriangulateMonotonePolygon(const IndexList& indexList, std::vector<SMeshFace>& outFaceList) const;

	void        BuildEdgeListHavingSameY(const IndexList& indexList, EdgeListMap& outEdgeList) const;
	void        BuildPointSideList(const IndexList& indexList, const EdgeListMap& edgeListHavingSameY, std::vector<EDirection>& outPointSideList, std::map<PointComp, std::pair<i32, i32>>& outSortedMarksMap) const;

	void        AddTriangle(i32 i0, i32 i1, i32 i2, std::vector<SMeshFace>& outFaceList) const;
	void        AddFace(i32 i0, i32 i1, i32 i2, const IndexList& indices, std::vector<SMeshFace>& outFaceList) const;
	bool        IsInside(BSPTree2D* pBSPTree, i32 i0, i32 i1) const;
	bool        IsOnEdge(BSPTree2D* pBSPTree, i32 i0, i32 i1) const;
	bool        IsCCW(i32 i0, i32 i1, i32 i2) const;
	bool        IsCW(i32 i0, i32 i1, i32 i2) const                { return !IsCCW(i0, i1, i2); }
	bool        IsCCW(const IndexList& indexList, i32 nCurr) const;
	bool        IsCW(const IndexList& indexList, i32 nCurr) const { return !IsCCW(indexList, nCurr); }
	BrushFloat  IsCCW(const BrushVec2& prev, const BrushVec2& current, const BrushVec2& next) const;
	BrushFloat  IsCW(const BrushVec2& prev, const BrushVec2& current, const BrushVec2& next) const;
	bool        IsConvex(const IndexList& indexList, bool bGetPrevNextFromPointList) const;
	BrushFloat  Cosine(i32 i0, i32 i1, i32 i2) const;
	bool        HasAlreadyAdded(i32 i0, i32 i1, i32 i2, const std::vector<SMeshFace>& faceList) const;
	bool        IsColinear(const BrushVec2& p0, const BrushVec2& p1, const BrushVec2& p2) const;
	bool        IsDifferenceOne(i32 i0, i32 i1, const IndexList& indexList) const;
	bool        IsInsideEdge(i32 i0, i32 i1, i32 i2) const;
	bool        GetNextIndex(i32 nCurr, const IndexList& indices, i32& nOutNextIndex) const;
	bool        HasArea(i32 i0, i32 i1, i32 i2) const;

	i32         FindLeftTopVertexIndex(const IndexList& indexList) const;
	i32         FindRightBottomVertexIndex(const IndexList& indexList) const;
	template<class _Pr>
	i32         FindExtreamVertexIndex(const IndexList& indexList) const;

	EDirection  QueryPointSide(i32 nIndex, const IndexList& indexList, i32 nLeftTopIndex, i32 nRightBottomIndex) const;
	EPointType  QueryPointType(i32 nIndex, const IndexList& indexList) const;
	EDirection  QueryInteriorDirection(i32 nIndex, const IndexList& indexList) const;

	i32         FindDirectlyLeftEdge(i32 nBeginIndex, const IndexList& edgeSearchList, const IndexList& indexList) const;
	void        EraseElement(i32 nIndex, IndexList& edgeSearchList) const;

	void        SearchMonotoneLoops(EdgeSet& diagonalSet, const IndexList& indexList, std::vector<IndexList>& monotonePieces) const;
	void        AddDiagonalEdge(i32 i0, i32 i1, EdgeSet& diagonalList) const;
	BSPTree2D*  GenerateBSPTree(const IndexList& indexList) const;

	void        RemoveIndexWithSameAdjacentPoint(IndexList& indexList) const;
	static void FillVertexListFromPolygon(PolygonPtr pPolygon, std::vector<Vertex>& outVertexList);

	void        CreateConvexes();
	i32         CheckFlag(i32 nFlag) const { return m_nFlag & nFlag; }

	BrushVec2   Convert3DTo2D(const BrushVec3& pos) const;

private: // Related to Triangulation
	i32                 m_nFlag;
	FlexibleMesh*       m_pOutData;
	std::vector<Vertex> m_VertexList;
	std::vector<Point>  m_PointList;
	BrushPlane          m_Plane;
	short               m_SubMatID;
	PolygonPtr          m_pPolygon;
	i32                 m_nBasedVertexIndex;
	bool                m_bGenerateConvexes;

private: // Related to Decomposition into convexes
	std::pair<i32, i32> GetSortedEdgePair(i32 i0, i32 i1) const
	{
		if (i1 < i0)
			std::swap(i0, i1);
		return std::pair<i32, i32>(i0, i1);
	}
	void FindMatchedConnectedVertexIndices(i32 iV0, i32 iV1, const IndexList& indexList, i32& nOutIndex0, i32& nOutIndex1) const;
	bool MergeTwoConvexes(i32 iV0, i32 iV1, i32 iConvex0, i32 iConvex1, IndexList& outMergedPolygon);
	void RemoveAllConvexData()
	{
		m_InitialEdgesSortedByEdge.clear();
		m_ConvexesSortedByEdge.clear();
		m_EdgesSortedByConvex.clear();
		m_Convexes.clear();
	}
	mutable std::set<std::pair<i32, i32>>                   m_InitialEdgesSortedByEdge;
	mutable std::map<std::pair<i32, i32>, std::vector<i32>> m_ConvexesSortedByEdge;
	mutable std::map<i32, std::set<std::pair<i32, i32>>>    m_EdgesSortedByConvex;
	mutable std::vector<IndexList>                          m_Convexes;
	mutable Convexes* m_pBrushConvexes;
};
}

