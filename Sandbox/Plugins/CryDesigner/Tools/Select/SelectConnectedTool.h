// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SelectGrowTool.h"

namespace Designer
{
class SelectConnectedTool : public SelectGrowTool
{
public:

	SelectConnectedTool(EDesignerTool tool) : SelectGrowTool(tool)
	{
	}

	void        Enter() override;

	static void SelectConnectedPolygons(MainContext& mc);
};
}

