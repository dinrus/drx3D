// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Polygon.h"

namespace Designer
{
class Model;
class UVIslandManager;

struct HE_Edge
{
	HE_Edge() :
		vertex(-1),
		pair_edge(-1),
		face(-1),
		next_edge(-1),
		sharpness(0),
		irregular(false)
	{
	}
	HE_Edge(i32 _vertex, i32 _pair_edge, i32 _face, i32 _next_edge, float _sharpness, bool _irregular) :
		vertex(_vertex),
		pair_edge(_pair_edge),
		face(_face),
		next_edge(_next_edge),
		sharpness(_sharpness),
		irregular(_irregular)
	{
	}
	i32   vertex;
	i32   pair_edge;
	i32   face;
	i32   next_edge;
	float sharpness;
	bool  irregular;
};

struct HE_Vertex
{
	HE_Vertex() : pos_index(-1), edge(-1)
	{
	}
	HE_Vertex(i32k& _pos_index, i32 _edge, const Vec2& _uv) : pos_index(_pos_index), edge(_edge), uv(_uv)
	{
	}
	i32  pos_index;
	i32  edge;
	Vec2 uv;
};

struct HE_Position
{
	HE_Position()
	{
	}
	HE_Position(const BrushVec3& _pos) : pos(_pos)
	{
	}
	BrushVec3        pos;
	std::vector<i32> edges;
	float            GetSharpness(const std::vector<HE_Edge>& he_edges) const
	{
		float sharpnessSum = 0;
		i32 iEdgeCount = edges.size();
		for (i32 i = 0; i < iEdgeCount; ++i)
			sharpnessSum += he_edges[edges[i]].sharpness;
		return sharpnessSum / (float)iEdgeCount;
	}
};

struct HE_Face
{
	HE_Face() : edge(-1)
	{
	}
	HE_Face(i32 _edge, PolygonPtr _pOriginPolygon) : edge(_edge), pOriginPolygon(_pOriginPolygon)
	{
	}
	i32        edge;
	PolygonPtr pOriginPolygon;
};

class HalfEdgeMesh : public _i_reference_target_t
{
public:

	void               ConstructMesh(Model* pModel);

	i32                GetVertexCount() const { return m_Vertices.size(); }
	i32                GetPosCount() const    { return m_Positions.size(); }
	i32                GetEdgeCount() const   { return m_Edges.size(); }
	i32                GetFaceCount() const   { return m_Faces.size(); }

	bool               IsIrregularFace(const HE_Face& f) const;

	const HE_Vertex&   GetVertex(i32 nIndex) const           { return m_Vertices[nIndex]; }
	const HE_Vertex&   GetVertex(const HE_Face& f) const     { return m_Vertices[m_Edges[f.edge].vertex]; }
	const HE_Vertex&   GetVertex(const HE_Edge& e) const     { return m_Vertices[e.vertex]; }

	const BrushVec3&   GetPos(i32 vertex_index) const        { return m_Positions[m_Vertices[vertex_index].pos_index].pos; }
	const BrushVec3&   GetPos(const HE_Vertex& v) const      { return m_Positions[v.pos_index].pos; }
	const BrushVec3&   GetPos(const HE_Face& f) const        { return m_Positions[m_Vertices[m_Edges[f.edge].vertex].pos_index].pos; }
	const BrushVec3&   GetPos(const HE_Edge& e) const        { return m_Positions[m_Vertices[e.vertex].pos_index].pos; }

	const Vec2&        GetUV(const HE_Edge& e) const         { return m_Vertices[e.vertex].uv; }
	const HE_Position& GetPosition(const HE_Vertex& v) const { return m_Positions[v.pos_index]; }

	const HE_Edge&     GetEdge(i32 nIndex) const             { return m_Edges[nIndex]; }
	const HE_Edge&     GetEdge(const HE_Face& f) const       { return m_Edges[f.edge]; }
	const HE_Edge&     GetEdge(const HE_Vertex& v) const     { return m_Edges[v.edge]; }
	BrushEdge3D        GetRealEdge(const HE_Edge& e) const   { return BrushEdge3D(GetPos(e), GetPos(e.next_edge)); }

	const HE_Edge& GetNextEdge(const HE_Edge& e) const   { return m_Edges[e.next_edge]; }
	const HE_Edge& GetPrevEdge(const HE_Edge& e) const;
	const HE_Edge* GetPairEdge(const HE_Edge& e) const
	{
		if (e.pair_edge == -1)
			return NULL;
		return &m_Edges[e.pair_edge];
	}

	const HE_Face&            GetFace(i32 nIndex) const       { return m_Faces[nIndex]; }
	const HE_Face&            GetFace(const HE_Edge& e) const { return m_Faces[e.face]; }

	bool                      GetValenceCount(const HE_Vertex& v, i32& nOutValenceCount) const;

	BrushVec3                 GetFaceAveragePos(const HE_Face& f) const;
	Vec2                      GetFaceAverageUV(const HE_Face& f) const;
	void                      GetFacePositions(const HE_Face& f, std::vector<BrushVec3>& outPositions) const;
	void                      GetFaceVertices(const HE_Face& f, std::vector<HE_Vertex>& outVertices) const;
	void                      GetFacePosIndices(const HE_Face& f, std::vector<i32>& outPosIndices) const;

	std::vector<HE_Vertex>&   GetVertices()  { return m_Vertices; }
	std::vector<HE_Position>& GetPositions() { return m_Positions; }
	std::vector<HE_Edge>&     GetEdges()     { return m_Edges; }
	std::vector<HE_Face>&     GetFaces()     { return m_Faces; }

	const HE_Edge*            FindNextEdgeClockwiseAroundVertex(const HE_Edge& edge) const;
	const HE_Edge& FindEndEdgeCounterClockwiseAroundVertex(const HE_Edge& edge) const;

	void           CreateMeshFaces(std::vector<FlexibleMeshPtr>& outMeshes, bool bGenerateBackFaces, bool bEnableSmoothingSurface);
	Model*         CreateModel(UVIslandManager* pUVIslandMgr);

	i32            AddPos(const BrushVec3& vPos);

private:

	void CalculateFaceSmoothingNormals(std::vector<BrushVec3>& outNormals);
	void SolveTJunction(Model* pModel, PolygonPtr pPolygon, Convexes* pConvexes);
	void FindEachPairEdge();
	void ConstructEdgeSharpnessTable(Model* pModel);
	void AddConvex(PolygonPtr pPolygon, const std::vector<Vertex>& vConvex);
	void Clear()
	{
		m_Vertices.clear();
		m_Positions.clear();
		m_Edges.clear();
		m_Faces.clear();
	}

	std::vector<HE_Vertex>   m_Vertices;
	std::vector<HE_Position> m_Positions;
	std::vector<HE_Edge>     m_Edges;
	std::vector<HE_Face>     m_Faces;
};
}

