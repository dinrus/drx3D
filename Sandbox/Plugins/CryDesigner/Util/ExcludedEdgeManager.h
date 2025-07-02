// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Designer
{
class Model;

class ExcludedEdgeManager
{
public:
	void Add(const BrushEdge3D& edge);
	void Clear();
	bool GetVisibleEdge(const BrushEdge3D& edge, const BrushPlane& plane, std::vector<BrushEdge3D>& outVisibleEdges) const;

private:
	std::vector<BrushEdge3D> m_Edges;
};
}

