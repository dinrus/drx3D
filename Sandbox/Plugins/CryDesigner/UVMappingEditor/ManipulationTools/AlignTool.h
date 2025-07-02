// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../BaseTool.h"

namespace Designer {
namespace UVMapping
{
class AlignTool : public BaseTool
{
public:
	AlignTool(EUVMappingTool tool) : BaseTool(tool) {}

	void Enter() override;

private:

	void AlignSelectedVertices();
};
}
}

