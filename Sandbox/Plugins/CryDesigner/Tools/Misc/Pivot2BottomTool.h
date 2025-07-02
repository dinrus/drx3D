// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class Pivot2BottomTool : public BaseTool
{
public:
	Pivot2BottomTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void Enter() override;
};
}

