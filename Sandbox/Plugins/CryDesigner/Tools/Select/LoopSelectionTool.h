// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class LoopSelectionTool : public BaseTool
{
public:

	LoopSelectionTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;

	static void LoopSelection(MainContext& mc);
	static bool SelectPolygonLoop(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon);
	static bool GetLoopPolygons(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon, std::vector<PolygonPtr>& outPolygons);
	static bool GetLoopPolygonsInBothWays(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon, std::vector<PolygonPtr>& outPolygons);

private:

	static PolygonPtr FindAdjacentNextPolygon(MainContext& mc, PolygonPtr pFristPolygon, PolygonPtr pSecondPolygon);
	static bool       SelectLoop(MainContext& mc, const BrushEdge3D& initialEdge);
	static bool       SelectBorderInOnePolygon(MainContext& mc, const BrushEdge3D& edge);
	static bool       SelectBorder(MainContext& mc, const BrushEdge3D& edge, ElementSet& outElementInfos);
	static i32        GetPolygonCountSharingEdge(MainContext& mc, const BrushEdge3D& edge, const BrushPlane* pPlane = NULL);
	static i32        CountAllEdges(MainContext& mc);
};
}

