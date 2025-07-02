// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class MergeTool : public BaseTool
{
public:
	MergeTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;

	static void MergePolygons(MainContext& mc);

private:
	void MergeObjects();
	void MergePolygons();
};
}

