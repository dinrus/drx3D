// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../BaseTool.h"
#include "Core/UVIsland.h"

namespace Designer {
namespace UVMapping
{
class UnmapTool : public BaseTool
{
public:
	UnmapTool(EUVMappingTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;
	static void UnmapSelectedElements();
};
}
}

