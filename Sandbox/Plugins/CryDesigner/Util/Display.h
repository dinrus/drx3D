// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Core/Polygon.h"

namespace Designer {
class Model;
class ExcludedEdgeManager;
namespace Display
{
void DisplayHighlightedElements(DisplayContext& dc, MainContext& mc, i32 nPickFlag, ExcludedEdgeManager* pExcludedEdgeMgr);
void DisplayHighlightedEdges(DisplayContext& dc, MainContext& mc, ExcludedEdgeManager* pExcludedEdgeMgr);
void DisplayHighlightedVertices(DisplayContext& dc, MainContext& mc, bool bExcludeVerticesFromSecondInOpenPolygon = false);
void DisplayHighlightedPolygons(DisplayContext& dc, MainContext& mc);
void DisplayModel(DisplayContext& dc, Designer::Model* pModel, ExcludedEdgeManager* pExcludedEdgeMgr, ShelfID nShelf = eShelf_Any, i32k nLineThickness = 2, const ColorB& lineColor = ColorB(0, 0, 0));
void DisplayPolygons(DisplayContext& dc, const std::vector<PolygonPtr>& polygons, ExcludedEdgeManager* pExcludedEdgeMgr, i32k nLineThickness, const ColorB& lineColor);
void DisplayPolygon(DisplayContext& dc, PolygonPtr polygon);
void DisplayBottomPolygon(DisplayContext& dc, PolygonPtr polygon, const ColorB& lineColor = PolygonLineColor);
void DisplaySubdividedMesh(DisplayContext& dc, Model* pModel);
void DisplayVertexNormals(DisplayContext& dc, MainContext& mc);
void DisplayPolygonNormals(DisplayContext& dc, MainContext& mc);
void DisplayTriangulation(DisplayContext& dc, MainContext& mc);
}
}

