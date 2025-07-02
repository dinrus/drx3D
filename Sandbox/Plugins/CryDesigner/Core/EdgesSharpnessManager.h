// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Core/Polygon.h"

namespace Designer
{
class ElementSet;

struct EdgeSharpness
{
	string                   name;
	std::vector<BrushEdge3D> edges;
	float                    sharpness;
	DrxGUID                  guid;
};

struct SharpEdgeInfo
{
	SharpEdgeInfo() : sharpnessindex(-1), edgeindex(-1) {}
	i32 sharpnessindex;
	i32 edgeindex;
};

class EdgeSharpnessManager : public _i_reference_target_t
{
public:
	EdgeSharpnessManager& operator=(const EdgeSharpnessManager& edgeSharpnessMgr);

	void                  Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bUndo);

	bool                  AddEdges(tukk name, ElementSet* pElements, float sharpness = 0);
	bool                  AddEdges(tukk name, const std::vector<BrushEdge3D>& edges, float sharpness = 0);
	void                  RemoveEdgeSharpness(tukk name);
	void                  RemoveEdgeSharpness(const BrushEdge3D& edge);

	void                  SetSharpness(tukk name, float sharpness);
	void                  Rename(tukk oldName, tukk newName);

	bool                  HasName(tukk name) const;
	string                GenerateValidName(tukk baseName = "EdgeGroup") const;

	i32                   GetCount() const { return m_EdgeSharpnessList.size(); }
	const EdgeSharpness& Get(i32 n) const { return m_EdgeSharpnessList[n]; }

	float                FindSharpness(const BrushEdge3D& edge) const;
	void                 Clear() { m_EdgeSharpnessList.clear(); }

	EdgeSharpness*       FindEdgeSharpness(tukk name);
private:

	SharpEdgeInfo GetEdgeInfo(const BrushEdge3D& edge);
	void          DeleteEdge(const SharpEdgeInfo& edgeInfo);

	std::vector<EdgeSharpness> m_EdgeSharpnessList;
};
}

