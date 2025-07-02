// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../BaseTool.h"

namespace Designer {
namespace UVMapping
{
class SelectSharedTool : public BaseTool
{
public:
	SelectSharedTool(EUVMappingTool tool) : BaseTool(tool) {}
	void        Enter() override;

	static void SelectShared();
};
}
}

