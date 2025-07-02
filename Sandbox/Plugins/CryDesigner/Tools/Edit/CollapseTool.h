// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class CollapseTool : public BaseTool
{
public:

	CollapseTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void Enter() override;
};
}

